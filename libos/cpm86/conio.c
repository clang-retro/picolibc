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
#include "cpm.h"

unsigned
bdos(int fn, unsigned dx)
{
    return __cpm_bdos((unsigned)fn & 0xff, dx);
}

static int pending = -1;

int
kbhit(void)
{
    if (pending >= 0)
        return 1;
    /* Function 6 with FFh returns a key, or 0 when there is none. */
    uint8_t c = (uint8_t)__cpm_bdos(BDOS_CONIO, 0xff);
    if (c == 0)
        return 0;
    pending = c;
    return 1;
}

int
getch(void)
{
    if (pending >= 0) {
        int c = pending;
        pending = -1;
        return c;
    }
    for (;;) {
        uint8_t c = (uint8_t)__cpm_bdos(BDOS_CONIO, 0xff);
        if (c != 0)
            return c;
    }
}

int
getche(void)
{
    int c = getch();
    putch(c);
    return c;
}

int
ungetch(int c)
{
    if (pending >= 0 || c < 0)
        return -1;
    pending = c & 0xff;
    return pending;
}

int
putch(int c)
{
    /* Function 6 would take FEh and FFh for input requests. */
    __cpm_bdos((uint8_t)c >= 0xfe ? BDOS_CONOUT : BDOS_CONIO, (uint8_t)c);
    return (uint8_t)c;
}

int
cputs(const char *s)
{
    while (*s)
        putch(*s++);
    return 0;
}
