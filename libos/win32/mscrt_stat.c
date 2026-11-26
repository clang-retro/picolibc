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

/* The Windows C library's _stat functions (sys/_win32stat.h), on stat
 * and fstat. */

#include "local-win32.h"
#include <limits.h>
#include <sys/stat.h>

/* Copies a struct stat to the Windows C library's struct _stat. */
static int
to_win32(int r, const struct stat *st, struct _stat *out)
{
    if (r != 0)
        return r;
    out->st_dev = st->st_dev;
    out->st_ino = st->st_ino;
    out->st_mode = (unsigned short)st->st_mode;
    out->st_nlink = (short)st->st_nlink;
    out->st_uid = (short)st->st_uid;
    out->st_gid = (short)st->st_gid;
    out->st_rdev = st->st_rdev;
    out->st_size = st->st_size;
    out->st_atim = st->st_atim;
    out->st_mtim = st->st_mtim;
    out->st_ctim = st->st_ctim;
    return 0;
}

/* Gets the status of a file by name. */
int
_stat(const char *path, struct _stat *out)
{
    struct stat st;
    return to_win32(stat(path, &st), &st, out);
}

/* Gets the status of an open file. */
int
_fstat(int fd, struct _stat *out)
{
    struct stat st;
    return to_win32(fstat(fd, &st), &st, out);
}

/* Gets the status of a file by wide name, through the ANSI code page. */
int
_wstat(const wchar_t *path, struct _stat *out)
{
    char name[PATH_MAX];
    if (!__win32_ansi_name(path, name))
        return -1;
    return _stat(name, out);
}

/* Sets the permissions of a file by wide name: only _S_IWRITE has an
 * effect, as for chmod. */
int
_wchmod(const wchar_t *path, int mode)
{
    char name[PATH_MAX];
    if (!__win32_ansi_name(path, name))
        return -1;
    return chmod(name, (mode_t)mode);
}

/* Sets the permissions of a file: only _S_IWRITE has an effect, as for
 * chmod. */
int
_chmod(const char *path, int mode)
{
    return chmod(path, (mode_t)mode);
}

/* As _stat. The structure has the same members. */
int
_stati64(const char *path, struct _stati64 *out)
{
    return _stat(path, (struct _stat *)out);
}

/* As _fstat. */
int
_fstati64(int fd, struct _stati64 *out)
{
    return _fstat(fd, (struct _stat *)out);
}

/* As _wstat. */
int
_wstati64(const wchar_t *path, struct _stati64 *out)
{
    return _wstat(path, (struct _stat *)out);
}

/* As _stat. */
int
_stat64(const char *path, struct _stat64 *out)
{
    return _stat(path, (struct _stat *)out);
}

/* As _fstat. */
int
_fstat64(int fd, struct _stat64 *out)
{
    return _fstat(fd, (struct _stat *)out);
}

/* As _wstat. */
int
_wstat64(const wchar_t *path, struct _stat64 *out)
{
    return _wstat(path, (struct _stat *)out);
}
