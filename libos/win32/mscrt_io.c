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

/* The Windows C library's low-level I/O functions (io.h) that are not
 * other names of the POSIX functions. */

#include "local-win32.h"
#include "include/io.h"
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Moves the file position, with a long offset. The new position, or -1
 * (also when it does not fit in a long). */
long
_lseek(int fd, long offset, int whence)
{
    off_t r = lseek(fd, offset, whence);
    if (r > LONG_MAX) {
        errno = EINVAL;
        return -1;
    }
    return (long)r;
}

/* The file position, or -1. */
long
_tell(int fd)
{
    return _lseek(fd, 0, SEEK_CUR);
}

/* The file position with 64 bits, or -1. */
long long
_telli64(int fd)
{
    return lseek(fd, 0, SEEK_CUR);
}

/* The length of an open file with 64 bits, or -1. The file position does
 * not change. */
long long
_filelengthi64(int fd)
{
    off_t here = lseek(fd, 0, SEEK_CUR);
    if (here < 0)
        return -1;
    off_t end = lseek(fd, 0, SEEK_END);
    lseek(fd, here, SEEK_SET);
    return end;
}

/* The length of an open file, or -1. */
long
_filelength(int fd)
{
    long long n = _filelengthi64(fd);
    if (n > LONG_MAX) {
        errno = EINVAL;
        return -1;
    }
    return (long)n;
}

/* Sets the length of an open file: it is cut, or made longer with zero
 * bytes. 0, or -1 on error. */
int
_chsize(int fd, long size)
{
    return ftruncate(fd, size);
}

/* As _chsize, with 64 bits. 0, or the errno value on error. */
int
_chsize_s(int fd, long long size)
{
    if (size < 0) {
        errno = EINVAL;
        return EINVAL;
    }
    return ftruncate(fd, size) ? errno : 0;
}

/* 1 when the position is at the end of the file, 0 before it, -1 on
 * error. */
int
_eof(int fd)
{
    off_t here = lseek(fd, 0, SEEK_CUR);
    if (here < 0)
        return -1;
    off_t end = lseek(fd, 0, SEEK_END);
    lseek(fd, here, SEEK_SET);
    if (end < 0)
        return -1;
    return here >= end;
}

/* Writes the data of the file to the disk. */
int
_commit(int fd)
{
    return fsync(fd);
}

/* Makes a file, or makes an existing file empty, open for writing. */
int
_creat(const char *path, int mode)
{
    return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

/* As access, by wide name through the ANSI code page. */
int
_waccess(const wchar_t *path, int mode)
{
    char name[PATH_MAX];
    if (!__win32_ansi_name(path, name))
        return -1;
    return access(name, mode);
}

/* Removes a file by wide name, through the ANSI code page. */
int
_wunlink(const wchar_t *path)
{
    char name[PATH_MAX];
    if (!__win32_ansi_name(path, name))
        return -1;
    return unlink(name);
}

/* Locks or unlocks n bytes from the file position. _LK_LOCK and
 * _LK_RLCK try 10 times, one second apart; _LK_NBLCK and _LK_NBRLCK try
 * once. Windows has no read locks, so the read locks are write locks. */
int
_locking(int fd, int mode, long n)
{
    __win32_handle h = __win32_handle_of(fd);
    if (!h)
        return -1;
    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos < 0 || n < 0) {
        errno = EINVAL;
        return -1;
    }
    __win32_dword low = (__win32_dword)(pos & 0xffffffff);
    __win32_dword high = (__win32_dword)(pos >> 32);
    switch (mode) {
    case _LK_UNLCK:
        if (!UnlockFile(h, low, high, (__win32_dword)n, 0)) {
            errno = EACCES;
            return -1;
        }
        return 0;
    case _LK_LOCK:
    case _LK_RLCK:
        for (int i = 0; i < 10; i++) {
            if (LockFile(h, low, high, (__win32_dword)n, 0))
                return 0;
            Sleep(1000);
        }
        errno = EDEADLK;
        return -1;
    case _LK_NBLCK:
    case _LK_NBRLCK:
        if (LockFile(h, low, high, (__win32_dword)n, 0))
            return 0;
        errno = EACCES;
        return -1;
    default:
        errno = EINVAL;
        return -1;
    }
}

/* Makes a unique name from a template that ends in XXXXXX, as the
 * Windows library does: a letter, then five digits of the process ID.
 * The template, or NULL when the template is not valid or the 26 names
 * exist. */
char *
_mktemp(char *templ)
{
    size_t n = strlen(templ);
    if (n < 6 || strcmp(templ + n - 6, "XXXXXX") != 0) {
        errno = EINVAL;
        return NULL;
    }
    char         *x = templ + n - 6;
    unsigned long pid = GetCurrentProcessId();
    for (int i = 5; i >= 1; i--) {
        x[i] = (char)('0' + pid % 10);
        pid /= 10;
    }
    for (char c = 'a'; c <= 'z'; c++) {
        x[0] = c;
        if (GetFileAttributesA(templ) == __WIN32_INVALID_FILE_ATTRIBUTES)
            return templ;
    }
    errno = EEXIST;
    return NULL;
}

/* The mask of permission bits for new files. Files on Windows have only
 * the read-only attribute, so the library keeps the mask but does not
 * use it. */
static mode_t file_mask = 022;

/* Sets the mask for new files; the old mask. */
mode_t
umask(mode_t mask)
{
    mode_t old = file_mask;
    file_mask = mask & 0777;
    return old;
}

/* As umask, with the Windows C library's int. */
int
_umask(int mask)
{
    return (int)umask((mode_t)mask);
}
