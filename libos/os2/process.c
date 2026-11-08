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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Starting programs with DosExecPgm. OS/2 has no fork: spawn starts a
 * program and waits for it (P_WAIT) or not (P_NOWAIT, then waitpid), and
 * exec is spawn and exit (P_OVERLAY). The arguments go to the child as
 * one string, quoted as start.c splits them: a space or tab separates
 * them, double quotes group them, and \" is a quote.
 */

#define EXEC_SYNC        0
#define EXEC_ASYNCRESULT 2

/* Bytes argv takes as a command line, after the program name. */
static size_t
args_size(char * const argv[])
{
    size_t n = 2;
    for (size_t i = 1; argv[i]; i++) {
        n += 3;
        for (const char *s = argv[i]; *s; s++)
            n += *s == '"' ? 2 : 1;
    }
    return n;
}

static char *
put_args(char *d, char * const argv[])
{
    for (size_t i = 1; argv[i]; i++) {
        const char *s = argv[i];
        int         quote = !*s || strpbrk(s, " \t\"") != NULL;
        if (i > 1)
            *d++ = ' ';
        if (quote)
            *d++ = '"';
        for (; *s; s++) {
            if (*s == '"')
                *d++ = '\\';
            *d++ = *s;
        }
        if (quote)
            *d++ = '"';
    }
    return d;
}

/* PROG.EXE when the name has no extension. */
static int
add_exe(char *buf, size_t size, const char *name)
{
    size_t      n = strlen(name);
    const char *base = name;
    for (const char *s = name; *s; s++)
        if (*s == '/' || *s == '\\' || *s == ':')
            base = s + 1;
    int ext = strchr(base, '.') == NULL;
    if (n + (ext ? 4 : 0) >= size) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(buf, name, n);
    memcpy(buf + n, ext ? ".EXE" : "", ext ? 5 : 1);
    return 0;
}

/*
 * The file of a program: NAME, with .EXE when it has no extension. With
 * search and a name without a directory, the current directory and then
 * those of PATH are looked in.
 */
int
__os2_find_program(char *buf, size_t size, const char *name, int search)
{
    char file[260];
    if (add_exe(file, sizeof(file), name) < 0)
        return -1;
    if (!search || strpbrk(file, "/\\:")) {
        if (add_exe(buf, size, name) < 0)
            return -1;
        return 0;
    }
    /* SEARCH_CUR_DIRECTORY | SEARCH_ENVIRONMENT */
    __os2_ret r = DosSearchPath(3, "PATH", file, buf, (__os2_word)size);
    return r ? __os2_error(r) : 0;
}

/* Runs PATH with the command line ARGS (the program's name, a NUL, the
   arguments, two NULs) and the environment ENVP. */
static int
exec(int mode, const char *path, const char *args, char * const envp[])
{
    if (mode != P_WAIT && mode != P_NOWAIT && mode != P_OVERLAY) {
        errno = EINVAL;
        return -1;
    }
    if (!envp)
        envp = environ;
    size_t size = 2;
    for (size_t i = 0; envp[i]; i++)
        size += strlen(envp[i]) + 1;
    char *env = malloc(size), *d = env;
    if (!env) {
        errno = ENOMEM;
        return -1;
    }
    for (size_t i = 0; envp[i]; i++) {
        size_t n = strlen(envp[i]) + 1;
        memcpy(d, envp[i], n);
        d += n;
    }
    d[0] = d[1] = '\0';

    /* What the program wrote comes before what the child writes. (This
       fflush takes no NULL: the C library keeps no list of streams.) */
    fflush(stdout);
    fflush(stderr);
    struct __os2_resultcodes rc;
    char                     fail[64];
    __os2_ret r = DosExecPgm(fail, sizeof(fail), mode == P_NOWAIT ? EXEC_ASYNCRESULT : EXEC_SYNC,
                             args, env, &rc, path);
    free(env);
    if (r)
        return __os2_error(r);
    if (mode == P_NOWAIT)
        return (int)rc.code; /* the process id */
    if (mode == P_OVERLAY)
        _exit(rc.code ? 255 : (int)rc.result);
    if (rc.code) {
        errno = EINTR;
        return -1;
    }
    return (int)rc.result;
}

