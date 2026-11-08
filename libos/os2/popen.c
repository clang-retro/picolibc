/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#include "local-stdio.h"
#include "local-os2.h"
#include "include/process.h"
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * popen without fork: the end of the pipe the command uses becomes this
 * program's stdin or stdout while the command processor starts (P_NOWAIT),
 * which inherits it, and then the handle is put back. This program's own
 * end is not inherited, so the command sees the end of its input when
 * pclose closes it.
 */

static int
__pipe_close(FILE *file)
{
    struct __file_pipe *pf = (struct __file_pipe *)file;
    int                *wstatus = pf->wstatus;
    pid_t               child = pf->child;
    int                 ret;

    ret = __bufio_close(file);
    (void)waitpid(child, wstatus, 0);
    return ret;
}

FILE *
popen(const char *command, const char *type)
{
    int                 pipe_fd[2];
    int                 my_fd;
    int                 stdio_flags;
    struct __file_pipe *pf;

    if (strchr(type, 'r')) {
        my_fd = 0;
        stdio_flags = __SRD;
    } else if (strchr(type, 'w')) {
        my_fd = 1;
        stdio_flags = __SWR;
    } else {
        errno = EINVAL;
        return NULL;
    }
    /* The command's stdout (reading) or stdin (writing) */
    int std = 1 - my_fd;

    pf = calloc(1, sizeof(struct __file_pipe) + BUFSIZ);
    if (pf == NULL)
        return NULL;
    if (pipe(pipe_fd) < 0) {
        free(pf);
        return NULL;
    }

    /* What this program wrote to stdout so far goes there, not to the
       command. */
    fflush(stdout);
    int saved = dup(std);
    int pid = -1;
    if (saved >= 0 && __os2_inherit(saved, 0) == 0 && __os2_inherit(pipe_fd[my_fd], 0) == 0
        && dup2(pipe_fd[std], std) == std) {
        pid = __os2_shell(P_NOWAIT, command);
        int e = errno;
        dup2(saved, std);
        errno = e;
    }
    if (saved >= 0)
        close(saved);
    close(pipe_fd[std]);
    if (pid < 0) {
        close(pipe_fd[my_fd]);
        free(pf);
        return NULL;
    }

    char *buf = (char *)(pf + 1);
    pf->bfile = (struct __file_bufio)FDEV_SETUP_POSIX(pipe_fd[my_fd], buf, BUFSIZ, stdio_flags,
                                                      __BFALL | __BPIPE);
    pf->bfile.xfile.cfile.close = __pipe_close;
    pf->wstatus = NULL;
    pf->child = pid;
    bufio_add_file(&pf->bfile.xfile.cfile.file);
    return &pf->bfile.xfile.cfile.file;
}
