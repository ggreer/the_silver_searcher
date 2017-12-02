/*
 * Module: _ptw32.h
 *
 * Purpose:
 *      Pthreads-win32 internal macros, to be shared by other headers
 *      comprising the pthreads-win32 package.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999-2012, 2016, Pthreads-win32 contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 */
#ifndef __PTW32_H
#define __PTW32_H

/* See the README file for an explanation of the pthreads-win32
 * version numbering scheme and how the DLL is named etc.
 *
 * FIXME: consider moving this to <_ptw32.h>; maybe also add a
 * leading underscore to the macro names.
 */
#define PTW32_VERSION_MAJOR 2
#define PTW32_VERSION_MINOR 10
#define PTW32_VERSION_MICRO 0
#define PTW32_VERION_BUILD 0
#define PTW32_VERSION 2,10,0,0
#define PTW32_VERSION_STRING "2, 10, 0, 0\0"

#if defined(__GNUC__)
# pragma GCC system_header
# if ! defined __declspec
#  error "Please upgrade your GNU compiler to one that supports __declspec."
# endif
#endif

#if defined (__cplusplus)
# define __PTW32_BEGIN_C_DECLS  extern "C" {
# define __PTW32_END_C_DECLS    }
#else
# define __PTW32_BEGIN_C_DECLS
# define __PTW32_END_C_DECLS
#endif

#if defined (PTW32_STATIC_LIB) && _MSC_VER >= 1400
# undef PTW32_STATIC_LIB
# define PTW32_STATIC_TLSLIB
#endif

/* When building the library, you should define PTW32_BUILD so that
 * the variables/functions are exported correctly. When using the library,
 * do NOT define PTW32_BUILD, and then the variables/functions will
 * be imported correctly.
 *
 * FIXME: Used defined feature test macros, such as PTW32_STATIC_LIB, (and
 * maybe even PTW32_BUILD), should be renamed with one initial underscore;
 * internally defined macros, such as PTW32_DLLPORT, should be renamed with
 * two initial underscores ... perhaps __PTW32_DECLSPEC is nicer anyway?
 */
#if defined PTW32_STATIC_LIB || defined PTW32_STATIC_TLSLIB
# define PTW32_DLLPORT

#elif defined PTW32_BUILD
# define PTW32_DLLPORT __declspec (dllexport)
#else
# define PTW32_DLLPORT /*__declspec (dllimport)*/
#endif

#ifndef PTW32_CDECL
/* FIXME: another internal macro; should have two initial underscores;
 * Nominally, we prefer to use __cdecl calling convention for all our
 * functions, but we map it through this macro alias to facilitate the
 * possible choice of alternatives; for example:
 */
# ifdef _OPEN_WATCOM_SOURCE
  /* The Open Watcom C/C++ compiler uses a non-standard default calling
   * convention, (similar to __fastcall), which passes function arguments
   * in registers, unless the __cdecl convention is explicitly specified
   * in exposed function prototypes.
   *
   * Our preference is to specify the __cdecl convention for all calls,
   * even though this could slow Watcom code down slightly.  If you know
   * that the Watcom compiler will be used to build both the DLL and your
   * application, then you may #define _OPEN_WATCOM_SOURCE, so disabling
   * the forced specification of __cdecl for all function declarations;
   * remember that this must be defined consistently, for both the DLL
   * build, and the application build.
   */
#  define PTW32_CDECL
# else
#  define PTW32_CDECL __cdecl
# endif
#endif

/*
 * This is more or less a duplicate of what is in the autoconf config.h,
 * which is only used when building the pthread-win32 libraries. They
 */

#if !defined(PTW32_CONFIG_H) && !defined(__PTW32_PSEUDO_CONFIG_H_SOURCED)
#  define __PTW32_PSEUDO_CONFIG_H_SOURCED
#  if defined(WINCE)
#    undef  HAVE_CPU_AFFINITY
#    define NEED_DUPLICATEHANDLE
#    define NEED_CREATETHREAD
#    define NEED_ERRNO
#    define NEED_CALLOC
#    define NEED_FTIME
#    define NEED_UNICODE_CONSTS
#    define NEED_PROCESS_AFFINITY_MASK
/* This may not be needed */
#    define RETAIN_WSALASTERROR
#  elif defined(_MSC_VER)
#    if _MSC_VER >= 1900
#      define HAVE_STRUCT_TIMESPEC
#    elif _MSC_VER < 1300
#      define PTW32_CONFIG_MSVC6
#    elif _MSC_VER < 1400
#      define PTW32_CONFIG_MSVC7
#    endif
#  elif defined(_UWIN)
#    define HAVE_MODE_T
#    define HAVE_STRUCT_TIMESPEC
#    define HAVE_SIGNAL_H
#  endif
#endif

/*
 * If HAVE_ERRNO_H is defined then assume that autoconf has been used
 * to overwrite config.h, otherwise the original config.h is in use
 * at build-time or the above block of defines is in use otherwise
 * and NEED_ERRNO is either defined or not defined.
 */
#if defined(HAVE_ERRNO_H) || !defined(NEED_ERRNO)
#  include <errno.h>
#else
#  include "need_errno.h"
#endif

#if defined(__BORLANDC__)
#  define int64_t LONGLONG
#  define uint64_t ULONGLONG
#elif !defined(__MINGW32__) && !defined(HAS_MSVCLIBX)
#  define int64_t _int64
#  define uint64_t unsigned _int64
#  if defined(PTW32_CONFIG_MSVC6)
     typedef long intptr_t;
#  endif
#elif defined(HAVE_STDINT_H) && HAVE_STDINT_H == 1
#  include <stdint.h>
#endif

/*
 * In case ETIMEDOUT hasn't been defined above somehow.
 */
#if !defined(ETIMEDOUT)
  /*
   * note: ETIMEDOUT is no longer defined in winsock.h
   * WSAETIMEDOUT is so use its value.
   */
#  include <winsock.h>
#  if defined(WSAETIMEDOUT)
#    define ETIMEDOUT WSAETIMEDOUT
#  else
#   define ETIMEDOUT 10060	/* This is the value of WSAETIMEDOUT in winsock.h. */
#  endif
#endif

/*
 * Several systems may not define some error numbers;
 * defining those which are likely to be missing here will let
 * us complete the library builds.
 */
#if !defined(ENOTSUP)
#  define ENOTSUP 48   /* This is the value in Solaris. */
#endif

#if !defined(ENOSYS)
#  define ENOSYS 140     /* Semi-arbitrary value */
#endif

#if !defined(EDEADLK)
#  if defined(EDEADLOCK)
#    define EDEADLK EDEADLOCK
#  else
#    define EDEADLK 36     /* This is the value in MSVC. */
#  endif
#endif

/* POSIX 2008 - related to robust mutexes */
/*
 * FIXME: These should be changed for version 3.0.0 onward.
 * 42 clashes with EILSEQ.
 */
#if PTW32_VERSION_MAJOR > 2
#  if !defined(EOWNERDEAD)
#    define EOWNERDEAD 1000
#  endif
#  if !defined(ENOTRECOVERABLE)
#    define ENOTRECOVERABLE 1001
#  endif
#else
#  if !defined(EOWNERDEAD)
#    define EOWNERDEAD 42
#  endif
#  if !defined(ENOTRECOVERABLE)
#    define ENOTRECOVERABLE 43
#  endif
#endif

#endif	/* !__PTW32_H */
