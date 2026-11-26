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
 * The calls of the 32-bit DOS library into DOS (INT 21h) and the DPMI host
 * (INT 31h). The argument is a struct __msdos_regs: EAX, EBX, ECX, EDX,
 * ESI and EDI at offsets 0 to 20. DS and ES stay the flat data selector.
 * The result registers go back into the structure, and the function gives
 * the carry flag (1 for an error).
 */

	.text

/* int __msdos_int21_pm(struct __msdos_regs *r): INT 21h in protected mode, as DOS/32A translates it. */
	.globl	__msdos_int21_pm
__msdos_int21_pm:
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	20(%esp), %ebp
	movl	0(%ebp), %eax
	movl	4(%ebp), %ebx
	movl	8(%ebp), %ecx
	movl	12(%ebp), %edx
	movl	16(%ebp), %esi
	movl	20(%ebp), %edi
	pushl	%ebp
	int	$0x21
	jmp	1f

/* int __dpmi_int31(struct __msdos_regs *r) */
	.globl	__dpmi_int31
__dpmi_int31:
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	20(%esp), %ebp
	movl	0(%ebp), %eax
	movl	4(%ebp), %ebx
	movl	8(%ebp), %ecx
	movl	12(%ebp), %edx
	movl	16(%ebp), %esi
	movl	20(%ebp), %edi
	pushl	%ebp
	int	$0x31

	/* Store the registers; MOV keeps the carry flag. */
1:	popl	%ebp
	movl	%eax, 0(%ebp)
	movl	%ebx, 4(%ebp)
	movl	%ecx, 8(%ebp)
	movl	%edx, 12(%ebp)
	movl	%esi, 16(%ebp)
	movl	%edi, 20(%ebp)
	sbbl	%eax, %eax
	negl	%eax
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret
