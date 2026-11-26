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

/* The Windows C library declares setmode, the file mode flags and the
 * functions that go between descriptors and Win32 handles here. */

#ifndef _IO_H_
#define _IO_H_

#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#ifndef __win32_wchar
#ifdef __cplusplus
#define __win32_wchar wchar_t
#else
#define __win32_wchar __WCHAR_TYPE__
#endif
#endif

_BEGIN_STD_C

/* The Win32 handle of a descriptor, or -1. */
intptr_t _get_osfhandle(int fd);
/* A new descriptor for a Win32 handle; flags may hold O_TEXT, O_BINARY
 * and O_APPEND. Closing the descriptor closes the handle. */
int      _open_osfhandle(intptr_t handle, int flags);

int      _close(int fd);
int      _dup(int fd);
int      _dup2(int fd, int fd2);
int      _isatty(int fd);
/* Opens a file by its wide (UTF-16) name. */
int      _wopen(const __win32_wchar *path, int flags, ...);

int       _open(const char *path, int flags, ...);
int       _creat(const char *path, int mode);
int       _read(int fd, void *buf, unsigned int n);
int       _write(int fd, const void *buf, unsigned int n);
long      _lseek(int fd, long offset, int whence);
long long _lseeki64(int fd, long long offset, int whence);
long      _tell(int fd);
long long _telli64(int fd);
/* The length of the file, or -1. */
long      _filelength(int fd);
long long _filelengthi64(int fd);
/* Sets the length of the file. */
int       _chsize(int fd, long size);
int       _chsize_s(int fd, long long size);
/* 1 at the end of the file, 0 before it, -1 on error. */
int       _eof(int fd);
int       _commit(int fd);
int       _access(const char *path, int mode);
int       _waccess(const __win32_wchar *path, int mode);
int       _unlink(const char *path);
int       _wunlink(const __win32_wchar *path);
int       _setmode(int fd, int mode);
int       _locking(int fd, int mode, long n);
/* Makes a unique name from a template that ends in XXXXXX. */
char     *_mktemp(char *templ);

/* The attributes of a file, in _finddata_t. */
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20

/* _locking modes. */
#define _LK_UNLCK  0
#define _LK_LOCK   1
#define _LK_NBLCK  2
#define _LK_RLCK   3
#define _LK_NBRLCK 4

typedef unsigned long _fsize_t;

/* A file that _findfirst or _findnext found. The times are seconds since
 * 1970; the 64-bit forms have the same layout. */
struct _finddata_t {
    unsigned  attrib;
    long long time_create;
    long long time_access;
    long long time_write;
    _fsize_t  size;
    char      name[260];
};

struct _finddatai64_t {
    unsigned  attrib;
    long long time_create;
    long long time_access;
    long long time_write;
    long long size;
    char      name[260];
};

struct _wfinddata_t {
    unsigned      attrib;
    long long     time_create;
    long long     time_access;
    long long     time_write;
    _fsize_t      size;
    __win32_wchar name[260];
};

#define _finddata64_t _finddatai64_t
#define _findfirst64  _findfirsti64
#define _findnext64   _findnexti64

/* Finds the first file that matches a pattern (with * and ?). The
 * result is a handle for _findnext and _findclose, or -1. */
intptr_t _findfirst(const char *pattern, struct _finddata_t *data);
int      _findnext(intptr_t handle, struct _finddata_t *data);
intptr_t _findfirsti64(const char *pattern, struct _finddatai64_t *data);
int      _findnexti64(intptr_t handle, struct _finddatai64_t *data);
intptr_t _wfindfirst(const __win32_wchar *pattern, struct _wfinddata_t *data);
int      _wfindnext(intptr_t handle, struct _wfinddata_t *data);
int      _findclose(intptr_t handle);

_END_STD_C

#endif /* _IO_H_ */
