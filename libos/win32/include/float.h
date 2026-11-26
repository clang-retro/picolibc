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

/* The Windows C library's additions to <float.h>: the floating-point
 * control word and status word, and the _isnan family. clang's float.h,
 * which has the ISO C macros, includes this header for Windows targets. */

#if defined(__has_include_next) && __has_include_next(<float.h>)
#include_next <float.h>
#endif

#ifndef _WIN32_FLOAT_H_
#define _WIN32_FLOAT_H_

#include <sys/cdefs.h>

_BEGIN_STD_C

/* The bits of _control87 and _controlfp. They are not the bits of the
 * x87 control word: the library changes one form to the other. */
#define _MCW_EM        0x0008001fU /* The exception masks. */
#define _EM_INEXACT    0x00000001U
#define _EM_UNDERFLOW  0x00000002U
#define _EM_OVERFLOW   0x00000004U
#define _EM_ZERODIVIDE 0x00000008U
#define _EM_INVALID    0x00000010U
#define _EM_DENORMAL   0x00080000U

#define _MCW_RC        0x00000300U /* Rounding. */
#define _RC_NEAR       0x00000000U
#define _RC_DOWN       0x00000100U
#define _RC_UP         0x00000200U
#define _RC_CHOP       0x00000300U

#define _MCW_PC        0x00030000U /* Precision. */
#define _PC_64         0x00000000U
#define _PC_53         0x00010000U
#define _PC_24         0x00020000U

#define _MCW_IC        0x00040000U /* Infinity (the 8087 and 80287 only). */
#define _IC_AFFINE     0x00040000U
#define _IC_PROJECTIVE 0x00000000U

/* The control word that _fpreset sets: all exceptions masked, round to
 * nearest, 53-bit precision. */
#define _CW_DEFAULT                                                                                \
    (_RC_NEAR | _PC_53 | _EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT \
     | _EM_DENORMAL)

/* The bits of _clearfp and _statusfp. */
#define _SW_INEXACT    0x00000001U
#define _SW_UNDERFLOW  0x00000002U
#define _SW_OVERFLOW   0x00000004U
#define _SW_ZERODIVIDE 0x00000008U
#define _SW_INVALID    0x00000010U
#define _SW_DENORMAL   0x00080000U

/* The classes of _fpclass. */
#define _FPCLASS_SNAN 0x0001
#define _FPCLASS_QNAN 0x0002
#define _FPCLASS_NINF 0x0004
#define _FPCLASS_NN   0x0008
#define _FPCLASS_ND   0x0010
#define _FPCLASS_NZ   0x0020
#define _FPCLASS_PZ   0x0040
#define _FPCLASS_PD   0x0080
#define _FPCLASS_PN   0x0100
#define _FPCLASS_PINF 0x0200

/* Changes the bits of the control word that are set in mask to the bits
 * of value. The result is the control word after the change. */
unsigned int _control87(unsigned int value, unsigned int mask);
/* As _control87; it does not change _EM_DENORMAL. */
unsigned int _controlfp(unsigned int value, unsigned int mask);
/* The status word, which it then clears. */
unsigned int _clearfp(void);
/* The status word. */
unsigned int _statusfp(void);
/* Sets the unit to its start state and the control word to _CW_DEFAULT. */
void         _fpreset(void);

int          _isnan(double);
int          _finite(double);
int          _fpclass(double);
double       _copysign(double, double);
double       _chgsign(double);
double       _hypot(double, double);
double       _logb(double);
double       _nextafter(double, double);
double       _scalb(double, long);

_END_STD_C

#endif /* _WIN32_FLOAT_H_ */
