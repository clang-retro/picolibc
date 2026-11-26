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
 * POSIX threads (pthread.h) for 32-bit DOS, and the functions that wait:
 * sched_yield and nanosleep.
 *
 * The threads are cooperative. Each thread has its own stack, from
 * malloc. A function that waits gives the processor to the next thread
 * in the ring (__dpmi_switch in switch.S), until the condition it waits
 * for is true. When no other thread can run, it gives the rest of the
 * time slice to the DPMI host or the multitasker (INT 2Fh, AX=1680h).
 * errno is one variable for all threads, so the functions that wait keep
 * its value across a switch.
 */

#define _DEFAULT_SOURCE
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* The stack of a new thread when the attributes give no size. */
#define DEFAULT_STACK (64 * 1024)
/* The smallest stack that pthread_attr_setstacksize accepts. */
#define MIN_STACK (4 * 1024)

/* One thread. The main thread has a static record; each other thread has
   a record from malloc. All threads that are not finished are in a ring,
   in the order that they run. */
struct __dpmi_thread {
    struct __dpmi_thread *next;  /* the next thread in the ring */
    void                 *sp;    /* the saved stack pointer, while it does not run */
    void                 *stack; /* the stack from malloc (NULL for the main thread) */
    void                 *(*start)(void *);
    void                 *arg;
    void                 *result;
    int                   done;     /* the thread has ended */
    int                   detached; /* release the record when the thread ends */
    jmp_buf               exit;     /* pthread_exit jumps here */
    const void           *values[PTHREAD_KEYS_MAX];
};

void                         __dpmi_switch(void **save_sp, void *new_sp);

static struct __dpmi_thread  main_thread = { &main_thread };
static struct __dpmi_thread *current = &main_thread;
/* A thread that has ended and whose stack the next thread releases. */
static struct __dpmi_thread *zombie;
static unsigned char         key_used[PTHREAD_KEYS_MAX];
static void                  (*key_destructor[PTHREAD_KEYS_MAX])(void *);

/* Gives the rest of the time slice to the DPMI host (INT 2Fh, AX=1680h).
   DOSBox, Windows and OS/2 then run other work; plain DOS ignores it. */
static void
idle(void)
{
    __asm__ volatile("int $0x2f" : : "a"(0x1680) : "memory", "cc");
}

/* Releases the stack of a thread that ended, once no thread runs on it. */
static void
reap(void)
{
    struct __dpmi_thread *z = zombie;
    if (z) {
        zombie = NULL;
        free(z->stack);
        z->stack = NULL;
        if (z->detached)
            free(z);
    }
}

/* Gives the processor to the next thread in the ring, and keeps errno.
   With no other thread, gives the time slice to the host. */
static void
yield(void)
{
    struct __dpmi_thread *from = current, *to = from->next;
    int                   saved = errno;
    if (to == from) {
        idle();
    } else {
        current = to;
        __dpmi_switch(&from->sp, to->sp);
        reap();
    }
    errno = saved;
}

/* Tells if the time abstime (CLOCK_REALTIME) has passed. */
static int
passed(const struct timespec *abstime)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec > abstime->tv_sec
        || (now.tv_sec == abstime->tv_sec && now.tv_nsec >= abstime->tv_nsec);
}

/* Calls the key destructors for the values of thread t. */
static void
run_destructors(struct __dpmi_thread *t)
{
    int round, k, again = 1;
    for (round = 0; again && round < PTHREAD_DESTRUCTOR_ITERATIONS; round++) {
        again = 0;
        for (k = 0; k < PTHREAD_KEYS_MAX; k++) {
            void *v = (void *)t->values[k];
            if (key_used[k] && key_destructor[k] && v) {
                t->values[k] = NULL;
                key_destructor[k](v);
                again = 1;
            }
        }
    }
}

/* Takes the current thread out of the ring and runs the next one. The
   next thread releases the stack. Does not return. */
