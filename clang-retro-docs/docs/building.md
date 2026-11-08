# Building

Each system layer is a meson option, off by default:

| Option | Layer | Cross files in `scripts/` |
| --- | --- | --- |
| `-Dos-msdos=true` | `libos/msdos`: DOS and 16-bit Windows | `cross-clang-i8086-msdos-<model>.txt`, `cross-clang-i8086-win16-<model>.txt` |
| `-Dos-os2=true` | `libos/os2`: OS/2 1.x and 2.x and later | `cross-clang-i8086-os2-<model>.txt`, `cross-clang-i386-os2.txt` |
| `-Dos-win32=true` | `libos/win32`: 32-bit Windows | `cross-clang-i386-windows.txt` |
| `-Dos-amigaos=true` | `libos/amigaos`: AmigaOS | `cross-clang-m68k-amigaos.txt` |
| `-Dos-tos=true` | `libos/tos`: Atari TOS | `cross-clang-m68k-atari-tos.txt` |

The 16-bit models are `tiny`, `small`, `medium`, `compact`, `large`,
`huge` and `huge16` for DOS, and `small` to `large` for Windows and OS/2.

You rarely need to run meson by hand: each SDK's `build-sysroot.sh` builds
picolibc with the right options and installs it with the compiler-rt
builtins and the system headers. See
[building a sysroot](https://clang-retro.github.io/dos-sdk/dev/sysroot/)
in dos-sdk.

## CI

`.github/workflows/clang-dos-toolchain.yml` builds the clang-dos
toolchain package with each pull request's commit of picolibc and runs
its smoke test on every target, using dos-sdk's toolchain workflow and
the prebuilt LLVM tools of clang-retro/llvm.
