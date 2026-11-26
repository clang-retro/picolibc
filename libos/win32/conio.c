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

/* The Windows C library's console functions (conio.h). They use the
 * console of the process (CONIN$ and CONOUT$), whatever standard input
 * and standard output are. */

#include "local-win32.h"
#include "include/conio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A key event of the console. */
struct key_event {
    __win32_bool  down;
    uint16_t      repeat;
    uint16_t      virtual_key;
    uint16_t      scan_code;
    uint16_t      character;
    __win32_dword control_state;
};

/* An input event of the console: KEY_EVENT (1) or another event. */
struct input_record {
    uint16_t type;
    union {
        struct key_event key;
        char             other[16];
    } event;
};

#define KEY_EVENT         1
#define ENHANCED_KEY      0x0100
#define ENABLE_LINE_INPUT 0x0002
#define ENABLE_ECHO_INPUT 0x0004
#define ENABLE_PROCESSED  0x0001

__WIN32_API __win32_bool GetConsoleMode(__win32_handle h, __win32_dword *mode);
__WIN32_API __win32_bool SetConsoleMode(__win32_handle h, __win32_dword mode);
__WIN32_API __win32_bool ReadConsoleInputA(__win32_handle h, struct input_record *buf,
                                           __win32_dword n, __win32_dword *done);
__WIN32_API __win32_bool PeekConsoleInputA(__win32_handle h, struct input_record *buf,
                                           __win32_dword n, __win32_dword *done);
__WIN32_API __win32_bool ReadConsoleA(__win32_handle h, void *buf, __win32_dword n,
                                      __win32_dword *done, void *reserved);
__WIN32_API __win32_bool WriteConsoleA(__win32_handle h, const void *buf, __win32_dword n,
                                       __win32_dword *done, void *reserved);

static __win32_handle    con_in, con_out;
/* A character for the next _getch: from _ungetch, or the scan code
 * after a 0 or 0xE0. -1 when there is none. */
static int               pushed = -1;

/* Opens a console device ("CONIN$" or "CONOUT$") once. NULL on error. */
static __win32_handle
console(__win32_handle *h, const char *name)
{
    if (!*h) {
        __win32_handle c = CreateFileA(name, __WIN32_GENERIC_READ | __WIN32_GENERIC_WRITE,
                                       __WIN32_FILE_SHARE_READ | __WIN32_FILE_SHARE_WRITE, NULL,
                                       __WIN32_OPEN_EXISTING, 0, NULL);
        if (c == __WIN32_INVALID_HANDLE)
            return NULL;
        *h = c;
    }
    return *h;
}

/* Not zero for a key that gives nothing to _getch: Shift, Ctrl, Alt and
 * the lock keys. */
static int
is_modifier(unsigned vk)
{
    return (vk >= 0x10 && vk <= 0x12) || vk == 0x14 || vk == 0x90 || vk == 0x91;
}

/* Not zero for a key event that _getch returns. */
static int
is_key(const struct input_record *r)
{
    return r->type == KEY_EVENT && r->event.key.down && !is_modifier(r->event.key.virtual_key);
}

/* Reads a key without echo. A key without a character gives 0 (or 0xE0
 * for the keys of the enhanced keyboard), and the next call gives its
 * scan code. EOF on error. */
int
_getch(void)
{
    if (pushed >= 0) {
        int c = pushed;
        pushed = -1;
        return c;
    }
    __win32_handle h = console(&con_in, "CONIN$");
    if (!h)
        return EOF;
    __win32_dword mode;
    int           have_mode = GetConsoleMode(h, &mode);
    SetConsoleMode(h, 0);
    int c = EOF;
    for (;;) {
        struct input_record r;
        __win32_dword       n;
        if (!ReadConsoleInputA(h, &r, 1, &n) || n != 1)
            break;
        if (!is_key(&r))
            continue;
        unsigned ch = r.event.key.character & 0xff;
        if (ch) {
            c = (int)ch;
        } else {
            c = (r.event.key.control_state & ENHANCED_KEY) ? 0xe0 : 0;
            pushed = r.event.key.scan_code & 0xff;
        }
        break;
    }
    if (have_mode)
        SetConsoleMode(h, mode);
    return c;
}

