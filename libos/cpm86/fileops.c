/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 clang-retro contributors
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

#include "local-cpm86.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int
unlink(const char *path)
{
    struct __cpm_fcb fcb;
    if (__cpm_parse_name(&fcb, path, 0) < 0)
        return -1;
    if ((uint8_t)__cpm_bdos(BDOS_DELETE, __CPM_PTR(&fcb)) == 0xff) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

/* As in POSIX, rename replaces a file that has the new name. */
int
rename(const char *from, const char *to)
{
    struct __cpm_fcb fcb, tofcb, probe;
    if (__cpm_parse_name(&fcb, from, 0) < 0 || __cpm_parse_name(&tofcb, to, 0) < 0)
        return -1;
    uint8_t cur = (uint8_t)(((uint8_t)__cpm_bdos(BDOS_GET_DISK, 0) & 15) + 1);
    if ((tofcb.dr ? tofcb.dr : cur) != (fcb.dr ? fcb.dr : cur)) {
        errno = EXDEV;
        return -1;
    }
    uint8_t dma[__CPM_RECORD];
    __cpm_bdos(BDOS_SET_DMA, __CPM_PTR(dma));
    probe = fcb;
    if ((uint8_t)__cpm_bdos(BDOS_SEARCH_FIRST, __CPM_PTR(&probe)) == 0xff) {
        errno = ENOENT;
        return -1;
    }
    if (memcmp(fcb.name, tofcb.name, 11) == 0)
        return 0;
    probe = tofcb;
    probe.dr = fcb.dr;
    __cpm_bdos(BDOS_DELETE, __CPM_PTR(&probe));
    /* The new name goes in the second half of the FCB, at offset 16. */
    uint8_t *p = (uint8_t *)&fcb;
    p[16] = 0;
    memcpy(p + 17, tofcb.name, 11);
    if ((uint8_t)__cpm_bdos(BDOS_RENAME, __CPM_PTR(&fcb)) == 0xff) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

static void
fill_stat(struct stat *st, int32_t records, int readonly)
{
    memset(st, 0, sizeof(*st));
    st->st_mode = S_IFREG | (readonly ? 0444 : 0666);
    st->st_nlink = 1;
    st->st_size = (off_t)records * __CPM_RECORD;
    st->st_blksize = __CPM_RECORD;
    st->st_blocks = (st->st_size + 511) / 512;
}

int
stat(const char *path, struct stat *st)
{
    if (__cpm_device_name(path) != __CPM_DEV_NONE) {
        memset(st, 0, sizeof(*st));
        st->st_mode = S_IFCHR | 0666;
        return 0;
    }
    struct __cpm_fcb fcb;
    if (__cpm_parse_name(&fcb, path, 0) < 0)
        return -1;
    struct __cpm_fcb probe = fcb;
    uint8_t          dma[__CPM_RECORD];
    __cpm_bdos(BDOS_SET_DMA, __CPM_PTR(dma));
    uint8_t i = (uint8_t)__cpm_bdos(BDOS_SEARCH_FIRST, __CPM_PTR(&probe));
    if (i == 0xff) {
        errno = ENOENT;
        return -1;
    }
    int readonly = dma[32 * i + 9] & 0x80;
    __cpm_bdos(BDOS_FILE_SIZE, __CPM_PTR(&fcb));
    fill_stat(st, fcb.r[0] | ((int32_t)fcb.r[1] << 8) | ((int32_t)fcb.r[2] << 16), readonly);
    return 0;
}

int
lstat(const char *path, struct stat *st)
{
    return stat(path, st);
}

int
fstat(int fd, struct stat *st)
{
    struct __cpm_file *f = __cpm_fd(fd);
    if (f == NULL)
        return -1;
    if (f->device != __CPM_DEV_NONE) {
        memset(st, 0, sizeof(*st));
        st->st_mode = S_IFCHR | 0666;
        return 0;
    }
    fill_stat(st, (int32_t)((f->size + __CPM_RECORD - 1) / __CPM_RECORD), 0);
    st->st_size = f->size;
    return 0;
}

int
access(const char *path, int mode)
{
    struct stat st;
    if (stat(path, &st) < 0)
        return -1;
    if ((mode & W_OK) && !(st.st_mode & 0222)) {
        errno = EACCES;
        return -1;
    }
    return 0;
}

int
setmode(int fd, int mode)
{
    struct __cpm_file *f = __cpm_fd(fd);
    if (f == NULL)
        return -1;
    if (mode != O_TEXT && mode != O_BINARY) {
        errno = EINVAL;
        return -1;
    }
    int old = (f->flags & __CPM_FD_TEXT) ? O_TEXT : O_BINARY;
    if (mode == O_TEXT)
        f->flags |= __CPM_FD_TEXT;
    else
        f->flags &= ~__CPM_FD_TEXT;
    return old;
}

int
_setmode(int fd, int mode)
{
    return setmode(fd, mode);
}

/* CP/M has drives but no directories. The current directory is the
 * current drive: chdir("b:") selects drive B, and getcwd gives "B:". */
int
chdir(const char *path)
{
    char d = path[0];
    if (d >= 'a' && d <= 'p')
        d -= 'a' - 'A';
    if (d < 'A' || d > 'P' || path[1] != ':'
        || (path[2] != '\0' && strcmp(path + 2, "/") != 0 && strcmp(path + 2, "\\") != 0)) {
        errno = ENOENT;
        return -1;
    }
    __cpm_bdos(BDOS_SELECT_DISK, (uint8_t)(d - 'A'));
    return 0;
}

char *
getcwd(char *buf, size_t size)
{
    static char cwd[3];
    if (buf == NULL) {
        buf = cwd;
        size = sizeof(cwd);
    }
    if (size < 3) {
        errno = ERANGE;
        return NULL;
    }
    buf[0] = (char)('A' + ((uint8_t)__cpm_bdos(BDOS_GET_DISK, 0) & 15));
    buf[1] = ':';
    buf[2] = '\0';
    return buf;
}

int
mkdir(const char *path, mode_t mode)
{
    (void)path;
    (void)mode;
    errno = ENOSYS;
    return -1;
}

int
rmdir(const char *path)
{
    (void)path;
    errno = ENOTDIR;
    return -1;
}
