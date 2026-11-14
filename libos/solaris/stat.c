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

#include "local-solaris.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Solaris's struct stat for 32-bit programs, without large file support
 * (the stat, lstat and fstat calls): 32-bit sizes and inode numbers, and
 * times as seconds and nanoseconds. It is followed by room for what
 * emulators that use 64-bit block counts write.
 */
struct solaris_stat {
    uint32_t st_dev;
    long     st_pad1[3];
    uint32_t st_ino;
    uint32_t st_mode;
    uint32_t st_nlink;
    int32_t  st_uid;
    int32_t  st_gid;
    uint32_t st_rdev;
    long     st_pad2[2];
    long     st_size;
    long     st_pad3;
    struct {
        long sec; /* time_t */
        long nsec;
    } st_atim, st_mtim, st_ctim;
    long st_blksize;
    long st_blocks;
    char st_fstype[16];
    long st_pad4[8];
    long st_spare[8];
};

/*
 * call(arg, &s), or on Solaris 11 fstatat(fd, path, &s, flags): fstatat
 * takes the same struct stat, and a null path for fstat.
 */
static int
solaris_stat(long call, long arg, long fd, const char *path, long flags, struct stat *st)
{
    struct solaris_stat s;
    long                r;
    if (__solaris_use_at())
        r = __solaris_call4(__SOLARIS_SYS_fstatat, fd, path, &s, flags);
    else
        r = __solaris_call(call, arg, &s, 0);
    if (r < 0)
        return -1;
    memset(st, 0, sizeof(*st));
    st->st_dev = s.st_dev;
    st->st_ino = s.st_ino;
    st->st_mode = s.st_mode;
    st->st_nlink = s.st_nlink;
    st->st_uid = s.st_uid;
    st->st_gid = s.st_gid;
    st->st_rdev = s.st_rdev;
    st->st_size = s.st_size;
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
    return solaris_stat(__SOLARIS_SYS_stat, (long)path, __SOLARIS_AT_FDCWD, path, 0, st);
}

int
lstat(const char * restrict path, struct stat * restrict st)
{
    return solaris_stat(__SOLARIS_SYS_lstat, (long)path, __SOLARIS_AT_FDCWD, path,
                        __SOLARIS_AT_SYMLINK_NOFOLLOW, st);
}

int
fstat(int fd, struct stat *st)
{
    return solaris_stat(__SOLARIS_SYS_fstat, fd, fd, NULL, 0, st);
}

int
isatty(int fd)
{
    struct stat st;
    if (fstat(fd, &st) < 0)
        return 0;
    if (!S_ISCHR(st.st_mode)) {
        errno = ENOTTY;
        return 0;
    }
    return 1;
}
