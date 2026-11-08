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
#include "include/process.h"
#include <stdlib.h>
#include <string.h>

/* The command processor: COMSPEC, or CMD.EXE from the PATH. */
int
__os2_find_shell(char *buf, size_t size, const char **shell)
{
    *shell = getenv("COMSPEC");
    if (!*shell || !**shell)
        *shell = "CMD.EXE";
    return __os2_find_program(buf, size, *shell, 1);
}

/* Starts the command processor with /c and COMMAND, as it is. */
int
__os2_shell(int mode, const char *command)
{
    const char *shell;
    char        path[260];
    if (__os2_find_shell(path, sizeof(path), &shell) < 0)
        return -1;
    char *line = malloc(strlen(command) + 4);
    if (!line) {
        errno = ENOMEM;
        return -1;
    }
    strcpy(line, "/c ");
    strcat(line, command);
    int r = __os2_spawn_line(mode, path, shell, line, NULL);
    free(line);
    return r;
}

/* The result is the shell's status, as waitpid gives it. */
int
system(const char *command)
{
    if (!command) {
        const char *shell;
        char        path[260];
        return __os2_find_shell(path, sizeof(path), &shell) == 0;
    }
    int r = __os2_shell(P_WAIT, command);
    return r < 0 ? (errno == EINTR ? 9 /* SIGKILL */ : -1) : (r & 0xff) << 8;
}
