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

/*
 * clang on AIX calls these string and memory functions by the names of
 * the "millicode" entry points AIX's libc provides (___memset and so on).
 * no_builtin keeps the calls below from being turned back into the same
 * entry points.
 */

#define _DEFAULT_SOURCE
#include <stddef.h>
#include <string.h>

void  *___memccpy(void *dst, const void *src, int c, size_t n);
int    ___memcmp(const void *a, const void *b, size_t n);
void  *___memmove(void *dst, const void *src, size_t n);
void  *___memset(void *dst, int c, size_t n);
void   ___bzero(void *dst, size_t n);
int    ___strcmp(const char *a, const char *b);
char  *___strcpy(char *dst, const char *src);
size_t ___strlen(const char *s);
char  *___strstr(const char *s, const char *find);

__attribute__((no_builtin)) void *
___memccpy(void *dst, const void *src, int c, size_t n)
{
    return memccpy(dst, src, c, n);
}

__attribute__((no_builtin)) int
___memcmp(const void *a, const void *b, size_t n)
{
    return memcmp(a, b, n);
}

__attribute__((no_builtin)) void *
___memmove(void *dst, const void *src, size_t n)
{
    return memmove(dst, src, n);
}

__attribute__((no_builtin)) void *
___memset(void *dst, int c, size_t n)
{
    return memset(dst, c, n);
}

__attribute__((no_builtin)) void
___bzero(void *dst, size_t n)
{
    memset(dst, 0, n);
}

__attribute__((no_builtin)) int
___strcmp(const char *a, const char *b)
{
    return strcmp(a, b);
}

__attribute__((no_builtin)) char *
___strcpy(char *dst, const char *src)
{
    return strcpy(dst, src);
}

__attribute__((no_builtin)) size_t
___strlen(const char *s)
{
    return strlen(s);
}

__attribute__((no_builtin)) char *
___strstr(const char *s, const char *find)
{
    return (char *)strstr(s, find);
}
