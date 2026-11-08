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
#include <sys/lock.h>

/*
 * The C library's locks for 32-bit OS/2, on mutex semaphores. These take
 * the place of the do-nothing ones in libc/misc/lock.c.
 */

/* The mutex semaphore of *h, made the first time it is wanted. */
unsigned long
__os2_mutex_of(volatile unsigned long *h)
{
    unsigned long sem = __atomic_load_n(h, __ATOMIC_ACQUIRE);
    if (sem)
        return sem;
    if (DosCreateMutexSem(NULL, &sem, 0, 0))
        return 0;
    unsigned long none = 0;
    if (!__atomic_compare_exchange_n(h, &none, sem, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        DosCloseMutexSem(sem);
        sem = none;
    }
    return sem;
}

#ifndef __SINGLE_THREAD

struct __lock {
    volatile unsigned long sem;
};

struct __lock __lock___libc_recursive_mutex;

void
__retarget_lock_init(_LOCK_T *lock)
{
    *lock = calloc(1, sizeof(struct __lock));
}

void
__retarget_lock_init_recursive(_LOCK_T *lock)
{
    __retarget_lock_init(lock);
}

void
__retarget_lock_close(_LOCK_T lock)
{
    if (lock && lock != &__lock___libc_recursive_mutex) {
        if (lock->sem)
            DosCloseMutexSem(lock->sem);
        free(lock);
    }
}

void
__retarget_lock_close_recursive(_LOCK_T lock)
{
    __retarget_lock_close(lock);
}

void __no_thread_safety_analysis
__retarget_lock_acquire(_LOCK_T lock)
{
    if (lock) {
        unsigned long sem = __os2_mutex_of(&lock->sem);
        if (sem)
            DosRequestMutexSem(sem, __OS2_SEM_INDEFINITE_WAIT);
    }
}

void __no_thread_safety_analysis
__retarget_lock_acquire_recursive(_LOCK_T lock)
{
    __retarget_lock_acquire(lock);
}

void __no_thread_safety_analysis
__retarget_lock_release(_LOCK_T lock)
{
    if (lock && lock->sem)
        DosReleaseMutexSem(lock->sem);
}

void __no_thread_safety_analysis
__retarget_lock_release_recursive(_LOCK_T lock)
{
    __retarget_lock_release(lock);
}

#endif /* __SINGLE_THREAD */
