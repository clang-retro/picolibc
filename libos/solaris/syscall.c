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

#include "local-solaris.h"

long
__solaris_syscall(long n, long a, long b, long c)
{
    register long g1 __asm__("g1") = n;
    register long o0 __asm__("o0") = a;
    register long o1 __asm__("o1") = b;
    register long o2 __asm__("o2") = c;
    long          err;
    /* The carry flag says whether the call failed. */
    __asm__ volatile("ta 8\n\t"
                     "addx %%g0, 0, %4"
                     : "+r"(g1), "+r"(o0), "+r"(o1), "+r"(o2), "=r"(err)
                     :
                     : "o3", "o4", "o5", "cc", "memory");
    if (err)
        return __solaris_error(o0);
    return o0;
}

/* Solaris uses the SVR4 numbers. The first 34 are the same as picolibc's. */
long
__solaris_error(long e)
{
    int r;
    switch (e) {
    case 35:
        r = ENOMSG;
        break;
    case 36:
        r = EIDRM;
        break;
    case 45:
        r = EDEADLK;
        break;
    case 46:
        r = ENOLCK;
        break;
    case 78:
        r = ENAMETOOLONG;
        break;
    case 79:
        r = EOVERFLOW;
        break;
    case 89:
        r = ENOSYS;
        break;
    case 90:
        r = ELOOP;
        break;
    case 93:
        r = ENOTEMPTY;
        break;
    case 95:
        r = ENOTSOCK;
        break;
    case 122:
        r = EOPNOTSUPP;
        break;
    case 145:
        r = ETIMEDOUT;
        break;
    case 151:
        r = ESTALE;
        break;
    default:
        r = e <= 34 ? (int)e : EINVAL;
        break;
    }
    errno = r;
    return -1;
}
