# picolibc on 32-bit Windows

This directory lets picolibc stand in for msvcrt.dll or crtdll.dll as the
C library of 32-bit Windows programs. It goes into `libc.a`, and a program
linked with it imports only `kernel32.dll`, so it runs on Windows 95, 98
and ME, and Windows NT 3.1 and later, with
nothing to install. It is built for the `i386-pc-windows*-gnu` targets of
the [clang-dos LLVM toolchain](https://github.com/clang-dos/llvm), and
libc++ builds on it with `RUNTIMES_USE_LIBC=picolibc`.

## Building

With clang, `llvm-ar` and the other LLVM tools on `PATH`:

```sh
meson setup build-win32 \
    --cross-file scripts/cross-clang-i386-windows.txt \
    -Dos-win32=true -Denable-malloc=false -Dinitfini-array=false \
    -Dmultilib=false -Dpicocrt=false -Dsemihost=false -Dtests=false \
    -Dspecsdir=none -Dposix-console=true -Dmb-capable=true \
    -Dio-wchar=true -Dio-pos-args=true -Dio-long-double=true \
    --prefix=$PWD/install-win32
ninja -C build-win32 install
```

The headers go over the mingw-w64 API headers (windows.h and the rest)
with mingw-w64's C library headers taken out; the win32-sdk repository's
build script puts such a sysroot together. Programs link with
`-nostartfiles -nodefaultlibs` and then `-lc -lclang_rt.builtins-i386
-lkernel32`, and are compiled with `-fno-emulated-tls`.

## What it does

- `start.c` has the entry points the linker looks for, `mainCRTStartup`
  and `WinMainCRTStartup`. It splits the command line with the Microsoft C
  rules, copies the environment, runs the constructors (`ctors.c`, from
  clang's `__main` call) and calls `main` or `WinMain`.
- `dllstart.c` is the entry point of DLLs, `DllMainCRTStartup`. Each
  DLL has its own copy of the library; it runs the DLL's constructors
  before `DllMain` and its atexit functions and destructors when it is
  unloaded.
- `tlssup.c` is the TLS directory, so `__thread` variables (errno among
  them) are native TLS, set up by Windows for every thread (see
  [Emulated TLS](#emulated-tls) for the other build).
  `cxa_thread_atexit.c` runs C++ `thread_local` destructors when a thread
  ends, and `keydtor.c` runs destructors for `TlsAlloc` keys, which libc++
  uses on Windows before Server 2003. Windows 9x does not run TLS
  callbacks, so there the library runs them itself: a DLL from its entry
  point, and a program when a thread that `_beginthreadex` or
  `_beginthread` started returns or calls `_endthreadex` or `_endthread`.
  On 9x, a thread that `CreateThread` starts and that ends with
  `ExitThread` does not run its destructors.
- Descriptors (`fd.c`) are a table of Win32 handles. Files open in text
  mode unless `O_BINARY`, `_fmode` or `setmode` say otherwise: `read`
  turns CR LF into LF and stops at Ctrl-Z, and `write` turns LF into
  CR LF. Consoles, pipes and files all go through `ReadFile` and
  `WriteFile`. `_wopen` and `_wfopen` open files by wide name, with
  `CreateFileW` on Windows NT and the ANSI name on Windows 9x.
- `malloc` uses the process heap (`HeapAlloc`); `lock.c` makes picolibc's
  locks critical sections, so the library is thread safe.
- Time comes from `GetSystemTime` (there is no `GetSystemTimeAsFileTime`
  on Windows NT 3.1), and the monotonic clock from
  `QueryPerformanceCounter`, or `GetTickCount` where there is none.
- `_beginthreadex` and the rest of `<process.h>` start threads with
  `CreateThread`. `<io.h>` has `_get_osfhandle` and `_open_osfhandle`.
- Error codes from `GetLastError` become errno values in `error.c`.
- The library has the functions of the Microsoft C library that programs
  for Windows use most, with the same names and rules:
  - `<string.h>`, `<stdlib.h>`, `<stdio.h>`, `<wchar.h>` and
    `<sys/stat.h>` declare them for `_WIN32` targets, from the
    `sys/_win32*.h` headers: `_stricmp`, `_strdup`, `_itoa`, `_splitpath`,
    `_fullpath`, `_putenv`, `_snprintf` (no NUL and -1 when the text does
    not fit), `_wcsicmp`, `_stat` and `struct _stat`, and others. The old
    names (`stricmp`, `strrev`, `ltoa` and others) are there unless the
    program asks for strict ISO C.
  - `<io.h>` has `_open`, `_read`, `_lseek`, `_filelength`, `_chsize`,
    `_eof`, `_locking`, `_mktemp` and `_findfirst` with `_findnext`.
  - `<direct.h>` has `_getcwd`, `_getdcwd`, `_mkdir`, `_getdrive`,
    `_chdrive` and `_getdiskfree`.
  - `<conio.h>` has `_getch`, `_kbhit`, `_putch`, `_cputs`, `_cgets` and
    `_cprintf`, which use the console (`CONIN$` and `CONOUT$`).
  - `<float.h>` adds `_controlfp`, `_control87`, `_clearfp`,
    `_statusfp`, `_fpreset` and `_fpclass` to clang's `<float.h>`.
  - `<tchar.h>` has the generic text names (`_TCHAR`, `_T`, `_tcslen`
    and the others). With `_UNICODE` they are the wide functions. `_MBCS`
    gives the byte functions, because the library has no `_mbs`
    functions. `_tmain` with `_UNICODE` is `wmain`, which the start code
    does not call.

  The wide forms of the file functions other than `_wopen` and `_wfopen`
  (`_wstat`, `_wmkdir`, `_wfindfirst` and the others) change the name to
  the ANSI code page. `_umask` keeps the mask, but new files do not use it.

`link` and `symlink` fail with `ENOSYS`, and there are no signals beyond
`raise`.

The library's thread-local variables are native TLS. Windows before Vista
sets that up only for the program and the DLLs it loads at startup, not
for a DLL loaded later with `LoadLibrary`.

## Emulated TLS

For a DLL that `LoadLibrary` loads before Vista, build the library a
second time with `-Dwin32-emutls=true`, and compile the DLL with
`-femulated-tls`. In this build:

- All thread-local variables go through `__emutls_get_address`
  (`emutls.c`). Each variable gets an index at its first use. Each thread
  keeps its objects in an array in one `TlsAlloc` slot. This works in a
  DLL on every Windows version.
- `tlssup.c` makes no TLS directory. Thus Windows does not run the TLS
  callbacks. The DLL entry point runs them for each thread that starts
  and ends, and at unload. A program runs them when a thread that
  `_beginthreadex` or `_beginthread` started ends.
- When a thread ends, a callback frees its objects. This callback runs
  after the `thread_local` and key destructors.

The normal build also has `emutls.c`. The linker uses it only for code
that is compiled with `-femulated-tls`. compiler-rt's own
`__emutls_get_address` calls the mingw-w64 C runtime. `-lc` comes before
the builtins on the link line, thus the linker uses this one.
