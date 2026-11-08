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
 * The OS/2 layer serves 16-bit OS/2 1.x (i8086, NE programs) and 32-bit
 * OS/2 2.0 and later (i386, LX programs). The system calls are calls into
 * DOSCALLS.DLL, imported by the os2.lnk list the driver gives the linker:
 * far Pascal functions with far pointers on 1.x, and _System functions
 * (the C calling convention) on 2.x.
 *
 * A file descriptor is the OS/2 file handle.
 */

#ifndef _LOCAL_OS2_H_
#define _LOCAL_OS2_H_

#define _GNU_SOURCE

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __i386__
#define __OS2_API
#define __OS2_FAR
typedef unsigned long __os2_word;
#else
#define __OS2_API __far __pascal
#define __OS2_FAR __far
typedef unsigned short __os2_word;
#endif

typedef __os2_word            __os2_ret;
typedef __os2_word            __os2_handle;
typedef const char __OS2_FAR *__os2_psz;

struct __os2_datetime {
    unsigned char  hours, minutes, seconds, hundredths;
    unsigned char  day, month;
    unsigned short year;
    short          timezone; /* minutes west of UTC, -1 if unknown */
    unsigned char  weekday;
};

/* FILESTATUS (1.x) and FILESTATUS3 (2.x), information level 1. */
struct __os2_filestatus {
    unsigned short cdate, ctime, adate, atime, wdate, wtime;
    unsigned long  size, alloc;
    __os2_word     attr;
};

/* FILEFINDBUF3 (2.x, level 1) and FILEFINDBUF (1.x), with room for a
   256-byte name. */
#ifdef __i386__
struct __os2_findbuf {
    unsigned long  next;
    unsigned short cdate, ctime, adate, atime, wdate, wtime;
    unsigned long  size, alloc, attr;
    unsigned char  namelen;
    char           name[256];
};
#else
struct __os2_findbuf {
    unsigned short cdate, ctime, adate, atime, wdate, wtime;
    unsigned long  size, alloc;
    unsigned short attr;
    unsigned char  namelen;
    char           name[256];
};
#endif

/* The process and thread ids, 1.x (DosGetPID). */
struct __os2_pidinfo {
    unsigned short pid, tid, ppid;
};

/* RESULTCODES: how a child process ended (DosExecPgm, DosCwait). For an
   asynchronous child, DosExecPgm gives its process id in code. */
struct __os2_resultcodes {
    __os2_word code;   /* TC_EXIT 0, TC_HARDERROR 1, TC_TRAP 2, TC_KILLPROCESS 3 */
    __os2_word result; /* its exit code */
};

#define __OS2_FILE_READONLY  0x01
#define __OS2_FILE_DIRECTORY 0x10

/* DosOpen */
#define __OS2_OPEN_EXISTING       0x0001 /* open if it exists */
#define __OS2_OPEN_REPLACE        0x0002 /* replace if it exists */
#define __OS2_OPEN_CREATE         0x0010 /* create if new */
#define __OS2_OPEN_SHARE_DENYNONE 0x0040
#define __OS2_OPEN_NOINHERIT      0x0080
#define __OS2_OPEN_FAIL_ON_ERROR  0x2000 /* no hard error pop-ups */

/* OS/2 error codes this layer looks at. */
#define __OS2_ERROR_FILE_NOT_FOUND 2
#define __OS2_ERROR_OPEN_FAILED    110

#ifdef __i386__
/* DOSCALLS, 32-bit */
void      DosExit(__os2_word, __os2_word);
__os2_ret DosWrite(__os2_handle, const void *, __os2_word, __os2_word *);
__os2_ret DosRead(__os2_handle, void *, __os2_word, __os2_word *);
__os2_ret DosOpen(__os2_psz, __os2_handle *, __os2_word *, unsigned long, __os2_word, __os2_word,
                  __os2_word, void *);
__os2_ret DosClose(__os2_handle);
__os2_ret DosSetFilePtr(__os2_handle, long, __os2_word, unsigned long *);
__os2_ret DosSetFileSize(__os2_handle, unsigned long);
__os2_ret DosDelete(__os2_psz);
__os2_ret DosMove(__os2_psz, __os2_psz);
__os2_ret DosCreateDir(__os2_psz, void *);
__os2_ret DosDeleteDir(__os2_psz);
__os2_ret DosSetCurrentDir(__os2_psz);
__os2_ret DosQueryHType(__os2_handle, __os2_word *, __os2_word *);
__os2_ret DosQueryPathInfo(__os2_psz, __os2_word, void *, __os2_word);
__os2_ret DosQueryFileInfo(__os2_handle, __os2_word, void *, __os2_word);
__os2_ret DosGetDateTime(struct __os2_datetime *);
__os2_ret DosAllocMem(void **, unsigned long, unsigned long);
__os2_ret DosSetMem(void *, unsigned long, unsigned long);
__os2_ret DosSleep(unsigned long);
__os2_ret DosFindFirst(__os2_psz, __os2_handle *, __os2_word, void *, __os2_word, __os2_word *,
                       __os2_word);
