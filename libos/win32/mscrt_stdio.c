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

/* The Windows C library's own <stdio.h> functions (sys/_win32stdio.h). */

#include "local-win32.h"
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

/* As vsnprintf, with the Windows rules: text that fills the buffer
 * exactly has no NUL, and text that does not fit gives -1. */
int
_vsnprintf(char * __restrict buf, size_t n, const char * __restrict fmt, va_list ap)
{
    va_list ap2;
    va_copy(ap2, ap);
    int r = vsnprintf(buf, n, fmt, ap);
    if (r >= 0 && (size_t)r >= n && n > 0) {
        /* vsnprintf put a NUL in the last place: put the character that
         * belongs there. */
        char *tmp = malloc((size_t)r + 1);
        if (tmp) {
            vsnprintf(tmp, (size_t)r + 1, fmt, ap2);
            memcpy(buf, tmp, n);
            free(tmp);
        }
    }
    va_end(ap2);
    if (r < 0 || (size_t)r > n)
        return -1;
    return r;
}

/* As snprintf, with the rules of _vsnprintf. */
int
_snprintf(char * __restrict buf, size_t n, const char * __restrict fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = _vsnprintf(buf, n, fmt, ap);
    va_end(ap);
    return r;
}

/* The number of characters that the format makes, without the NUL. */
int
_vscprintf(const char * __restrict fmt, va_list ap)
{
    return vsnprintf(NULL, 0, fmt, ap);
}

/* As _vscprintf, with the arguments after the format. */
int
_scprintf(const char * __restrict fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    return r;
}

/* As vswprintf, with -1 when the text does not fit. The buffer then has
 * no NUL in the Windows library; here its contents are not defined. */
int
_vsnwprintf(wchar_t * __restrict buf, size_t n, const wchar_t * __restrict fmt, va_list ap)
{
    int r = vswprintf(buf, n, fmt, ap);
    return r < 0 ? -1 : r;
}

/* As _vsnwprintf, with the arguments after the format. */
int
_snwprintf(wchar_t * __restrict buf, size_t n, const wchar_t * __restrict fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = _vsnwprintf(buf, n, fmt, ap);
    va_end(ap);
    return r;
}

/* The descriptor of a stream. */
int
_fileno(FILE *f)
{
    return fileno(f);
}

/* A stream for a descriptor. */
FILE *
_fdopen(int fd, const char *mode)
{
    return fdopen(fd, mode);
}

/* Flushes the output of stdout and stderr. The library keeps no list of
 * the streams that fopen opens, so a program flushes those itself. */
int
_flushall(void)
{
    fflush(stdout);
    fflush(stderr);
    return 0;
}

/* Reads an int (in the bytes of the machine) from a stream. EOF on
 * error, which is also a value that the stream can hold: use feof and
 * ferror. */
int
_getw(FILE *f)
{
    int v;
    if (fread(&v, sizeof(v), 1, f) != 1)
        return EOF;
    return v;
}

/* Writes an int (in the bytes of the machine) to a stream. The value, or
 * EOF on error. */
int
_putw(int v, FILE *f)
{
    if (fwrite(&v, sizeof(v), 1, f) != 1)
        return EOF;
    return v;
}

/* Opens a file by its wide name for a stream that is open now. The name
 * goes through the ANSI code page. */
FILE *
_wfreopen(const wchar_t *path, const wchar_t *wmode, FILE *f)
{
    char   name[PATH_MAX], mode[8];
    size_t i;
    if (!__win32_ansi_name(path, name))
        return NULL;
    for (i = 0; wmode[i] && i < sizeof(mode) - 1; i++)
        mode[i] = (char)wmode[i];
    mode[i] = '\0';
    return freopen(name, mode, f);
}

/* Removes a file by its wide name, through the ANSI code page. */
int
_wremove(const wchar_t *path)
{
    char name[PATH_MAX];
    if (!__win32_ansi_name(path, name))
        return -1;
    return remove(name);
}

/* Renames a file by wide names, through the ANSI code page. */
int
_wrename(const wchar_t *from, const wchar_t *to)
{
    char a[PATH_MAX], b[PATH_MAX];
    if (!__win32_ansi_name(from, a) || !__win32_ansi_name(to, b))
        return -1;
    return rename(a, b);
}

/* A wide name as an ANSI name, for the wide functions that have no
 * W form here. */
char *
__win32_ansi_name(const wchar_t *wide, char *buf)
{
    if (!WideCharToMultiByte(__WIN32_CP_ACP, 0, wide, -1, buf, PATH_MAX, NULL, NULL)) {
        errno = ENAMETOOLONG;
        return NULL;
    }
    return buf;
}
