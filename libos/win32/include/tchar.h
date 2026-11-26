/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 clang-dos contributors
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

/* The generic text names of the Windows C library. With _UNICODE
 * defined, _TCHAR is wchar_t and the names are the wide functions;
 * otherwise _TCHAR is char and the names are the byte functions. _MBCS
 * gives the byte functions too: this library has no _mbs functions, so
 * text in a multibyte code page is handled one byte at a time. */

#ifndef _TCHAR_H_
#define _TCHAR_H_

#include <ctype.h>
#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

#ifdef _UNICODE

#ifndef __TCHAR_DEFINED
typedef wchar_t _TCHAR;
typedef wchar_t _TSCHAR;
typedef wchar_t _TUCHAR;
typedef wchar_t _TXCHAR;
typedef wint_t  _TINT;
#define __TCHAR_DEFINED
#endif
#if !defined(_TCHAR_DEFINED) && !__STRICT_ANSI__
typedef wchar_t TCHAR;
#define _TCHAR_DEFINED
#endif

#define __T(x)       L##x
#define _TEOF        WEOF

#define _tmain       wmain
#define _tWinMain    wWinMain

#define _tprintf     wprintf
#define _ftprintf    fwprintf
#define _sntprintf   _snwprintf
#define _vtprintf    vwprintf
#define _vftprintf   vfwprintf
#define _vsntprintf  _vsnwprintf
#define _tscanf      wscanf
#define _ftscanf     fwscanf
#define _stscanf     swscanf

#define _fgettc      fgetwc
#define _fgetts      fgetws
#define _fputtc      fputwc
#define _fputts      fputws
#define _gettc       getwc
#define _puttc       putwc
#define _gettchar    getwchar
#define _puttchar    putwchar
#define _ungettc     ungetwc

#define _tfopen      _wfopen
#define _tfreopen    _wfreopen
#define _topen       _wopen
#define _tremove     _wremove
#define _trename     _wrename
#define _tunlink     _wunlink
#define _taccess     _waccess
#define _tchmod      _wchmod
#define _tstat       _wstat
#define _tstati64    _wstati64
#define _tstat64     _wstat64
#define _tgetcwd     _wgetcwd
#define _tchdir      _wchdir
#define _tmkdir      _wmkdir
#define _trmdir      _wrmdir
#define _tfinddata_t _wfinddata_t
#define _tfindfirst  _wfindfirst
#define _tfindnext   _wfindnext

#define _tcslen      wcslen
#define _tcscpy      wcscpy
#define _tcsncpy     wcsncpy
#define _tcscat      wcscat
#define _tcsncat     wcsncat
#define _tcscmp      wcscmp
#define _tcsncmp     wcsncmp
#define _tcsicmp     _wcsicmp
#define _tcsnicmp    _wcsnicmp
#define _tcscoll     wcscoll
#define _tcsicoll    _wcsicoll
#define _tcschr      wcschr
#define _tcsrchr     wcsrchr
#define _tcsstr      wcsstr
#define _tcspbrk     wcspbrk
#define _tcsspn      wcsspn
#define _tcscspn     wcscspn
#define _tcstok      wcstok
#define _tcsdup      _wcsdup
#define _tcslwr      _wcslwr
#define _tcsupr      _wcsupr
#define _tcsrev      _wcsrev
#define _tcsset      _wcsset
#define _tcsnset     _wcsnset
#define _tcsftime    wcsftime

#define _tcstol      wcstol
#define _tcstoul     wcstoul
#define _tcstoi64    wcstoll
#define _tcstoui64   wcstoull
#define _tcstod      wcstod
#define _ttoi        _wtoi
#define _ttol        _wtol
#define _ttoi64      _wtoi64
#define _itot        _itow
#define _ltot        _ltow
#define _ultot       _ultow

#define _istalnum    iswalnum
#define _istalpha    iswalpha
#define _istcntrl    iswcntrl
#define _istdigit    iswdigit
#define _istgraph    iswgraph
#define _istlower    iswlower
#define _istprint    iswprint
#define _istpunct    iswpunct
#define _istspace    iswspace
#define _istupper    iswupper
#define _istxdigit   iswxdigit
#define _totlower    towlower
#define _totupper    towupper

