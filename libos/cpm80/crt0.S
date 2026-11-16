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
 * Startup code for CP/M-80 programs, linked first. CP/M loads the .COM
 * file at 0100h and jumps to its first byte. The stack starts at the top
 * of the TPA, the BDOS entry address in the word at 0006h; cpm80.ld puts
 * the BSS after the file and the heap after the BSS.
 */

	/* The base page: the BDOS entry, the default FCBs at 5Ch and 6Ch and
	 * the command tail at 80h. */
	.globl	___cpm_base_page
	.set	___cpm_base_page, 0

	.section .text.start,"ax"
	.globl	_start
_start:
	ld	hl, (6)
	ld	sp, hl
	/* Clear the BSS. */
	ld	hl, __bss_start
	ld	de, __bss_end
1:	ld	a, l
	cp	e
	jp	nz, 2f
	ld	a, h
	cp	d
	jp	z, 3f
2:	ld	(hl), 0
	inc	hl
	jp	1b
3:	call	__cpm_start
	/* Not reached: __cpm_start calls exit. */
	jp	0
