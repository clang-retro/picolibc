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
#include <unistd.h>

/* fds 0, 1 and 2 are the console, in text mode. */
static struct __cpm_file con_in
    = { .flags = __CPM_FD_OPEN | __CPM_FD_READ | __CPM_FD_TEXT, .device = __CPM_DEV_CON };
static struct __cpm_file con_out
    = { .flags = __CPM_FD_OPEN | __CPM_FD_WRITE | __CPM_FD_TEXT, .device = __CPM_DEV_CON };
static struct __cpm_file con_err
    = { .flags = __CPM_FD_OPEN | __CPM_FD_WRITE | __CPM_FD_TEXT, .device = __CPM_DEV_CON };

struct __cpm_file *__cpm_fds[__CPM_MAX_FD] = { &con_in, &con_out, &con_err };

struct __cpm_file *
__cpm_fd(int fd)
{
    if (fd < 0 || fd >= __CPM_MAX_FD || __cpm_fds[fd] == NULL) {
        errno = EBADF;
        return NULL;
    }
    return __cpm_fds[fd];
}

int
__cpm_new_fd(struct __cpm_file *f)
{
    for (int fd = 0; fd < __CPM_MAX_FD; fd++) {
        if (__cpm_fds[fd] == NULL) {
            __cpm_fds[fd] = f;
            return fd;
        }
    }
    errno = EMFILE;
    return -1;
}

/* Whether a file is read and written in text mode, for stdio, which then
 * keeps its position from the file's. */
int
__cpm_is_text(int fd)
{
    struct __cpm_file *f = fd >= 0 && fd < __CPM_MAX_FD ? __cpm_fds[fd] : NULL;
    return f && (f->flags & __CPM_FD_TEXT) && f->device == __CPM_DEV_NONE;
}
