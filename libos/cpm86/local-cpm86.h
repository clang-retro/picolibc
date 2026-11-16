
#ifndef _LOCAL_CPM86_H_
#define _LOCAL_CPM86_H_

#define _GNU_SOURCE

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* CP/M-86 BDOS calls (INT E0h): the function in CL, the parameter in DX
 * (an offset in DS for addresses). Returns AX (byte results in AL). */
uint16_t __cpm_bdos(unsigned fn, uint16_t dx);

#define __CPM_PTR(p) ((uint16_t)(uintptr_t)(const void *)(p))

/* BDOS functions. */
#define BDOS_RESET        0
#define BDOS_CONIN        1
#define BDOS_CONOUT       2
#define BDOS_AUXIN        3
#define BDOS_AUXOUT       4
#define BDOS_LSTOUT       5
#define BDOS_CONIO        6
#define BDOS_PRINT        9
#define BDOS_READLINE     10
#define BDOS_CONST        11
#define BDOS_VERSION      12
#define BDOS_OPEN         15
#define BDOS_CLOSE        16
#define BDOS_SEARCH_FIRST 17
#define BDOS_SEARCH_NEXT  18
#define BDOS_DELETE       19
#define BDOS_MAKE         22
#define BDOS_RENAME       23
#define BDOS_SELECT_DISK  14
#define BDOS_GET_DISK     25
#define BDOS_SET_DMA      26
#define BDOS_SET_ATTR     30
#define BDOS_USER         32
#define BDOS_READ_RANDOM  33
#define BDOS_WRITE_RANDOM 34
#define BDOS_FILE_SIZE    35
#define BDOS_WRITE_ZERO   40
#define BDOS_SET_DMA_SEG  51
#define BDOS_RETURN_CODE  108 /* CP/M-86 Plus; ignored by CP/M-86 1.1 */
#define BDOS_DATE_TIME    105 /* CP/M-86 Plus */

/* A file control block, with the random record. */
struct __cpm_fcb {
    uint8_t dr;
    char    name[8];
    char    type[3];
    uint8_t ex, s1, s2, rc;
    uint8_t d[16];
    uint8_t cr;
    uint8_t r[3];
};

/* Fill an FCB from a name such as "b:hello.txt" (any case). Returns 0, or
 * -1 with errno set for a name CP/M can't have. Wildcards (* and ?) are
 * taken when wild is set. */
int __cpm_parse_name(struct __cpm_fcb *fcb, const char *path, int wild);

/* The character devices that open() knows. */
enum {
    __CPM_DEV_NONE,
    __CPM_DEV_CON, /* CON: the console */
    __CPM_DEV_LST, /* LST: the printer */
    __CPM_DEV_PUN, /* PUN: or AUX: the punch (output) */
    __CPM_DEV_RDR, /* RDR: the reader (input) */
    __CPM_DEV_NUL, /* NUL: nothing */
};
int __cpm_device_name(const char *path);

#define __CPM_RECORD 128

/* An open file descriptor. Files are read and written a record at a time
 * through buf, with random reads and writes. */
struct __cpm_file {
    uint8_t          flags;
    uint8_t          device;
    struct __cpm_fcb fcb;
    off_t            pos;     /* byte position */
    off_t            size;    /* bytes: the end of the file as known */
    int32_t          bufrec;  /* the record in buf, or -1 */
    uint8_t          dirty;
    uint8_t          buf[__CPM_RECORD];
};

#define __CPM_FD_OPEN   0x01
#define __CPM_FD_READ   0x02
#define __CPM_FD_WRITE  0x04
#define __CPM_FD_TEXT   0x08 /* CR LF line ends, Ctrl-Z ends */
#define __CPM_FD_APPEND 0x10
#define __CPM_FD_EOF    0x20 /* a text read reached Ctrl-Z */
#define __CPM_FD_WROTE  0x40 /* written: close updates the directory */

#define __CPM_MAX_FD 16
extern struct __cpm_file *__cpm_fds[__CPM_MAX_FD];

/* The file of a descriptor, or NULL with errno EBADF. */
struct __cpm_file *__cpm_fd(int fd);
int                __cpm_new_fd(struct __cpm_file *f);
/* Whether stdio sees fd as a text file (see local-stdio.h). */
int                __cpm_is_text(int fd);

/* Write out buf if it holds changes. Returns 0 or -1 (errno set). */
int                __cpm_flush(struct __cpm_file *f);
/* Make buf hold record rec. Returns 0, 1 past the end of the file, or -1. */
int                __cpm_load(struct __cpm_file *f, int32_t rec);

/* Console I/O for fds 0 to 2 and the devices. */
ssize_t            __cpm_dev_read(struct __cpm_file *f, char *buf, size_t n);
ssize_t            __cpm_dev_write(struct __cpm_file *f, const char *buf, size_t n);
void               __cpm_conout(uint8_t device, char c);

/* The base page, at DS:0000 (crt0 reserves it). */
extern uint8_t     __cpm_base_page[256] __asm__("___cpm_base_page");

/* Heap bounds. crt0 defines __heap_start after the stack. */
extern char        __cpm_heap_start[] __asm__("___heap_start");
extern char       *__cpm_brk;
extern char       *__cpm_heap_end;

/* The BDOS version (function 12), read at startup: 0022h for CP/M-86 1.x,
 * 1031h for CP/M-86 Plus, 1431h for Concurrent CP/M. */
extern uint16_t    __cpm_version;

void               __cpm_start(void);

#endif /* _LOCAL_CPM86_H_ */
