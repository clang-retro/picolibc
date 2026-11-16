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
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* The mode for open() calls that give neither O_TEXT nor O_BINARY. */
int _fmode = O_TEXT;

/* The size of an open file: its records, and in text mode the bytes of
 * the last record up to the first Ctrl-Z. */
static off_t
file_size(struct __cpm_file *f)
{
    struct __cpm_fcb fcb = f->fcb;
    __cpm_bdos(BDOS_FILE_SIZE, __CPM_PTR(&fcb));
    int32_t records = fcb.r[0] | ((int32_t)fcb.r[1] << 8) | ((int32_t)fcb.r[2] << 16);
    off_t   size = (off_t)records * __CPM_RECORD;
    if (records > 0 && (f->flags & __CPM_FD_TEXT)) {
        f->size = size;
        if (__cpm_load(f, records - 1) == 0) {
            uint8_t *z = memchr(f->buf, 0x1a, __CPM_RECORD);
            if (z)
                size -= __CPM_RECORD - (z - f->buf);
        }
    }
    return size;
}

int
open(const char *path, int flags, ...)
{
    int acc = flags & O_ACCMODE;
    int fl = __CPM_FD_OPEN;
    if (acc == O_RDONLY || acc == O_RDWR)
        fl |= __CPM_FD_READ;
    if (acc == O_WRONLY || acc == O_RDWR)
        fl |= __CPM_FD_WRITE;
    if ((flags & O_TEXT) || (!(flags & O_BINARY) && _fmode != O_BINARY))
        fl |= __CPM_FD_TEXT;
    if (flags & O_APPEND)
        fl |= __CPM_FD_APPEND;

    struct __cpm_file *f = calloc(1, sizeof(*f));
    if (f == NULL)
        return -1;
    f->flags = fl;
    f->bufrec = -1;

    int dev = __cpm_device_name(path);
    if (dev != __CPM_DEV_NONE) {
        f->device = dev;
    } else {
        if (__cpm_parse_name(&f->fcb, path, 0) < 0)
            goto fail;
        struct __cpm_fcb fcb = f->fcb;
        int exists = (uint8_t)__cpm_bdos(BDOS_OPEN, __CPM_PTR(&fcb)) != 0xff;
        if (exists && (flags & O_CREAT) && (flags & O_EXCL)) {
            errno = EEXIST;
            goto fail;
        }
        if (exists && (fcb.type[0] & 0x80) && (fl & __CPM_FD_WRITE)) {
            /* A read-only file (attribute t1'). */
            errno = EACCES;
            goto fail;
        }
        if (exists && (flags & O_TRUNC) && (fl & __CPM_FD_WRITE)) {
            __cpm_bdos(BDOS_DELETE, __CPM_PTR(&f->fcb));
            exists = 0;
        }
        if (!exists) {
            if (!(flags & O_CREAT)) {
                errno = ENOENT;
                goto fail;
            }
            fcb = f->fcb;
            if ((uint8_t)__cpm_bdos(BDOS_MAKE, __CPM_PTR(&fcb)) == 0xff) {
                errno = ENOSPC;
                goto fail;
            }
            f->flags |= __CPM_FD_WROTE;
        }
        /* The BDOS keeps the open file's state in the FCB. */
        f->fcb = fcb;
        f->fcb.cr = 0;
        f->size = exists ? file_size(f) : 0;
    }
    int fd = __cpm_new_fd(f);
    if (fd < 0) {
        if (f->device == __CPM_DEV_NONE)
            __cpm_bdos(BDOS_CLOSE, __CPM_PTR(&f->fcb));
        goto fail;
    }
    return fd;
fail:
    free(f);
    return -1;
}

int
close(int fd)
{
    struct __cpm_file *f = __cpm_fd(fd);
    if (f == NULL)
        return -1;
    int r = 0;
    if (f->device == __CPM_DEV_NONE) {
        if (__cpm_flush(f) < 0)
            r = -1;
        /* Closing updates the directory, which only matters after writes;
         * CP/M-86 1.1 has no limit on open files to free up. */
        if ((uint8_t)__cpm_bdos(BDOS_CLOSE, __CPM_PTR(&f->fcb)) == 0xff && r == 0
            && (f->flags & __CPM_FD_WROTE)) {
            errno = EIO;
            r = -1;
        }
    }
    __cpm_fds[fd] = NULL;
    /* fds 0 to 2 start out static. */
    if (fd > 2)
        free(f);
    return r;
}
