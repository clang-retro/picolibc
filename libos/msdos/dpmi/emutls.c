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

/*
 * Emulated thread-local variables for 32-bit DOS. clang compiles each
 * access to a thread-local variable as a call to __emutls_get_address,
 * because DOS has no thread pointer. Each thread gets its own copy of a
 * variable at its first access, through a key of pthread.h (thread.c).
 * The copies of a thread are released when the thread ends.
 */

#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* The control block that clang makes for each thread-local variable. */
struct __emutls_control {
    size_t size;  /* the size of the variable */
    size_t align; /* its alignment */
    union {
        uintptr_t index; /* 0 before the first access, then the slot + 1 */
        void     *address;
    } object;
    void *value; /* the initial value, or NULL for zeros */
};

/* The copies of one thread: an array of pointers, with its size first. */
struct copies {
    uintptr_t size;
    void     *data[];
};

static pthread_key_t  key;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static uintptr_t      next_index;

/* Releases the copies of a thread when it ends. */
static void
release(void *p)
{
    struct copies *c = p;
    uintptr_t      i;
    for (i = 0; i < c->size; i++)
        free(c->data[i]);
    free(c);
}

/* Makes the key, one time. */
static void
make_key(void)
{
    pthread_key_create(&key, release);
}

/* Makes a copy of the variable, with its initial value. The address is
   aligned to the alignment of the variable. */
static void *
new_copy(struct __emutls_control *control)
{
    size_t align = control->align < sizeof(void *) ? sizeof(void *) : control->align;
    void  *p;
    if (posix_memalign(&p, align, control->size))
        abort();
    if (control->value)
        memcpy(p, control->value, control->size);
    else
        memset(p, 0, control->size);
    return p;
}

/* Gives the address of the current thread's copy of a variable. */
void *
__emutls_get_address(struct __emutls_control *control)
{
    uintptr_t      index = control->object.index;
    struct copies *c;
    pthread_once(&once, make_key);
    if (index == 0)
        index = control->object.index = ++next_index;
    c = pthread_getspecific(key);
    if (!c || c->size < index) {
        uintptr_t      size = index + 16;
        struct copies *n = realloc(c, sizeof *c + size * sizeof(void *));
        if (!n)
            abort();
        memset(&n->data[c ? n->size : 0], 0, (size - (c ? n->size : 0)) * sizeof(void *));
        n->size = size;
        c = n;
        pthread_setspecific(key, c);
    }
    if (!c->data[index - 1])
        c->data[index - 1] = new_copy(control);
    return c->data[index - 1];
}
