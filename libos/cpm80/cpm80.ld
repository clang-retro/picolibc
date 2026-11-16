/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2026 clang-retro contributors */

/* CP/M-80 programs: CP/M loads the .COM file at 0100h. The file holds the
 * code, starting with crt0, the read-only data and the data; the BSS and
 * then the heap follow it. */
ENTRY(_start)
SECTIONS {
  . = 0x100;
  .text : {
    KEEP(*(.text.start))
    *(.text .text.*)
  }
  .rodata : {
    *(.rodata .rodata.*)
    . = ALIGN(2);
    __bothinit_array_start = .;
    __preinit_array_start = .;
    KEEP(*(.preinit_array))
    __preinit_array_end = .;
    __init_array_start = .;
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP(*(.init_array .ctors))
    __init_array_end = .;
    __bothinit_array_end = .;
    __fini_array_start = .;
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP(*(.fini_array .dtors))
    __fini_array_end = .;
  }
  .data : { *(.data .data.*) }
  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss .bss.* COMMON)
    __bss_end = .;
  }
  ___heap_start = .;
  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
