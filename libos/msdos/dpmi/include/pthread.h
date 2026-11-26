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
 * pthread.h - POSIX threads for 32-bit DOS programs (i386-pc-msdos).
 *
 * DOS has no threads, so the library switches the threads itself. The
 * threads are cooperative: a thread runs until it calls a function that
 * waits (pthread_join, pthread_mutex_lock on a locked mutex,
 * pthread_cond_wait, sched_yield, sleep, usleep or nanosleep). Then the
 * next thread runs. A thread that does not wait stops all other threads.
 */

#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <sched.h>
#include <stddef.h>
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __dpmi_thread *pthread_t;

typedef struct {
    int    __detach;    /* PTHREAD_CREATE_JOINABLE or PTHREAD_CREATE_DETACHED */
    size_t __stacksize; /* 0: the default, 64 KiB */
} pthread_attr_t;

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

typedef struct {
    int       __type;  /* a PTHREAD_MUTEX_ value */
    unsigned  __count; /* how many times the owner locked it */
    pthread_t __owner; /* NULL when not locked */
} pthread_mutex_t;

typedef struct {
    int __type;
} pthread_mutexattr_t;

#define PTHREAD_MUTEX_NORMAL                   0
#define PTHREAD_MUTEX_RECURSIVE                1
#define PTHREAD_MUTEX_ERRORCHECK               2
#define PTHREAD_MUTEX_DEFAULT                  PTHREAD_MUTEX_NORMAL

#define PTHREAD_MUTEX_INITIALIZER              { PTHREAD_MUTEX_NORMAL, 0, 0 }
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP { PTHREAD_MUTEX_RECURSIVE, 0, 0 }

typedef struct {
    unsigned long __seq; /* changes at each signal or broadcast */
} pthread_cond_t;

typedef struct {
    int __reserved;
} pthread_condattr_t;

#define PTHREAD_COND_INITIALIZER { 0 }

typedef unsigned pthread_key_t;

#define PTHREAD_KEYS_MAX              64
#define PTHREAD_DESTRUCTOR_ITERATIONS 4

typedef struct {
    int __done; /* 0: not run, 1: running, 2: done */
} pthread_once_t;

#define PTHREAD_ONCE_INIT { 0 }

int       pthread_attr_init(pthread_attr_t *attr);
int       pthread_attr_destroy(pthread_attr_t *attr);
int       pthread_attr_setdetachstate(pthread_attr_t *attr, int state);
int       pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state);
int       pthread_attr_setstacksize(pthread_attr_t *attr, size_t size);
int       pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *size);

int       pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *),
                         void *arg);
int       pthread_join(pthread_t thread, void **result);
int       pthread_detach(pthread_t thread);
void      pthread_exit(void *result) __attribute__((__noreturn__));
pthread_t pthread_self(void);
/* Cancellation is not available: gives ENOTSUP. */
int       pthread_cancel(pthread_t thread);
int       pthread_equal(pthread_t a, pthread_t b);

int       pthread_mutexattr_init(pthread_mutexattr_t *attr);
int       pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int       pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int       pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int       pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int       pthread_mutex_destroy(pthread_mutex_t *mutex);
int       pthread_mutex_lock(pthread_mutex_t *mutex);
int       pthread_mutex_trylock(pthread_mutex_t *mutex);
int       pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime);
int       pthread_mutex_unlock(pthread_mutex_t *mutex);

int       pthread_condattr_init(pthread_condattr_t *attr);
int       pthread_condattr_destroy(pthread_condattr_t *attr);
int       pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int       pthread_cond_destroy(pthread_cond_t *cond);
int       pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int       pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                                 const struct timespec *abstime);
int       pthread_cond_signal(pthread_cond_t *cond);
int       pthread_cond_broadcast(pthread_cond_t *cond);

int       pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int       pthread_key_delete(pthread_key_t key);
void     *pthread_getspecific(pthread_key_t key);
int       pthread_setspecific(pthread_key_t key, const void *value);

int       pthread_once(pthread_once_t *once, void (*init)(void));

/* sched.h declares this only with _POSIX_THREADS, which this target does
   not define. */
int       sched_yield(void);

#ifdef __cplusplus
}
#endif

#endif
