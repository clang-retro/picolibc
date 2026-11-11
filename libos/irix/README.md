# picolibc on SGI IRIX

This directory has SGI IRIX support (IRIX 5.3 and 6.x) for static
programs, for picolibc on the `mips-sgi-irix` and `mips64-sgi-irix` targets
of the [clang-retro LLVM toolchain](https://github.com/clang-retro/llvm).
It goes into `libc.a` itself, with `crt1.o`, so a program needs nothing
else from picolibc. It uses no SGI headers or libraries: the system calls
it makes are written out in `local-irix.h`.

Programs that link dynamically against IRIX's own `libc.so.1` use IRIX's
headers and libraries instead, and don't need this.

Tested with qemu-irix (user-mode emulation of IRIX o32, n32 and 64
programs): `printf` with floating point, `malloc`, files, `stat`, errors,
`argv`, `isatty` and the exit status.

## Building

With clang, `llvm-ar` and the other LLVM tools on `PATH`, for each ABI
(`o32`, `n32`, `n64`):

```sh
meson setup build-irix-n32 \
    --cross-file scripts/cross-clang-mips-sgi-irix-n32.txt \
    -Dos-irix=true -Dmultilib=false -Dpicocrt=false -Dsemihost=false \
    -Dtests=false -Dposix-console=true -Dsingle-thread=true \
    -Dthread-local-storage=false -Dspecsdir=none \
    --prefix=$PWD/install-irix-n32
ninja -C build-irix-n32 install
```

Copy `include/` to `<sysroot>/usr/include` and `lib/crt1.o`, `lib/libc.a`
and `lib/libm.a` to `<sysroot>/usr/lib` (o32), `usr/lib32` (n32) or
`usr/lib64` (64). Then:

```sh
clang --target=mips-sgi-irix6.5 --sysroot=<sysroot> -static hello.c
clang --target=mips64-sgi-irix6.5 -mabi=n32 --sysroot=<sysroot> -static hello.c
```

## What it does

- `crt1.S` sets `gp`, and calls `__irix_start` in `start.c` with the
  stack the kernel set up, which makes `argv` and `environ` and calls
  `main`.
- System calls go through `syscall` with `v0` = 1000 + the SVR4 number;
  IRIX error numbers become picolibc's `errno` values.
- Files: `open` (with IRIX's `O_` flag values), `read`, `write`, `lseek`,
  `close`, `stat`, `lstat`, `fstat` (IRIX's `xstat` calls, struct version
  2 for o32 and 3, `stat64`, for n32 and 64), `isatty`, `unlink`,
  `rename`, `mkdir`, `rmdir`, `chdir`, `access`.
- `sbrk` grows the heap from the end of the bss with `brk`.
- `getpid`, `kill`, `gettimeofday`, `_exit`. Signals are those of the
  fallback library: `raise` works.

## Limits

- `lseek` passes the offset in one register, so on n32 offsets are
  limited to 32 bits.
- In the 64-bit ABI, `time_t` is taken to be a `long`, as in IRIX's
  headers. qemu-irix agrees for `gettimeofday` but writes 32-bit times
  into `stat64`, so file times there are wrong under qemu-irix; this
  needs checking on a real IRIX 6.5 system.
