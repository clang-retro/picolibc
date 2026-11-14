# picolibc on Solaris

This directory has Solaris support (Solaris 2.5 to 11.4) on 32-bit SPARC
for static programs, for picolibc on the `sparc-sun-solaris` target
of the [clang-retro LLVM toolchain](https://github.com/clang-retro/llvm).
It goes into `libc.a` itself, with `crt1.o`, so a program needs nothing
else from picolibc. It uses no Sun headers or libraries: the system calls
it makes are written out in `local-solaris.h`.

The library is built for SPARC V8 (`-mcpu=v8`), which runs on every SPARC
machine these Solaris releases support, from the SPARCstation 2 to the
UltraSPARC. Programs can be built for V9 CPUs (clang's default on Solaris,
V8+) and link with it.

Tested with qemu-solaris (user-mode emulation of 32-bit Solaris/SPARC
programs, part of qemu-irix), as Solaris 2.x and as Solaris 11: `printf` with floating point, `long
double`, `malloc`, files, `stat`, errors, `argv`, `isatty`, static
constructors and the exit status.

## Building

With clang, `llvm-ar` and the other LLVM tools on `PATH`:

```sh
meson setup build-solaris \
    --cross-file scripts/cross-clang-sparc-sun-solaris.txt \
    -Dos-solaris=true -Dmultilib=false -Dpicocrt=false -Dsemihost=false \
    -Dtests=false -Dposix-console=true -Dsingle-thread=true \
    -Dthread-local-storage=false -Dspecsdir=none \
    --prefix=$PWD/install-solaris
ninja -C build-solaris install
```

Copy `include/` to `<sysroot>/usr/include` and `lib/crt1.o`, `lib/libc.a`
and `lib/libm.a` to `<sysroot>/usr/lib`, with compiler-rt's builtins for
`sparc-sun-solaris` (built with `-mcpu=v8`) as
`usr/lib/libclang_rt.builtins.a`. Then:

```sh
clang --target=sparc-sun-solaris -mcpu=v8 --sysroot=<sysroot> -static hello.c
```

## What it does

- `crt1.S` calls `__solaris_start` in `start.c` with the stack the kernel
  set up (argc, argv and the environment above a 64-byte register save
  area), which makes `argv` and `environ`, runs the static constructors
  and calls `main`.
- System calls trap with `ta 8`, the SVR4 number in `g1`; on failure the
  kernel sets the carry flag, and Solaris error numbers become picolibc's
  `errno` values.
- Solaris 11 dropped `open`, `stat`, `lstat`, `unlink`, `mkdir` and other
  system calls for `openat`, `fstatat`, `unlinkat` and the other `*at`
  calls. The first time it needs one, the library asks the kernel its
  release (`sysinfo(SI_RELEASE)`), and on SunOS 5.11 and later (Solaris
  11, and illumos) it uses the `*at` calls; before that, the old ones.
- Files: `open` (with Solaris's `O_` flag values), `read`, `write`,
  `lseek`, `close`, `stat`, `lstat`, `fstat`, `isatty`, `unlink`,
  `rename`, `mkdir`, `rmdir`, `chdir`, `access`.
- `sbrk` grows the heap from the end of the bss with `brk`.
- `getpid`, `kill`, `gettimeofday`, `_exit`. Signals are those of the
  fallback library: `raise` works.
- `qp.c` has the SPARC ABI's quad precision routines (`_Q_add`, `_Q_cmp`,
  `_Q_qtod` and the others), which the compiler calls for `long double`
  and which Solaris has in its libc. They call compiler-rt's soft-float
  routines.

## Limits

- The Solaris 11 path is tested in qemu-solaris run as SunOS 5.11 (`-r
  5.11`, with unix-sdk's patch), not yet on a real Solaris 11 system.
- `lseek` offsets are limited to 32 bits (it doesn't use `llseek`).
- Only 32-bit SPARC for now: not 64-bit SPARC (`sparcv9`) or x86.
