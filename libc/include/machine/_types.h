/*
 *  Copyright (c) 2005 Ralf Corsepious  <ralf.corsepius@rtems.org>
 */

#ifndef _MACHINE__TYPES_H
#define _MACHINE__TYPES_H
#include <machine/_default_types.h>

#if defined(__i8080__)
/* CP/M-80 (the 8080 and the Z80): a file is at most 8 MiB, or 32 MiB on
   CP/M 3, so 32-bit offsets are plenty, and 64-bit arithmetic is large
   and slow on these processors. */
#define __machine_off_t_defined
typedef __int32_t __off_t;
#define __machine_blkcnt_t_defined
typedef __int32_t __blkcnt_t;
#endif
#endif
