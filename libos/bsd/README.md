# picolibc on the BSDs

This directory has BSD support for static programs on 32-bit x86, for
picolibc on the `i386-unknown-freebsd` and `i386-unknown-netbsd` targets
of the [clang-retro LLVM toolchain](https://github.com/clang-retro/llvm):
FreeBSD from 3.0 (the first ELF release, 1998) and NetBSD from 1.5 (the
first ELF release on x86, 2000) to the current releases. The BSD is the
one the compiler targets. It goes into `libc.a` itself, with `crt1.o`, so a
program needs nothing else from picolibc. It uses no BSD headers or
libraries: the system calls it makes are written out in `local-bsd.h`.

The library is built for the i486 (`-march=i486`, x87 floating point);
programs for clang's default CPU, the Pentium 4, link with it too.

Tested with bsdrun (unix-sdk's `bsd/tools/bsdrun.c`, which runs static
i386 FreeBSD and NetBSD programs on Linux by translating their system
calls):
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

For NetBSD, use `scripts/cross-clang-i386-unknown-netbsd.txt`.

unix-sdk's `bsd/build-sysroot.sh` puts this together with compiler-rt's
builtins into a sysroot laid out as clang's FreeBSD or NetBSD driver
expects.

## What it does

- `crt1.S` calls `__bsd_start` in `start.c` with the stack the kernel set
  up (argc, argv and the environment), which makes `argv` and `environ`,
  runs the static constructors and calls `main`. For NetBSD it also has
  the `.note.netbsd.ident` note, without which NetBSD's kernel won't run
  the program as a NetBSD one.
- System calls (`syscall-i386.S`) trap with `int $0x80`, the number in
  `eax` and the arguments on the stack, as on every BSD for x86. On
  failure the kernel sets the carry flag, and BSD error numbers become
  picolibc's `errno` values (`error.c`).
- The calls are the 4.4BSD ones every FreeBSD since 3.0 and NetBSD since
  1.5 has. On FreeBSD, `stat` (188) and `lseek` (199) are the old ones,
  which FreeBSD 12 and 7 and later keep under `COMPAT_FREEBSD11` and
  `COMPAT_FREEBSD6` (both in the GENERIC kernel). On NetBSD, `stat` is
  `__stat13` (278), and `gettimeofday` has a 32-bit `time_t`, which NetBSD
  3.0 and 5.0 and later keep under `COMPAT_30` and `COMPAT_50` (in
  GENERIC).
- Files: `open` (with the BSD `O_` flag values), `read`, `write`, `lseek`
  (64-bit offsets), `close`, `stat`, `lstat`, `fstat`, `isatty`
  (`TIOCGETA`), `unlink`, `rename`, `mkdir`, `rmdir`, `chdir`, `access`.
- `sbrk` grows the heap from the end of the bss with `break`.
- `getpid`, `kill`, `gettimeofday`, `_exit`. Signals are those of the
  fallback library: `raise` works.

## Limits

- Only FreeBSD and NetBSD on 32-bit x86 for now. OpenBSD, which drops
  old system calls instead of keeping them, comes later.
- Not yet tested on a real FreeBSD or NetBSD machine, only in bsdrun.
- A FreeBSD 12 or later kernel built without `COMPAT_FREEBSD11` can't run
  the programs: they use the old `stat`.
