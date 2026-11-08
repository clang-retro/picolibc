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
#include "local-os2.h"
#include <sys/_initfini.h>

/*
 * 16-bit OS/2 DLLs: libentry.S calls __os2_dll_start when a process loads
 * the DLL (the driver links DLLs with initinstance and their own DGROUP
 * for each process, as manyautodata). It sets up the heap and environ, and
 * runs the constructors. 1.x has no call at unload, so destructors and
 * atexit functions of a DLL do not run.
 */

#ifndef __i386__
__os2_word __os2_dll_module;

unsigned
__os2_dll_start(void)
{
    /* Static, not locals: in the near data models a local whose address
     * escapes needs a frame from the heap (dllframe.c), which is not set
     * up yet. */
    static __os2_word envsel, cmdoff;
    __os2_init_heap();
    if (DosGetEnv(&envsel, &cmdoff) == 0) {
        __os2_env = (__os2_fstr)((uint32_t)envsel << 16);
        __os2_cmdline = (__os2_fstr)(((uint32_t)envsel << 16) | cmdoff);
    }
    __os2_init_environ();
    __libc_init_array();
    return 1;
}
#endif
