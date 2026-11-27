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

/* The 32-bit kernel's struct stat, which statx and fstatx fill in. */
struct __aix_stat {
    uint32_t st_dev;
    uint32_t st_ino;
    uint32_t st_mode;
    int16_t  st_nlink;
    uint16_t st_flag;
    uint32_t st_uid;
    uint32_t st_gid;
    uint32_t st_rdev;
    int32_t  st_size;
    int32_t  st_atime_sec, st_atime_nsec;
    int32_t  st_mtime_sec, st_mtime_nsec;
    int32_t  st_ctime_sec, st_ctime_nsec;
    uint32_t st_blksize;
    uint32_t st_blocks;
    int32_t  st_vfstype;
    uint32_t st_vfs;
    uint32_t st_type;
    uint32_t st_gen;
    uint32_t st_reserved[9];
};

_Static_assert(sizeof(struct __aix_stat) == 116, "AIX struct stat is 116 bytes");

#define __AIX_STX_NORMAL 0

int statx(const char *path, struct __aix_stat *buf, int len, int cmd);
int fstatx(int fd, struct __aix_stat *buf, int len, int cmd);

struct stat;
void __aix_stat_convert(const struct __aix_stat *a, struct stat *st);

/* gettimer(TIMEOFDAY, ...) reads the time of day in seconds and
   nanoseconds. */
struct __aix_timestruc {
    uint32_t tv_sec;
    int32_t  tv_nsec;
};

#define __AIX_TIMEOFDAY 9

int                 gettimer(int timer, struct __aix_timestruc *value);

/*
 * The kernel stores the AIX error number of a failed call in the exported
 * errno variable, and error.c's __errno maps it to picolibc's numbering
 * when errno is next read or written. __aix_call clears it before the call
 * so a failure with no number can be given EIO.
 */
extern volatile int __aix_kernel_errno __asm__("errno");
long                __aix_result(long r);

#define __aix_call(expr)            \
    ({                              \
        __aix_kernel_errno = 0;     \
        __aix_result((long)(expr)); \
    })

#endif /* _LOCAL_AIX_H_ */