static __attribute__((noreturn)) void
finish(void)
{
    struct __dpmi_thread *t = current, *p = t;
    void                 *unused;
    while (p->next != t)
        p = p->next;
    p->next = t->next;
    t->done = 1;
    zombie = t;
    current = t->next;
    __dpmi_switch(&unused, current->sp);
    __builtin_unreachable();
}

/* The first function of each new thread, on its own stack. */
static __attribute__((noreturn, used)) void
thread_entry(void)
{
    struct __dpmi_thread *t = current;
    reap();
    if (setjmp(t->exit) == 0)
        t->result = t->start(t->arg);
    run_destructors(t);
    finish();
}

/* Tells if t is the record of a thread that is not released. */
static int
known(struct __dpmi_thread *t)
{
    struct __dpmi_thread *p = &main_thread;
    if (!t)
        return 0;
    if (t->done)
        return 1;
    do {
        if (p == t)
            return 1;
        p = p->next;
    } while (p != &main_thread);
    return 0;
}

/* Sets the attributes to their defaults: joinable, default stack. */
int
pthread_attr_init(pthread_attr_t *attr)
{
    attr->__detach = PTHREAD_CREATE_JOINABLE;
    attr->__stacksize = 0;
    return 0;
}

/* Releases the attributes. They hold nothing. */
int
pthread_attr_destroy(pthread_attr_t *attr)
{
    (void)attr;
    return 0;
}

/* Sets whether a new thread is joinable or detached. */
int
pthread_attr_setdetachstate(pthread_attr_t *attr, int state)
{
    if (state != PTHREAD_CREATE_JOINABLE && state != PTHREAD_CREATE_DETACHED)
        return EINVAL;
    attr->__detach = state;
    return 0;
}

/* Gives whether a new thread is joinable or detached. */
int
pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state)
{
    *state = attr->__detach;
    return 0;
}

/* Sets the stack size of a new thread. */
int
pthread_attr_setstacksize(pthread_attr_t *attr, size_t size)
{
    if (size < MIN_STACK)
        return EINVAL;
    attr->__stacksize = size;
    return 0;
}

/* Gives the stack size of a new thread. */
int
pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *size)
{
    *size = attr->__stacksize ? attr->__stacksize : DEFAULT_STACK;
    return 0;
}

/* Makes a thread that runs start(arg). It goes into the ring after the
   current thread, and starts when the current thread waits. */
int
pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg)
{
    size_t                size = attr && attr->__stacksize ? attr->__stacksize : DEFAULT_STACK;
    struct __dpmi_thread *t = calloc(1, sizeof *t);
    uint32_t             *sp;
    if (!t)
        return EAGAIN;
    size = (size + 15) & ~(size_t)15;
    t->stack = malloc(size);
    if (!t->stack) {
        free(t);
        return EAGAIN;
    }
    t->start = start;
    t->arg = arg;
    t->detached = attr && attr->__detach == PTHREAD_CREATE_DETACHED;
    /* The frame that __dpmi_switch pops: EDI, ESI, EBX, EBP, then the
       return address thread_entry. Above it, a return address of 0, so
       that thread_entry starts with a stack aligned to 16 bytes. */
    sp = (uint32_t *)(((uintptr_t)t->stack + size) & ~(uintptr_t)15);
    *--sp = 0;
    *--sp = (uint32_t)(uintptr_t)thread_entry;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    t->sp = sp;
    t->next = current->next;
    current->next = t;
    *thread = t;
    return 0;
}

/* Waits until the thread ends, gives its result and releases it. */
int
pthread_join(pthread_t thread, void **result)
{
    if (!known(thread) || thread->detached)
        return EINVAL;
    if (thread == current)
        return EDEADLK;
    while (!thread->done)
        yield();
    if (zombie == thread)
        reap();
    if (result)
        *result = thread->result;
    free(thread->stack);
    free(thread);
    return 0;
}

