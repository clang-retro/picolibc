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
#include "include/threads.h"
#include <stdlib.h>
#include <string.h>

/*
 * Threads for 32-bit OS/2: C11 <threads.h> on DosCreateThread and the
 * mutex and event semaphores, and the thread-local storage clang's
 * emulated TLS asks for (__emutls_get_address), which is where errno and
 * the C library's other per-thread state live.
 *
 * Each thread's state is a struct thread, found through a word of OS/2's
 * thread-local memory. Threads started some other way get one on first
 * use.
 */

#define ERROR_TOO_MANY_POSTS 298
#define ERROR_ALREADY_POSTED 299
#define ERROR_ALREADY_RESET  300
#define STACK_SPARSE         2
#define EXIT_THREAD          0
#define TSS_KEYS             64
#define THREAD_STACK         (1024 * 1024)

#define DONE                 1
#define DETACHED             2

struct dtor {
    struct dtor *next;
    void         (*func)(void *);
    void        *obj;
};

struct thread {
    thrd_start_t  func;
    void         *arg;
    int           res;
    volatile int  state;  /* DONE, DETACHED */
    unsigned long done;   /* HEV posted at the end, for thrd_join */
    unsigned long wait;   /* HEV this thread waits on in cnd_wait */
    void        **emutls; /* its __thread objects, by index */
    size_t        nemutls;
    void         *tss[TSS_KEYS];
    struct dtor  *dtors; /* C++ thread_local objects, newest first */
};

static unsigned long *slot; /* the thread-local word */

static void
init_slot(void)
{
    static volatile int once;
    /* Before any thread is started: no race. */
    if (!once) {
        DosAllocThreadLocalMemory(1, &slot);
        once = 1;
    }
}

static struct thread *
self(void)
{
    if (!slot)
        init_slot();
    struct thread *t = (struct thread *)*slot;
    if (!t) {
        /* The first thread, or one not started by thrd_create: it is not
           joined, and its state stays. */
        t = calloc(1, sizeof(*t));
        if (!t)
            abort();
        t->state = DETACHED;
        *slot = (unsigned long)t;
    }
    return t;
}

/* --- __thread variables (clang's emulated TLS) --- */

struct __emutls_control {
    size_t size, align;
    union {
        uintptr_t index;
        void     *address;
    } object;
    void *value; /* the initial value, or NULL for zeros */
};

static uintptr_t emutls_count;

void *
__emutls_get_address(struct __emutls_control *c)
{
    uintptr_t index = __atomic_load_n(&c->object.index, __ATOMIC_ACQUIRE);
    if (!index) {
        DosEnterCritSec();
        index = c->object.index;
        if (!index) {
            index = ++emutls_count;
            __atomic_store_n(&c->object.index, index, __ATOMIC_RELEASE);
        }
        DosExitCritSec();
    }
    struct thread *t = self();
    if (index > t->nemutls) {
        size_t n = index + 16;
        void **a = realloc(t->emutls, n * sizeof(void *));
        if (!a)
            abort();
        memset(a + t->nemutls, 0, (n - t->nemutls) * sizeof(void *));
        t->emutls = a;
        t->nemutls = n;
    }
    void **p = &t->emutls[index - 1];
    if (!*p) {
        /* The object, aligned, with what malloc returned before it. */
        size_t align = c->align < sizeof(void *) ? sizeof(void *) : c->align;
        char  *raw = malloc(c->size + align + sizeof(void *));
        if (!raw)
            abort();
        char *obj = (char *)(((uintptr_t)raw + sizeof(void *) + align - 1) & ~(align - 1));
        ((void **)obj)[-1] = raw;
        if (c->value)
            memcpy(obj, c->value, c->size);
        else
            memset(obj, 0, c->size);
        *p = obj;
    }
    return *p;
}

/* --- Destructors of C++ thread_local objects --- */

int __cxa_atexit(void (*func)(void *), void *arg, void *dso);
int __cxa_thread_atexit_impl(void (*func)(void *), void *obj, void *dso);

/* Called by libc++abi for each thread_local object it has made. A thread
   that thrd_create started runs them when it ends; the first thread, and
   threads started some other way, run them at exit. */
int
__cxa_thread_atexit_impl(void (*func)(void *), void *obj, void *dso)
{
    struct thread *t = self();
    if (!t->func)
        return __cxa_atexit(func, obj, dso);
    struct dtor *d = malloc(sizeof(*d));
    if (!d)
        return -1;
    d->func = func;
    d->obj = obj;
    d->next = t->dtors;
    t->dtors = d;
    return 0;
}

/* --- Thread-specific storage --- */

static tss_dtor_t   tss_dtors[TSS_KEYS];
static volatile int tss_used[TSS_KEYS];

int
tss_create(tss_t *key, tss_dtor_t dtor)
{
    for (unsigned i = 0; i < TSS_KEYS; i++)
        if (!__atomic_exchange_n(&tss_used[i], 1, __ATOMIC_ACQ_REL)) {
            tss_dtors[i] = dtor;
            *key = i;
            return thrd_success;
        }
    return thrd_error;
}

