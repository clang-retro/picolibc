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

#ifndef _CPM_H_
#define _CPM_H_

#include <sys/cdefs.h>
#include <stdint.h>

_BEGIN_STD_C

/* Call the CP/M-86 BDOS (INT E0h): function fn in CL, the parameter in
 * DX, an offset in DS for addresses. Returns AX; byte results are in the
 * low byte. As bdos() in BDS C, Aztec C and other CP/M compilers. */
unsigned   bdos(int __fn, unsigned __dx);

/* The BDOS version (function 12): 0022h for CP/M-86 1.x, 1031h for CP/M-86
 * Plus, 1431h for Concurrent CP/M. */
extern uint16_t __cpm_version;
#define _cpm_version __cpm_version

/* The base page: the group addresses, the default FCBs at 5Ch and 6Ch and
 * the command tail at 80h. */
extern uint8_t __cpm_base_page[256] __asm__("___cpm_base_page");
#define _cpm_base_page __cpm_base_page

/* Console I/O without line editing or echo, with BDOS function 6. */
int        getch(void);  /* a key, waiting for it */
int        getche(void); /* a key, echoed */
int        kbhit(void);  /* whether a key is waiting */
int        ungetch(int __c);
int        putch(int __c);
int        cputs(const char *__s);

_END_STD_C

#endif /* _CPM_H_ */
