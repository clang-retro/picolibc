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

#include "local-aix.h"
#include <string.h>
#include <sys/stat.h>

void
__aix_stat_convert(const struct __aix_stat *a, struct stat *st)
{
    memset(st, 0, sizeof(*st));
    st->st_dev = a->st_dev;
    st->st_ino = a->st_ino;
    st->st_mode = a->st_mode;
    st->st_nlink = (uint16_t)a->st_nlink;
    st->st_uid = a->st_uid;
    st->st_gid = a->st_gid;
    st->st_rdev = a->st_rdev;
    st->st_size = a->st_size;
    st->st_atim.tv_sec = (time_t)(uint32_t)a->st_atime_sec;
    st->st_atim.tv_nsec = a->st_atime_nsec;
    st->st_mtim.tv_sec = (time_t)(uint32_t)a->st_mtime_sec;
    st->st_mtim.tv_nsec = a->st_mtime_nsec;
    st->st_ctim.tv_sec = (time_t)(uint32_t)a->st_ctime_sec;
    st->st_ctim.tv_nsec = a->st_ctime_nsec;
    st->st_blksize = a->st_blksize;
    st->st_blocks = a->st_blocks;
}

int
stat(const char * restrict path, struct stat * restrict st)
{
    struct __aix_stat a;
    if (__aix_call(statx(path, &a, sizeof(a), __AIX_STX_NORMAL)) < 0)
        return -1;
    __aix_stat_convert(&a, st);
    return 0;
}