#else /* !_UNICODE */

#ifndef __TCHAR_DEFINED
typedef char          _TCHAR;
typedef signed char   _TSCHAR;
typedef unsigned char _TUCHAR;
typedef char          _TXCHAR;
typedef int           _TINT;
#define __TCHAR_DEFINED
#endif
#if !defined(_TCHAR_DEFINED) && !__STRICT_ANSI__
typedef char TCHAR;
#define _TCHAR_DEFINED
#endif

#define __T(x)          x
#define _TEOF           EOF

#define _tmain          main
#define _tWinMain       WinMain

#define _tprintf        printf
#define _ftprintf       fprintf
#define _stprintf       sprintf
#define _sntprintf      _snprintf
#define _vtprintf       vprintf
#define _vftprintf      vfprintf
#define _vstprintf      vsprintf
#define _vsntprintf     _vsnprintf
#define _tscanf         scanf
#define _ftscanf        fscanf
#define _stscanf        sscanf

#define _fgettc         fgetc
#define _fgetts         fgets
#define _fputtc         fputc
#define _fputts         fputs
#define _gettc          getc
#define _puttc          putc
#define _gettchar       getchar
#define _puttchar       putchar
#define _putts          puts
#define _ungettc        ungetc

#define _tfopen         fopen
#define _tfreopen       freopen
#define _topen          _open
#define _tremove        remove
#define _trename        rename
#define _tunlink        _unlink
#define _taccess        _access
#define _tchmod         _chmod
#define _tstat          _stat
#define _tstati64       _stati64
#define _tstat64        _stat64
#define _tgetcwd        _getcwd
#define _tchdir         _chdir
#define _tmkdir         _mkdir
#define _trmdir         _rmdir
#define _tfinddata_t    _finddata_t
#define _tfindfirst     _findfirst
#define _tfindnext      _findnext
#define _tgetenv        getenv
#define _tsystem        system
#define _tfullpath      _fullpath
#define _tsplitpath     _splitpath
#define _tmakepath      _makepath
#define _tputenv        _putenv

#define _tcslen         strlen
#define _tcsclen        strlen
#define _tcscpy         strcpy
#define _tcsncpy        strncpy
#define _tcscat         strcat
#define _tcsncat        strncat
#define _tcscmp         strcmp
#define _tcsncmp        strncmp
#define _tcsicmp        _stricmp
#define _tcsnicmp       _strnicmp
#define _tcscoll        strcoll
#define _tcsicoll       _stricoll
#define _tcschr         strchr
#define _tcsrchr        strrchr
#define _tcsstr         strstr
#define _tcspbrk        strpbrk
#define _tcsspn         strspn
#define _tcscspn        strcspn
#define _tcstok         strtok
#define _tcsdup         _strdup
#define _tcslwr         _strlwr
#define _tcsupr         _strupr
#define _tcsrev         _strrev
#define _tcsset         _strset
#define _tcsnset        _strnset
#define _tcsftime       strftime
#define _tcsinc(_p)     ((_p) + 1)
#define _tcsdec(_s, _p) ((_p) > (_s) ? (_p) - 1 : NULL)
#define _tcsnextc(_p)   ((unsigned)*(const unsigned char *)(_p))

#define _tcstol         strtol
#define _tcstoul        strtoul
#define _tcstoi64       _strtoi64
#define _tcstoui64      _strtoui64
#define _tcstod         strtod
#define _ttoi           atoi
#define _ttol           atol
#define _ttoi64         _atoi64
#define _itot           _itoa
#define _ltot           _ltoa
#define _ultot          _ultoa

#define _istalnum       isalnum
#define _istalpha       isalpha
#define _istcntrl       iscntrl
#define _istdigit       isdigit
#define _istgraph       isgraph
#define _istlower       islower
#define _istprint       isprint
#define _istpunct       ispunct
#define _istspace       isspace
#define _istupper       isupper
#define _istxdigit      isxdigit
#define _totlower       tolower
#define _totupper       toupper

#endif /* _UNICODE */

#define _T(x)    __T(x)
#define _TEXT(x) __T(x)

#endif /* _TCHAR_H_ */
