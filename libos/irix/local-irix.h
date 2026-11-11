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

#ifndef _LOCAL_IRIX_H_
#define _LOCAL_IRIX_H_

#define _GNU_SOURCE

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/*
 * IRIX system calls. The call number, 1000 plus the SVR4 number, goes in
 * v0 and the arguments in a0-a3 (o32) or a0-a7 (n32 and 64). The kernel
 * returns the result in v0 (and v1), and sets a3 to nonzero with the
 * errno value in v0 on failure. It may change the temporaries.
 */

#define __IRIX_SYS_exit         1001
#define __IRIX_SYS_read         1003
#define __IRIX_SYS_write        1004
#define __IRIX_SYS_open         1005
#define __IRIX_SYS_close        1006
#define __IRIX_SYS_unlink       1010
#define __IRIX_SYS_chdir        1012
#define __IRIX_SYS_time         1013
#define __IRIX_SYS_brk          1017
#define __IRIX_SYS_lseek        1019
#define __IRIX_SYS_getpid       1020
#define __IRIX_SYS_access       1033
#define __IRIX_SYS_kill         1037
#define __IRIX_SYS_rmdir        1079
#define __IRIX_SYS_mkdir        1080
#define __IRIX_SYS_rename       1114
#define __IRIX_SYS_gettimeofday 1128
#define __IRIX_SYS_xstat        1158
#define __IRIX_SYS_lxstat       1159
#define __IRIX_SYS_fxstat       1160

/* Make system call n with up to three arguments. On failure, sets errno
 * and returns -1. */
long __irix_syscall(long n, long a, long b, long c);

/* Set errno for an IRIX (SVR4) error number; returns -1. */
long __irix_error(long e);

#define __irix_call(n, a, b, c) __irix_syscall(n, (long)(a), (long)(b), (long)(c))

struct stat;
int __irix_xstat(long call, long arg, struct stat *st);

#endif /* _LOCAL_IRIX_H_ */
