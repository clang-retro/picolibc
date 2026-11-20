# picolibc on the BSDs

This directory has BSD support for static programs on 32-bit x86, for
picolibc on the `i386-unknown-freebsd` target of the
[clang-retro LLVM toolchain](https://github.com/clang-retro/llvm). For
now that is FreeBSD, from 3.0 (the first ELF release, 1998) to the
current releases. It goes into `libc.a` itself, with `crt1.o`, so a
program needs nothing else from picolibc. It uses no BSD headers or
libraries: the system calls it makes are written out in `local-bsd.h`.

The library is built for the i486 (`-march=i486`, x87 floating point);
programs for clang's default CPU, the Pentium 4, link with it too.

Tested with bsdrun (unix-sdk's `bsd/tools/bsdrun.c`, which runs static
i386 FreeBSD programs on Linux by translating their system calls):
`printf` with floating point, `long double`, `malloc`, files, `stat`,
errors, `argv`, `isatty`, static constructors and the exit status.

## Building

With clang, `llvm-ar` and the other LLVM tools on `PATH`:

```sh
meson setup build-freebsd \
    --cross-file scripts/cross-clang-i386-unknown-freebsd.txt \
    -Dos-bsd=true -Dmultilib=false -Dpicocrt=false -Dsemihost=false \
    -Dtests=false -Dposix-console=true -Dsingle-thread=true \
    -Dthread-local-storage=false -Dspecsdir=none \
    --prefix=$PWD/install-freebsd
ninja -C build-freebsd install
```

unix-sdk's `bsd/build-sysroot.sh` puts this together with compiler-rt's
builtins into a sysroot laid out as clang's FreeBSD driver expects.

## What it does

- `crt1.S` calls `__bsd_start` in `start.c` with the stack the kernel set
  up (argc, argv and the environment), which makes `argv` and `environ`,
  runs the static constructors and calls `main`.
- System calls (`syscall-i386.S`) trap with `int $0x80`, the number in
  `eax` and the arguments on the stack, as on every BSD for x86. On
  failure the kernel sets the carry flag, and BSD error numbers become
  picolibc's `errno` values (`error.c`).
- The calls are the 4.4BSD ones every FreeBSD since 3.0 has. `stat`
  (188) and `lseek` (199) are the old ones, which FreeBSD 12 and 7 and
  later keep under `COMPAT_FREEBSD11` and `COMPAT_FREEBSD6` (both in the
  GENERIC kernel).
- Files: `open` (with the BSD `O_` flag values), `read`, `write`, `lseek`
  (64-bit offsets), `close`, `stat`, `lstat`, `fstat`, `isatty`
  (`TIOCGETA`), `unlink`, `rename`, `mkdir`, `rmdir`, `chdir`, `access`.
- `sbrk` grows the heap from the end of the bss with `break`.
- `getpid`, `kill`, `gettimeofday`, `_exit`. Signals are those of the
  fallback library: `raise` works.

## Limits

- Only FreeBSD on 32-bit x86 for now. NetBSD and OpenBSD, which use the
  same system call convention with some different numbers and
  structures, come next.
- Not yet tested on a real FreeBSD machine, only in bsdrun.
- A FreeBSD 12 or later kernel built without `COMPAT_FREEBSD11` can't run
  the programs: they use the old `stat`.
