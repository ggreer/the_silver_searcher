## aclocal.m4
## --------------------------------------------------------------------------
##
##      Pthreads4w - POSIX Threads for Windows
##      Copyright 1998 John E. Bossom
##      Copyright 1999-2018, Pthreads4w contributors
##
##      Homepage: https://sourceforge.net/projects/pthreads4w/
##
##      The current list of contributors is contained
##      in the file CONTRIBUTORS included with the source
##      code distribution. The list can also be seen at the
##      following World Wide Web location:
##
##      https://sourceforge.net/p/pthreads4w/wiki/Contributors/
##
##      This library is free software; you can redistribute it and/or
##      modify it under the terms of the GNU Lesser General Public
##      License as published by the Free Software Foundation; either
##      version 3 of the License, or (at your option) any later version.
## 
##      This library is distributed in the hope that it will be useful,
##      but WITHOUT ANY WARRANTY; without even the implied warranty of
##      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##      Lesser General Public License for more details.
## 
##      You should have received a copy of the GNU Lesser General Public
##      License along with this library in the file COPYING.LIB;
##      if not, write to the Free Software Foundation, Inc.,
##      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
##
#
# PTW32_AC_CHECK_TYPEDEF( TYPENAME, [HEADER] )
# --------------------------------------------
# Set HAVE_TYPENAME in config.h, if either HEADER, or any default
# header which autoconf checks automatically, defines TYPENAME.
#
AC_DEFUN([PTW32_AC_CHECK_TYPEDEF],dnl
[m4_ifnblank([$2],[AC_CHECK_HEADERS_ONCE([$2])])
 AC_CHECK_TYPE([$1],dnl
 [AC_DEFINE(AS_TR_CPP([HAVE_$1]),[1],[Define if your compiler knows about $1])],,dnl
 [AC_INCLUDES_DEFAULT
  m4_ifnblank([$2],[[
#ifdef ]AS_TR_CPP([HAVE_$2])[
# include <$2>
#endif
]])])dnl
])

# PTW32_AC_NEED_FUNC( WITNESS, FUNCNAME )
# ---------------------------------------
# Add a WITNESS definition in config.h, if FUNCNAME is not provided
# by the standard library, and a replacement must be provided.
#
AC_DEFUN([PTW32_AC_NEED_FUNC],dnl
[AC_CHECK_FUNCS([$2],,[AC_DEFINE([$1],[1],[Define if you do not have $2])])dnl
])

# PTW32_AC_NEED_ERRNO
# -------------------
# Check if the host provides the <errno.h> header, and supports the
# errno global symbol, otherwise, add a NEED_ERRNO request in config.h
#
AC_DEFUN([PTW32_AC_NEED_ERRNO],[dnl
AC_CHECK_HEADERS_ONCE([errno.h])
AC_MSG_CHECKING([for errno])
AC_LINK_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif
int main(){ return errno; }
]])],dnl
[AC_MSG_RESULT([yes])],dnl
[AC_DEFINE([NEED_ERRNO],[1],[Define if you do not have errno])
 AC_MSG_RESULT([no])dnl
])
])

# PTW32_AC_CHECK_WINAPI_FUNC( FUNCNAME, ARGUMENTS, ... )
# ------------------------------------------------------
# Check if the WinAPI function FUNCNAME is available on the host;
# unlike __cdecl functions, which can be detected by AC_CHECK_FUNCS,
# WinAPI functions need a full argument list specification in the
# function call.  (Additional 3rd and 4th arguments provide for
# qualification of the yes/no messages, respectively; they may
# be exploited, for example, to add config.h annotations).
#
AC_DEFUN([PTW32_AC_CHECK_WINAPI_FUNC],
[AC_MSG_CHECKING([for $1])
 AC_LINK_IFELSE([AC_LANG_SOURCE([[
#include <windows.h>
int APIENTRY WinMain(HINSTANCE curr, HINSTANCE prev, LPSTR argv, int mode)
{ (void)($1($2)); return 0; }
 ]])],dnl
 [AC_MSG_RESULT([yes])$3],
 [AC_MSG_RESULT([no])$4
 ])
])

# PTW32_AC_NEED_WINAPI_FUNC( FUNCNAME, ARGUMENTS )
# ------------------------------------------------
# Check if WinAPI function FUNCNAME is available on the host; add a
# NEED_FUNCNAME annotation in config.h, if it is not.
#
AC_DEFUN([PTW32_AC_NEED_WINAPI_FUNC],
[PTW32_AC_CHECK_WINAPI_FUNC([$1],[$2],,dnl
 [AC_DEFINE(AS_TR_CPP([NEED_$1]),[1],[Define if $1 is unsupported])dnl
 ])
])

# PTW32_AC_CHECK_CPU_AFFINITY
# ---------------------------
# Check if the host supports the GetProcessAffinityMask() WinAPI
# function; (all Windows versions since Win95 should, but WinCE may
# not).  Add the HAVE_CPU_AFFINITY annotation in config.h, for hosts
# which do have this support.
#
AC_DEFUN([PTW32_AC_CHECK_CPU_AFFINITY],
[PTW32_AC_CHECK_WINAPI_FUNC([GetProcessAffinityMask],[NULL,NULL,NULL],dnl
 [AC_DEFINE([HAVE_CPU_AFFINITY],[1],[Define if CPU_AFFINITY is supported])dnl
 ])
])
