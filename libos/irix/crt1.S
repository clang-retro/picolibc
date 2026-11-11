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
 * Program entry for static IRIX programs. The kernel starts __start with
 * sp pointing at argc, then argv[], a null, envp[] and a null, each a
 * long (o32, 64) or a pointer (n32). It doesn't set gp or t9, so they are
 * loaded here from absolute addresses, which a static program has.
 */

#if _MIPS_SIM == _ABI64
#define PTR_ADDIU daddiu
#define REG_S     sd
#else
#define PTR_ADDIU addiu
#define REG_S     sw
#endif

	.text
	.globl	__start
	.ent	__start
	.type	__start, @function
	.set	noreorder
__start:
	lui	$gp, %hi(_gp)
	addiu	$gp, $gp, %lo(_gp)
	move	$4, $sp
	li	$8, -16
	and	$sp, $sp, $8
	PTR_ADDIU $sp, $sp, -32
	lui	$25, %hi(__irix_start)
	addiu	$25, $25, %lo(__irix_start)
	jalr	$25
	move	$31, $0
1:	b	1b
	nop
	.end	__start
	.size	__start, . - __start