/* Reads a key and writes its character on the console. */
int
_getche(void)
{
    int c = _getch();
    if (c != EOF && c != 0 && c != 0xe0)
        _putch(c);
    return c;
}

/* Not zero when a key is waiting for _getch. Other console events that
 * come before it are removed. */
int
_kbhit(void)
{
    if (pushed >= 0)
        return 1;
    __win32_handle h = console(&con_in, "CONIN$");
    if (!h)
        return 0;
    for (;;) {
        struct input_record r;
        __win32_dword       n;
        if (!PeekConsoleInputA(h, &r, 1, &n) || n == 0)
            return 0;
        if (is_key(&r))
            return 1;
        ReadConsoleInputA(h, &r, 1, &n);
    }
}

/* Gives a character back to the next _getch. The character, or EOF when
 * one is waiting already. */
int
_ungetch(int c)
{
    if (c == EOF || pushed >= 0)
        return EOF;
    pushed = c & 0xff;
    return pushed;
}

/* Writes a character on the console. The character, or EOF. */
int
_putch(int c)
{
    __win32_handle h = console(&con_out, "CONOUT$");
    char           b = (char)c;
    __win32_dword  n;
    if (!h || !WriteConsoleA(h, &b, 1, &n, NULL) || n != 1)
        return EOF;
    return c & 0xff;
}

/* Writes a string on the console. 0, or -1 on error. */
int
_cputs(const char *s)
{
    __win32_handle h = console(&con_out, "CONOUT$");
    size_t         len = strlen(s);
    __win32_dword  n;
    if (!h || !WriteConsoleA(h, s, (__win32_dword)len, &n, NULL) || n != len)
        return -1;
    return 0;
}

/* Reads a line from the console, with echo and editing. buf[0] is the
 * most characters to read; the number read goes in buf[1] and the text,
 * with a NUL, from buf + 2. The result is buf + 2, or NULL on error. */
char *
_cgets(char *buf)
{
    __win32_handle h = console(&con_in, "CONIN$");
    if (!h)
        return NULL;
    unsigned      max = (unsigned char)buf[0];
    char          tmp[258];
    __win32_dword mode, n;
    int           have_mode = GetConsoleMode(h, &mode);
    SetConsoleMode(h, ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED);
    __win32_bool ok = ReadConsoleA(h, tmp, max + 2, &n, NULL);
    if (have_mode)
        SetConsoleMode(h, mode);
    if (!ok)
        return NULL;
    unsigned len = 0;
    while (len < n && len < max && tmp[len] != '\r' && tmp[len] != '\n')
        len++;
    memcpy(buf + 2, tmp, len);
    buf[2 + len] = '\0';
    buf[1] = (char)len;
    return buf + 2;
}

/* As vprintf, on the console. The number of characters, or -1. */
int
_vcprintf(const char *fmt, va_list ap)
{
    char    small[256];
    char   *p = small;
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(small, sizeof(small), fmt, ap);
    if (n >= (int)sizeof(small)) {
        p = malloc((size_t)n + 1);
        if (p)
            vsnprintf(p, (size_t)n + 1, fmt, ap2);
    }
    va_end(ap2);
    if (n < 0 || !p)
        return -1;
    int r = _cputs(p) ? -1 : n;
    if (p != small)
        free(p);
    return r;
}

/* As printf, on the console. */
int
_cprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = _vcprintf(fmt, ap);
    va_end(ap);
    return r;
}

/* The old names. */
int   getch(void) __attribute__((__alias__("_getch")));
int   getche(void) __attribute__((__alias__("_getche")));
int   kbhit(void) __attribute__((__alias__("_kbhit")));
int   ungetch(int) __attribute__((__alias__("_ungetch")));
int   putch(int) __attribute__((__alias__("_putch")));
int   cputs(const char *) __attribute__((__alias__("_cputs")));
char *cgets(char *) __attribute__((__alias__("_cgets")));
int   cprintf(const char *, ...) __attribute__((__alias__("_cprintf")));
