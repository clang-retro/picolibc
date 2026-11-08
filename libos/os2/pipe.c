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

#include "local-os2.h"
#include <unistd.h>

/* An anonymous pipe: fds[0] reads what fds[1] writes. */
int
pipe(int fds[2])
{
    __os2_handle r, w;
    __os2_ret    rc = __os2_pipe(&r, &w, 4096);
    if (rc)
        return __os2_error(rc);
    fds[0] = (int)r;
    fds[1] = (int)w;
    return 0;
}

static int
dup_to(int fd, __os2_handle h)
{
    __os2_ret rc = DosDupHandle((__os2_handle)fd, &h);
    if (rc)
        return __os2_error(rc);
    int nfd = (int)h;
    if (nfd >= 0 && nfd < __OS2_MAX_FD)
        __os2_fd_flags[nfd] = fd >= 0 && fd < __OS2_MAX_FD ? __os2_fd_flags[fd] : 0;
    return nfd;
}

int
dup(int fd)
{
    return dup_to(fd, (__os2_handle)-1); /* a new handle */
}

/* OS/2 closes fd2 first when it is open. */
int
dup2(int fd, int fd2)
{
    if (fd == fd2)
        return fd;
    return dup_to(fd, (__os2_handle)fd2);
}

int
__os2_inherit(int fd, int inherit)
{
    __os2_word state;
    __os2_ret  rc = __os2_getfhstate((__os2_handle)fd, &state);
    if (rc)
        return __os2_error(rc);
    /* Only write-through, fail-on-error and no-inherit may be set. */
    state = (state & 0x6000) | (inherit ? 0 : __OS2_OPEN_NOINHERIT);
    rc = __os2_setfhstate((__os2_handle)fd, state);
    return rc ? __os2_error(rc) : 0;
}