/* Lets the thread release itself when it ends. */
int
pthread_detach(pthread_t thread)
{
    if (!known(thread) || thread->detached)
        return EINVAL;
    if (thread->done) {
        if (zombie == thread)
            reap();
        free(thread->stack);
        free(thread);
        return 0;
    }
    thread->detached = 1;
    return 0;
}

/* Ends the current thread. In the main thread, waits for the other
   threads, then ends the program. */
void
pthread_exit(void *result)
{
    if (current == &main_thread) {
        while (main_thread.next != &main_thread)
            yield();
        run_destructors(&main_thread);
        exit(0);
    }
    current->result = result;
    longjmp(current->exit, 1);
}

/* Gives the current thread. */
pthread_t
pthread_self(void)
{
    return current;
}

/* Cancellation is not available. */
int
pthread_cancel(pthread_t thread)
{
    (void)thread;
    return ENOTSUP;
}

/* Tells if two threads are the same. */
int
pthread_equal(pthread_t a, pthread_t b)
{
    return a == b;
}

/* Sets the attributes of a mutex to their default: a normal mutex. */
int
pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    attr->__type = PTHREAD_MUTEX_DEFAULT;
    return 0;
}

/* Releases the attributes of a mutex. They hold nothing. */
int
pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    (void)attr;
    return 0;
}

/* Sets the type of a mutex: normal, recursive or error checking. */
int
pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (type < PTHREAD_MUTEX_NORMAL || type > PTHREAD_MUTEX_ERRORCHECK)
        return EINVAL;
    attr->__type = type;
    return 0;
}

/* Gives the type of a mutex. */
int
pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    *type = attr->__type;
    return 0;
}

/* Makes an unlocked mutex. */
int
pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    mutex->__type = attr ? attr->__type : PTHREAD_MUTEX_DEFAULT;
    mutex->__count = 0;
    mutex->__owner = NULL;
    return 0;
}

/* Releases a mutex. It must not be locked. */
int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    return mutex->__owner ? EBUSY : 0;
}

/* Locks the mutex if no thread has it, or again if it is recursive and
   the current thread has it. Gives EBUSY when another thread has it. */
int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    if (!mutex->__owner) {
        mutex->__owner = current;
        mutex->__count = 1;
        return 0;
    }
    if (mutex->__owner == current) {
        if (mutex->__type == PTHREAD_MUTEX_RECURSIVE) {
            mutex->__count++;
            return 0;
        }
        if (mutex->__type == PTHREAD_MUTEX_ERRORCHECK)
            return EDEADLK;
    }
    return EBUSY;
}

/* Locks the mutex. While another thread has it, the other threads run. */
int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
    int r;
    while ((r = pthread_mutex_trylock(mutex)) == EBUSY)
        yield();
    return r;
}

/* Locks the mutex, or gives ETIMEDOUT at the time abstime. */
int
pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
    int r;
    while ((r = pthread_mutex_trylock(mutex)) == EBUSY) {
        if (passed(abstime))
            return ETIMEDOUT;
        yield();
    }
    return r;
}

/* Unlocks the mutex. The current thread must have it. */
int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (mutex->__owner != current)
        return EPERM;
    if (--mutex->__count == 0)
        mutex->__owner = NULL;
    return 0;
}

/* Sets the attributes of a condition variable. They hold nothing. */
int
pthread_condattr_init(pthread_condattr_t *attr)
{
    attr->__reserved = 0;
    return 0;
}

/* Releases the attributes of a condition variable. */
int
pthread_condattr_destroy(pthread_condattr_t *attr)
{
    (void)attr;
    return 0;
}

/* Makes a condition variable. */
int
pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    (void)attr;
    cond->__seq = 0;
    return 0;
}

/* Releases a condition variable. It holds nothing. */
int
pthread_cond_destroy(pthread_cond_t *cond)
{
    (void)cond;
    return 0;
}

