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

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <sys/cdefs.h>
#include <unistd.h>

_BEGIN_STD_C

/*
 * Starting programs, as OS/2's C compilers have it. P_WAIT returns the
 * program's exit code, P_NOWAIT its process id (for waitpid), and
 * P_OVERLAY exits with its exit code (as exec does). A name without an
 * extension gets .EXE; the p forms look in the current directory and the
 * PATH; the e forms pass the environment ENVP, the others environ.
 */
#define P_WAIT    0
#define P_NOWAIT  1
#define P_OVERLAY 2

int spawnl(int mode, const char *path, const char *arg0, ...);
int spawnle(int mode, const char *path, const char *arg0, ...);
int spawnlp(int mode, const char *file, const char *arg0, ...);
int spawnlpe(int mode, const char *file, const char *arg0, ...);
int spawnv(int mode, const char *path, char * const argv[]);
int spawnve(int mode, const char *path, char * const argv[], char * const envp[]);
int spawnvp(int mode, const char *file, char * const argv[]);
int spawnvpe(int mode, const char *file, char * const argv[], char * const envp[]);

_END_STD_C

#endif /* _PROCESS_H_ */
