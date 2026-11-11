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

#include "local-irix.h"
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/* <fcntl.h> on IRIX. */
#define IRIX_O_APPEND   0x0008
#define IRIX_O_SYNC     0x0010
#define IRIX_O_NONBLOCK 0x0080
#define IRIX_O_CREAT    0x0100
#define IRIX_O_TRUNC    0x0200
#define IRIX_O_EXCL     0x0400
#define IRIX_O_NOCTTY   0x0800

int
open(const char *path, int flags, ...)
{
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = (mode_t)va_arg(ap, int);
        va_end(ap);
    }
    long f = flags & O_ACCMODE;
    if (flags & O_APPEND)
        f |= IRIX_O_APPEND;
    if (flags & O_SYNC)
        f |= IRIX_O_SYNC;
    if (flags & O_NONBLOCK)
        f |= IRIX_O_NONBLOCK;
    if (flags & O_CREAT)
        f |= IRIX_O_CREAT;
    if (flags & O_TRUNC)
        f |= IRIX_O_TRUNC;
    if (flags & O_EXCL)
        f |= IRIX_O_EXCL;
    if (flags & O_NOCTTY)
        f |= IRIX_O_NOCTTY;
    return (int)__irix_call(__IRIX_SYS_open, path, f, mode);
}

int
close(int fd)
{
    return (int)__irix_call(__IRIX_SYS_close, fd, 0, 0);
}

ssize_t
read(int fd, void *buf, size_t n)
{
    return __irix_call(__IRIX_SYS_read, fd, buf, n);
}

ssize_t
write(int fd, const void *buf, size_t n)
{
    return __irix_call(__IRIX_SYS_write, fd, buf, n);
}

off_t
lseek(int fd, off_t off, int whence)
{
    return __irix_call(__IRIX_SYS_lseek, fd, off, whence);
}

int
unlink(const char *path)
{
    return (int)__irix_call(__IRIX_SYS_unlink, path, 0, 0);
}

int
rmdir(const char *path)
{
    return (int)__irix_call(__IRIX_SYS_rmdir, path, 0, 0);
}

int
mkdir(const char *path, mode_t mode)
{
    return (int)__irix_call(__IRIX_SYS_mkdir, path, mode, 0);
}

int
rename(const char *from, const char *to)
{
    return (int)__irix_call(__IRIX_SYS_rename, from, to, 0);
}

int
chdir(const char *path)
{
    return (int)__irix_call(__IRIX_SYS_chdir, path, 0, 0);
}

int
access(const char *path, int mode)
{
    return (int)__irix_call(__IRIX_SYS_access, path, mode, 0);
}

pid_t
getpid(void)
{
    return (pid_t)__irix_call(__IRIX_SYS_getpid, 0, 0, 0);
}

int
kill(pid_t pid, int sig)
{
    return (int)__irix_call(__IRIX_SYS_kill, pid, sig, 0);
}

int
gettimeofday(struct timeval * restrict tv, void * restrict tz)
{
    /* IRIX's struct timeval: time_t is a long, as is tv_usec. */
    struct {
        long sec;
        long usec;
    } t;
    (void)tz;
    if (__irix_call(__IRIX_SYS_gettimeofday, &t, 0, 0) < 0)
        return -1;
    if (tv) {
        tv->tv_sec = t.sec;
        tv->tv_usec = t.usec;
    }
    return 0;
}

void
_exit(int status)
{
    for (;;)
        __irix_call(__IRIX_SYS_exit, status, 0, 0);
}

/* The heap grows from the end of the bss with brk(). */
extern char  _end[];
static char *cur_brk;

void *
sbrk(ptrdiff_t incr)
{
    if (!cur_brk)
        cur_brk = (char *)(((uintptr_t)_end + 15) & ~(uintptr_t)15);
    char *old = cur_brk;
    if (incr && __irix_call(__IRIX_SYS_brk, old + incr, 0, 0) < 0) {
        errno = ENOMEM;
        return (void *)-1;
    }
    cur_brk = old + incr;
    return old;
}