void *
tss_get(tss_t key)
{
    return key < TSS_KEYS ? self()->tss[key] : NULL;
}

int
tss_set(tss_t key, void *val)
{
    if (key >= TSS_KEYS)
        return thrd_error;
    self()->tss[key] = val;
    return thrd_success;
}

void
tss_delete(tss_t key)
{
    if (key < TSS_KEYS) {
        tss_dtors[key] = NULL;
        __atomic_store_n(&tss_used[key], 0, __ATOMIC_RELEASE);
    }
}

/* --- Threads --- */

static void
free_thread(struct thread *t)
{
    if (t->done)
        DosCloseEventSem(t->done);
    free(t);
}

/* The end of a thread: the destructors of its thread-specific storage and
   its __thread objects, then thrd_join may go on. */
static void
finish(struct thread *t)
{
    /* thread_local objects first: their destructors may use tss. */
    while (t->dtors) {
        struct dtor *d = t->dtors;
        t->dtors = d->next;
        d->func(d->obj);
        free(d);
    }
    for (int round = 0; round < TSS_DTOR_ITERATIONS; round++) {
        int any = 0;
        for (unsigned i = 0; i < TSS_KEYS; i++) {
            void *v = t->tss[i];
            if (v && tss_dtors[i]) {
                t->tss[i] = NULL;
                tss_dtors[i](v);
                any = 1;
            }
        }
        if (!any)
            break;
    }
    if (t->wait)
        DosCloseEventSem(t->wait);
    void **emutls = t->emutls;
    size_t n = t->nemutls;
    *slot = 0;
    for (size_t i = 0; i < n; i++)
        if (emutls[i])
            free(((void **)emutls[i])[-1]);
    free(emutls);
    unsigned long done = t->done;
    if (__atomic_fetch_or(&t->state, DONE, __ATOMIC_ACQ_REL) & DETACHED)
        free_thread(t);
    else
        DosPostEventSem(done); /* t is the joiner's now */
}

static void
start(unsigned long arg)
{
    struct thread *t = (struct thread *)arg;
    *slot = arg;
    t->res = t->func(t->arg);
    finish(t);
}

int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
    self(); /* the slot, and the creating thread's state */
    struct thread *t = calloc(1, sizeof(*t));
    if (!t)
        return thrd_nomem;
    t->func = func;
    t->arg = arg;
    unsigned long tid;
    if (DosCreateEventSem(NULL, &t->done, 0, 0)
        || DosCreateThread(&tid, start, (unsigned long)t, STACK_SPARSE, THREAD_STACK)) {
        free_thread(t);
        return thrd_error;
    }
    *thr = (thrd_t)t;
    return thrd_success;
}

thrd_t
thrd_current(void)
{
    return (thrd_t)self();
}

int
thrd_equal(thrd_t a, thrd_t b)
{
    return a == b;
}

int
thrd_join(thrd_t thr, int *res)
{
    struct thread *t = (struct thread *)thr;
    if (!t || (t->state & DETACHED))
        return thrd_error;
    if (DosWaitEventSem(t->done, __OS2_SEM_INDEFINITE_WAIT))
        return thrd_error;
    if (res)
        *res = t->res;
    free_thread(t);
    return thrd_success;
}

int
thrd_detach(thrd_t thr)
{
    struct thread *t = (struct thread *)thr;
    if (!t)
        return thrd_error;
    if (__atomic_fetch_or(&t->state, DETACHED, __ATOMIC_ACQ_REL) & DONE)
        free_thread(t);
    return thrd_success;
}

void
thrd_exit(int res)
{
    struct thread *t = self();
    t->res = res;
    finish(t);
    DosExit(EXIT_THREAD, 0);
    for (;;)
        ;
}

/* Milliseconds from a duration, rounded up. */
static unsigned long
ms_of(const struct timespec *ts)
{
    if (ts->tv_sec < 0 || (ts->tv_sec == 0 && ts->tv_nsec <= 0))
        return 0;
    unsigned long long ms
        = (unsigned long long)ts->tv_sec * 1000 + (ts->tv_nsec + 999999) / 1000000;
    return ms >= __OS2_SEM_INDEFINITE_WAIT ? __OS2_SEM_INDEFINITE_WAIT - 1 : (unsigned long)ms;
}

/* Milliseconds until an absolute TIME_UTC time. */
static unsigned long
ms_until(const struct timespec *ts)
{
    struct timespec now, d;
    timespec_get(&now, TIME_UTC);
    d.tv_sec = ts->tv_sec - now.tv_sec;
    d.tv_nsec = ts->tv_nsec - now.tv_nsec;
    if (d.tv_nsec < 0) {
        d.tv_nsec += 1000000000;
        d.tv_sec--;
    }
    return ms_of(&d);
}

int
thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
    DosSleep(ms_of(duration));
    if (remaining)
        remaining->tv_sec = remaining->tv_nsec = 0;
    return 0;
}

