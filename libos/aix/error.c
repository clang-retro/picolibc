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

#include "local-aix.h"

/*
 * AIX error numbers past the first 34, which match picolibc's. From AIX's
 * <sys/errno.h>; the networking ones are left out, as no call here can
 * fail with them.
 */
static int
aix_errno(int e)
{
    switch (e) {
    case 35: return ENOMSG;
    case 36: return EIDRM;
    case 45: return EDEADLK;
    case 49: return ENOLCK;
    case 52: return ESTALE;
    case 85: return ELOOP;
    case 86: return ENAMETOOLONG;
    case 87: return ENOTEMPTY;
    case 109: return ENOSYS;
    case 127: return EOVERFLOW;
    default: return e > 0 && e <= 34 ? e : EINVAL;
    }
}

/*
 * The kernel stores the AIX error number of a failed call in the program's
 * exported errno variable (the SDK's libc.exp exports it). picolibc's errno
 * is (*__errno()) on AIX, a separate variable: __errno moves a number the
 * kernel left into it, mapped to picolibc's numbering, first. So calls bound
 * straight to the kernel (close, unlink, rmdir and the rest) set errno as
 * the wrappers do.
 */
volatile int __aix_kernel_errno __asm__("errno");
static int   __aix_errno_value;

int *
__errno(void)
{
    int k = __aix_kernel_errno;
    if (k) {
        __aix_kernel_errno = 0;
        __aix_errno_value = aix_errno(k);
    }
    return &__aix_errno_value;
}

long
__aix_result(long r)
{
    /* A failure the kernel gave no error number for. */
    if (r == -1 && __aix_kernel_errno == 0)
        __aix_kernel_errno = 5; /* EIO */
    return r;
}
