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

/* The DPMI functions of <dpmi.h>, through INT 31h. */

#include "../local-msdos.h"
#include <dpmi.h>

unsigned short __dpmi_error;

/* Calls INT 31h with *r. Gives 0, or -1 with the error code (AX) in
 * __dpmi_error. */
static int
call(struct __msdos_regs *r)
{
    if (__dpmi_int31(r)) {
        __dpmi_error = (unsigned short)r->ax;
        return -1;
    }
    return 0;
}

/* Calls a real-mode interrupt with the registers of *regs (0300h). */
int
__dpmi_simulate_real_mode_interrupt(int vector, __dpmi_regs *regs)
{
    struct __msdos_regs r = { .ax = 0x0300, .bx = (uint8_t)vector, .di = (uintptr_t)regs };
    return call(&r);
}

/* The same as __dpmi_simulate_real_mode_interrupt. */
int
__dpmi_int(int vector, __dpmi_regs *regs)
{
    return __dpmi_simulate_real_mode_interrupt(vector, regs);
}

/* Allocates DOS memory (0100h). Gives the segment, or -1. */
int
__dpmi_allocate_dos_memory(int paragraphs, int *selector)
{
    struct __msdos_regs r = { .ax = 0x0100, .bx = (uint16_t)paragraphs };
    if (call(&r))
        return -1;
    if (selector)
        *selector = (uint16_t)r.dx;
    return (uint16_t)r.ax;
}

/* Frees DOS memory (0101h). */
int
__dpmi_free_dos_memory(int selector)
{
    struct __msdos_regs r = { .ax = 0x0101, .dx = (uint16_t)selector };
    return call(&r);
}

/* Allocates memory (0501h): the size in BX:CX, the address back in BX:CX
 * and the handle in SI:DI. */
int
__dpmi_allocate_memory(__dpmi_meminfo *info)
{
    struct __msdos_regs r = { .ax = 0x0501, .bx = info->size >> 16, .cx = info->size & 0xffff };
    if (call(&r))
        return -1;
    info->address = ((r.bx & 0xffff) << 16) | (r.cx & 0xffff);
    info->handle = ((r.si & 0xffff) << 16) | (r.di & 0xffff);
    return 0;
}

/* Frees memory (0502h): the handle in SI:DI. */
int
__dpmi_free_memory(unsigned long handle)
{
    struct __msdos_regs r = { .ax = 0x0502, .si = handle >> 16, .di = handle & 0xffff };
    return call(&r);
}

/* Gives the free memory (0500h) in the 48 bytes at ES:EDI. */
int
__dpmi_get_free_memory_information(__dpmi_free_mem_info *info)
{
    struct __msdos_regs r = { .ax = 0x0500, .di = (uintptr_t)info };
    return call(&r);
}

/* Maps physical memory (0800h): the address in BX:CX and the size in
 * SI:DI, the linear address back in BX:CX. */
int
__dpmi_physical_address_mapping(__dpmi_meminfo *info)
{
    struct __msdos_regs r = { .ax = 0x0800,
                              .bx = info->address >> 16,
                              .cx = info->address & 0xffff,
                              .si = info->size >> 16,
                              .di = info->size & 0xffff };
    if (call(&r))
        return -1;
    info->address = ((r.bx & 0xffff) << 16) | (r.cx & 0xffff);
    return 0;
}

/* Frees a physical mapping (0801h): the linear address in BX:CX. */
int
__dpmi_free_physical_address_mapping(__dpmi_meminfo *info)
{
    struct __msdos_regs r
        = { .ax = 0x0801, .bx = info->address >> 16, .cx = info->address & 0xffff };
    return call(&r);
}

/* Locks (0600h) or unlocks (0601h) a region: the address in BX:CX and
 * the size in SI:DI. */
static int
lock(unsigned ax, __dpmi_meminfo *info)
{
    struct __msdos_regs r = { .ax = ax,
                              .bx = info->address >> 16,
                              .cx = info->address & 0xffff,
                              .si = info->size >> 16,
                              .di = info->size & 0xffff };
    return call(&r);
}

/* Locks a linear region (0600h). */
int
__dpmi_lock_linear_region(__dpmi_meminfo *info)
{
    return lock(0x0600, info);
}

/* Unlocks a linear region (0601h). */
int
__dpmi_unlock_linear_region(__dpmi_meminfo *info)
{
    return lock(0x0601, info);
}

/* Gives the base of a selector (0006h): back in CX:DX. */
int
__dpmi_get_segment_base_address(int selector, unsigned long *addr)
{
    struct __msdos_regs r = { .ax = 0x0006, .bx = (uint16_t)selector };
    if (call(&r))
        return -1;
    *addr = ((r.cx & 0xffff) << 16) | (r.dx & 0xffff);
    return 0;
}

/* Gets a protected-mode vector (0204h): back in CX:EDX. */
int
__dpmi_get_protected_mode_interrupt_vector(int vector, __dpmi_paddr *address)
{
    struct __msdos_regs r = { .ax = 0x0204, .bx = (uint8_t)vector };
    if (call(&r))
        return -1;
    address->selector = (unsigned short)r.cx;
    address->offset32 = r.dx;
    return 0;
}

/* Sets a protected-mode vector (0205h): CX:EDX. */
int
__dpmi_set_protected_mode_interrupt_vector(int vector, __dpmi_paddr *address)
{
    struct __msdos_regs r = {
        .ax = 0x0205, .bx = (uint8_t)vector, .cx = address->selector, .dx = address->offset32
    };
    return call(&r);
}

/* Gets a real-mode vector (0200h): back in CX:DX. */
int
__dpmi_get_real_mode_interrupt_vector(int vector, __dpmi_paddr *address)
{
    struct __msdos_regs r = { .ax = 0x0200, .bx = (uint8_t)vector };
    if (call(&r))
        return -1;
    address->selector = (unsigned short)r.cx;
    address->offset32 = r.dx & 0xffff;
    return 0;
}

/* Sets a real-mode vector (0201h): CX:DX. */
int
__dpmi_set_real_mode_interrupt_vector(int vector, __dpmi_paddr *address)
{
    struct __msdos_regs r = {
        .ax = 0x0201, .bx = (uint8_t)vector, .cx = address->selector, .dx = address->offset32 & 0xffff
    };
    return call(&r);
}

/* Gives the DPMI version (0400h): AH major, AL minor. */
int
__dpmi_get_version(void)
{
    struct __msdos_regs r = { .ax = 0x0400 };
    if (call(&r))
        return -1;
    return r.ax & 0xffff;
}
