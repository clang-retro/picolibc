#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2026 clang-dos contributors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.


# Static programs for 32-bit AIX: startup code, and wrappers for the kernel
# calls that /unix exports under names other than POSIX ones. These go into
# libc itself; a program links with crt0.o, libc, and unix.exp, the list of
# kernel calls for the linker.

has_os_library = true
has_os_posix = true
has_os_fileio = true

# One call per file, so a program imports only the kernel calls it uses.
srcs_os_aix = [
  'access.c',
  'error.c',
  'getpid.c',
  'heap.c',
  'isatty.c',
  'lseek.c',
  'millicode.c',
  'open.c',
  'read.c',
  'write.c',
]

# Signals, time and sysconf come from the fallback library.
srcs_os_aix_fallback = [
  'fpathconf.c',
  'pathconf.c',
  'raise.c',
  'signal.c',
  'sigprocmask.c',
  'sysconf.c',
]

srcs_libos += srcs_os_aix + srcs_os_aix_fallback

src_os_aix = files(srcs_os_aix)
foreach f : srcs_os_aix_fallback
  src_os_aix += files('..' / 'fallback' / f)
endforeach

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']

  instdir = join_paths(lib_dir, target_dir)

  custom_target('crt0' + target,
                input : 'crt0.c',
                output : 'crt0.o',
                command : cc.cmd_array() + target_c_args + c_args +
                          ['-I' + meson.current_source_dir(),
                           '-I' + meson.project_build_root(),
                           '-I' + meson.project_source_root() / 'libc' / 'include',
                           '-c', '@INPUT@', '-o', '@OUTPUT@'],
                install : really_install,
                install_dir : instdir,
                build_by_default : true)

  install_data('unix.exp', 'libc.exp', install_dir : instdir)
endforeach
