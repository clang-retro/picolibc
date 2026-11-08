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

#include "local-os2.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

/* A directory is read with DosFindFirst and DosFindNext, one entry at a
 * time. The DIR keeps the search handle (in fd), the find buffer (in buf)
 * and, after itself, the search pattern for rewinddir. d->count is 1 while
 * an entry is waiting in the buffer. */

#define DIR_FIND(d)    ((struct __os2_findbuf *)(d)->buf)
#define DIR_PATTERN(d) ((char *)((d) + 1))
#define NO_HANDLE      ((__os2_handle)-1) /* HDIR_CREATE */

_Static_assert(sizeof(struct __os2_findbuf) <= sizeof(((DIR *)0)->buf), "find buffer");

/* Files, hidden and system files, and directories. */
#define FIND_ATTR 0x16

static __os2_ret
find_first(DIR *d)
{
    __os2_handle h = NO_HANDLE;
    __os2_word   n = 1;
    __os2_ret    r = __os2_findfirst(DIR_PATTERN(d), &h, FIND_ATTR, DIR_FIND(d), &n);
    d->fd = (int)h;
    d->count = r == 0 && n == 1;
    /* No files is an empty directory, such as an empty FAT root. */
    if (r == 18 /* ERROR_NO_MORE_FILES */ || r == 2 /* ERROR_FILE_NOT_FOUND */)
        r = 0;
    return r;
}

static void
find_close(DIR *d)
{
    if (d->fd != (int)NO_HANDLE) {
        __os2_findclose((__os2_handle)d->fd);
        d->fd = (int)NO_HANDLE;
    }
}

/* The attributes of a path, or -1 if it has none. */
static long
attributes(const char *path)
{
    struct __os2_filestatus st;
    if (__os2_pathinfo(path, &st))
        return -1;
    return (long)st.attr;
}

/* "X:\", "X:/", "\" or "/", which have no directory entry of their own. */
static int
is_root(const char *p)
{
    if (p[0] && p[1] == ':')
        p += 2;
    return (p[0] == '\\' || p[0] == '/') && p[1] == '\0';
}

DIR *
opendir(const char *path)
{
    size_t n = strlen(path);
    if (n == 0) {
        errno = ENOENT;
        return NULL;
    }
    DIR *d = malloc(sizeof(DIR) + n + 3);
    if (d == NULL)
        return NULL;
    d->offset = 0;
    char *pat = DIR_PATTERN(d);
    memcpy(pat, path, n);
    if (pat[n - 1] != '/' && pat[n - 1] != '\\' && pat[n - 1] != ':')
        pat[n++] = '\\';
    memcpy(pat + n, "*", 2);
    __os2_ret err = find_first(d);
    if (err == 0 && d->count == 0 && !is_root(path)) {
        /* No entries at all: make sure that the path is a directory. */
        long a = attributes(path);
        if (a < 0 || !(a & __OS2_FILE_DIRECTORY))
            err = 3;
    }
    if (err) {
        find_close(d);
        free(d);
        if (err == 2 || err == 3) {
            /* A path that names a file is not a directory. */
            long a = attributes(path);
            errno = (a >= 0 && !(a & __OS2_FILE_DIRECTORY)) ? ENOTDIR : ENOENT;
        } else {
            __os2_error(err);
        }
        return NULL;
    }
    return d;
}

struct dirent *
readdir(DIR *d)
{
    if (d->count == 0)
        return NULL;
    struct __os2_findbuf *f = DIR_FIND(d);
    struct dirent        *e = &d->dirent;
    size_t                len = f->namelen;
    if (len > sizeof(e->d_name) - 1)
        len = sizeof(e->d_name) - 1;
    e->d_ino = 0;
    e->d_type = (f->attr & __OS2_FILE_DIRECTORY) ? DT_DIR : DT_REG;
    memcpy(e->d_name, f->name, len);
    e->d_name[len] = '\0';
    d->offset++;
    /* Find the next entry now; the buffer is ours until then. */
    __os2_word n = 1;
    d->count = __os2_findnext((__os2_handle)d->fd, f, &n) == 0 && n == 1;
    return e;
}

void
rewinddir(DIR *d)
{
    find_close(d);
    d->offset = 0;
    find_first(d);
}

int
closedir(DIR *d)
{
    find_close(d);
    free(d);
    return 0;
}

int
dirfd(DIR *d)
{
    (void)d;
    errno = ENOTSUP;
    return -1;
}

DIR *
fdopendir(int fd)
{
    (void)fd;
    errno = ENOSYS;
    return NULL;
}
