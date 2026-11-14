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
 * The SPARC ABI's quad precision routines. On 32-bit SPARC, long double
 * is IEEE quad precision, and the compiler calls these for arithmetic on
 * it; Solaris has them in libc. Arguments are passed by address. They
 * call compiler-rt's soft-float routines, which do the work.
 */

#include <math.h>

typedef long double quad;

quad __addtf3(quad, quad);
quad __subtf3(quad, quad);
quad __multf3(quad, quad);
quad __divtf3(quad, quad);
int __eqtf2(quad, quad);
int __lttf2(quad, quad);
int __gttf2(quad, quad);
int __unordtf2(quad, quad);
int __fixtfsi(quad);
unsigned int __fixunstfsi(quad);
long long __fixtfdi(quad);
unsigned long long __fixunstfdi(quad);
quad __floatsitf(int);
quad __floatunsitf(unsigned int);
quad __floatditf(long long);
quad __floatunditf(unsigned long long);
quad __extendsftf2(float);
quad __extenddftf2(double);
float __trunctfsf2(quad);
double __trunctfdf2(quad);

quad _Q_add(const quad *a, const quad *b);
quad _Q_sub(const quad *a, const quad *b);
quad _Q_mul(const quad *a, const quad *b);
quad _Q_div(const quad *a, const quad *b);
quad _Q_neg(const quad *a);
quad _Q_sqrt(const quad *a);
int _Q_cmp(const quad *a, const quad *b);
int _Q_cmpe(const quad *a, const quad *b);
int _Q_feq(const quad *a, const quad *b);
int _Q_fne(const quad *a, const quad *b);
int _Q_flt(const quad *a, const quad *b);
int _Q_fle(const quad *a, const quad *b);
int _Q_fgt(const quad *a, const quad *b);
int _Q_fge(const quad *a, const quad *b);
int _Q_qtoi(const quad *a);
unsigned int _Q_qtou(const quad *a);
long long _Q_qtoll(const quad *a);
unsigned long long _Q_qtoull(const quad *a);
quad _Q_itoq(int a);
quad _Q_utoq(unsigned int a);
quad _Q_lltoq(long long a);
quad _Q_ulltoq(unsigned long long a);
quad _Q_stoq(float a);
quad _Q_dtoq(double a);
float _Q_qtos(const quad *a);
double _Q_qtod(const quad *a);

quad _Q_add(const quad *a, const quad *b) { return __addtf3(*a, *b); }
quad _Q_sub(const quad *a, const quad *b) { return __subtf3(*a, *b); }
quad _Q_mul(const quad *a, const quad *b) { return __multf3(*a, *b); }
quad _Q_div(const quad *a, const quad *b) { return __divtf3(*a, *b); }
quad _Q_sqrt(const quad *a) { return sqrtl(*a); }

quad
_Q_neg(const quad *a)
{
    union {
        quad          q;
        unsigned char c[sizeof(quad)];
    } u;
    u.q = *a;
    u.c[0] ^= 0x80; /* big-endian: the sign is in the first byte */
    return u.q;
}

/* 0: equal, 1: less, 2: greater, 3: unordered. */
int
_Q_cmp(const quad *a, const quad *b)
{
    if (__unordtf2(*a, *b))
        return 3;
    if (__eqtf2(*a, *b) == 0)
        return 0;
    return __lttf2(*a, *b) < 0 ? 1 : 2;
}

int _Q_cmpe(const quad *a, const quad *b) { return _Q_cmp(a, b); }
int _Q_feq(const quad *a, const quad *b) { return _Q_cmp(a, b) == 0; }
int _Q_fne(const quad *a, const quad *b) { return _Q_cmp(a, b) != 0; }
int _Q_flt(const quad *a, const quad *b) { return _Q_cmp(a, b) == 1; }
int _Q_fle(const quad *a, const quad *b) { int r = _Q_cmp(a, b); return r == 0 || r == 1; }
int _Q_fgt(const quad *a, const quad *b) { return _Q_cmp(a, b) == 2; }
int _Q_fge(const quad *a, const quad *b) { int r = _Q_cmp(a, b); return r == 0 || r == 2; }

int _Q_qtoi(const quad *a) { return __fixtfsi(*a); }
unsigned int _Q_qtou(const quad *a) { return __fixunstfsi(*a); }
long long _Q_qtoll(const quad *a) { return __fixtfdi(*a); }
unsigned long long _Q_qtoull(const quad *a) { return __fixunstfdi(*a); }
quad _Q_itoq(int a) { return __floatsitf(a); }
quad _Q_utoq(unsigned int a) { return __floatunsitf(a); }
quad _Q_lltoq(long long a) { return __floatditf(a); }
quad _Q_ulltoq(unsigned long long a) { return __floatunditf(a); }
quad _Q_stoq(float a) { return __extendsftf2(a); }
quad _Q_dtoq(double a) { return __extenddftf2(a); }
float _Q_qtos(const quad *a) { return __trunctfsf2(*a); }
double _Q_qtod(const quad *a) { return __trunctfdf2(*a); }
