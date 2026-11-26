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

#include "local-sco.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * The old (SVR3) struct stat of the stat and fstat calls, which both
 * UnixWare and OpenServer have: 16-bit device, inode and ids.
 */
struct sco_stat {
    int16_t  st_dev;
    uint16_t st_ino;
    uint16_t st_mode;
    int16_t  st_nlink;
    uint16_t st_uid;
    uint16_t st_gid;
    int16_t  st_rdev;
    int32_t  st_size;
    int32_t  s_atime;
    int32_t  s_mtime;
    int32_t  s_ctime;
};

_Static_assert(sizeof(struct sco_stat) == 32, "struct stat is 32 bytes");

static int
sco_stat(long call, long arg, struct stat *st)
{
    struct sco_stat s;
    if (__sco_call(call, arg, &s, 0) < 0)
        return -1;
    memset(st, 0, sizeof(*st));
    st->st_dev = (uint16_t)s.st_dev;
    st->st_ino = s.st_ino;
    st->st_mode = s.st_mode;
    st->st_nlink = (uint16_t)s.st_nlink;
    st->st_uid = s.st_uid;
    st->st_gid = s.st_gid;
    st->st_rdev = (uint16_t)s.st_rdev;
    st->st_size = s.st_size;
    st->st_atime = (time_t)(uint32_t)s.s_atime;
    st->st_mtime = (time_t)(uint32_t)s.s_mtime;
    st->st_ctime = (time_t)(uint32_t)s.s_ctime;
    st->st_blksize = 512;
    st->st_blocks = (s.st_size + 511) / 512;
    return 0;
}

int
stat(const char * restrict path, struct stat * restrict st)
{
    return sco_stat(__SCO_SYS_stat, (long)path, st);
}

/* The two systems number lstat differently; it is stat here. */
int
lstat(const char * restrict path, struct stat * restrict st)
{
    return stat(path, st);
}

int
fstat(int fd, struct stat *st)
{
    return sco_stat(__SCO_SYS_fstat, fd, st);
}

int
isatty(int fd)
{
    char termio[32];
    if (__sco_call(__SCO_SYS_ioctl, fd, __SCO_TCGETA, termio) < 0) {
        errno = ENOTTY;
        return 0;
    }
    return 1;
}
