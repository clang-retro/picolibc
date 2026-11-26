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
 * dpmi.h - the DPMI host of 32-bit DOS programs (i386-pc-msdos), with the
 * names and types of DJGPP. The program runs in flat protected mode: a
 * linear address is also a pointer.
 */

#ifndef _DPMI_H_
#define _DPMI_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The registers of a real-mode call (DPMI function 0300h). */
typedef union {
    struct {
        unsigned long edi, esi, ebp, res, ebx, edx, ecx, eax;
    } d;
    struct {
        unsigned short di, di_hi, si, si_hi, bp, bp_hi, res, res_hi;
        unsigned short bx, bx_hi, dx, dx_hi, cx, cx_hi, ax, ax_hi;
        unsigned short flags, es, ds, fs, gs, ip, cs, sp, ss;
    } x;
    struct {
        unsigned char edi[4], esi[4], ebp[4], res[4];
        unsigned char bl, bh, ebx_b2, ebx_b3, dl, dh, edx_b2, edx_b3;
        unsigned char cl, ch, ecx_b2, ecx_b3, al, ah, eax_b2, eax_b3;
    } h;
} __attribute__((packed)) __dpmi_regs;

/* A block of memory: its handle, its size and its linear address. */
typedef struct {
    unsigned long handle;
    unsigned long size;
    unsigned long address;
} __dpmi_meminfo;

/* The free memory (DPMI function 0500h), in bytes and in pages. */
typedef struct {
    unsigned long largest_available_free_block_in_bytes;
    unsigned long maximum_unlocked_page_allocation_in_pages;
    unsigned long maximum_locked_page_allocation_in_pages;
    unsigned long linear_address_space_size_in_pages;
    unsigned long total_number_of_unlocked_pages;
    unsigned long total_number_of_free_pages;
    unsigned long total_number_of_physical_pages;
    unsigned long free_linear_address_space_in_pages;
    unsigned long size_of_paging_file_partition_in_pages;
    unsigned long reserved[3];
} __dpmi_free_mem_info;

/* A protected-mode address: a selector and an offset. */
typedef struct {
    unsigned long  offset32;
    unsigned short selector;
} __dpmi_paddr;

/* The DPMI error code of the last call that failed. */
extern unsigned short __dpmi_error;

/* Calls real-mode interrupt vector with the registers in *regs, and stores
 * the result registers. regs->x.ss and regs->x.sp 0 use a stack of the
 * host. Gives 0, or -1 on an error. */
int                   __dpmi_simulate_real_mode_interrupt(int vector, __dpmi_regs *regs);
/* The same as __dpmi_simulate_real_mode_interrupt. */
int                   __dpmi_int(int vector, __dpmi_regs *regs);

/* Allocates paragraphs paragraphs of DOS memory (below 1 MB). Gives its
 * real-mode segment, and its selector in *selector, or -1 on an error.
 * The memory is also at the linear address segment * 16. */
int                   __dpmi_allocate_dos_memory(int paragraphs, int *selector);
/* Frees the DOS memory of a selector. Gives 0, or -1 on an error. */
int                   __dpmi_free_dos_memory(int selector);

/* Allocates info->size bytes. Sets info->handle and info->address. Gives
 * 0, or -1 on an error. */
int                   __dpmi_allocate_memory(__dpmi_meminfo *info);
/* Frees the memory of a handle. Gives 0, or -1 on an error. */
int                   __dpmi_free_memory(unsigned long handle);
/* Gives the free memory in *info. Gives 0, or -1 on an error. */
int                   __dpmi_get_free_memory_information(__dpmi_free_mem_info *info);

/* Maps info->size bytes at the physical address info->address (for
 * example the frame buffer or the registers of a PCI card), and sets
 * info->address to the linear address. Gives 0, or -1 on an error. */
int                   __dpmi_physical_address_mapping(__dpmi_meminfo *info);
/* Frees a mapping (DPMI 1.0; DOS/32A has it). info->address is the
 * linear address. Gives 0, or -1 on an error. */
int                   __dpmi_free_physical_address_mapping(__dpmi_meminfo *info);

/* Locks and unlocks a linear region, which a handler of a hardware
 * interrupt uses. Gives 0, or -1 on an error. */
int                   __dpmi_lock_linear_region(__dpmi_meminfo *info);
int                   __dpmi_unlock_linear_region(__dpmi_meminfo *info);

/* Gives the base address of a selector in *addr. Gives 0, or -1. */
int                   __dpmi_get_segment_base_address(int selector, unsigned long *addr);

/* Gets and sets the protected-mode handler of an interrupt. Give 0, or
 * -1 on an error. */
int                   __dpmi_get_protected_mode_interrupt_vector(int vector, __dpmi_paddr *address);
int                   __dpmi_set_protected_mode_interrupt_vector(int vector, __dpmi_paddr *address);

/* Gets and sets the real-mode handler of an interrupt (segment:offset in
 * address->selector:address->offset32). Give 0, or -1 on an error. */
int                   __dpmi_get_real_mode_interrupt_vector(int vector, __dpmi_paddr *address);
int                   __dpmi_set_real_mode_interrupt_vector(int vector, __dpmi_paddr *address);

/* Gives the DPMI version: major * 256 + minor. */
int                   __dpmi_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* _DPMI_H_ */
