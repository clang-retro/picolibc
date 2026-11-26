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

/* The Windows C library's own string functions, which <string.h> declares
 * there. string.h includes this header for _WIN32 targets. The names
 * without the underscore (stricmp, strlwr, ...) are the old names that
 * Microsoft's oldnames.lib gives; they are declared unless the program
 * asks for strict ISO C. */

#ifndef _SYS__WIN32STRING_H_
#define _SYS__WIN32STRING_H_

_BEGIN_STD_C

int _stricmp(const char *, const char *) __pure __picolibc_export;
int
_strnicmp(const char *, const char *, size_t)
__pure __picolibc_export;
int _strcmpi(const char *, const char *) __pure __picolibc_export;
int _stricoll(const char *, const char *) __pure __picolibc_export;
int
_strnicoll(const char *, const char *, size_t)
__pure __picolibc_export;
int
_memicmp(const void *, const void *, size_t)
__pure __picolibc_export;
void *_memccpy(void * __restrict, const void * __restrict, int, size_t) __picolibc_export;
char *_strdup(const char *) __malloc_like __warn_unused_result __picolibc_export;
char *_strlwr(char *) __picolibc_export;
char *_strupr(char *) __picolibc_export;
char *_strrev(char *) __picolibc_export;
char *_strset(char *, int) __picolibc_export;
char *_strnset(char *, int, size_t) __picolibc_export;
char *_strerror(const char *) __picolibc_export;

#if __MISC_VISIBLE
int stricmp(const char *, const char *) __pure __picolibc_export;
int
strnicmp(const char *, const char *, size_t)
__pure __picolibc_export;
int strcmpi(const char *, const char *) __pure __picolibc_export;
int
memicmp(const void *, const void *, size_t)
__pure __picolibc_export;
char *strlwr(char *) __picolibc_export;
char *strupr(char *) __picolibc_export;
char *strrev(char *) __picolibc_export;
char *strset(char *, int) __picolibc_export;
char *strnset(char *, int, size_t) __picolibc_export;
#endif

_END_STD_C

#endif /* _SYS__WIN32STRING_H_ */
