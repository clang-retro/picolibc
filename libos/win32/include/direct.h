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

/* The Windows C library's directory functions. */

#ifndef _DIRECT_H_
#define _DIRECT_H_

#include <stddef.h>
#include <sys/cdefs.h>

#ifndef __win32_wchar
#ifdef __cplusplus
#define __win32_wchar wchar_t
#else
#define __win32_wchar __WCHAR_TYPE__
#endif
#endif

_BEGIN_STD_C

/* The space on a drive, as _getdiskfree gives it. */
struct _diskfree_t {
    unsigned total_clusters;
    unsigned avail_clusters;
    unsigned sectors_per_cluster;
    unsigned bytes_per_sector;
};

char          *_getcwd(char *, int);
/* The current directory of a drive (1 is A:, 0 the current drive). */
char          *_getdcwd(int, char *, int);
int            _chdir(const char *);
int            _mkdir(const char *);
int            _rmdir(const char *);
/* The current drive: 1 for A:, 2 for B: and so on. */
int            _getdrive(void);
int            _chdrive(int);
/* A bit for each drive that exists: bit 0 is A:. */
unsigned long  _getdrives(void);
unsigned       _getdiskfree(unsigned, struct _diskfree_t *);

__win32_wchar *_wgetcwd(__win32_wchar *, int);
int            _wchdir(const __win32_wchar *);
int            _wmkdir(const __win32_wchar *);
int            _wrmdir(const __win32_wchar *);

#if __MISC_VISIBLE
#define diskfree_t _diskfree_t
#endif

_END_STD_C

#endif /* _DIRECT_H_ */
