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

/*
 * i86.h - registers and software interrupts for 32-bit DOS programs
 * (i386-pc-msdos), as in Open Watcom's 32-bit i86.h. The program runs in
 * flat protected mode under DOS/32A. A software interrupt goes to the
 * extender, which translates the DOS (INT 21h) calls that take pointers
 * and gives the other interrupts to real mode with the registers as they
 * are. For a real-mode call with a buffer, use <dpmi.h>.
 */

#ifndef _I86_H_
#define _I86_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The flags. */
enum {
    INTR_CF = 0x0001, /* carry */
    INTR_PF = 0x0004, /* parity */
    INTR_AF = 0x0010, /* auxiliary carry */
    INTR_ZF = 0x0040, /* zero */
    INTR_SF = 0x0080, /* sign */
    INTR_TF = 0x0100, /* trace */
    INTR_IF = 0x0200, /* interrupt */
    INTR_DF = 0x0400, /* direction */
    INTR_OF = 0x0800  /* overflow */
};

/* Interrupts off (CLI) and on (STI). */
static __inline__ void
_disable(void)
{
    __asm__ __volatile__("cli" ::: "memory");
}

static __inline__ void
_enable(void)
{
    __asm__ __volatile__("sti" ::: "memory");
}

/* A pointer to real-mode memory at segment:offset. The program is flat
 * and DOS memory is at its linear address (segment * 16 + offset), so the
 * result is a near pointer. FP_SEG and FP_OFF give back a segment and an
 * offset for a pointer below 1 MB. */
#ifndef MK_FP
#define MK_FP(__s, __o)                                                              \
    ((void *)((((unsigned long)(unsigned short)(__s)) << 4) + (unsigned long)(__o)))
#define FP_SEG(__p) ((unsigned short)((unsigned long)(void *)(__p) >> 4))
#define FP_OFF(__p) ((unsigned short)((unsigned long)(void *)(__p) & 15))
#endif

/* The registers of int386: x for the 32-bit registers, w for the 16-bit
 * ones and h for the 8-bit ones. cflag is the carry flag after the call. */
struct DWORDREGS {
    unsigned int eax, ebx, ecx, edx, esi, edi;
    unsigned int cflag;
};
struct WORDREGS {
    unsigned short ax, __ax_hi, bx, __bx_hi, cx, __cx_hi, dx, __dx_hi;
    unsigned short si, __si_hi, di, __di_hi;
    unsigned int   cflag;
};
struct BYTEREGS {
    unsigned char  al, ah;
    unsigned short __eax_hi;
    unsigned char  bl, bh;
    unsigned short __ebx_hi;
    unsigned char  cl, ch;
    unsigned short __ecx_hi;
    unsigned char  dl, dh;
    unsigned short __edx_hi;
};
union REGS {
    struct DWORDREGS x;
    struct WORDREGS  w;
    struct BYTEREGS  h;
};
/* The segment registers. */
struct SREGS {
    unsigned short es, cs, ss, ds, fs, gs;
};

/* Calls software interrupt __intno with the registers in *__in, and
 * stores the result registers in *__out. Gives EAX. int386x also loads
 * DS and ES from *__sr and stores them back. intdos and intdosx call
 * INT 21h. */
int  int386(int __intno, union REGS *__in, union REGS *__out);
int  int386x(int __intno, union REGS *__in, union REGS *__out, struct SREGS *__sr);
int  intdos(union REGS *__in, union REGS *__out);
int  intdosx(union REGS *__in, union REGS *__out, struct SREGS *__sr);
/* Gives the current segment registers. */
void segread(struct SREGS *__sr);

/* The 16-bit names. */
#define int86  int386
#define int86x int386x

#ifdef __cplusplus
}
#endif

#endif /* _I86_H_ */
