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

/* int386, int386x, intdos, intdosx and segread of <i86.h>. */

#include "../local-msdos.h"
#include <i86.h>

int __msdos_int386(int intno, struct __msdos_regs *r, int sregs);

/* Calls the interrupt with the registers of *in (and *sr), and stores the
 * result registers in *out (and *sr). Gives EAX. */
static int
call(int intno, union REGS *in, union REGS *out, struct SREGS *sr)
{
    struct __msdos_regs r = { .ax = in->x.eax,
                              .bx = in->x.ebx,
                              .cx = in->x.ecx,
                              .dx = in->x.edx,
                              .si = in->x.esi,
                              .di = in->x.edi };
    if (sr) {
        r.ds = sr->ds;
        r.es = sr->es;
    }
    int cf = __msdos_int386(intno, &r, sr != 0);
    out->x.eax = r.ax;
    out->x.ebx = r.bx;
    out->x.ecx = r.cx;
    out->x.edx = r.dx;
    out->x.esi = r.si;
    out->x.edi = r.di;
    out->x.cflag = cf;
    if (sr) {
        sr->ds = r.ds;
        sr->es = r.es;
    }
    return (int)r.ax;
}

/* Calls software interrupt intno. */
int
int386(int intno, union REGS *in, union REGS *out)
{
    return call(intno, in, out, 0);
}

/* Calls software interrupt intno with DS and ES from *sr. */
int
int386x(int intno, union REGS *in, union REGS *out, struct SREGS *sr)
{
    return call(intno, in, out, sr);
}

/* Calls DOS (INT 21h). */
int
intdos(union REGS *in, union REGS *out)
{
    return call(0x21, in, out, 0);
}

/* Calls DOS (INT 21h) with DS and ES from *sr. */
int
intdosx(union REGS *in, union REGS *out, struct SREGS *sr)
{
    return call(0x21, in, out, sr);
}

/* Gives the current segment registers. */
void
segread(struct SREGS *sr)
{
    __asm__("movw %%es, %0\n\t"
            "movw %%cs, %1\n\t"
            "movw %%ss, %2\n\t"
            "movw %%ds, %3\n\t"
            "movw %%fs, %4\n\t"
            "movw %%gs, %5"
            : "=m"(sr->es), "=m"(sr->cs), "=m"(sr->ss), "=m"(sr->ds), "=m"(sr->fs),
              "=m"(sr->gs));
}
