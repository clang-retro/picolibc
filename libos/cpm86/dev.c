/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 clang-retro contributors
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

#include "local-cpm86.h"
#include <string.h>
#include <unistd.h>

void
__cpm_conout(uint8_t device, char c)
{
    switch (device) {
    case __CPM_DEV_CON:
        __cpm_bdos(BDOS_CONOUT, (uint8_t)c);
        break;
    case __CPM_DEV_LST:
        __cpm_bdos(BDOS_LSTOUT, (uint8_t)c);
        break;
    case __CPM_DEV_PUN:
        __cpm_bdos(BDOS_AUXOUT, (uint8_t)c);
        break;
    default:
        break;
    }
}

ssize_t
__cpm_dev_write(struct __cpm_file *f, const char *buf, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (buf[i] == '\n' && (f->flags & __CPM_FD_TEXT))
            __cpm_conout(f->device, '\r');
        __cpm_conout(f->device, buf[i]);
    }
    return n;
}

/* The console reads a line at a time with BDOS function 10, which lets
 * the user edit it; the line then reads as its text and LF. Ctrl-Z at the
 * start of a line is the end of the input. */
static struct {
    uint8_t max, len;
    char    text[128];
} line = { .max = 126 };
static uint8_t line_pos, line_left;

static ssize_t
con_read(char *buf, size_t n)
{
    if (line_left == 0) {
        line.max = 126;
        __cpm_bdos(BDOS_READLINE, __CPM_PTR(&line));
        /* The BDOS leaves the cursor at the end of the line. */
        __cpm_bdos(BDOS_CONOUT, '\r');
        __cpm_bdos(BDOS_CONOUT, '\n');
        if (line.len > 0 && line.text[0] == 0x1a)
            return 0;
        line.text[line.len] = '\n';
        line_left = line.len + 1;
        line_pos = 0;
    }
    size_t got = 0;
    while (got < n && line_left > 0) {
        buf[got++] = line.text[line_pos++];
        line_left--;
    }
    return got;
}

ssize_t
__cpm_dev_read(struct __cpm_file *f, char *buf, size_t n)
{
    switch (f->device) {
    case __CPM_DEV_CON:
        return con_read(buf, n);
    case __CPM_DEV_RDR: {
        size_t got = 0;
        if (f->flags & __CPM_FD_EOF)
            return 0;
        while (got < n) {
            char c = (char)__cpm_bdos(BDOS_AUXIN, 0);
            if (c == 0x1a && (f->flags & __CPM_FD_TEXT)) {
                f->flags |= __CPM_FD_EOF;
                break;
            }
            if (c == '\r' && (f->flags & __CPM_FD_TEXT))
                continue;
            buf[got++] = c;
            if (c == '\n')
                break;
        }
        return got;
    }
    default:
        return 0;
    }
}

int
isatty(int fd)
{
    struct __cpm_file *f = __cpm_fd(fd);
    if (f == NULL)
        return 0;
    if (f->device == __CPM_DEV_NONE) {
        errno = ENOTTY;
        return 0;
    }
    return 1;
}
