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
 * The atomic operations that a 386 cannot do in one instruction: 64-bit
 * values, and values of other sizes through the generic calls. clang
 * calls these functions for them.
 *
 * The threads of the library are cooperative (thread.c), so no other
 * thread runs during a call, and a plain operation is atomic. Do not use
 * these operations on a value that an interrupt handler also changes.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* The generic calls have the names of builtins, so the C functions get
   other names here. */
#pragma redefine_extname __atomic_load_c __atomic_load
#pragma redefine_extname __atomic_store_c __atomic_store
#pragma redefine_extname __atomic_exchange_c __atomic_exchange
#pragma redefine_extname __atomic_compare_exchange_c __atomic_compare_exchange

/* Gives the 64-bit value at p. */
uint64_t
__atomic_load_8(const volatile void *p, int order)
{
    uint64_t v = *(const volatile uint64_t *)p;
    (void)order;
    return v;
}

/* Writes the 64-bit value v at p. */
void
__atomic_store_8(volatile void *p, uint64_t v, int order)
{
    (void)order;
    *(volatile uint64_t *)p = v;
}

/* Writes v at p and gives the earlier value. */
uint64_t
__atomic_exchange_8(volatile void *p, uint64_t v, int order)
{
    uint64_t old = *(volatile uint64_t *)p;
    (void)order;
    *(volatile uint64_t *)p = v;
    return old;
}

/* Writes desired at p if p holds *expected, and tells if it did. Else
   writes the value at p to *expected. */
bool
__atomic_compare_exchange_8(volatile void *p, void *expected, uint64_t desired, bool weak,
                            int success, int failure)
{
    uint64_t old = *(volatile uint64_t *)p;
    bool     same = old == *(uint64_t *)expected;
    (void)weak;
    (void)success;
    (void)failure;
    if (same)
        *(volatile uint64_t *)p = desired;
    else
        *(uint64_t *)expected = old;
    return same;
}

/* Defines __atomic_fetch_NAME_8: applies OP with v to the value at p and
   gives the earlier value. */
#define FETCH_OP(name, expr)                                                    \
    uint64_t __atomic_fetch_##name##_8(volatile void *p, uint64_t v, int order) \
    {                                                                           \
        uint64_t old = *(volatile uint64_t *)p;                                 \
        (void)order;                                                            \
        *(volatile uint64_t *)p = (expr);                                       \
        return old;                                                             \
    }

FETCH_OP(add, old + v)
FETCH_OP(sub, old - v)
FETCH_OP(and, old &v)
FETCH_OP(or, old | v)
FETCH_OP(xor, old ^ v)
FETCH_OP(nand, ~(old &v))

/* The generic calls, for objects of any size. */

/* Copies size bytes from src to dest. */
void
__atomic_load_c(size_t size, const volatile void *src, void *dest, int order)
{
    (void)order;
    memcpy(dest, (const void *)src, size);
}

/* Copies size bytes from src to dest. */
void
__atomic_store_c(size_t size, volatile void *dest, void *src, int order)
{
    (void)order;
    memcpy((void *)dest, src, size);
}

/* Writes val at p and copies the earlier bytes to old. */
void
__atomic_exchange_c(size_t size, volatile void *p, void *val, void *old, int order)
{
    (void)order;
    memcpy(old, (const void *)p, size);
    memcpy((void *)p, val, size);
}

/* Writes desired at p if p holds the bytes at expected, and tells if it
   did. Else copies the bytes at p to expected. */
bool
__atomic_compare_exchange_c(size_t size, volatile void *p, void *expected, void *desired,
                            int success, int failure)
{
    bool same = memcmp((const void *)p, expected, size) == 0;
    (void)success;
    (void)failure;
    if (same)
        memcpy((void *)p, desired, size);
    else
        memcpy(expected, (const void *)p, size);
    return same;
}
