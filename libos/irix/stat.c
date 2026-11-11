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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * IRIX's stat calls take a structure version: 2 is the SVR4 struct stat
 * with a 32-bit size, 3 the struct stat64. o32 uses version 2, which every
 * IRIX has; n32 and 64 use version 3, as their libc does. "long" below is
 * the ABI's long, as in IRIX's <sys/stat.h>.
 */
#if _MIPS_SIM == _ABIO32
#define IRIX_STAT_VER 2
typedef unsigned long irix_ino_t;
typedef long          irix_off_t;
#else
#define IRIX_STAT_VER 3
typedef unsigned long long irix_ino_t;
typedef long long          irix_off_t;
#endif

struct irix_stat {
    uint32_t   st_dev;
    long       st_pad0[3];
    irix_ino_t st_ino;
    uint32_t   st_mode;
    uint32_t   st_nlink;
    int32_t    st_uid;
    int32_t    st_gid;
    uint32_t   st_rdev;
    long       st_pad1[2];
    irix_off_t st_size;
    long       st_pad2;
    struct {
        long sec; /* time_t */
        long nsec;
    } st_atim, st_mtim, st_ctim;
    long       st_blksize;
    irix_off_t st_blocks;
    char       st_fstype[16];
    long       st_projid;
    long       st_pad[7];
};

int
__irix_xstat(long call, long arg, struct stat *st)
{
    struct irix_stat s;
    if (__irix_call(call, IRIX_STAT_VER, arg, &s) < 0)
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
    return __irix_xstat(__IRIX_SYS_xstat, (long)path, st);
}

int
lstat(const char * restrict path, struct stat * restrict st)
{
    return __irix_xstat(__IRIX_SYS_lxstat, (long)path, st);
}

int
fstat(int fd, struct stat *st)
{
    return __irix_xstat(__IRIX_SYS_fxstat, fd, st);
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
