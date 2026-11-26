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

/* The Windows C library's own string functions (sys/_win32string.h). */

#include "local-win32.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* Compares two strings and ignores the case of letters. */
int
_stricmp(const char *a, const char *b)
{
    return strcasecmp(a, b);
}

/* Compares at most n characters of two strings and ignores case. */
int
_strnicmp(const char *a, const char *b, size_t n)
{
    return strncasecmp(a, b, n);
}

/* Compares two strings with the collation of the locale and ignores
 * case. The library has only the C locale, so this is _stricmp. */
int
_stricoll(const char *a, const char *b)
{
    return strcasecmp(a, b);
}

/* As _stricoll, for at most n characters. */
int
_strnicoll(const char *a, const char *b, size_t n)
{
    return strncasecmp(a, b, n);
}

/* Compares n bytes of two blocks and ignores the case of letters. */
int
_memicmp(const void *a, const void *b, size_t n)
{
    const unsigned char *p = a, *q = b;
    for (; n; n--, p++, q++) {
        int d = tolower(*p) - tolower(*q);
        if (d)
            return d;
    }
    return 0;
}

/* Copies bytes until it copies the byte c or n bytes. The result is the
 * byte after c in the destination, or NULL when c was not copied. */
void *
_memccpy(void * __restrict dst, const void * __restrict src, int c, size_t n)
{
    return memccpy(dst, src, c, n);
}

/* A copy of the string in a block from malloc. */
char *
_strdup(const char *s)
{
    return strdup(s);
}

/* Changes the letters of the string to lower case. */
char *
_strlwr(char *s)
{
    return strlwr(s);
}

/* Changes the letters of the string to upper case. */
char *
_strupr(char *s)
{
    return strupr(s);
}

/* Puts the characters of the string in the opposite order. */
char *
_strrev(char *s)
{
    size_t n = strlen(s);
    for (size_t i = 0; i < n / 2; i++) {
        char t = s[i];
        s[i] = s[n - 1 - i];
        s[n - 1 - i] = t;
    }
    return s;
}

/* Sets each character of the string to c. */
char *
_strset(char *s, int c)
{
    for (char *p = s; *p; p++)
        *p = (char)c;
    return s;
}

/* Sets at most n characters of the string to c. */
char *
_strnset(char *s, int c, size_t n)
{
    for (char *p = s; n && *p; p++, n--)
        *p = (char)c;
    return s;
}

/* The message for errno, after the text s and ": " when s is not NULL,
 * with a newline at the end. The text is in a static buffer. */
char *
_strerror(const char *s)
{
    static char buf[256];
    const char *msg = strerror(errno);
    if (s && *s)
        snprintf(buf, sizeof(buf), "%s: %s\n", s, msg);
    else
        snprintf(buf, sizeof(buf), "%s\n", msg);
    return buf;
}

/* The old names. strlwr and strupr are in the C library already. */
int   _strcmpi(const char *, const char *) __attribute__((__alias__("_stricmp")));
int   stricmp(const char *, const char *) __attribute__((__alias__("_stricmp")));
int   strcmpi(const char *, const char *) __attribute__((__alias__("_stricmp")));
int   strnicmp(const char *, const char *, size_t) __attribute__((__alias__("_strnicmp")));
int   memicmp(const void *, const void *, size_t) __attribute__((__alias__("_memicmp")));
char *strrev(char *) __attribute__((__alias__("_strrev")));
char *strset(char *, int) __attribute__((__alias__("_strset")));
char *strnset(char *, int, size_t) __attribute__((__alias__("_strnset")));