__os2_ret DosFindNext(__os2_handle, void *, __os2_word, __os2_word *);
__os2_ret DosFindClose(__os2_handle);
__os2_ret DosQueryCurrentDisk(__os2_word *, unsigned long *);
__os2_ret DosQueryCurrentDir(__os2_word, char *, __os2_word *);
__os2_ret DosGetInfoBlocks(void **, void **);
__os2_ret DosExecPgm(char *, long, __os2_word, __os2_psz, __os2_psz, struct __os2_resultcodes *,
                     __os2_psz);
__os2_ret DosWaitChild(__os2_word, __os2_word, struct __os2_resultcodes *, __os2_word *,
                       __os2_word);
__os2_ret DosSearchPath(__os2_word, __os2_psz, __os2_psz, char *, __os2_word);
#define __os2_cwait(s, w, r, p, i) DosWaitChild(s, w, r, p, i)
#define __os2_delete(p)                DosDelete(p)
#define __os2_move(f, t)               DosMove(f, t)
#define __os2_mkdir(p)                 DosCreateDir(p, 0)
#define __os2_rmdir(p)                 DosDeleteDir(p)
#define __os2_chdir(p)                 DosSetCurrentDir(p)
#define __os2_seek(h, d, m, p)         DosSetFilePtr(h, d, m, p)
#define __os2_htype(h, t, a)           DosQueryHType(h, t, a)
#define __os2_pathinfo(p, b)           DosQueryPathInfo(p, 1, b, sizeof(*(b)))
#define __os2_fileinfo(h, b)           DosQueryFileInfo(h, 1, b, sizeof(*(b)))
#define __os2_truncate(h, s)           DosSetFileSize(h, s)
#define __os2_sleep(ms)                DosSleep(ms)
#define __os2_findfirst(p, h, a, b, n) DosFindFirst(p, h, a, b, sizeof(*(b)), n, 1)
#define __os2_findnext(h, b, n)        DosFindNext(h, b, sizeof(*(b)), n)
#define __os2_findclose(h)             DosFindClose(h)
#define __os2_curdisk(d, m)            DosQueryCurrentDisk(d, m)
#define __os2_curdir(d, b, n)          DosQueryCurrentDir(d, b, n)
#else
/* DOSCALLS, 16-bit */
void __OS2_API      DosExit(__os2_word, __os2_word);
__os2_ret __OS2_API DosWrite(__os2_handle, const void __far *, __os2_word, __os2_word __far *);
__os2_ret __OS2_API DosRead(__os2_handle, void __far *, __os2_word, __os2_word __far *);
__os2_ret __OS2_API DosOpen(__os2_psz, __os2_handle __far *, __os2_word __far *, unsigned long,
                            __os2_word, __os2_word, __os2_word, unsigned long);
__os2_ret __OS2_API DosClose(__os2_handle);
__os2_ret __OS2_API DosChgFilePtr(__os2_handle, long, __os2_word, unsigned long __far *);
__os2_ret __OS2_API DosNewSize(__os2_handle, unsigned long);
__os2_ret __OS2_API DosDelete(__os2_psz, unsigned long);
__os2_ret __OS2_API DosMove(__os2_psz, __os2_psz, unsigned long);
__os2_ret __OS2_API DosMkDir(__os2_psz, unsigned long);
__os2_ret __OS2_API DosRmDir(__os2_psz, unsigned long);
__os2_ret __OS2_API DosChDir(__os2_psz, unsigned long);
__os2_ret __OS2_API DosQHandType(__os2_handle, __os2_word __far *, __os2_word __far *);
__os2_ret __OS2_API DosQPathInfo(__os2_psz, __os2_word, void __far *, __os2_word, unsigned long);
__os2_ret __OS2_API DosQFileInfo(__os2_handle, __os2_word, void __far *, __os2_word);
__os2_ret __OS2_API DosGetDateTime(struct __os2_datetime __far *);
__os2_ret __OS2_API DosAllocSeg(__os2_word, __os2_word __far *, __os2_word);
__os2_ret __OS2_API DosReallocSeg(__os2_word, __os2_word);
__os2_ret __OS2_API DosSleep(unsigned long);
__os2_ret __OS2_API DosFindFirst(__os2_psz, __os2_handle __far *, __os2_word, void __far *,
                                 __os2_word, __os2_word __far *, unsigned long);
