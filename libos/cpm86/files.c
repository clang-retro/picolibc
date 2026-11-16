/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 clang-retro contributors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "local-cpm86.h"
#include <string.h>

static void
set_record(struct __cpm_fcb *fcb, int32_t rec)
{
    fcb->r[0] = (uint8_t)rec;
    fcb->r[1] = (uint8_t)(rec >> 8);
    fcb->r[2] = (uint8_t)(rec >> 16);
}

int
__cpm_flush(struct __cpm_file *f)
{
    if (!f->dirty)
        return 0;
    set_record(&f->fcb, f->bufrec);
    __cpm_bdos(BDOS_SET_DMA, __CPM_PTR(f->buf));
    uint8_t r = (uint8_t)__cpm_bdos(BDOS_WRITE_RANDOM, __CPM_PTR(&f->fcb));
    if (r != 0) {
        /* 2: no more data blocks, 5: no more directory space. */
        errno = ENOSPC;
        return -1;
    }
    f->dirty = 0;
    f->flags |= __CPM_FD_WROTE;
    return 0;
}

int
__cpm_load(struct __cpm_file *f, int32_t rec)
{
    if (f->bufrec == rec)
        return 0;
    if (__cpm_flush(f) < 0)
        return -1;
    f->bufrec = -1;
    /* Records at or past the end are not read: a new record is all
     * Ctrl-Z, CP/M's end of text. */
    if ((off_t)rec * __CPM_RECORD >= f->size) {
        memset(f->buf, 0x1a, sizeof(f->buf));
        f->bufrec = rec;
        return 1;
    }
    set_record(&f->fcb, rec);
    __cpm_bdos(BDOS_SET_DMA, __CPM_PTR(f->buf));
    uint8_t r = (uint8_t)__cpm_bdos(BDOS_READ_RANDOM, __CPM_PTR(&f->fcb));
    if (r == 1 || r == 4) {
        /* Unwritten data, in a file with holes, reads as zeros. */
        memset(f->buf, 0, sizeof(f->buf));
    } else if (r != 0) {
        errno = EIO;
        return -1;
    }
    f->bufrec = rec;
    return 0;
}
