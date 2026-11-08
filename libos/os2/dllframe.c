/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 clang-dos contributors
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
#include "local-os2.h"
#include <stdlib.h>
#include <unistd.h>

/*
 * Frames in DGROUP for the locals of 16-bit DLL code.
 *
 * A DLL runs on the stack of the thread that calls it, so SS is not DS, and
 * a near pointer (which means DS) cannot point to a local. Code compiled
 * with -mwindows-dll in the small and medium models keeps the locals whose
 * address escapes in a frame in DGROUP instead, below a pointer that this
 * function finds. Each thread gets a region of its own, found by its
 * thread id in the local information segment, so that no locking is
 * needed: a thread only uses its own slot. A new thread with the id of
 * one that ended takes over its region, which is idle by then.
 */

#ifndef __i386__

/* OS/2 1.x has at most 255 threads in a process; few use as many. */
#define NTHREADS 64

struct region {
    char *ptr; /* the frame pointer of the code, at the top when idle */
};

static struct region regions[NTHREADS];
static __os2_word    gsel, lsel;

extern uint16_t      __os2_dll_frame_size;

static void
fail(const char *msg, size_t len)
{
    write(2, msg, len);
    abort();
}

char **
__win16_frame_pointer_address(void)
{
    if (!lsel && DosGetInfoSeg(&gsel, &lsel))
        fail("DosGetInfoSeg failed\r\n", 22);
    /* The current thread id is the fourth word of LINFOSEG. */
    unsigned tid = ((const volatile uint16_t __far *)((uint32_t)lsel << 16))[3];
    if (tid >= NTHREADS)
        fail("DLL frame regions exhausted\r\n", 29);
    struct region *r = &regions[tid];
    if (!r->ptr) {
        char *base = malloc(__os2_dll_frame_size);
        if (!base)
            fail("DLL frame regions exhausted\r\n", 29);
        /* Frames grow down from the top, which stays 2-byte aligned. */
        r->ptr = base + (__os2_dll_frame_size & ~1u);
    }
    return &r->ptr;
}
#endif
