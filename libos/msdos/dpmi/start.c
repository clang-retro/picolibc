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

/* The startup of 32-bit DOS programs: the heap, the environment and the
 * command line, then main. crt0 saved the PSP selector. */

#include "../local-msdos.h"
#include <dpmi.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_initfini.h>

uint16_t __msdos_psp;
uint16_t __msdos_psp_sel;
char   **environ;

extern int main(int, char **, char **);

/* Most arguments a command line (at most 126 bytes) can hold. */
#define MAX_ARGS 64

/* The heap. picolibc's malloc needs it in one piece, so it is one block of
 * DPMI memory: the largest free block, less a part for the other users of
 * DPMI memory, up to HEAP_MAX. */
#define HEAP_MAX  (256ul << 20)
#define HEAP_MIN  (64ul << 10)
#define HEAP_KEEP (256ul << 10)

/* Allocates the heap. Without DPMI memory the heap is empty, and malloc
 * fails. */
static void
init_heap(void)
{
    __dpmi_free_mem_info info;
    unsigned long        size = HEAP_MAX;
    if (__dpmi_get_free_memory_information(&info) == 0) {
        unsigned long free = info.largest_available_free_block_in_bytes;
        unsigned long keep = free / 8 > HEAP_KEEP ? HEAP_KEEP : free / 8;
        size = free - keep;
    }
    if (size > HEAP_MAX)
        size = HEAP_MAX;
    size &= ~0xffful;
    while (size >= HEAP_MIN) {
        __dpmi_meminfo m = { .size = size };
        if (__dpmi_allocate_memory(&m) == 0) {
            __msdos_heap_base = __msdos_brk = (char *)m.address;
            __msdos_heap_end = __msdos_brk + size;
            return;
        }
        size /= 2;
    }
}

/* Gives the flat address of a selector, or NULL. */
static char *
selector_base(uint16_t sel)
{
    unsigned long base;
    if (sel == 0 || __dpmi_get_segment_base_address(sel, &base) < 0)
        return NULL;
    return (char *)base;
}

/* Copies the environment block, whose selector is at PSP:2Ch. It holds
 * NUL-terminated NAME=value strings and an empty string, then a word
 * count and the program's full path, which becomes argv[0]. */
static char *
init_environ(const char *psp)
{
    static char *empty[1];
    environ = empty;
    const char *block = selector_base(*(const uint16_t *)(psp + 0x2c));
    if (block == NULL)
        return NULL;
    size_t off = 0, count = 0;
    while (block[off] != 0 && off < 0x7fff) {
        off += strlen(block + off) + 1;
        count++;
    }
    size_t end = off + 1, path = 0;
    if (*(const uint16_t *)(block + end) >= 1) {
        path = end + 2;
        end = path + strlen(block + path) + 1;
    }
    char  *copy = malloc(end);
    char **env = malloc((count + 1) * sizeof(char *));
    if (copy == NULL || env == NULL)
        return NULL;
    memcpy(copy, block, end);
    char *p = copy;
    for (size_t i = 0; i < count; i++) {
        env[i] = p;
        p += strlen(p) + 1;
    }
    env[count] = NULL;
    environ = env;
    return path ? copy + path : NULL;
}

/* Splits the command tail at PSP:80h into words. Double quotes group
 * words with spaces, and \" is a literal quote, as in the Microsoft C
 * runtime. */
static int
init_args(const char *psp, char **argv)
{
    static char tail[128];
    uint8_t     len = (uint8_t)psp[0x80];
    if (len > 126)
        len = 126;
    memcpy(tail, psp + 0x81, len);
    tail[len] = '\0';

    int argc = 1;
    char *in = tail, *out = tail;
    while (argc < MAX_ARGS) {
        while (*in == ' ' || *in == '\t' || *in == '\r')
            in++;
        if (*in == '\0')
            break;
        argv[argc++] = out;
        int quoted = 0;
        while (*in && *in != '\r' && (quoted || (*in != ' ' && *in != '\t'))) {
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

/* Called by crt0 with the flat segments set up and the PSP selector
 * saved. */
void
__msdos_start(void)
{
    static char *argv[MAX_ARGS + 1];
    const char  *psp = selector_base(__msdos_psp_sel);
    __msdos_psp = (uint16_t)((uintptr_t)psp >> 4);

    init_heap();
    int   argc = psp ? init_args(psp, argv) : 1;
    char *argv0 = psp ? init_environ(psp) : NULL;
    argv[0] = argv0 ? argv0 : (char *)"";
#ifdef __INIT_FINI_ARRAY
    __libc_init_array();
#endif
    exit(main(argc, argv, environ));
}
