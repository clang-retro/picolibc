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
 * The startup code of 32-bit DOS programs (i386-pc-msdos). DOS starts the
 * DOS/32A extender, which is the stub of the LE program. The extender
 * switches to flat protected mode, loads the program, clears its BSS and
 * jumps here with CS, DS and SS flat (base 0), ESP at the top of the
 * stack and ES the selector of the PSP.
 */

	/* Undefined weak symbols resolve to this absolute 0 (JWlink). */
	.globl	__elf_weak_null
	.set	__elf_weak_null, 0

	.text
	.globl	_start
_start:
	movw	%es, __msdos_psp_sel
	/* The C code takes ES to be the data segment, as DS is. */
	pushl	%ds
	popl	%es
	xorl	%ebp, %ebp
	cld
	call	__msdos_start
	/* Not reached: __msdos_start calls exit. */
	movw	$0x4cff, %ax
	int	$0x21

	/* The constructor and destructor tables. The linker puts a module's
	 * .init_array and .fini_array after the ones here, which mark their
	 * start, and before the end markers, which come later. */
	.section .init_array.a,"aw"
	.p2align 2
	.globl	__bothinit_array_start, __preinit_array_start
	.globl	__preinit_array_end, __init_array_start
__bothinit_array_start:
__preinit_array_start:
__preinit_array_end:
__init_array_start:
	.section .init_array,"aw"
	.section .init_array.z,"aw"
	.p2align 2
	.globl	__bothinit_array_end, __init_array_end
__bothinit_array_end:
__init_array_end:
	.section .fini_array.a,"aw"
	.p2align 2
	.globl	__fini_array_start
__fini_array_start:
	.section .fini_array,"aw"
	.section .fini_array.z,"aw"
	.p2align 2
	.globl	__fini_array_end
__fini_array_end:

	/* The stack: the linker's option stack sets its size. */
	.section .stack,"aw",@nobits
	.p2align 4
	.zero	16

	/* No variable may be at address 0 of the data, where a pointer to it
	 * would be a null pointer. */
	.section .rodata
	.zero	16
