/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 clang-retro contributors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "local-cpm86.h"
#include <sys/time.h>
#include <time.h>

/* CP/M-86 Plus, CP/M 3, MP/M and Concurrent CP/M keep the date and time:
 * BDOS function 105 fills in the days since 31 December 1977 and the hour
 * and minute in BCD, and returns the second in BCD. It is local time,
 * taken as UTC here. CP/M-86 1.1 and CP/M 2.2 have no clock: the low byte
 * of the version is below 30h. */
static int
cpm_time(struct timespec *ts)
{
    if ((__cpm_version & 0xff) < 0x30) {
        errno = ENOSYS;
        return -1;
    }
    struct {
        uint16_t days;
        uint8_t  hour, minute;
    } t;
    uint8_t s = (uint8_t)__cpm_bdos(BDOS_DATE_TIME, __CPM_PTR(&t));
#define BCD(x) ((((x) >> 4) & 15) * 10 + ((x) & 15))
    /* Day 1 is 1 January 1978, 2922 days after 1 January 1970. */
    int64_t days = (int64_t)t.days - 1 + 2922;
    ts->tv_sec = (time_t)(days * 86400 + BCD(t.hour) * 3600L + BCD(t.minute) * 60L + BCD(s));
    ts->tv_nsec = 0;
    return 0;
}

int
clock_gettime(clockid_t clock_id, struct timespec *ts)
{
    (void)clock_id;
    return cpm_time(ts);
}

int
gettimeofday(struct timeval * restrict tv, void * restrict tz)
{
    (void)tz;
    struct timespec ts;
    if (cpm_time(&ts) < 0)
        return -1;
    if (tv) {
        tv->tv_sec = ts.tv_sec;
        tv->tv_usec = 0;
    }
    return 0;
}
