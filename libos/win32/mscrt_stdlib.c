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

/* The Windows C library's own <stdlib.h> functions
 * (sys/_win32stdlib.h). */

#include "local-win32.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* Writes v in the radix (2 to 36) to buf, with a minus sign before it
 * when neg is set. An unknown radix gives an empty string. */
static char *
to_text(unsigned long long v, int neg, char *buf, int radix)
{
    char  tmp[66];
    char *p = tmp;
    char *out = buf;
    if (radix < 2 || radix > 36) {
        *buf = '\0';
        return buf;
    }
    do {
        int d = (int)(v % (unsigned)radix);
        *p++ = (char)(d < 10 ? '0' + d : 'a' + d - 10);
        v /= (unsigned)radix;
    } while (v);
    if (neg)
        *out++ = '-';
    while (p > tmp)
        *out++ = *--p;
    *out = '\0';
    return buf;
}

/* Writes a signed number. As in the Windows library, only radix 10 shows
 * a minus sign; other radixes show the bits of the number. */
static char *
signed_text(long long v, unsigned long long mask, char *buf, int radix)
{
    if (radix == 10 && v < 0)
        return to_text(-(unsigned long long)v, 1, buf, radix);
    return to_text((unsigned long long)v & mask, 0, buf, radix);
}

/* Writes an int in the radix to buf. */
char *
_itoa(int v, char *buf, int radix)
{
    return signed_text(v, UINT_MAX, buf, radix);
}

/* Writes a long in the radix to buf. */
char *
_ltoa(long v, char *buf, int radix)
{
    return signed_text(v, ULONG_MAX, buf, radix);
}

/* Writes an unsigned long in the radix to buf. */
char *
_ultoa(unsigned long v, char *buf, int radix)
{
    return to_text(v, 0, buf, radix);
}

/* Writes a long long in the radix to buf. */
char *
_i64toa(long long v, char *buf, int radix)
{
    return signed_text(v, ULLONG_MAX, buf, radix);
}

/* Writes an unsigned long long in the radix to buf. */
char *
_ui64toa(unsigned long long v, char *buf, int radix)
{
    return to_text(v, 0, buf, radix);
}

/* Reads a decimal long long. */
long long
_atoi64(const char *s)
{
    return strtoll(s, NULL, 10);
}

/* Reads a long long in the radix, as strtoll. */
long long
_strtoi64(const char * __restrict s, char ** __restrict end, int radix)
{
    return strtoll(s, end, radix);
}

/* Reads an unsigned long long in the radix, as strtoull. */
unsigned long long
_strtoui64(const char * __restrict s, char ** __restrict end, int radix)
{
    return strtoull(s, end, radix);
}

/* Copies the ASCII text in s to the wide buffer w. */
static wchar_t *
widen(const char *s, wchar_t *w)
{
    wchar_t *p = w;
    while ((*p++ = (unsigned char)*s++) != 0)
        ;
    return w;
}

/* Writes an int in the radix to a wide buffer. */
wchar_t *
_itow(int v, wchar_t *buf, int radix)
{
    char tmp[66];
    return widen(_itoa(v, tmp, radix), buf);
}

/* Writes a long in the radix to a wide buffer. */
wchar_t *
_ltow(long v, wchar_t *buf, int radix)
{
    char tmp[66];
    return widen(_ltoa(v, tmp, radix), buf);
}

/* Writes an unsigned long in the radix to a wide buffer. */
wchar_t *
_ultow(unsigned long v, wchar_t *buf, int radix)
{
    char tmp[66];
    return widen(_ultoa(v, tmp, radix), buf);
}

/* Reads a decimal int from a wide string. */
int
_wtoi(const wchar_t *s)
{
    return (int)wcstol(s, NULL, 10);
}

/* Reads a decimal long from a wide string. */
long
_wtol(const wchar_t *s)
{
    return wcstol(s, NULL, 10);
}

