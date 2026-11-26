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

/* The Windows C library's directory functions (direct.h). */

#include "local-win32.h"
#include "include/direct.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* The current directory. With buf NULL, the name goes in a block from
 * malloc of at least size bytes. */
char *
_getcwd(char *buf, int size)
{
    if (size < 0) {
        errno = EINVAL;
        return NULL;
    }
    if (buf)
        return getcwd(buf, (size_t)size);
    char *p = getcwd(NULL, 0);
    if (p && strlen(p) + 1 < (size_t)size) {
        char *q = realloc(p, (size_t)size);
        if (q)
            p = q;
    }
    return p;
}

/* The current directory of a drive: 1 is A:, 0 the current drive. */
char *
_getdcwd(int drive, char *buf, int size)
{
    if (drive == 0)
        return _getcwd(buf, size);
    if (drive < 1 || drive > 26) {
        errno = EACCES;
        return NULL;
    }
    char          tmp[PATH_MAX];
    const char    path[3] = { (char)('A' + drive - 1), ':', '\0' };
    /* "X:" is the current directory of drive X. */
    __win32_dword n = GetFullPathNameA(path, sizeof(tmp), tmp, NULL);
    if (n == 0 || n >= sizeof(tmp)) {
        __win32_error(0);
        return NULL;
    }
    if (!buf) {
        size_t len = (size_t)size > n ? (size_t)size : n + 1;
        buf = malloc(len);
        if (!buf) {
            errno = ENOMEM;
            return NULL;
        }
    } else if (n >= (size_t)size) {
        errno = ERANGE;
        return NULL;
    }
    memcpy(buf, tmp, n + 1);
    return buf;
}

/* Makes a directory. */
int
_mkdir(const char *path)
{
    return mkdir(path, 0777);
}

/* The current drive: 1 for A:, 2 for B: and so on; 0 for a UNC path. */
int
_getdrive(void)
{
    char          buf[PATH_MAX];
    __win32_dword n = GetCurrentDirectoryA(sizeof(buf), buf);
    if (n == 0 || n >= sizeof(buf) || buf[1] != ':')
        return 0;
    int c = buf[0] | 0x20;
    return c >= 'a' && c <= 'z' ? c - 'a' + 1 : 0;
}

/* Makes a drive the current drive: 1 for A:. 0, or -1 on error. */
int
_chdrive(int drive)
{
    if (drive < 1 || drive > 26) {
        errno = EACCES;
        return -1;
    }
    const char path[3] = { (char)('A' + drive - 1), ':', '\0' };
    if (!SetCurrentDirectoryA(path))
        return __win32_error(0);
    return 0;
}

/* A bit for each drive that exists: bit 0 is A:. */
unsigned long
_getdrives(void)
{
    return GetLogicalDrives();
}

/* The space on a drive (1 is A:, 0 the current drive). 0, or the Win32
 * error code. */
unsigned
_getdiskfree(unsigned drive, struct _diskfree_t *out)
{
    char          root[4] = { '\0', ':', '\\', '\0' };
    __win32_dword spc, bps, free_clusters, total;
    if (drive > 26)
        return 15; /* ERROR_INVALID_DRIVE */
    root[0] = (char)('A' + drive - 1);
    if (!GetDiskFreeSpaceA(drive ? root : NULL, &spc, &bps, &free_clusters, &total)) {
        __win32_dword e = GetLastError();
        __win32_error(e);
        return (unsigned)e;
    }
    out->total_clusters = total;
    out->avail_clusters = free_clusters;
    out->sectors_per_cluster = spc;
    out->bytes_per_sector = bps;
    return 0;
}

/* The current directory as a wide name. With buf NULL, the name goes in
 * a block from malloc of at least size characters. */
wchar_t *
_wgetcwd(wchar_t *buf, int size)
{
    char tmp[PATH_MAX];
    if (!getcwd(tmp, sizeof(tmp)))
        return NULL;
    int n = MultiByteToWideChar(__WIN32_CP_ACP, 0, tmp, -1, NULL, 0);
    if (n <= 0) {
        __win32_error(0);
        return NULL;
    }
    if (!buf) {
        int len = size > n ? size : n;
        buf = malloc((size_t)len * sizeof(wchar_t));
        if (!buf) {
            errno = ENOMEM;
            return NULL;
        }
        size = len;
    } else if (n > size) {
        errno = ERANGE;
        return NULL;
    }
    MultiByteToWideChar(__WIN32_CP_ACP, 0, tmp, -1, buf, size);
    return buf;
}

/* Changes the current directory by wide name, through the ANSI code
 * page. */
int
_wchdir(const wchar_t *path)
{
    char name[PATH_MAX];
    if (!__win32_ansi_name(path, name))
        return -1;
    return chdir(name);
}

/* Makes a directory by wide name, through the ANSI code page. */
int
_wmkdir(const wchar_t *path)
{
    char name[PATH_MAX];
    if (!__win32_ansi_name(path, name))
        return -1;
    return mkdir(name, 0777);
}

/* Removes a directory by wide name, through the ANSI code page. */
int
_wrmdir(const wchar_t *path)
{
    char name[PATH_MAX];
    if (!__win32_ansi_name(path, name))
        return -1;
    return rmdir(name);
}
