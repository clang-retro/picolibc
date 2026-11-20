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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__FreeBSD__)
/* FreeBSD 3 to 11's struct stat on 32-bit x86 (96 bytes). */
struct bsd_stat {
    uint32_t st_dev;
    uint32_t st_ino;
    uint16_t st_mode;
    uint16_t st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    uint32_t st_rdev;
    struct {
        long sec; /* time_t */
        long nsec;
    } st_atim, st_mtim, st_ctim;
    int64_t  st_size;
    int64_t  st_blocks;
    uint32_t st_blksize;
    uint32_t st_flags;
    uint32_t st_gen;
    int32_t  st_lspare;
    int64_t  st_qspare[2];
};
#endif

static int
bsd_stat(long call, long arg, struct stat *st)
{
    struct bsd_stat s;
    if (__bsd_call(call, arg, &s, 0) < 0)
        return -1;
    memset(st, 0, sizeof(*st));
    st->st_dev = s.st_dev;
    st->st_ino = s.st_ino;
    st->st_mode = s.st_mode;
    st->st_nlink = s.st_nlink;
    st->st_uid = s.st_uid;
    st->st_gid = s.st_gid;
    st->st_rdev = s.st_rdev;
    st->st_size = (off_t)s.st_size;
    st->st_atime = s.st_atim.sec;
    st->st_mtime = s.st_mtim.sec;
    st->st_ctime = s.st_ctim.sec;
    st->st_blksize = s.st_blksize;
    st->st_blocks = s.st_blocks;
    return 0;
}

int
stat(const char * restrict path, struct stat * restrict st)
{
    return bsd_stat(__BSD_SYS_stat, (long)path, st);
}

int
lstat(const char * restrict path, struct stat * restrict st)
{
    return bsd_stat(__BSD_SYS_lstat, (long)path, st);
}

int
fstat(int fd, struct stat *st)
{
    return bsd_stat(__BSD_SYS_fstat, fd, st);
}

int
isatty(int fd)
{
    /* tcgetattr: struct termios is 44 bytes on the 32-bit BSDs. */
    long t[11];
    if (__bsd_call(__BSD_SYS_ioctl, fd, __BSD_TIOCGETA, t) < 0) {
        if (errno != EBADF)
            errno = ENOTTY;
        return 0;
    }
    return 1;
}