/* Reads a decimal long long from a wide string. */
long long
_wtoi64(const wchar_t *s)
{
    return wcstoll(s, NULL, 10);
}

/* The full path of a file. With buf NULL, the path goes in a block from
 * malloc and size is not used. NULL on error. */
char *
_fullpath(char *buf, const char *path, size_t size)
{
    char tmp[_MAX_PATH];
    if (!path || !*path)
        path = ".";
    __win32_dword n = GetFullPathNameA(path, sizeof(tmp), tmp, NULL);
    if (n == 0) {
        __win32_error(0);
        return NULL;
    }
    if (n >= sizeof(tmp)) {
        errno = ENAMETOOLONG;
        return NULL;
    }
    if (!buf)
        return strdup(tmp);
    if (n >= size) {
        errno = ERANGE;
        return NULL;
    }
    memcpy(buf, tmp, n + 1);
    return buf;
}

/* Copies n characters of s to out (when out is not NULL), with a NUL. */
static void
part(char *out, const char *s, size_t n)
{
    if (!out)
        return;
    memcpy(out, s, n);
    out[n] = '\0';
}

/* Splits a path into its drive, directory, name and extension. Each
 * output can be NULL. The directory keeps its last slash, and the
 * extension its dot. */
void
_splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
    const char *p = path;
    if (p[0] && p[1] == ':') {
        part(drive, p, 2);
        p += 2;
    } else {
        part(drive, p, 0);
    }
    const char *name = p;
    for (const char *q = p; *q; q++)
        if (*q == '/' || *q == '\\')
            name = q + 1;
    part(dir, p, (size_t)(name - p));
    const char *dot = strrchr(name, '.');
    if (!dot)
        dot = name + strlen(name);
    part(fname, name, (size_t)(dot - name));
    part(ext, dot, strlen(dot));
}

/* Puts a path together from a drive, a directory, a name and an
 * extension. Each part can be NULL or empty. Slashes, the colon and the
 * dot are added where they are missing. */
void
_makepath(char *path, const char *drive, const char *dir, const char *fname, const char *ext)
{
    char *p = path;
    if (drive && *drive) {
        *p++ = *drive;
        *p++ = ':';
    }
    if (dir && *dir) {
        size_t n = strlen(dir);
        memcpy(p, dir, n);
        p += n;
        if (dir[n - 1] != '/' && dir[n - 1] != '\\')
            *p++ = '\\';
    }
    if (fname) {
        size_t n = strlen(fname);
        memcpy(p, fname, n);
        p += n;
    }
    if (ext && *ext) {
        if (*ext != '.')
            *p++ = '.';
        size_t n = strlen(ext);
        memcpy(p, ext, n);
        p += n;
    }
    *p = '\0';
}

/* Sets ("NAME=value") or removes ("NAME=") an environment variable, in
 * environ and in the environment of the process, which child processes
 * get. 0, or -1 on error. */
int
_putenv(const char *s)
{
    const char *eq = strchr(s, '=');
    if (!eq || eq == s) {
        errno = EINVAL;
        return -1;
    }
    size_t n = (size_t)(eq - s);
    char  *name = malloc(n + 1);
    if (!name) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(name, s, n);
    name[n] = '\0';
    int r;
    if (eq[1]) {
        r = setenv(name, eq + 1, 1);
        SetEnvironmentVariableA(name, eq + 1);
    } else {
        r = unsetenv(name);
        SetEnvironmentVariableA(name, NULL);
    }
    free(name);
    return r;
}

/* Waits for a number of milliseconds. */
void
_sleep(unsigned long ms)
{
    Sleep(ms);
}

/* Sounds the speaker. Windows 9x makes the default sound and does not
 * use the frequency and the time. */
void
_beep(unsigned frequency, unsigned ms)
{
    Beep(frequency, ms);
}

/* The old names. itoa is in the C library already. */
char *ltoa(long, char *, int) __attribute__((__alias__("_ltoa")));
char *ultoa(unsigned long, char *, int) __attribute__((__alias__("_ultoa")));
