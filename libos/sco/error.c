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

#include "local-sco.h"

/*
 * UnixWare uses the SVR4 numbers, and OpenServer the same for all but a
 * few (ENOTEMPTY and ELOOP). The first 34 are the same as picolibc's.
 */
long
__sco_error(long e)
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
    case 90:  /* SVR4 ELOOP */
    case 150: /* OpenServer ELOOP */
        r = ELOOP;
        break;
    case 93:  /* SVR4 ENOTEMPTY */
    case 145: /* OpenServer ENOTEMPTY */
        r = ENOTEMPTY;
        break;
    case 95:
        r = ENOTSOCK;
        break;
    case 122:
        r = EOPNOTSUPP;
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
