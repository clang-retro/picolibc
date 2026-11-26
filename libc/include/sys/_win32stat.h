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

/* The Windows C library's names for <sys/stat.h>: struct _stat and
 * _stat, _fstat and _wstat, with their 64-bit forms, and the _S_ mode
 * bits. sys/stat.h includes this header for _WIN32 targets. Files have
 * 64-bit sizes and times here, so the 64-bit forms are the same. */

#ifndef _SYS__WIN32STAT_H_
#define _SYS__WIN32STAT_H_

#ifndef __win32_wchar
#ifdef __cplusplus
#define __win32_wchar wchar_t
#else
#define __win32_wchar __WCHAR_TYPE__
#endif
#endif

_BEGIN_STD_C

#define _S_IFMT   S_IFMT
#define _S_IFDIR  S_IFDIR
#define _S_IFCHR  S_IFCHR
#define _S_IFIFO  S_IFIFO
#define _S_IFREG  S_IFREG
#define _S_IREAD  S_IRUSR
#define _S_IWRITE S_IWUSR
#define _S_IEXEC  S_IXUSR

/* The members that the Windows C library's struct _stat has. The times
 * are timespecs, so st_atime, st_mtime and st_ctime work as for struct
 * stat. */
#define __WIN32_STAT_MEMBERS  \
    dev_t           st_dev;   \
    ino_t           st_ino;   \
    unsigned short  st_mode;  \
    short           st_nlink; \
    short           st_uid;   \
    short           st_gid;   \
    dev_t           st_rdev;  \
    off_t           st_size;  \
    struct timespec st_atim;  \
    struct timespec st_mtim;  \
    struct timespec st_ctim

struct _stat {
    __WIN32_STAT_MEMBERS;
};
struct _stati64 {
    __WIN32_STAT_MEMBERS;
};
struct _stat64 {
    __WIN32_STAT_MEMBERS;
};

int _stat(const char *, struct _stat *) __picolibc_export;
int _fstat(int, struct _stat *) __picolibc_export;
int _wstat(const __win32_wchar *, struct _stat *) __picolibc_export;
int _stati64(const char *, struct _stati64 *) __picolibc_export;
int _fstati64(int, struct _stati64 *) __picolibc_export;
int _wstati64(const __win32_wchar *, struct _stati64 *) __picolibc_export;
int _stat64(const char *, struct _stat64 *) __picolibc_export;
int _fstat64(int, struct _stat64 *) __picolibc_export;
int _wstat64(const __win32_wchar *, struct _stat64 *) __picolibc_export;
int _chmod(const char *, int) __picolibc_export;
int _wchmod(const __win32_wchar *, int) __picolibc_export;
int _umask(int) __picolibc_export;

_END_STD_C

#endif /* _SYS__WIN32STAT_H_ */
