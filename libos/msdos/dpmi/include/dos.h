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
 * dos.h - DOS functions of 32-bit DOS programs (i386-pc-msdos), as in
 * Turbo C and Open Watcom: the PC speaker, delays, I/O ports and memory
 * at a real-mode segment:offset.
 */

#ifndef _DOS_H_
#define _DOS_H_

#include <i86.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The real-mode segment of the program segment prefix. */
extern unsigned short __msdos_psp;
#define _psp __msdos_psp

/* The arguments of main: argv[0] is the program's path. */
extern int    __argc;
extern char **__argv;

/* Interrupts off and on. */
static __inline__ void
disable(void)
{
    _disable();
}

static __inline__ void
enable(void)
{
    _enable();
}

/* Waits __ms milliseconds, timed by the 8253 timer. */
void delay(unsigned __ms);
/* Sounds the PC speaker at __hz hertz, until nosound(). */
void sound(unsigned __hz);
void nosound(void);

/* I/O ports, as in Turbo C, and the double words of the 386. */
static __inline__ unsigned char
inportb(unsigned __port)
{
    unsigned char __v;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(__v) : "Nd"((unsigned short)__port));
    return __v;
}

static __inline__ unsigned
inport(unsigned __port)
{
    unsigned short __v;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(__v) : "Nd"((unsigned short)__port));
    return __v;
}

static __inline__ unsigned long
inportl(unsigned __port)
{
    unsigned long __v;
    __asm__ __volatile__("inl %w1, %0" : "=a"(__v) : "Nd"((unsigned short)__port));
    return __v;
}

static __inline__ void
outportb(unsigned __port, unsigned char __value)
{
    __asm__ __volatile__("outb %b0, %w1" : : "a"(__value), "Nd"((unsigned short)__port));
}

static __inline__ void
outport(unsigned __port, unsigned __value)
{
    __asm__ __volatile__("outw %w0, %w1"
                         :
                         : "a"((unsigned short)__value), "Nd"((unsigned short)__port));
}

static __inline__ void
outportl(unsigned __port, unsigned long __value)
{
    __asm__ __volatile__("outl %0, %w1" : : "a"(__value), "Nd"((unsigned short)__port));
}

/* Reads and writes memory at a real-mode segment:offset, as in Turbo C:
 * peek and poke move 16 bits. */
static __inline__ int
peek(unsigned __s, unsigned __o)
{
    return *(volatile short *)MK_FP(__s, __o);
}

static __inline__ char
peekb(unsigned __s, unsigned __o)
{
    return *(volatile char *)MK_FP(__s, __o);
}

static __inline__ void
poke(unsigned __s, unsigned __o, int __v)
{
    *(volatile short *)MK_FP(__s, __o) = (short)__v;
}

static __inline__ void
pokeb(unsigned __s, unsigned __o, char __v)
{
    *(volatile char *)MK_FP(__s, __o) = __v;
}

#ifdef __cplusplus
}
#endif

#endif /* _DOS_H_ */
