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

#ifndef _THREADS_H_
#define _THREADS_H_

#include <sys/cdefs.h>
#include <stdint.h>
#include <time.h>

_BEGIN_STD_C

/*
 * C11 threads, on OS/2's threads and semaphores (32-bit OS/2). A thrd_t
 * names a thread for as long as it can be joined. mtx_t and cnd_t may
 * also be zero-initialized, as libc++ does: they set themselves up on
 * first use.
 */

#if !defined(__cplusplus) && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L)
#define thread_local _Thread_local
#endif
#define TSS_DTOR_ITERATIONS 4

typedef uintptr_t thrd_t;
typedef int       (*thrd_start_t)(void *);
typedef unsigned  tss_t;
typedef void      (*tss_dtor_t)(void *);

/* As <stdlib.h> has it for C23. */
#ifndef __once_flag_defined
#define __once_flag_defined
typedef int once_flag;
#define ONCE_FLAG_INIT 0
#endif

typedef struct {
    volatile unsigned long __sem; /* HMTX, made on first use */
} mtx_t;

typedef struct {
    volatile unsigned long __lock; /* HMTX for the queue of waiters */
    void                  *__head, *__tail;
} cnd_t;

enum { thrd_success, thrd_nomem, thrd_timedout, thrd_busy, thrd_error };
enum { mtx_plain = 0, mtx_recursive = 1, mtx_timed = 2 };

int             thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
thrd_t          thrd_current(void);
int             thrd_detach(thrd_t thr);
int             thrd_equal(thrd_t a, thrd_t b);
__noreturn void thrd_exit(int res);
int             thrd_join(thrd_t thr, int *res);
int             thrd_sleep(const struct timespec *duration, struct timespec *remaining);
void            thrd_yield(void);

int             mtx_init(mtx_t *mtx, int type);
int             mtx_lock(mtx_t *mtx);
int             mtx_timedlock(mtx_t             *__restrict mtx, const struct timespec             *__restrict ts);
int             mtx_trylock(mtx_t *mtx);
int             mtx_unlock(mtx_t *mtx);
void            mtx_destroy(mtx_t *mtx);

void            call_once(once_flag *flag, void (*func)(void));

int             cnd_init(cnd_t *cond);
int             cnd_signal(cnd_t *cond);
int             cnd_broadcast(cnd_t *cond);
int             cnd_wait(cnd_t *cond, mtx_t *mtx);
int             cnd_timedwait(cnd_t             *__restrict cond, mtx_t             *__restrict mtx,
                              const struct timespec             *__restrict ts);
void            cnd_destroy(cnd_t *cond);

int             tss_create(tss_t *key, tss_dtor_t dtor);
void           *tss_get(tss_t key);
int             tss_set(tss_t key, void *val);
void            tss_delete(tss_t key);

_END_STD_C

#endif /* _THREADS_H_ */
