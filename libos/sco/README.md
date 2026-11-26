# picolibc on SCO UnixWare and OpenServer

This directory has support for SCO UnixWare (2 and later) and OpenServer
(5 and later) on 32-bit x86 for static programs, for picolibc on the
`i386-pc-sco` target of the
[clang-retro LLVM toolchain](https://github.com/clang-retro/llvm).
It goes into `libc.a` itself, with `crt1.o`, so a program needs nothing
else from picolibc. It uses no SCO headers or libraries: the system calls
it makes are written out in `local-sco.h`.

The library is built for the i386 with x87 floating point, clang's default
for `i386-pc-sco`, so it runs on every PC these systems support.

Tested with scorun (in [sco-sdk](https://github.com/clang-retro/sco-sdk),
a ptrace tool that runs static SCO programs on Linux), with UnixWare's and
with OpenServer's error numbers: `printf` with floating point, `malloc`,
files, directories, `stat`, errors, `argv`, static constructors and the
exit status. Not yet on a real UnixWare or OpenServer system.

## Building

With clang, `llvm-ar` and the other LLVM tools on `PATH`:

```sh
meson setup build-sco \
    --cross-file scripts/cross-clang-i386-pc-sco.txt \
    -Dos-sco=true -Dmultilib=false -Dpicocrt=false -Dsemihost=false \
    -Dtests=false -Dposix-console=true -Dsingle-thread=true \
    -Dthread-local-storage=false -Dspecsdir=none \
    --prefix=$PWD/install-sco
ninja -C build-sco install
```

Copy `include/` to `<sysroot>/usr/include` and `lib/crt1.o`, `lib/libc.a`
and `lib/libm.a` to `<sysroot>/usr/lib`, with compiler-rt's builtins for
`i386-pc-sco` (with the C versions of the `__float*di*` conversions, since
compiler-rt's assembly for them uses SSE2) as
`usr/lib/libclang_rt.builtins.a`. sco-sdk's `build-sysroot.sh` does all of
this. Then:

```sh
clang --target=i386-pc-sco --sysroot=<sysroot> hello.c
```

## What it does

- `crt1.S` calls `__sco_start` in `start.c` with the stack the kernel set
  up (argc, argv and the environment), which makes `argv` and `environ`,
  runs the static constructors and calls `main`.
- System calls go through the `lcall $7,$0` gate (`syscall-i386.S`), the
  number in `eax` and the arguments on the stack, as on every SVR4 system
  for x86. On failure the kernel sets the carry flag.
- UnixWare is SVR4.2 and OpenServer SVR3.2 with iBCS2, so the layer uses
  only the calls both number the same (`local-sco.h`), the open flags they
  share, and the old (SVR3) `struct stat` of the `stat` and `fstat` calls.
- Error numbers are UnixWare's (SVR4's), and OpenServer's where they
  differ: `ENOTEMPTY` is 93 or 145 and `ELOOP` 90 or 150 (`error.c`).
- Files: `open`, `read`, `write`, `lseek`, `close`, `stat`, `lstat`,
  `fstat`, `isatty` (with the `TCGETA` ioctl), `unlink`, `rename`, `mkdir`,
  `rmdir`, `chdir`, `access`.
- `sbrk` grows the heap from the end of the bss with `brk`.
- `getpid`, `kill`, `gettimeofday`, `_exit`. Signals are those of the
  fallback library: `raise` works.

## Limits

- `rename` is `link` and then `unlink`: it replaces an existing file, but
  isn't atomic and doesn't rename directories. (OpenServer has no `rename`
  call of the SVR4 number.)
- `lstat` is `stat`, since the two systems number `lstat` differently.
- `gettimeofday` has whole seconds: it uses `time`, the one clock call
  both have.
- The old `struct stat` has 16-bit inode numbers, user and group IDs and
  device numbers, and `lseek` offsets are 32 bits.
