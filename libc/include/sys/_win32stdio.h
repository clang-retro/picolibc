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

/* The Windows C library's own <stdio.h> functions. stdio.h includes this
 * header for _WIN32 targets. */

#ifndef _SYS__WIN32STDIO_H_
#define _SYS__WIN32STDIO_H_

#ifndef __win32_wchar
#ifdef __cplusplus
#define __win32_wchar wchar_t
#else
#define __win32_wchar __WCHAR_TYPE__
#endif
#endif

_BEGIN_STD_C

/* As snprintf, with the Windows rules: when the text does not fit, the
 * buffer gets no terminating NUL and the result is -1. */
int _snprintf(char * __restrict, size_t, const char * __restrict, ...) __picolibc_export;
int _vsnprintf(char * __restrict, size_t, const char * __restrict, __gnuc_va_list)
    __picolibc_export;
/* The number of characters that the format makes, without the NUL. */
int _scprintf(const char * __restrict, ...) __picolibc_export;
int _vscprintf(const char * __restrict, __gnuc_va_list) __picolibc_export;
/* As _snprintf, for wide strings. */
int _snwprintf(__win32_wchar * __restrict, size_t, const __win32_wchar * __restrict, ...)
    __picolibc_export;
int   _vsnwprintf(__win32_wchar   *__restrict, size_t, const __win32_wchar   *__restrict,
                  __gnuc_va_list) __picolibc_export;

int   _fileno(FILE *) __picolibc_export;
FILE *_fdopen(int, const char *) __picolibc_export;
/* Flushes stdout and stderr, the streams that the library can find; 0. */
int   _flushall(void) __picolibc_export;
int   _unlink(const char *) __picolibc_export;
int   _getw(FILE *) __picolibc_export;
int   _putw(int, FILE *) __picolibc_export;
/* Opens a file by its wide name with a wide mode, as _wfopen, again. */
FILE *_wfreopen(const __win32_wchar *, const __win32_wchar *, FILE *) __picolibc_export;
int   _wremove(const __win32_wchar *) __picolibc_export;
int   _wrename(const __win32_wchar *, const __win32_wchar *) __picolibc_export;

_END_STD_C

#endif /* _SYS__WIN32STDIO_H_ */
