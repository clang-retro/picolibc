/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 clang-retro contributors
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
 * Startup code for CP/M-86 programs, linked first. The linker writes a
 * .COM (tiny model) or .EXE (small model) file, which llvm-exe2cmd turns
 * into a CMD file.
 *
 * Tiny model: an 8080-model program. CP/M-86 loads the code group with
 * CS = DS = ES at its base, puts the base page at offset 0 and starts at
 * 0100h, as DOS does a .COM program.
 * Small model: a code group, which starts at CS:0000, and a data group,
 * DGROUP, with the base page at DS:0000.
 *
 * The CCP calls the program on a 96-byte stack of its own, so SS:SP moves
 * to a stack of __CPM_STACK_SIZE bytes after the BSS. The heap is the rest
 * of the group, up to 64 KiB, as much as CP/M-86 gave the program.
 *
 * The program starts with a short jump over "CP/M" and a word, which
 * llvm-exe2cmd reads: in the tiny model the end of the memory the program
 * needs (BSS and stack), in the small model DGROUP's segment, so that it
 * knows where the data group starts.
 */

#ifndef __CPM_STACK_SIZE
#define __CPM_STACK_SIZE 4096
#endif

#if !defined(__TINY__) && !defined(__SMALL__)
#error "CP/M-86 programs are built in the tiny or small model"
#endif

	.text
#ifdef __TINY__
	/* The base page is at offset 0 of the program's one segment. */
	.globl	___cpm_base_page
___cpm_base_page:
#endif
	/* Undefined weak symbols resolve here, at offset 0, as in ELF. */
	.globl	__omf_weak_null
__omf_weak_null:
#ifdef __TINY__
	.org	0x100
#endif
	.globl	_start
_start:
	.byte	0xeb, 6			/* jmp short 1f */
	.ascii	"CP/M"
#ifdef __TINY__
	.word	___heap_start
#else
	.word	___cpm_base_page@SEG
#endif
1:	movw	%ds, %ax
	cli
	movw	%ax, %ss
	movw	$___heap_start, %sp
	sti
	movw	%ax, %es
	cld
	/* Clear _BSS and the communal variables. */
	movw	$___bss_start, %di
	movw	$___bss_end, %cx
	subw	%di, %cx
	xorw	%ax, %ax
	rep stosb
	xorw	%bp, %bp
	call	___cpm_start
	/* Not reached: __cpm_start calls exit. */
	xorw	%cx, %cx
	xorw	%dx, %dx
	int	$0xe0

#ifndef __TINY__
	/* The base page is the first 256 bytes of DGROUP. This also keeps
	 * variables and strings from offset 0, where a near pointer to them
	 * would be a null pointer. */
	.section .rodata
	.p2align 4
	.globl	___cpm_base_page
___cpm_base_page:
	.zero	256
#endif

	/* Bounds of the constructor and destructor tables, for picolibc's
	 * __libc_init_array and __libc_fini_array. There is no separate
	 * preinit table. */
	.section XIB,"aw"
	.p2align 1
	.globl	___bothinit_array_start, ___preinit_array_start
	.globl	___preinit_array_end, ___init_array_start
___bothinit_array_start:
___preinit_array_start:
___preinit_array_end:
___init_array_start:
	.section XIE,"aw"
	.p2align 1
	.globl	___bothinit_array_end, ___init_array_end
___bothinit_array_end:
___init_array_end:
	.section YIB,"aw"
	.p2align 1
	.globl	___fini_array_start
___fini_array_start:
	.section YIE,"aw"
	.p2align 1
	.globl	___fini_array_end
___fini_array_end:

	.bss
___bss_start:

	.section .stack,"aw",@nobits
	.p2align 1
___bss_end:
	.zero	__CPM_STACK_SIZE
	.globl	___heap_start
___heap_start:
