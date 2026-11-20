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

#include "local-bsd.h"

/* BSD error numbers: the first 34 are the same as picolibc's, except 11. */
long long
__bsd_error(long e)
{
    int r;
    switch (e) {
    case 11:
        r = EDEADLK;
        break;
    case 35:
        r = EAGAIN;
        break;
    case 36:
        r = EINPROGRESS;
        break;
    case 38:
        r = ENOTSOCK;
        break;
    case 45:
        r = EOPNOTSUPP;
        break;
    case 60:
        r = ETIMEDOUT;
        break;
    case 62:
        r = ELOOP;
        break;
    case 63:
        r = ENAMETOOLONG;
        break;
    case 66:
        r = ENOTEMPTY;
        break;
    case 77:
        r = ENOLCK;
        break;
    case 78:
        r = ENOSYS;
        break;
    case 84:
        r = EOVERFLOW;
        break;
    default:
        r = e <= 34 ? (int)e : EINVAL;
        break;
    }
    errno = r;
    return -1;
}
