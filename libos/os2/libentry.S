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
 * Startup code for 16-bit OS/2 1.x DLLs (clang -shared), linked first in
 * place of crt0.obj. OS/2 calls the entry point with a far call when a
 * process loads the DLL, with AX = the module handle, CX = the local heap
 * size (heapsize=), DS = the DLL's DGROUP and the caller's stack: SS is
 * not DS. It returns AX = nonzero for success. DGROUP is laid out as for a
 * program (see crt0.S), without the stack.
 */
#ifndef __i386__
#if defined(__MEDIUM__) || defined(__LARGE__)
#define FAR_CODE 1
#endif
	/* No variable may be at offset 0 of DGROUP, where a near pointer to
	 * it would be a null pointer. */
	.section .rodata
	.zero	16

	.text
#ifdef FAR_CODE
	.globl	__omf_weak_null
	.set	__omf_weak_null, 0
#else
	.globl	__omf_weak_null
__omf_weak_null:
#endif
	.globl	_start
_start:
	pushw	%bx
	pushw	%cx
	pushw	%dx
	pushw	%si
	pushw	%di
	pushw	%bp
	pushw	%es
	cld
	movw	%ax, %dx		/* module handle */
	/* Clear _BSS and the communal variables. */
	pushw	%ds
	popw	%es
	movw	$___bss_start, %di
	movw	$___bss_end, %cx
	subw	%di, %cx
	xorw	%ax, %ax
	rep stosb
	movw	%dx, ___os2_dll_module
	xorw	%bp, %bp
#ifdef FAR_CODE
	lcallw	$___os2_dll_start@SEG, $___os2_dll_start
#else
	call	___os2_dll_start
#endif
	popw	%es
	popw	%bp
	popw	%di
	popw	%si
	popw	%dx
	popw	%cx
	popw	%bx
	lretw

	/* A DLL has no stack of its own; the heap starts at __heap_start. */
	.data
	.globl	___os2_stack_top
	.p2align 1
___os2_stack_top:
	.short	0

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
	/* These empty segments place the far data classes. */
	.section .fardata,"aw",@progbits
	.section .farbss,"aw",@nobits
	.bss
___bss_start:
	.section .stack,"aw",@nobits
	.p2align 1
___bss_end:
	.globl	___heap_start
___heap_start:
#endif