__os2_ret __OS2_API DosFindNext(__os2_handle, void __far *, __os2_word, __os2_word __far *);
__os2_ret __OS2_API DosFindClose(__os2_handle);
__os2_ret __OS2_API DosQCurDisk(__os2_word __far *, unsigned long __far *);
__os2_ret __OS2_API DosQCurDir(__os2_word, char __far *, __os2_word __far *);
__os2_ret __OS2_API DosGetPID(struct __os2_pidinfo __far *);
__os2_ret __OS2_API DosGetEnv(__os2_word __far *, __os2_word __far *);
__os2_ret __OS2_API DosGetInfoSeg(__os2_word __far *, __os2_word __far *);
__os2_ret __OS2_API DosExecPgm(char __far *, short, __os2_word, __os2_psz, __os2_psz,
                               struct __os2_resultcodes __far *, __os2_psz);
__os2_ret __OS2_API DosCwait(__os2_word, __os2_word, struct __os2_resultcodes __far *,
                             __os2_word __far *, __os2_word);
__os2_ret __OS2_API DosSearchPath(__os2_word, __os2_psz, __os2_psz, char __far *, __os2_word);
#define __os2_cwait(s, w, r, p, i) DosCwait(s, w, r, p, i)
#define __os2_delete(p)                DosDelete(p, 0)
#define __os2_move(f, t)               DosMove(f, t, 0)
#define __os2_mkdir(p)                 DosMkDir(p, 0)
#define __os2_rmdir(p)                 DosRmDir(p, 0)
#define __os2_chdir(p)                 DosChDir(p, 0)
#define __os2_seek(h, d, m, p)         DosChgFilePtr(h, d, m, p)
#define __os2_htype(h, t, a)           DosQHandType(h, t, a)
/* FILESTATUS has a 16-bit attribute word. */
#define __os2_pathinfo(p, b)           DosQPathInfo(p, 1, b, 22, 0)
#define __os2_fileinfo(h, b)           DosQFileInfo(h, 1, b, 22)
#define __os2_truncate(h, s)           DosNewSize(h, s)
#define __os2_sleep(ms)                DosSleep(ms)
#define __os2_findfirst(p, h, a, b, n) DosFindFirst(p, h, a, b, sizeof(*(b)), n, 0)
#define __os2_findnext(h, b, n)        DosFindNext(h, b, sizeof(*(b)), n)
#define __os2_findclose(h)             DosFindClose(h)
#define __os2_curdisk(d, m)            DosQCurDisk(d, m)
#define __os2_curdir(d, b, n)          DosQCurDir(d, b, n)
#endif

/* Most bytes one DosRead or DosWrite moves. */
#define __OS2_MAX_IO ((__os2_word) - 1 > 0x7fffffffUL ? 0x7fffffffUL : 0xfff0u)

/* Per-handle flags kept by the C library. */
#define __OS2_MAX_FD    64
#define __OS2_FD_APPEND 0x01
extern unsigned char __os2_fd_flags[__OS2_MAX_FD];

/* The heap (sbrk.c). */
extern char         *__os2_heap_base, *__os2_brk, *__os2_heap_end;
#ifndef __i386__
void __os2_init_heap(void);
#endif

/* The environment and the command line (env.c). */
typedef const char __OS2_FAR *__os2_fstr;
extern __os2_fstr             __os2_env, __os2_cmdline;
size_t                        __os2_fstrlen(__os2_fstr s);
char                         *__os2_fdup(__os2_fstr s, size_t n);
void                          __os2_init_environ(void);
extern char                 **environ;

#ifndef __i386__
/* 16-bit DLLs (dllstart.c, dllframe.c). */
extern __os2_word __os2_dll_module;
unsigned          __os2_dll_start(void);
char            **__win16_frame_pointer_address(void);
#endif

/* Programs (process.c). */
int __os2_spawnve(int mode, const char *path, char *const argv[], char *const envp[]);
int __os2_spawn_line(int mode, const char *path, const char *name, const char *line,
                     char *const envp[]);
int __os2_find_program(char *buf, size_t size, const char *name, int search);

int  __os2_error(unsigned code);
int  __os2_isdev(int fd);
long __os2_unix_time(unsigned short date, unsigned short time);

#endif /* _LOCAL_OS2_H_ */
