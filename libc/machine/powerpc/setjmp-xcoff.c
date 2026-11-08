/*
 *
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

/*
 * setjmp and longjmp for 32-bit PowerPC objects in XCOFF, as clang's
 * powerpc-apple-macos9 target (classic Mac OS) makes them. setjmp.S is
 * written for ELF assemblers; this saves the registers it saves (the
 * nonvolatile ones, LR and CR), in a jmp_buf of the same size (32
 * doubles), with clang's integrated assembler:
 *
 *    0  r1 (stack)     4  r2 (TOC)     8  LR     12  CR
 *   16  r13 to r31 (19 words)         96  f14 to f31 (18 doubles)
 */

#include <setjmp.h>

__attribute__((naked)) int
setjmp(jmp_buf env)
{
    __asm__ volatile("stw 1,0(3)\n\t"
                     "stw 2,4(3)\n\t"
                     "mflr 0\n\t"
                     "stw 0,8(3)\n\t"
                     "mfcr 0\n\t"
                     "stw 0,12(3)\n\t"
                     "stmw 13,16(3)\n\t"
                     "stfd 14,96(3)\n\t"
                     "stfd 15,104(3)\n\t"
                     "stfd 16,112(3)\n\t"
                     "stfd 17,120(3)\n\t"
                     "stfd 18,128(3)\n\t"
                     "stfd 19,136(3)\n\t"
                     "stfd 20,144(3)\n\t"
                     "stfd 21,152(3)\n\t"
                     "stfd 22,160(3)\n\t"
                     "stfd 23,168(3)\n\t"
                     "stfd 24,176(3)\n\t"
                     "stfd 25,184(3)\n\t"
                     "stfd 26,192(3)\n\t"
                     "stfd 27,200(3)\n\t"
                     "stfd 28,208(3)\n\t"
                     "stfd 29,216(3)\n\t"
                     "stfd 30,224(3)\n\t"
                     "stfd 31,232(3)\n\t"
                     "li 3,0\n\t"
                     "blr");
}

__attribute__((naked, noreturn)) void
longjmp(jmp_buf env, int val)
{
    __asm__ volatile("lwz 1,0(3)\n\t"
                     "lwz 2,4(3)\n\t"
                     "lwz 0,8(3)\n\t"
                     "mtlr 0\n\t"
                     "lwz 0,12(3)\n\t"
                     "mtcrf 0xff,0\n\t"
                     "lmw 13,16(3)\n\t"
                     "lfd 14,96(3)\n\t"
                     "lfd 15,104(3)\n\t"
                     "lfd 16,112(3)\n\t"
                     "lfd 17,120(3)\n\t"
                     "lfd 18,128(3)\n\t"
                     "lfd 19,136(3)\n\t"
                     "lfd 20,144(3)\n\t"
                     "lfd 21,152(3)\n\t"
                     "lfd 22,160(3)\n\t"
                     "lfd 23,168(3)\n\t"
                     "lfd 24,176(3)\n\t"
                     "lfd 25,184(3)\n\t"
                     "lfd 26,192(3)\n\t"
                     "lfd 27,200(3)\n\t"
                     "lfd 28,208(3)\n\t"
                     "lfd 29,216(3)\n\t"
                     "lfd 30,224(3)\n\t"
                     "lfd 31,232(3)\n\t"
                     /* setjmp returns val, or 1 when val is 0. */
                     "mr. 3,4\n\t"
                     "bnelr\n\t"
                     "li 3,1\n\t"
                     "blr");
}
