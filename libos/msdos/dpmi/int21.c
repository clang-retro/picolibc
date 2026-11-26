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

/* DOS calls of the 32-bit DOS library: most go to DOS/32A, which takes
 * flat pointers. The calls that it does not translate go to real mode
 * through the transfer buffer. Also the copy from real-mode memory. */

#include "../local-msdos.h"
#include <dpmi.h>
#include <string.h>

int __msdos_int21_pm(struct __msdos_regs *r);

uint16_t __dpmi_tb_seg;
char    *__dpmi_tb;

/* Allocates the transfer buffer the first time. Gives 0, or -1. */
static int
transfer_buffer(void)
{
    if (__dpmi_tb)
        return 0;
    int sel, seg = __dpmi_allocate_dos_memory(__DPMI_TB_SIZE / 16, &sel);
    if (seg < 0)
        return -1;
    __dpmi_tb_seg = (uint16_t)seg;
    __dpmi_tb = (char *)((uintptr_t)seg << 4);
    return 0;
}

/* INT 21h function 60h (the full name of a path), which DOS/32A gives to
 * real mode as it is: DS:SI is the path and ES:DI the 128-byte result. */
static int
truename(struct __msdos_regs *r)
{
    const char *path = (const char *)(uintptr_t)r->si;
    char       *full = (char *)(uintptr_t)r->di;
    size_t      n = strlen(path) + 1;
    if (n > __DPMI_TB_SIZE - 128 || transfer_buffer() < 0) {
        r->ax = 0x03; /* path not found */
        return 1;
    }
    memcpy(__dpmi_tb + 128, path, n);
    __dpmi_regs rm = { 0 };
    rm.x.ax = (uint16_t)r->ax;
    rm.x.ds = rm.x.es = __dpmi_tb_seg;
    rm.x.si = 128;
    rm.x.di = 0;
    if (__dpmi_int(0x21, &rm) < 0) {
        r->ax = 0x01; /* invalid function */
        return 1;
    }
    r->ax = rm.x.ax;
    if (rm.x.flags & 1)
        return 1;
    memcpy(full, __dpmi_tb, 128);
    return 0;
}

/* Calls DOS with *r (see local-msdos.h). */
int
__msdos_int21(struct __msdos_regs *r)
{
    if (((r->ax >> 8) & 0xff) == 0x60)
        return truename(r);
    return __msdos_int21_pm(r);
}

/* Copies n bytes from seg:off: real-mode memory is at its linear address,
 * which is a pointer in the flat program. */
void
__msdos_fromfar(void *dst, uint16_t seg, uint16_t off, size_t n)
{
    memcpy(dst, (const char *)(((uintptr_t)seg << 4) + off), n);
}
