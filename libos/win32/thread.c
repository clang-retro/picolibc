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
#include "include/process.h"
#include <stdlib.h>

/*
 * Threads. picolibc keeps its per-thread state (errno and the rest) in
 * native TLS, which Windows sets up for every thread however it was
 * started. When a thread ends, Windows NT runs the TLS callbacks, which
 * run the thread_local destructors and the TlsAlloc key destructors.
 * Windows 9x does not, so on 9x a thread that these functions start runs
 * them itself when it returns or calls _endthreadex or _endthread. A
 * thread that CreateThread starts and that ends with ExitThread does not.
 */

/* Runs the TLS callbacks for DLL_THREAD_DETACH on Windows 9x, in a
 * program. A DLL gets DLL_THREAD_DETACH in its entry point, which runs
 * them (dllstart.c). */
void
__win32_thread_end(void)
{
    if (!__win32_in_dll && __win32_is_9x())
        __win32_tls_callbacks(GetModuleHandleA(NULL), __WIN32_DLL_THREAD_DETACH);
}

/* The start routine of a _beginthreadex thread, and its argument. */
struct thunk_ex {
    unsigned(__stdcall *start)(void *);
    void *arg;
};

/* Runs the start routine of a _beginthreadex thread, then the thread end
 * work. The result is the exit code of the thread. */
static __win32_dword __stdcall
run_ex(void *data)
{
    struct thunk_ex t = *(struct thunk_ex *)data;
    free(data);
    unsigned status = t.start(t.arg);
    __win32_thread_end();
    return status;
}

/* Starts a thread, as CreateThread does, with errno set on error. The
 * result is the thread handle, or 0. */
uintptr_t
_beginthreadex(void *security, unsigned stack, unsigned(__stdcall *start)(void *), void *arg,
               unsigned flags, unsigned *id)
{
    struct thunk_ex *t = malloc(sizeof(*t));
    if (!t) {
        errno = ENOMEM;
        return 0;
    }
    t->start = start;
    t->arg = arg;
    __win32_dword  tid;
    __win32_handle h = CreateThread(security, stack, run_ex, t, flags, &tid);
    if (!h) {
        free(t);
        __win32_error(GetLastError());
        return 0;
    }
    if (id)
        *id = tid;
    return (uintptr_t)h;
}

/* Ends the thread with an exit code, after the thread end work. */
void
_endthreadex(unsigned status)
{
    __win32_thread_end();
    ExitThread(status);
}

/* The start routine of a _beginthread thread, and its argument. */
struct thunk {
    void  (*start)(void *);
    void *arg;
};

/* Runs the start routine of a _beginthread thread, then the thread end
 * work. */
static __win32_dword __stdcall
run(void *data)
{
    struct thunk t = *(struct thunk *)data;
    free(data);
    t.start(t.arg);
    __win32_thread_end();
    return 0;
}

/* Starts a thread whose handle closes at once. The result is the handle,
 * which the caller must not use to wait, or -1. */
uintptr_t
_beginthread(void (*start)(void *), unsigned stack, void *arg)
{
    struct thunk *t = malloc(sizeof(*t));
    if (!t)
        return (uintptr_t)-1;
    t->start = start;
    t->arg = arg;
    __win32_dword  tid;
    __win32_handle h = CreateThread(NULL, stack, run, t, 0, &tid);
    if (!h) {
        free(t);
        __win32_error(GetLastError());
        return (uintptr_t)-1;
    }
    /* The thread's handle is not the caller's to close, as in the
     * Windows C library; this one closes it at once. */
    CloseHandle(h);
    return (uintptr_t)h;
}

/* Ends the thread, after the thread end work. */
void
_endthread(void)
{
    __win32_thread_end();
    ExitThread(0);
}
