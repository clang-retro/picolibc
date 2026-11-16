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
#include <stdlib.h>
#include <string.h>
#include <sys/_initfini.h>
#include <unistd.h>

char     **environ;
uint16_t   __cpm_version;
char      *__cpm_brk;
char      *__cpm_heap_end;

extern int main(int, char **, char **);

/* Most arguments a command tail (at most 127 bytes) can hold. */
#define MAX_ARGS 64

#ifdef __CPM80__
/* Bytes kept for the stack, below the top of the TPA. */
#ifndef __CPM80_STACK_SIZE
#define __CPM80_STACK_SIZE 2048
#endif

/* The heap runs from the end of the BSS to the stack, which crt0 started
 * at the top of the TPA: the BDOS entry address in the word at 0006h. */
static void
init_heap(void)
{
    uint8_t *bp = __cpm_base_page;
    uint16_t top = bp[6] | ((uint16_t)bp[7] << 8);
    uint16_t start = (uint16_t)(uintptr_t)__cpm_heap_start;
    uint16_t room = top > start ? top - start : 0;
    __cpm_brk = __cpm_heap_start;
    __cpm_heap_end = __cpm_heap_start + (room > __CPM80_STACK_SIZE ? room - __CPM80_STACK_SIZE : 0);
}
#else
/* The heap runs from the end of the stack to the end of the group that
 * holds the data, as much of 64 KiB as CP/M-86 gave the program: the last
 * offset of the data group (the code group in the 8080 model) is in the
 * base page. */
static void
init_heap(void)
{
    uint8_t *bp = __cpm_base_page;
    int      m80 = bp[5];
    uint8_t *last = bp + (m80 ? 0 : 6);
    uint32_t end = (last[0] | ((uint32_t)last[1] << 8) | ((uint32_t)last[2] << 16)) + 1;
    if (end > 0xfffe)
        end = 0xfffe;
    __cpm_brk = __cpm_heap_start;
    uint16_t start = (uint16_t)(uintptr_t)__cpm_heap_start;
    __cpm_heap_end = __cpm_heap_start + (end > start ? end - start : 0);
}
#endif

/* Split the command tail at 80h into words, as DOS programs do; double
 * quotes group words. The CCP has made it upper case. */
static int
init_args(char **argv)
{
    static char tail[128];
    uint8_t     len = __cpm_base_page[0x80];
    if (len > 127)
        len = 127;
    memcpy(tail, __cpm_base_page + 0x81, len);
    tail[len] = '\0';

    int argc = 0;
    argv[argc++] = (char *)"";
    char *in = tail, *out = tail;
    while (argc < MAX_ARGS) {
        while (*in == ' ' || *in == '\t')
            in++;
        if (*in == '\0')
            break;
        argv[argc++] = out;
        int quoted = 0;
        while (*in && (quoted || (*in != ' ' && *in != '\t'))) {
            if (in[0] == '\\' && in[1] == '"') {
                *out++ = '"';
                in += 2;
            } else if (*in == '"') {
                quoted = !quoted;
                in++;
            } else {
                *out++ = *in++;
            }
        }
        if (*in)
            in++;
        *out++ = '\0';
    }
    argv[argc] = NULL;
    return argc;
}

/* Called by crt0 with the segment registers and the stack set up and the
 * BSS cleared. */
void
__cpm_start(void)
{
    static char *argv[MAX_ARGS + 1];
    static char *envp[1];

    __cpm_version = __cpm_bdos(BDOS_VERSION, 0);
    init_heap();
    /* Read the command tail before anything uses the DMA buffer at 80h. */
    int argc = init_args(argv);
    environ = envp;
#ifdef __INIT_FINI_ARRAY
    __libc_init_array();
#endif
    exit(main(argc, argv, environ));
}

void *
sbrk(ptrdiff_t incr)
{
    char *old = __cpm_brk;
    if (incr<0 ? (size_t)-incr > (size_t)(old - __cpm_heap_start) : (size_t)incr>(size_t)(
            __cpm_heap_end - old)) {
        errno = ENOMEM;
        return (void *)-1;
    }
    __cpm_brk = old + incr;
    return old;
}

/* CP/M-86 Plus and CP/M 3 keep a return code for the program, which a
 * submit file can test; CP/M-86 1.1 and CP/M 2.2 ignore the call. As in CP/M 3, 0 to FEFFh is
 * success and FF00h to FFFEh failure, so a failing status becomes
 * FF00h | status. */
void
_exit(int status)
{
    /* CP/M doesn't close files when a program ends, and a file written
     * but not closed loses its last extent's directory entry. */
    for (int fd = 3; fd < __CPM_MAX_FD; fd++)
        if (__cpm_fds[fd])
            close(fd);
    __cpm_bdos(BDOS_RETURN_CODE, status == 0 ? 0 : 0xff00 | (status & 0xff));
    for (;;)
        __cpm_bdos(BDOS_RESET, 0);
}
