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

/* The Windows C library's console functions. They read the keyboard and
 * write the screen through the console, whatever standard input and
 * standard output are. */

#ifndef _CONIO_H_
#define _CONIO_H_

#include <stdarg.h>
#include <sys/cdefs.h>

_BEGIN_STD_C

/* A key, without echo; 0 or 0xE0 comes before the scan code of a key that
 * has no character (a function key, an arrow). */
int   _getch(void);
/* As _getch, with echo. */
int   _getche(void);
/* Not zero when a key is waiting. */
int   _kbhit(void);
/* Gives a key back to the next _getch. */
int   _ungetch(int);
int   _putch(int);
int   _cputs(const char *);
/* Reads a line: buf[0] is the most characters to read; the length goes
 * in buf[1] and the text from buf + 2. */
char *_cgets(char *);
int   _cprintf(const char *, ...);
int   _vcprintf(const char *, va_list);

#if __MISC_VISIBLE
/* The old names, as Microsoft's oldnames.lib gives them. */
int   getch(void);
int   getche(void);
int   kbhit(void);
int   ungetch(int);
int   putch(int);
int   cputs(const char *);
char *cgets(char *);
int   cprintf(const char *, ...);
#endif

_END_STD_C

#endif /* _CONIO_H_ */
