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
 * Kernel calls for static programs on 32-bit AIX.
 *
 * AIX has no fixed system call numbers. The kernel exports its calls by
 * name from /unix, the way /lib/syscalls.exp lists them, and the system
 * loader binds a program's imports to them when it starts. So these are
 * ordinary external functions: the linker (xcoff-ld, given the SDK's
 * unix.exp import list) makes the glue code that calls them through their
 * function descriptors.
 *
 * Calls whose kernel name is the POSIX name (close, unlink, rename, mkdir,
 * rmdir, chdir, kill, _exit) are not defined here at all: a program's
 * reference to them binds straight to the kernel, as with AIX's own libc,
 * and the kernel sets errno. The others are wrappers around kernel calls
 * with other names.
 *
 * The names below are those that AIX's truss shows for the calls its own
 * libc makes. They still have to be checked against a real syscalls.exp.
 */

#ifndef _LOCAL_AIX_H_
#define _LOCAL_AIX_H_

#define _GNU_SOURCE
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

int  kopen(const char *path, int flags, int mode);
long kread(int fd, void *buf, unsigned long n);
long kwrite(int fd, const void *buf, unsigned long n);
long klseek(int fd, long off, int whence);
int  kioctl(int fd, int req, void *arg, void *ext);
int  accessx(const char *path, int mode, int who);
int  _getpid(void);

/*
 * The kernel sets the thread's errno variable when a call fails. The
 * program's errno is exported (the SDK's libc.exp) so the loader can find
 * it. __aix_result turns a result into picolibc's convention, with AIX
 * error numbers mapped to picolibc's, and gives errno a value even if the
 * kernel left it alone.
 */
long __aix_result(long r, int saved);

#define __aix_call(expr)                                                      \
    ({                                                                        \
        int  __saved = errno;                                                 \
        errno = 0;                                                            \
        long __r = (long)(expr);                                              \
        __aix_result(__r, __saved);                                           \
    })

#endif /* _LOCAL_AIX_H_ */
