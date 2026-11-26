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

/* The Windows C library's floating-point functions (float.h), on the x87
 * control word and status word. */

#include "local-win32.h"
/* The header includes the next float.h, clang's; from here that search
 * starts again from the top, which is what it must do. */
#pragma clang diagnostic ignored "-Winclude-next-absolute-path"
#include "include/float.h"
#include <math.h>

/* The x87 bits of the masks, in the order of the _EM_ bits. */
static const struct {
    unsigned ms, x87;
} masks[] = {
    { _EM_INVALID,    0x01 },
    { _EM_DENORMAL,   0x02 },
    { _EM_ZERODIVIDE, 0x04 },
    { _EM_OVERFLOW,   0x08 },
    { _EM_UNDERFLOW,  0x10 },
    { _EM_INEXACT,    0x20 },
};

/* An x87 control word in the Windows form. */
static unsigned
from_x87(unsigned cw)
{
    unsigned r = 0;
    for (unsigned i = 0; i < sizeof(masks) / sizeof(masks[0]); i++)
        if (cw & masks[i].x87)
            r |= masks[i].ms;
    r |= ((cw >> 10) & 3) << 8; /* Rounding: the same order. */
    switch ((cw >> 8) & 3) {
    case 0:
        r |= _PC_24;
        break;
    case 2:
        r |= _PC_53;
        break;
    default:
        r |= _PC_64;
        break;
    }
    if (cw & 0x1000)
        r |= _IC_AFFINE;
    return r;
}

/* A control word in the Windows form as an x87 control word. */
static unsigned
to_x87(unsigned v)
{
    unsigned cw = 0x40; /* Bit 6 is always set. */
    for (unsigned i = 0; i < sizeof(masks) / sizeof(masks[0]); i++)
        if (v & masks[i].ms)
            cw |= masks[i].x87;
    cw |= ((v >> 8) & 3) << 10;
    switch (v & _MCW_PC) {
    case _PC_24:
        break;
    case _PC_53:
        cw |= 0x200;
        break;
    default:
        cw |= 0x300;
        break;
    }
    if (v & _IC_AFFINE)
        cw |= 0x1000;
    return cw;
}

/* An x87 status word in the Windows form. */
static unsigned
status(unsigned sw)
{
    unsigned r = 0;
    if (sw & 0x01)
        r |= _SW_INVALID;
    if (sw & 0x02)
        r |= _SW_DENORMAL;
    if (sw & 0x04)
        r |= _SW_ZERODIVIDE;
    if (sw & 0x08)
        r |= _SW_OVERFLOW;
    if (sw & 0x10)
        r |= _SW_UNDERFLOW;
    if (sw & 0x20)
        r |= _SW_INEXACT;
    return r;
}

/* Changes the bits of the control word that are set in mask. */
unsigned int
_control87(unsigned int value, unsigned int mask)
{
    unsigned short cw;
    __asm__ volatile("fnstcw %0" : "=m"(cw));
    unsigned now = from_x87(cw);
    if (mask) {
        now = (now & ~mask) | (value & mask);
        cw = (unsigned short)to_x87(now);
        __asm__ volatile("fldcw %0" : : "m"(cw));
    }
    return now;
}

/* As _control87, without the denormal mask. */
unsigned int
_controlfp(unsigned int value, unsigned int mask)
{
    return _control87(value, mask & ~_EM_DENORMAL);
}

/* The status word, which it then clears. */
unsigned int
_clearfp(void)
{
    unsigned short sw;
    __asm__ volatile("fnstsw %0\n\tfnclex" : "=m"(sw));
    return status(sw);
}

/* The status word. */
unsigned int
_statusfp(void)
{
    unsigned short sw;
    __asm__ volatile("fnstsw %0" : "=m"(sw));
    return status(sw);
}

/* Sets the unit to its start state with the default control word. */
void
_fpreset(void)
{
    __asm__ volatile("fninit");
    _control87(_CW_DEFAULT, 0xffffffffU);
}

/* Not zero when x is a NaN. */
int
_isnan(double x)
{
    return isnan(x);
}

/* Not zero when x is not infinite and not a NaN. */
int
_finite(double x)
{
    return isfinite(x);
}

/* The class of x: one of the _FPCLASS_ values. */
int
_fpclass(double x)
{
    int neg = signbit(x) != 0;
    switch (fpclassify(x)) {
    case FP_NAN:
        return issignaling(x) ? _FPCLASS_SNAN : _FPCLASS_QNAN;
    case FP_INFINITE:
        return neg ? _FPCLASS_NINF : _FPCLASS_PINF;
    case FP_ZERO:
        return neg ? _FPCLASS_NZ : _FPCLASS_PZ;
    case FP_SUBNORMAL:
        return neg ? _FPCLASS_ND : _FPCLASS_PD;
    default:
        return neg ? _FPCLASS_NN : _FPCLASS_PN;
    }
}

/* x with the sign of y. */
double
_copysign(double x, double y)
{
    return copysign(x, y);
}

/* x with the opposite sign. */
double
_chgsign(double x)
{
    return -x;
}

/* The length of the hypotenuse of a right triangle. */
double
_hypot(double x, double y)
{
    return hypot(x, y);
}

/* The exponent of x. */
double
_logb(double x)
{
    return logb(x);
}

/* The next value after x in the direction of y. */
double
_nextafter(double x, double y)
{
    return nextafter(x, y);
}

/* x times 2 to the power n. */
double
_scalb(double x, long n)
{
    return scalbln(x, n);
}
