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
 * System calls for SCO UnixWare (2 and later) and OpenServer (5 and
 * later) on 32-bit x86. Both take the SVR4 calls below through the lcall
 * $7,$0 gate; these numbers, the old struct stat and the open flags are
 * the same on both. OpenServer (an SVR3.2 system) has its own numbers for
 * the newer calls (lstat, rename, gettimeofday), so those are left out.
 */

#ifndef _LOCAL_SCO_H_
#define _LOCAL_SCO_H_

#define _GNU_SOURCE
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define __SCO_SYS_exit   1
#define __SCO_SYS_read   3
#define __SCO_SYS_write  4
#define __SCO_SYS_open   5
#define __SCO_SYS_close  6
#define __SCO_SYS_link   9
#define __SCO_SYS_unlink 10
#define __SCO_SYS_chdir  12
#define __SCO_SYS_time   13
#define __SCO_SYS_brk    17
#define __SCO_SYS_stat   18
#define __SCO_SYS_lseek  19
#define __SCO_SYS_getpid 20
#define __SCO_SYS_fstat  28
#define __SCO_SYS_access 33
#define __SCO_SYS_kill   37
#define __SCO_SYS_ioctl  54
#define __SCO_SYS_rmdir  79
#define __SCO_SYS_mkdir  80

/* ioctl: TCGETA, ('T' << 8) | 1, with a struct termio (18 bytes). */
#define __SCO_TCGETA 0x5401

long __sco_syscall(long n, long a, long b, long c);
long __sco_error(long e);

#define __sco_call(n, a, b, c) __sco_syscall(n, (long)(a), (long)(b), (long)(c))

#endif /* _LOCAL_SCO_H_ */
