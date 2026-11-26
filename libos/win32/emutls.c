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

#include "local-win32.h"
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Emulated thread-local storage, for code built with -femulated-tls.
 * clang turns each access to a thread-local variable into a call to
 * __emutls_get_address with the variable's control block. Each variable
 * gets an index the first time it is used; each thread keeps an array
 * of its objects, by index, in one TlsAlloc slot. This works in a DLL
 * that LoadLibrary loads on every Windows version, where native TLS
 * works only from Vista on.
 *
 * compiler-rt has its own __emutls_get_address, but it calls the C
 * runtime of mingw-w64. libc comes before the builtins on the link line,
 * so this one takes its place. When a thread ends, a TLS callback frees
 * the thread's objects, after the thread_local and key destructors.
 */

/* The control block that clang makes for each thread-local variable. */
struct __emutls_control {
    size_t size;  /* the size of the variable */
    size_t align; /* its alignment */
    union {
        uintptr_t index;   /* its index, from 1; 0 before its first use */
        void     *address; /* (not used) */
    } object;
    void *value; /* its initial value, or NULL for zero */
};

/* The objects of one thread, by index less 1. */
struct emutls_array {
    uintptr_t size;
    void     *data[];
};

#define TLS_OUT_OF_INDEXES 0xffffffff

static __win32_dword emutls_slot = TLS_OUT_OF_INDEXES;
static uintptr_t     emutls_count;
static volatile long emutls_lock;

void                *__emutls_get_address(struct __emutls_control *control);

/* Takes the lock that guards the indices and the slot. A spin lock
 * needs no setup, so it works before the library is started. */
static void
lock(void)
{
    while (__atomic_exchange_n(&emutls_lock, 1, __ATOMIC_ACQUIRE))
        Sleep(0);
}

/* Releases the lock that lock() took. */
static void
unlock(void)
{
    __atomic_store_n(&emutls_lock, 0, __ATOMIC_RELEASE);
}

/* Returns the index of a variable, and gives it one on its first use.
 * The first index also allocates the TlsAlloc slot. */
static uintptr_t
index_of(struct __emutls_control *control)
{
    uintptr_t index = __atomic_load_n(&control->object.index, __ATOMIC_ACQUIRE);
    if (index)
        return index;
    lock();
    if (emutls_slot == TLS_OUT_OF_INDEXES) {
        emutls_slot = TlsAlloc();
        if (emutls_slot == TLS_OUT_OF_INDEXES)
            abort();
    }
    index = control->object.index;
    if (!index) {
        index = ++emutls_count;
        __atomic_store_n(&control->object.index, index, __ATOMIC_RELEASE);
    }
    unlock();
    return index;
}

/* Returns the array of the calling thread, made large enough for index.
 * TlsGetValue and TlsSetValue change the last error, so the caller's
 * value is kept. */
static struct emutls_array *
array_for(uintptr_t index)
{
    __win32_dword        error = GetLastError();
    struct emutls_array *array = TlsGetValue(emutls_slot);
    if (!array || array->size < index) {
        uintptr_t old = array ? array->size : 0;
        uintptr_t size = old * 2 > index ? old * 2 : index + 16;
        array = realloc(array, sizeof(*array) + size * sizeof(void *));
        if (!array)
            abort();
        memset(array->data + old, 0, (size - old) * sizeof(void *));
        array->size = size;
        if (!TlsSetValue(emutls_slot, array))
            abort();
    }
    SetLastError(error);
    return array;
}

/* Returns the address of the calling thread's copy of a thread-local
 * variable. The first call in a thread makes the copy, with the
 * variable's initial value. */
void *
__emutls_get_address(struct __emutls_control *control)
{
    uintptr_t            index = index_of(control);
    struct emutls_array *array = array_for(index);
    void                *object = array->data[index - 1];
    if (!object) {
        size_t align = control->align > sizeof(void *) ? control->align : sizeof(void *);
        object = memalign(align, control->size ? control->size : 1);
        if (!object)
            abort();
        if (control->value)
            memcpy(object, control->value, control->size);
        else
            memset(object, 0, control->size);
        array->data[index - 1] = object;
    }
    return object;
}

/* Frees the objects of the calling thread when it ends, or when the
 * module is unloaded or the program exits. */
static void __attribute__((__stdcall__))
emutls_detach(void *module, __win32_dword reason, void *reserved)
{
    (void)module;
    (void)reserved;
    if (reason != __WIN32_DLL_THREAD_DETACH && reason != __WIN32_DLL_PROCESS_DETACH)
        return;
    if (emutls_slot == TLS_OUT_OF_INDEXES)
        return;
    struct emutls_array *array = TlsGetValue(emutls_slot);
    if (!array)
        return;
    TlsSetValue(emutls_slot, NULL);
    for (uintptr_t i = 0; i < array->size; i++)
        free(array->data[i]);
    free(array);
}

/* After the thread_local destructors (.CRT$XLC) and the TlsAlloc key
 * destructors (.CRT$XLD), which can still use thread-local variables. */
static void(__attribute__((__stdcall__)) * const emutls_callback)(void *, __win32_dword, void *)
    __attribute__((__section__(".CRT$XLY"), __used__))
    = emutls_detach;
