# OS/2

`libos/os2` implements picolibc's system calls on the OS/2 Dos API, for
16-bit OS/2 1.x (`i8086-pc-os2`, NE programs) and 32-bit OS/2 2.0 and
later (`i386-pc-os2`, LX programs and DLLs). Build it with
`-Dos-os2=true`.

- A file descriptor is the OS/2 file handle. Files are binary, and `\n`
  is written as `\r\n` to the console and other devices.
- `crt0` sets up the NE or LX entry: OS/2 passes the environment and the
  command line, which become `environ` and `argv`. `dllcrt0` starts a
  32-bit DLL, which is initialized for each process that loads it.
- `sbrk` grows DGROUP with `DosReallocSeg` in the 16-bit models with near
  data, uses one far segment from `DosAllocSeg` in those with far data,
  and commits a `DosAllocMem` reservation in 32-bit programs.
- Threads (32-bit, with `-Dsingle-thread=false -Dthread-local-storage=true
  -Dthread-local-storage-api=false`): `<threads.h>` is C11 threads on
  `DosCreateThread` and the mutex and event semaphores, and the C
  library's own locks are mutex semaphores. clang keeps `__thread`
  variables, `errno` among them, in emulated TLS: `__emutls_get_address`
  finds each thread's copies through a word of `DosAllocThreadLocalMemory`.
  `__cxa_thread_atexit_impl` runs the destructors of C++ `thread_local`
  objects when a thread ends. A zeroed `mtx_t` or `cnd_t` works, as
  libc++ needs.
- `clock_gettime` has `CLOCK_REALTIME`, and on 32-bit OS/2
  `CLOCK_MONOTONIC` (milliseconds since the system started).

The cross files are `scripts/cross-clang-i8086-os2-<model>.txt` (small,
medium, compact and large) and `scripts/cross-clang-i386-os2.txt`. The
[OS/2 SDK](https://clang-retro.github.io/os2-sdk/) builds the sysroots
and has the headers and examples.
