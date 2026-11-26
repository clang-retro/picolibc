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

/* _findfirst, _findnext and _findclose (io.h), on FindFirstFileA. */

#include "local-win32.h"
#include "include/io.h"
#include <limits.h>
#include <string.h>

/* A FILETIME as seconds since 1970, or -1 when it is not set. */
static long long
seconds(const struct __win32_filetime *ft)
{
    if (!ft->low && !ft->high)
        return -1;
    return (__win32_filetime(ft) - __WIN32_EPOCH_DIFF) / 10000000LL;
}

/* Copies what FindFirstFileA found to a struct _finddatai64_t. */
static void
to_data64(const struct __win32_find_data *fd, struct _finddatai64_t *out)
{
    out->attrib = (unsigned)fd->attributes & 0x3f;
    out->time_create = seconds(&fd->creation);
    out->time_access = seconds(&fd->access);
    out->time_write = seconds(&fd->write);
    out->size = (long long)(((unsigned long long)fd->size_high << 32) | fd->size_low);
    strncpy(out->name, fd->name, sizeof(out->name) - 1);
    out->name[sizeof(out->name) - 1] = '\0';
}

/* Copies what FindFirstFileA found to a struct _finddata_t. */
static void
to_data(const struct __win32_find_data *fd, struct _finddata_t *out)
{
    struct _finddatai64_t d;
    to_data64(fd, &d);
    out->attrib = d.attrib;
    out->time_create = d.time_create;
    out->time_access = d.time_access;
    out->time_write = d.time_write;
    out->size = (_fsize_t)d.size;
    memcpy(out->name, d.name, sizeof(out->name));
}

/* Starts a search. The handle, or -1 with errno ENOENT when no file
 * matches. */
static intptr_t
first(const char *pattern, struct __win32_find_data *fd)
{
    __win32_handle h = FindFirstFileA(pattern, fd);
    if (h == __WIN32_INVALID_HANDLE) {
        __win32_dword e = GetLastError();
        if (e == __WIN32_ERROR_FILE_NOT_FOUND || e == __WIN32_ERROR_NO_MORE_FILES)
            errno = ENOENT;
        else
            __win32_error(e);
        return -1;
    }
    return (intptr_t)h;
}

/* Finds the next file of a search. 0, or -1 with errno ENOENT at the end. */
static int
next(intptr_t handle, struct __win32_find_data *fd)
{
    if (handle == -1) {
        errno = EINVAL;
        return -1;
    }
    if (!FindNextFileA((__win32_handle)handle, fd)) {
        __win32_dword e = GetLastError();
        if (e == __WIN32_ERROR_NO_MORE_FILES)
            errno = ENOENT;
        else
            __win32_error(e);
        return -1;
    }
    return 0;
}

/* Finds the first file that matches the pattern. */
intptr_t
_findfirst(const char *pattern, struct _finddata_t *data)
{
    struct __win32_find_data fd;
    intptr_t                 h = first(pattern, &fd);
    if (h != -1)
        to_data(&fd, data);
    return h;
}

/* Finds the next file that matches the pattern of _findfirst. */
int
_findnext(intptr_t handle, struct _finddata_t *data)
{
    struct __win32_find_data fd;
    if (next(handle, &fd))
        return -1;
    to_data(&fd, data);
    return 0;
}

/* As _findfirst, with a 64-bit size. */
intptr_t
_findfirsti64(const char *pattern, struct _finddatai64_t *data)
{
    struct __win32_find_data fd;
    intptr_t                 h = first(pattern, &fd);
    if (h != -1)
        to_data64(&fd, data);
    return h;
}

/* As _findnext, with a 64-bit size. */
int
_findnexti64(intptr_t handle, struct _finddatai64_t *data)
{
    struct __win32_find_data fd;
    if (next(handle, &fd))
        return -1;
    to_data64(&fd, data);
    return 0;
}

/* Copies what FindFirstFileA found to a struct _wfinddata_t. */
static void
to_wdata(const struct __win32_find_data *fd, struct _wfinddata_t *out)
{
    struct _finddatai64_t d;
    to_data64(fd, &d);
    out->attrib = d.attrib;
    out->time_create = d.time_create;
    out->time_access = d.time_access;
    out->time_write = d.time_write;
    out->size = (_fsize_t)d.size;
    if (!MultiByteToWideChar(__WIN32_CP_ACP, 0, d.name, -1, out->name, 260))
        out->name[0] = 0;
}

/* As _findfirst, with wide names. The names go through the ANSI code
 * page. */
intptr_t
_wfindfirst(const wchar_t *pattern, struct _wfinddata_t *data)
{
    char                     name[PATH_MAX];
    struct __win32_find_data fd;
    if (!__win32_ansi_name(pattern, name))
        return -1;
    intptr_t h = first(name, &fd);
    if (h != -1)
        to_wdata(&fd, data);
    return h;
}

/* As _findnext, with wide names. */
int
_wfindnext(intptr_t handle, struct _wfinddata_t *data)
{
    struct __win32_find_data fd;
    if (next(handle, &fd))
        return -1;
    to_wdata(&fd, data);
    return 0;
}

/* Ends a search. 0, or -1 on error. */
int
_findclose(intptr_t handle)
{
    if (handle == -1 || !FindClose((__win32_handle)handle)) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}