/* Unlocks the mutex, waits for a signal or for the time abstime (when
   not NULL), and locks the mutex again. The mutex keeps its lock count. */
static int
cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    unsigned long seq = cond->__seq;
    unsigned      count = mutex->__count;
    int           r = 0;
    if (mutex->__owner != current)
        return EPERM;
    mutex->__owner = NULL;
    mutex->__count = 0;
    do {
        yield();
        if (abstime && cond->__seq == seq && passed(abstime)) {
            r = ETIMEDOUT;
            break;
        }
    } while (cond->__seq == seq);
    while (mutex->__owner)
        yield();
    mutex->__owner = current;
    mutex->__count = count;
    return r;
}

/* Waits for a signal. */
int
pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    return cond_wait(cond, mutex, NULL);
}

/* Waits for a signal, or gives ETIMEDOUT at the time abstime. */
int
pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    return cond_wait(cond, mutex, abstime);
}

/* Wakes the threads that wait. A thread can wake without a signal, as
   POSIX allows, so a signal wakes all of them. */
int
pthread_cond_signal(pthread_cond_t *cond)
{
    cond->__seq++;
    return 0;
}

/* Wakes all threads that wait. */
int
pthread_cond_broadcast(pthread_cond_t *cond)
{
    cond->__seq++;
    return 0;
}

/* Makes a key for a value of each thread. */
int
pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    unsigned k;
    for (k = 0; k < PTHREAD_KEYS_MAX; k++) {
        if (!key_used[k]) {
            struct __dpmi_thread *p = &main_thread;
            key_used[k] = 1;
            key_destructor[k] = destructor;
            do {
                p->values[k] = NULL;
                p = p->next;
            } while (p != &main_thread);
            *key = k;
            return 0;
        }
    }
    return EAGAIN;
}

/* Releases a key. The destructor does not run. */
int
pthread_key_delete(pthread_key_t key)
{
    if (key >= PTHREAD_KEYS_MAX || !key_used[key])
        return EINVAL;
    key_used[key] = 0;
    key_destructor[key] = NULL;
    return 0;
}

/* Gives the value of the key for the current thread. */
void *
pthread_getspecific(pthread_key_t key)
{
    return key < PTHREAD_KEYS_MAX ? (void *)current->values[key] : NULL;
}

/* Sets the value of the key for the current thread. */
int
pthread_setspecific(pthread_key_t key, const void *value)
{
    if (key >= PTHREAD_KEYS_MAX || !key_used[key])
        return EINVAL;
    current->values[key] = value;
    return 0;
}

/* Calls init one time. A thread that comes while another thread runs
   init waits until init returns. */
int
pthread_once(pthread_once_t *once, void (*init)(void))
{
    if (once->__done == 0) {
        once->__done = 1;
        init();
        once->__done = 2;
    }
    while (once->__done != 2)
        yield();
    return 0;
}

/* Lets the other threads run one time. */
int
sched_yield(void)
{
    yield();
    return 0;
}

/* Waits for the time req. DOS has no sleep call: the threads run while
   the clock passes. */
int
nanosleep(const struct timespec *req, struct timespec *rem)
{
    struct timespec end;
    if (req->tv_nsec < 0 || req->tv_nsec >= 1000000000L || req->tv_sec < 0) {
        errno = EINVAL;
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    end.tv_sec += req->tv_sec;
    end.tv_nsec += req->tv_nsec;
    if (end.tv_nsec >= 1000000000L) {
        end.tv_nsec -= 1000000000L;
        end.tv_sec++;
    }
    while (!passed(&end))
        yield();
    if (rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }
    return 0;
}

/* Waits for usec microseconds, as nanosleep does. */
int
usleep(useconds_t usec)
{
    struct timespec t = { usec / 1000000, (long)(usec % 1000000) * 1000 };
    return nanosleep(&t, NULL);
}
