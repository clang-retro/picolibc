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
#include <stdlib.h>
#include <unistd.h>

/*
 * The environment of the process, for programs (crt0.S keeps where OS/2
 * put it) and for 16-bit DLLs (libentry.S asks for it): NAME=value
 * strings, each ending with a NUL, and an empty string. The command line
 * follows it: the program name, a NUL, the arguments, and two NULs.
 */

__os2_fstr __os2_env;
__os2_fstr __os2_cmdline;

size_t
__os2_fstrlen(__os2_fstr s)
{
    size_t n = 0;
    while (s[n])
        n++;
    return n;
}

/* A copy of a far string, in the C library's memory. */
char *
__os2_fdup(__os2_fstr s, size_t n)
{
    char *d = malloc(n + 1);
    if (!d)
        _exit(255);
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    d[n] = '\0';
    return d;
}

/* environ gets a copy of the environment. */
void
__os2_init_environ(void)
{
    static char *empty[1];
    environ = empty;
    if (!__os2_env)
        return;
    size_t count = 0;
    for (__os2_fstr p = __os2_env; *p; p += __os2_fstrlen(p) + 1)
        count++;
    char **env = malloc((count + 1) * sizeof(char *));
    if (!env)
        return;
    size_t i = 0;
    for (__os2_fstr p = __os2_env; *p; p += __os2_fstrlen(p) + 1)
        env[i++] = __os2_fdup(p, __os2_fstrlen(p));
    env[i] = NULL;
    environ = env;
}
