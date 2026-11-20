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
 * long long __bsd_syscall(long n, long a, long b, long c, long d, long e,
 * long f) on x86: int $0x80 with the number in eax and the arguments on
 * the stack above a return address, as the BSD kernels find them for a
 * libc stub. The result comes back in edx:eax, which is where a function
 * returns a long long. On failure the kernel sets the carry flag and
 * puts the error in eax.
 */

	.text
	.align	4
	.globl	__bsd_syscall
	.type	__bsd_syscall, @function
__bsd_syscall:
	movl	4(%esp), %eax
	pushl	28(%esp)
	pushl	28(%esp)
	pushl	28(%esp)
	pushl	28(%esp)
	pushl	28(%esp)
	pushl	28(%esp)
	pushl	$0
	int	$0x80
	jc	1f
	addl	$28, %esp
	ret
1:	addl	$28, %esp
	movl	%eax, 4(%esp)
	jmp	__bsd_error
	.size	__bsd_syscall, . - __bsd_syscall

	.section .note.GNU-stack,"",@progbits
