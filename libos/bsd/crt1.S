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
 * Program entry for static BSD programs on x86. The kernel starts _start
 * with esp pointing at argc, then argv[], a null, envp[] and a null.
 */

	.text
	.align	4
	.globl	_start
	.type	_start, @function
_start:
	xorl	%ebp, %ebp
	movl	%esp, %eax
	andl	$-16, %esp
	subl	$12, %esp
	pushl	%eax
	call	__bsd_start
	hlt
	.size	_start, . - _start

#ifdef __NetBSD__
/*
 * NetBSD's kernel runs an ELF program as a NetBSD one only with this note
 * (NT_NETBSD_IDENT, with the release it was built for: 1.5, the first
 * ELF release on x86).
 */
	.section .note.netbsd.ident, "a", @note
	.p2align 2
	.long	7
	.long	4
	.long	1
	.asciz	"NetBSD"
	.p2align 2
	.long	105000000
#endif

	.section .note.GNU-stack,"",@progbits
