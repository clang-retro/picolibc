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
 * int __msdos_int386(int intno, struct __msdos_regs *r, int sregs): the
 * software interrupt intno with EAX to EDI from *r, and the result
 * registers stored back. With sregs nonzero, DS and ES come from r->ds
 * and r->es (0 for the flat data selector) and go back there. Gives the
 * carry flag.
 */

	.text
	.globl	__msdos_int386
__msdos_int386:
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	pushl	%ds
	pushl	%es
	movl	28(%esp), %eax		/* intno */
	movzbl	%al, %eax
	leal	1f(,%eax,4), %eax
	movl	32(%esp), %ebp		/* r */
	pushl	%ebp
	pushl	%eax
	cmpl	$0, 44(%esp)		/* sregs */
	je	2f
	movw	26(%ebp), %ax
	testw	%ax, %ax
	jz	3f
	movw	%ax, %es
3:	movw	24(%ebp), %ax
	testw	%ax, %ax
	jz	2f
	movw	%ax, %ds
2:	movl	0(%ebp), %eax
	movl	4(%ebp), %ebx
	movl	8(%ebp), %ecx
	movl	12(%ebp), %edx
	movl	16(%ebp), %esi
	movl	20(%ebp), %edi
	/* The interrupt, through its entry of the table: call *(%esp) with
	 * the return address after it. */
	call	*(%esp)
	/* DS can be another selector: store through SS, which is flat. */
	addl	$4, %esp
	popl	%ebp
	movl	%eax, %ss:0(%ebp)
	movl	%ebx, %ss:4(%ebp)
	movl	%ecx, %ss:8(%ebp)
	movl	%edx, %ss:12(%ebp)
	movl	%esi, %ss:16(%ebp)
	movl	%edi, %ss:20(%ebp)
	movw	%ds, %ss:24(%ebp)
	movw	%es, %ss:26(%ebp)
	sbbl	%eax, %eax
	negl	%eax
	popl	%es
	popl	%ds
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret

	/* 256 entries of 4 bytes: INT n, RET, and a NOP. */
	.p2align 2
1:
	.irp	hi, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	.irp	lo, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	.byte	0xcd, \hi * 16 + \lo, 0xc3, 0x90
	.endr
	.endr
