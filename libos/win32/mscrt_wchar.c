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

/* The Windows C library's own wide string functions
 * (sys/_win32wchar.h). */

#include "local-win32.h"
#include <wchar.h>
#include <wctype.h>

/* Compares two wide strings and ignores the case of letters. */
int
_wcsicmp(const wchar_t *a, const wchar_t *b)
{
    for (;; a++, b++) {
        wint_t x = towlower(*a), y = towlower(*b);
        if (x != y)
            return x < y ? -1 : 1;
        if (!x)
            return 0;
    }
}

/* Compares at most n characters of two wide strings and ignores case. */
int
_wcsnicmp(const wchar_t *a, const wchar_t *b, size_t n)
{
    for (; n; n--, a++, b++) {
        wint_t x = towlower(*a), y = towlower(*b);
        if (x != y)
            return x < y ? -1 : 1;
        if (!x)
            return 0;
    }
    return 0;
}

/* Compares with the collation of the locale and ignores case. The
 * library has only the C locale, so this is _wcsicmp. */
int
_wcsicoll(const wchar_t *a, const wchar_t *b)
{
    return _wcsicmp(a, b);
}

/* A copy of the wide string in a block from malloc. */
wchar_t *
_wcsdup(const wchar_t *s)
{
    return wcsdup(s);
}

/* Changes the letters of the wide string to lower case. */
wchar_t *
_wcslwr(wchar_t *s)
{
    for (wchar_t *p = s; *p; p++)
        *p = (wchar_t)towlower(*p);
    return s;
}

/* Changes the letters of the wide string to upper case. */
wchar_t *
_wcsupr(wchar_t *s)
{
    for (wchar_t *p = s; *p; p++)
        *p = (wchar_t)towupper(*p);
    return s;
}

/* Puts the characters of the wide string in the opposite order. */
wchar_t *
_wcsrev(wchar_t *s)
{
    size_t n = wcslen(s);
    for (size_t i = 0; i < n / 2; i++) {
        wchar_t t = s[i];
        s[i] = s[n - 1 - i];
        s[n - 1 - i] = t;
    }
    return s;
}

/* Sets each character of the wide string to c. */
wchar_t *
_wcsset(wchar_t *s, wchar_t c)
{
    for (wchar_t *p = s; *p; p++)
        *p = c;
    return s;
}

/* Sets at most n characters of the wide string to c. */
wchar_t *
_wcsnset(wchar_t *s, wchar_t c, size_t n)
{
    for (wchar_t *p = s; n && *p; p++, n--)
        *p = c;
    return s;
}

/* The old names. */
int      wcsicmp(const wchar_t *, const wchar_t *) __attribute__((__alias__("_wcsicmp")));
int      wcsnicmp(const wchar_t *, const wchar_t *, size_t) __attribute__((__alias__("_wcsnicmp")));
wchar_t *wcslwr(wchar_t *) __attribute__((__alias__("_wcslwr")));
wchar_t *wcsupr(wchar_t *) __attribute__((__alias__("_wcsupr")));
