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

/* The Windows C library's own wide string functions, which <wchar.h>
 * declares there. wchar.h includes this header for _WIN32 targets. */

#ifndef _SYS__WIN32WCHAR_H_
#define _SYS__WIN32WCHAR_H_

_BEGIN_STD_C

int _wcsicmp(const wchar_t *, const wchar_t *) __pure __picolibc_export;
int
_wcsnicmp(const wchar_t *, const wchar_t *, size_t)
__pure __picolibc_export;
int      _wcsicoll(const wchar_t *, const wchar_t *) __pure __picolibc_export;
wchar_t *_wcsdup(const wchar_t *) __malloc_like __warn_unused_result __picolibc_export;
wchar_t *_wcslwr(wchar_t *) __picolibc_export;
wchar_t *_wcsupr(wchar_t *) __picolibc_export;
wchar_t *_wcsrev(wchar_t *) __picolibc_export;
wchar_t *_wcsset(wchar_t *, wchar_t) __picolibc_export;
wchar_t *_wcsnset(wchar_t *, wchar_t, size_t) __picolibc_export;

#if __MISC_VISIBLE
int wcsicmp(const wchar_t *, const wchar_t *) __pure __picolibc_export;
int
wcsnicmp(const wchar_t *, const wchar_t *, size_t)
__pure __picolibc_export;
wchar_t *wcslwr(wchar_t *) __picolibc_export;
wchar_t *wcsupr(wchar_t *) __picolibc_export;
#endif

_END_STD_C

#endif /* _SYS__WIN32WCHAR_H_ */
