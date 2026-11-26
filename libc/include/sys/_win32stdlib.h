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

/* The Windows C library's own <stdlib.h> functions and limits. stdlib.h
 * includes this header for _WIN32 targets. */

#ifndef _SYS__WIN32STDLIB_H_
#define _SYS__WIN32STDLIB_H_

#ifndef __win32_wchar
#ifdef __cplusplus
#define __win32_wchar wchar_t
#else
#define __win32_wchar __WCHAR_TYPE__
#endif
#endif

_BEGIN_STD_C

/* The sizes of a path and of its parts, with the terminating NUL. */
#define _MAX_PATH  260
#define _MAX_DRIVE 3
#define _MAX_DIR   256
#define _MAX_FNAME 256
#define _MAX_EXT   256

/* The number of elements of an array. */
#ifndef _countof
#define _countof(__a) (sizeof(__a) / sizeof((__a)[0]))
#endif

#ifndef __cplusplus
#ifndef __min
#define __min(__a, __b) (((__a) < (__b)) ? (__a) : (__b))
#endif
#ifndef __max
#define __max(__a, __b) (((__a) > (__b)) ? (__a) : (__b))
#endif
#endif

char              *_itoa(int, char *, int) __picolibc_export;
char              *_ltoa(long, char *, int) __picolibc_export;
char              *_ultoa(unsigned long, char *, int) __picolibc_export;
char              *_i64toa(long long, char *, int) __picolibc_export;
char              *_ui64toa(unsigned long long, char *, int) __picolibc_export;
long long          _atoi64(const char *) __pure __picolibc_export;
long long          _strtoi64(const char          *__restrict, char          **__restrict, int) __picolibc_export;
unsigned long long _strtoui64(const char * __restrict, char ** __restrict, int) __picolibc_export;

__win32_wchar     *_itow(int, __win32_wchar *, int) __picolibc_export;
__win32_wchar     *_ltow(long, __win32_wchar *, int) __picolibc_export;
__win32_wchar     *_ultow(unsigned long, __win32_wchar *, int) __picolibc_export;
int                _wtoi(const __win32_wchar *) __picolibc_export;
long               _wtol(const __win32_wchar *) __picolibc_export;
long long          _wtoi64(const __win32_wchar *) __picolibc_export;

/* The full path of a file, in buf (or, with buf NULL, in a block from
 * malloc); NULL on error. */
char              *_fullpath(char *, const char *, size_t) __picolibc_export;
/* The drive, directory, name and extension of a path, and the reverse. */
void               _splitpath(const char *, char *, char *, char *, char *) __picolibc_export;
void _makepath(char *, const char *, const char *, const char *, const char *) __picolibc_export;

/* Sets ("NAME=value") or removes ("NAME=") an environment variable. */
int  _putenv(const char *) __picolibc_export;

/* Waits for a number of milliseconds. */
void _sleep(unsigned long) __picolibc_export;
/* Sounds the speaker: a frequency in Hz for a number of milliseconds. */
void _beep(unsigned, unsigned) __picolibc_export;

#if __MISC_VISIBLE
char *itoa(int, char *, int) __picolibc_export;
char *ltoa(long, char *, int) __picolibc_export;
char *ultoa(unsigned long, char *, int) __picolibc_export;
#endif

_END_STD_C

#endif /* _SYS__WIN32STDLIB_H_ */
