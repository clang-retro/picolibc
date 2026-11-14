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

#ifndef _LOCAL_SOLARIS_H_
#define _LOCAL_SOLARIS_H_

#define _GNU_SOURCE

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/*
 * Solaris 2.x system calls, the SVR4 numbers. On SPARC the number goes in
 * g1 and the arguments in o0-o5, and "ta 8" traps to the kernel, which
 * returns the result in o0 (and o1). On failure it sets the carry flag
 * and puts the errno value in o0. These are the calls every Solaris from
 * 2.5 to 10 has; Solaris 11 dropped some of them (open, stat and others
 * became openat and fstatat), so these programs don't run there.
 */

#define __SOLARIS_SYS_exit         1
#define __SOLARIS_SYS_read         3
#define __SOLARIS_SYS_write        4
#define __SOLARIS_SYS_open         5
#define __SOLARIS_SYS_close        6
#define __SOLARIS_SYS_unlink       10
#define __SOLARIS_SYS_chdir        12
#define __SOLARIS_SYS_time         13
#define __SOLARIS_SYS_brk          17
#define __SOLARIS_SYS_stat         18
#define __SOLARIS_SYS_lseek        19
#define __SOLARIS_SYS_getpid       20
#define __SOLARIS_SYS_fstat        28
#define __SOLARIS_SYS_access       33
#define __SOLARIS_SYS_kill         37
#define __SOLARIS_SYS_rmdir        79
#define __SOLARIS_SYS_mkdir        80
#define __SOLARIS_SYS_lstat        88
#define __SOLARIS_SYS_rename       134
#define __SOLARIS_SYS_gettimeofday 156

/* Make system call n with up to three arguments. On failure, sets errno
 * and returns -1. */
long __solaris_syscall(long n, long a, long b, long c);

/* Set errno for a Solaris (SVR4) error number; returns -1. */
long __solaris_error(long e);

#define __solaris_call(n, a, b, c) __solaris_syscall(n, (long)(a), (long)(b), (long)(c))

#endif /* _LOCAL_SOLARIS_H_ */