void
thrd_yield(void)
{
    DosSleep(0);
}

/* --- Mutexes: OS/2 mutex semaphores, which a thread may take again --- */

int
mtx_init(mtx_t *mtx, int type)
{
    (void)type;
    mtx->__sem = 0;
    return __os2_mutex_of(&mtx->__sem) ? thrd_success : thrd_error;
}

static int
request(mtx_t *mtx, unsigned long ms)
{
    unsigned long sem = __os2_mutex_of(&mtx->__sem);
    if (!sem)
        return thrd_error;
    __os2_ret r = DosRequestMutexSem(sem, ms);
    return r == 0                  ? thrd_success
        : r == __OS2_ERROR_TIMEOUT ? (ms ? thrd_timedout : thrd_busy)
                                   : thrd_error;
}

int
mtx_lock(mtx_t *mtx)
{
    return request(mtx, __OS2_SEM_INDEFINITE_WAIT);
}

int
mtx_trylock(mtx_t *mtx)
{
    return request(mtx, __OS2_SEM_IMMEDIATE_RETURN);
}

int
mtx_timedlock(mtx_t * __restrict mtx, const struct timespec * __restrict ts)
{
    int r = request(mtx, ms_until(ts));
    return r == thrd_busy ? thrd_timedout : r;
}

int
mtx_unlock(mtx_t *mtx)
{
    return mtx->__sem && DosReleaseMutexSem(mtx->__sem) == 0 ? thrd_success : thrd_error;
}

void
mtx_destroy(mtx_t *mtx)
{
    if (mtx->__sem)
        DosCloseMutexSem(mtx->__sem);
    mtx->__sem = 0;
}

/* --- Condition variables: a queue of waiting threads, each woken by its
   own event semaphore --- */

struct waiter {
    struct waiter *next;
    unsigned long  ev;
    int            queued;
};

int
cnd_init(cnd_t *cond)
{
    cond->__lock = 0;
    cond->__head = cond->__tail = NULL;
    return __os2_mutex_of(&cond->__lock) ? thrd_success : thrd_error;
}

static int
wake(cnd_t *cond, int all)
{
    unsigned long lock = __os2_mutex_of(&cond->__lock);
    if (!lock)
        return thrd_error;
    DosRequestMutexSem(lock, __OS2_SEM_INDEFINITE_WAIT);
    struct waiter *w;
    while ((w = cond->__head) != NULL) {
        cond->__head = w->next;
        w->queued = 0;
        DosPostEventSem(w->ev);
        if (!all)
            break;
    }
    if (!cond->__head)
        cond->__tail = NULL;
    DosReleaseMutexSem(lock);
    return thrd_success;
}

int
cnd_signal(cnd_t *cond)
{
    return wake(cond, 0);
}

int
cnd_broadcast(cnd_t *cond)
{
    return wake(cond, 1);
}

static int
wait_for(cnd_t *cond, mtx_t *mtx, const struct timespec *ts)
{
    struct thread *t = self();
    unsigned long  lock = __os2_mutex_of(&cond->__lock);
    unsigned long  count;
    if (!lock || (!t->wait && DosCreateEventSem(NULL, &t->wait, 0, 0)))
        return thrd_error;
    DosResetEventSem(t->wait, &count);
    struct waiter w = { NULL, t->wait, 1 };

    DosRequestMutexSem(lock, __OS2_SEM_INDEFINITE_WAIT);
    if (cond->__tail)
        ((struct waiter *)cond->__tail)->next = &w;
    else
        cond->__head = &w;
    cond->__tail = &w;
    DosReleaseMutexSem(lock);

    mtx_unlock(mtx);
    DosWaitEventSem(w.ev, ts ? ms_until(ts) : __OS2_SEM_INDEFINITE_WAIT);

    /* Still queued: the time ran out before a signal. */
    DosRequestMutexSem(lock, __OS2_SEM_INDEFINITE_WAIT);
    int timedout = w.queued;
    if (w.queued) {
        struct waiter *prev = NULL;
        for (struct waiter *p = cond->__head; p; prev = p, p = p->next)
            if (p == &w) {
                if (prev)
                    prev->next = w.next;
                else
                    cond->__head = w.next;
                if (cond->__tail == &w)
                    cond->__tail = prev;
                break;
            }
    }
    DosReleaseMutexSem(lock);
    mtx_lock(mtx);
    return timedout ? thrd_timedout : thrd_success;
}

int
cnd_wait(cnd_t *cond, mtx_t *mtx)
{
    return wait_for(cond, mtx, NULL);
}

int
cnd_timedwait(cnd_t * __restrict cond, mtx_t * __restrict mtx,
              const struct timespec * __restrict ts)
{
    return wait_for(cond, mtx, ts);
}

void
cnd_destroy(cnd_t *cond)
{
    if (cond->__lock)
        DosCloseMutexSem(cond->__lock);
    cond->__lock = 0;
}
