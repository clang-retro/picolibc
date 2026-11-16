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

/* setjmp and longjmp for the Z80 and the 8080. C code keeps no registers
 * across calls but the frame pointer IX, so jmp_buf holds the return
 * address, the stack pointer, and IX and IY on the Z80. The buffer is in
 * HL and longjmp's value in DE. */

	.text
	.globl	setjmp, _setjmp
setjmp:
_setjmp:
	pop	bc		/* the return address */
	ld	(hl), c
	inc	hl
	ld	(hl), b
	inc	hl
	ex	de, hl
	ld	hl, 0
	add	hl, sp		/* the stack pointer after the return */
	ex	de, hl
	ld	(hl), e
	inc	hl
	ld	(hl), d
#ifdef __z80__
	inc	hl
	push	ix
	pop	de
	ld	(hl), e
	inc	hl
	ld	(hl), d
	inc	hl
	push	iy
	pop	de
	ld	(hl), e
	inc	hl
	ld	(hl), d
#endif
	ld	hl, 0
	push	bc
	ret

	.globl	longjmp, _longjmp
longjmp:
_longjmp:
	ld	a, d
	or	e
	jp	nz, 1f
	inc	de		/* longjmp(env, 0) returns 1 */
1:
#ifdef __z80__
	push	hl
	pop	iy
	ld	c, (iy+0)
	ld	b, (iy+1)
	ld	l, (iy+2)
	ld	h, (iy+3)
	ld	sp, hl
	ld	l, (iy+4)
	ld	h, (iy+5)
	push	hl
	pop	ix
	ld	l, (iy+6)
	ld	h, (iy+7)
	push	hl
	pop	iy
#else
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	inc	hl
	ld	a, (hl)
	inc	hl
	ld	h, (hl)
	ld	l, a
	ld	sp, hl
#endif
	ex	de, hl
	push	bc
	ret
