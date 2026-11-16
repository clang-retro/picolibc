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
#include <ctype.h>
#include <string.h>
#include <strings.h>

/* Characters CP/M's CCP would not put in a file name. */
static int
bad_char(char c)
{
    return c <= ' ' || c >= 0x7f || strchr("<>.,;:=?*[]|()/\\\"", c) != NULL;
}

static int
fill(char *out, size_t len, const char **pp, int wild)
{
    const char *p = *pp;
    size_t      n = 0;
    memset(out, ' ', len);
    while (*p && *p != '.') {
        char c = *p++;
        if (wild && c == '*') {
            while (n < len)
                out[n++] = '?';
            continue;
        }
        if (wild && c == '?') {
            if (n < len)
                out[n++] = '?';
            continue;
        }
        if (bad_char(c) || n >= len)
            return -1;
        out[n++] = (char)toupper((unsigned char)c);
    }
    *pp = p;
    return 0;
}

int
__cpm_parse_name(struct __cpm_fcb *fcb, const char *path, int wild)
{
    memset(fcb, 0, sizeof(*fcb));
    const char *p = path;
    /* A drive: "b:name.typ". */
    if (p[0] && p[1] == ':') {
        char d = (char)toupper((unsigned char)p[0]);
        if (d < 'A' || d > 'P')
            goto bad;
        fcb->dr = (uint8_t)(d - 'A' + 1);
        p += 2;
    }
    if (*p == '\0' || *p == '.')
        goto bad;
    if (fill(fcb->name, sizeof(fcb->name), &p, wild) < 0)
        goto bad;
    if (*p == '.') {
        p++;
        if (fill(fcb->type, sizeof(fcb->type), &p, wild) < 0 || *p == '.')
            goto bad;
    }
    if (*p != '\0')
        goto bad;
    return 0;
bad:
    errno = ENOENT;
    return -1;
}

/* CON:, LST:, PUN:, AUX:, RDR: and NUL:, in any case. */
int
__cpm_device_name(const char *path)
{
    static const struct {
        char    name[5];
        uint8_t dev;
    } devices[] = {
        { "CON:", __CPM_DEV_CON },
        { "LST:", __CPM_DEV_LST },
        { "PRN:", __CPM_DEV_LST },
        { "PUN:", __CPM_DEV_PUN },
        { "AUX:", __CPM_DEV_PUN },
        { "RDR:", __CPM_DEV_RDR },
        { "NUL:", __CPM_DEV_NUL },
    };
    for (size_t i = 0; i < sizeof(devices) / sizeof(devices[0]); i++)
        if (strcasecmp(path, devices[i].name) == 0)
            return devices[i].dev;
    return __CPM_DEV_NONE;
}
