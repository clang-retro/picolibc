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

#ifndef _LOCAL_BSD_H_
#define _LOCAL_BSD_H_

#define _GNU_SOURCE

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/*
 * BSD system calls on 32-bit x86: int $0x80, the number in eax and the
 * arguments on the stack above a return address, the result in eax (and
 * edx); on failure the kernel sets the carry flag and puts the BSD error
 * number in eax. The numbers are the 4.4BSD ones. Where the BSDs went
 * different ways (stat), these are the calls the oldest ELF releases
 * have, which the later ones keep for compatibility.
 */

/* The 4.4BSD calls, the same on FreeBSD and NetBSD. */
#define __BSD_SYS_exit   1
#define __BSD_SYS_read   3
#define __BSD_SYS_write  4
#define __BSD_SYS_open   5
#define __BSD_SYS_close  6
#define __BSD_SYS_unlink 10
#define __BSD_SYS_chdir  12
#define __BSD_SYS_break  17
#define __BSD_SYS_getpid 20
#define __BSD_SYS_access 33
#define __BSD_SYS_kill   37
#define __BSD_SYS_ioctl  54
/* 32-bit time_t (compat_50 on NetBSD 5 and later) */
#define __BSD_SYS_gettimeofday 116
#define __BSD_SYS_rename       128
#define __BSD_SYS_mkdir        136
#define __BSD_SYS_rmdir        137
/* lseek(fd, pad, off_t, whence) (freebsd6_lseek on FreeBSD 7 and later) */
#define __BSD_SYS_lseek 199

#if defined(__FreeBSD__)
/* FreeBSD 3 to 11's stat (freebsd11_stat in 12 and later) */
#define __BSD_SYS_stat  188
#define __BSD_SYS_fstat 189
#define __BSD_SYS_lstat 190
#elif defined(__NetBSD__)
/* NetBSD 1.3 to 2.x's __stat13 (compat_30 in 3.0 and later) */
#define __BSD_SYS_stat  278
#define __BSD_SYS_fstat 279
#define __BSD_SYS_lstat 280
#else
#error "Build with --target=i386-unknown-freebsd or i386-unknown-netbsd"
#endif

/* ioctl that tcgetattr makes: TIOCGETA, struct termios. */
#define __BSD_TIOCGETA 0x402c7413

/* Make system call n with up to six arguments; the result is eax, with edx
 * as the high word. On failure, sets errno and returns -1. */
long long __bsd_syscall(long n, long a, long b, long c, long d, long e, long f);

/* Set errno for a BSD error number; returns -1. */
long long __bsd_error(long e);

#define __bsd_call(n, a, b, c) ((long)__bsd_syscall(n, (long)(a), (long)(b), (long)(c), 0, 0, 0))

#endif /* _LOCAL_BSD_H_ */
