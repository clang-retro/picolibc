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

/* The next byte of a file, without moving: -1 at the end. */
static int
peek_byte(struct __cpm_file *f, off_t pos)
{
    if (pos >= f->size)
        return -1;
    int r = __cpm_load(f, (int32_t)(pos / __CPM_RECORD));
    if (r != 0)
        return -1;
    return f->buf[pos % __CPM_RECORD];
}

/* In text mode, CR LF reads as LF and Ctrl-Z ends the file. */
ssize_t
read(int fd, void *buf, size_t count)
{
    struct __cpm_file *f = __cpm_fd(fd);
    if (f == NULL)
        return -1;
    if (!(f->flags & __CPM_FD_READ)) {
        errno = EBADF;
        return -1;
    }
    if (f->device != __CPM_DEV_NONE)
        return __cpm_dev_read(f, buf, count);
    if (f->flags & __CPM_FD_EOF)
        return 0;

    char  *out = buf;
    size_t n = 0;
    if (!(f->flags & __CPM_FD_TEXT)) {
        while (n < count && f->pos < f->size) {
            if (__cpm_load(f, (int32_t)(f->pos / __CPM_RECORD)) < 0)
                return n ? (ssize_t)n : -1;
            size_t at = f->pos % __CPM_RECORD;
            size_t chunk = __CPM_RECORD - at;
            if (chunk > count - n)
                chunk = count - n;
            if ((off_t)chunk > f->size - f->pos)
                chunk = f->size - f->pos;
            memcpy(out + n, f->buf + at, chunk);
            n += chunk;
            f->pos += chunk;
        }
        return n;
    }
    while (n < count) {
        int c = peek_byte(f, f->pos);
        if (c < 0)
            break;
        if (c == 0x1a) {
            /* Leave the file at the Ctrl-Z, and read nothing more. */
            f->flags |= __CPM_FD_EOF;
            break;
        }
        f->pos++;
        if (c == '\r' && peek_byte(f, f->pos) == '\n')
            continue;
        out[n++] = (char)c;
    }
    return n;
}

ssize_t
write(int fd, const void *buf, size_t count)
{
    struct __cpm_file *f = __cpm_fd(fd);
    if (f == NULL)
        return -1;
    if (!(f->flags & __CPM_FD_WRITE)) {
        errno = EBADF;
        return -1;
    }
    if (f->device != __CPM_DEV_NONE)
        return __cpm_dev_write(f, buf, count);
    if (f->flags & __CPM_FD_APPEND)
        f->pos = f->size;
    f->flags &= ~__CPM_FD_EOF;

    const char *in = buf;
    int         text = f->flags & __CPM_FD_TEXT;
    size_t      done = 0;
    int         cr = 0; /* the CR of a CR LF has been written */
    while (done < count) {
        if (__cpm_load(f, (int32_t)(f->pos / __CPM_RECORD)) < 0)
            return done ? (ssize_t)done : -1;
        size_t at = f->pos % __CPM_RECORD;
        char   c = in[done];
        if (text && c == '\n' && !cr) {
            f->buf[at] = '\r';
            cr = 1;
        } else {
            f->buf[at] = (uint8_t)c;
            cr = 0;
            done++;
        }
        f->dirty = 1;
        f->pos++;
        if (f->pos > f->size)
            f->size = f->pos;
    }
    return done;
}

off_t
lseek(int fd, off_t offset, int whence)
{
    struct __cpm_file *f = __cpm_fd(fd);
    if (f == NULL)
        return -1;
    if (f->device != __CPM_DEV_NONE) {
        errno = ESPIPE;
        return -1;
    }
    off_t base = whence == SEEK_SET ? 0 : whence == SEEK_CUR ? f->pos : whence == SEEK_END ? f->size : -1;
    if (base < 0) {
        errno = EINVAL;
        return -1;
    }
    off_t pos = base + offset;
    /* CP/M-86 files have at most 65536 records of 128 bytes. */
    if (pos < 0 || pos > 8 * 1024 * 1024L) {
        errno = EINVAL;
        return -1;
    }
    f->pos = pos;
    f->flags &= ~__CPM_FD_EOF;
    return pos;
}
