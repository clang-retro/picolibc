---
hide:
  - navigation
---

# clang-retro picolibc

This is [picolibc](https://github.com/picolibc/picolibc), the C library
for small systems, with the system layers that make it the C library of
every clang-retro target: MS-DOS and 16-bit Windows, OS/2, 32-bit
Windows, AmigaOS, Atari TOS and SGI IRIX. These pages describe only what the fork
adds; for the library itself see [picolibc's own
documentation](https://github.com/picolibc/picolibc/tree/main/doc).

<div class="grid cards" markdown>

-   :material-console: **[MS-DOS and 16-bit Windows](msdos.md)**

    ---

    Startup code for every memory model, DOS system calls, text files,
    interrupt handlers, resident programs, `<conio.h>` and Win16
    programs and DLLs.

-   :material-alpha-o-box: **[OS/2](os2.md)**

    ---

    16-bit OS/2 1.x (NE) and 32-bit OS/2 (LX) programs and DLLs.

-   :material-microsoft-windows: **[32-bit Windows](win32.md)**

    ---

    picolibc as the C library of Win32 programs, with no C runtime DLL,
    from Windows NT 3.1 and 95 on.

-   :material-alpha-a-box: **[AmigaOS](amigaos.md)**

    ---

    Startup code, `malloc`, files and time on Kickstart 1.3 to 3.x.

-   :material-alpha-t-box: **[Atari TOS](tos.md)**

    ---

    GEMDOS startup code, memory, files, directories and time.

-   :material-alpha-i-box: **[SGI IRIX](irix.md)**

    ---

    Static programs for IRIX 5.3 and 6.x on MIPS, in the o32, n32 and
    64-bit ABIs.

-   :material-hammer-wrench: **[Building](building.md)**

    ---

    The meson options and cross files for each system.

</div>

## Other changes

- An `i8086` target, with the far and huge data models, and
  integer-only `printf` and `scanf` entry points that clang calls when the
  format allows.
- `stdout` and `stderr` without a read function, and the declarations
  that libc++ needs visible in C++.
- `remove` removes empty directories on DOS.

## License

The fork's changes are under picolibc's licenses; see
[COPYING.picolibc](https://github.com/clang-retro/picolibc/blob/main/COPYING.picolibc).
The SDKs that use it are documented on their own sites: see [all
clang-retro projects](https://clang-retro.github.io/).