/* The command line: NAME, a NUL, ARGS as they are, two NULs. */
int
__os2_spawn_line(int mode, const char *path, const char *name, const char *line,
                 char * const envp[])
{
    size_t n = strlen(name) + 1, m = strlen(line);
    char  *args = malloc(n + m + 2);
    if (!args) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(args, name, n);
    memcpy(args + n, line, m);
    args[n + m] = args[n + m + 1] = '\0';
    int r = exec(mode, path, args, envp);
    free(args);
    return r;
}

int
__os2_spawnve(int mode, const char *path, char * const argv[], char * const envp[])
{
    static char * const noargs[] = { NULL, NULL };
    if (!argv || !argv[0])
        argv = noargs;
    const char *name = argv[0] ? argv[0] : path;
    size_t      n = strlen(name) + 1;
    char       *args = malloc(n + args_size(argv));
    if (!args) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(args, name, n);
    char *d = put_args(args + n, argv);
    d[0] = d[1] = '\0';
    int r = exec(mode, path, args, envp);
    free(args);
    return r;
}

static int
spawn(int mode, const char *name, char * const argv[], char * const envp[], int search)
{
    char path[260];
    if (__os2_find_program(path, sizeof(path), name, search) < 0)
        return -1;
    return __os2_spawnve(mode, path, argv, envp);
}

int
spawnv(int mode, const char *path, char * const argv[])
{
    return spawn(mode, path, argv, NULL, 0);
}

int
spawnve(int mode, const char *path, char * const argv[], char * const envp[])
{
    return spawn(mode, path, argv, envp, 0);
}

int
spawnvp(int mode, const char *file, char * const argv[])
{
    return spawn(mode, file, argv, NULL, 1);
}

int
spawnvpe(int mode, const char *file, char * const argv[], char * const envp[])
{
    return spawn(mode, file, argv, envp, 1);
}

/* The spawnl forms: the arguments, a NULL, and for spawnle and spawnlpe
   the environment. */
static int
spawnl_(int mode, const char *name, const char *arg0, va_list ap, int env, int search)
{
    va_list count;
    size_t  n = 1;
    va_copy(count, ap);
    while (va_arg(count, char *))
        n++;
    va_end(count);
    char **argv = malloc((n + 1) * sizeof(char *));
    if (!argv) {
        errno = ENOMEM;
        return -1;
    }
    argv[0] = (char *)arg0;
    for (size_t i = 1; i <= n; i++)
        argv[i] = va_arg(ap, char *);
    char * const *envp = env ? va_arg(ap, char * const *) : NULL;
    int           r = spawn(mode, name, argv, envp, search);
    free(argv);
    return r;
}

int
spawnl(int mode, const char *path, const char *arg0, ...)
{
    va_list ap;
    va_start(ap, arg0);
    int r = spawnl_(mode, path, arg0, ap, 0, 0);
    va_end(ap);
    return r;
}

int
spawnle(int mode, const char *path, const char *arg0, ...)
{
    va_list ap;
    va_start(ap, arg0);
    int r = spawnl_(mode, path, arg0, ap, 1, 0);
    va_end(ap);
    return r;
}

int
spawnlp(int mode, const char *file, const char *arg0, ...)
{
    va_list ap;
    va_start(ap, arg0);
    int r = spawnl_(mode, file, arg0, ap, 0, 1);
    va_end(ap);
    return r;
}

int
spawnlpe(int mode, const char *file, const char *arg0, ...)
{
    va_list ap;
    va_start(ap, arg0);
    int r = spawnl_(mode, file, arg0, ap, 1, 1);
    va_end(ap);
    return r;
}

/* A child started with P_NOWAIT: its status as wait.h decodes it. */
pid_t
waitpid(pid_t pid, int *status, int options)
{
    struct __os2_resultcodes rc;
    __os2_word               got;
    /* DCWA_PROCESS; DCWW_NOWAIT or DCWW_WAIT. Process id 0 is any child. */
    __os2_ret                r
        = __os2_cwait(0, (options & WNOHANG) ? 1 : 0, &rc, &got, (__os2_word)(pid > 0 ? pid : 0));
    if (r == 129) /* ERROR_CHILD_NOT_COMPLETE */
        return 0;
    if (r)
        return __os2_error(r);
    if (status) {
        static const unsigned char sig[] = { 0, 1 /* SIGHUP */, 11 /* SIGSEGV */, 9 /* SIGKILL */ };
        *status = rc.code == 0 ? (int)(rc.result & 0xff) << 8
            : rc.code < 4      ? sig[rc.code]
                               : 6 /* SIGABRT */;
    }
    return (pid_t)got;
}
