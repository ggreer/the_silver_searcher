/*
 * Module: _ptw32.h
 *
 * Purpose:
 *      pthreads-win32 internal macros, to be shared by other headers
 *      comprising the pthreads4w package.
 *
 * --------------------------------------------------------------------------
 *
 *      pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999-2021 pthreads-win32 / pthreads4w contributors
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
 * --------------------------------------------------------------------------
 */

#ifndef PTW32_H
#define PTW32_H

/* See the README file for an explanation of the pthreads4w
 * version numbering scheme and how the DLL is named etc.
 */
#define  PTW32_VERSION_MAJOR 3
#define  PTW32_VERSION_MINOR 0
#define  PTW32_VERSION_MICRO 3
#define  PTW32_VERION_BUILD 1
#define  PTW32_VERSION 3,0,3,1
#define  PTW32_VERSION_STRING "3, 0, 3, 1\0"

// skip the rest when running this through the Microsoft Resource Compiler, which is a VERY brittle piece of machinery! -> RC2188 & RC1116 cryptic failures will be your part! 
#ifndef RC_INVOKED

#if defined(__GNUC__)
# pragma GCC system_header
# if ! defined __declspec
#  error "Please upgrade your GNU compiler to one that supports __declspec."
# endif
#endif

#if defined (__cplusplus)
# define PTW32_BEGIN_C_DECLS  extern "C" {
# define PTW32_END_C_DECLS    }
#else
# define PTW32_BEGIN_C_DECLS
# define PTW32_END_C_DECLS
#endif

#if defined PTW32_STATIC_LIB
# define PTW32_DLLPORT

#elif defined  PTW32_BUILD
# define  PTW32_DLLPORT __declspec (dllexport)
#else
# define  PTW32_DLLPORT /*__declspec (dllimport)*/
#endif

#ifndef  PTW32_CDECL
/* Nominally, we prefer to use __cdecl calling convention for all our
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
#  define  PTW32_CDECL
# else
#  define  PTW32_CDECL __cdecl
# endif
#endif

/*
 * This is more or less a duplicate of what is in the autoconf config.h,
 * which is only used when building the pthreads4w libraries.
 */

#if !defined (PTW32_CONFIG_H) && !defined(PTW32_PSEUDO_CONFIG_H_SOURCED)
#  define PTW32_PSEUDO_CONFIG_H_SOURCED
#  if defined(WINCE)
#    undef  HAVE_CPU_AFFINITY
#    define NEED_DUPLICATEHANDLE
#    define NEED_CREATETHREAD
#    define NEED_ERRNO
#    define NEED_CALLOC
#    define NEED_UNICODE_CONSTS
#    define NEED_PROCESS_AFFINITY_MASK
/* This may not be needed */
#    define RETAIN_WSALASTERROR
#  elif defined(_MSC_VER)
#    if _MSC_VER >= 1900 && !defined(HAVE_STRUCT_TIMESPEC)
#      define HAVE_STRUCT_TIMESPEC
#    elif _MSC_VER < 1300
#      define  PTW32_CONFIG_MSVC6
#    elif _MSC_VER < 1400
#      define  PTW32_CONFIG_MSVC7
#    endif
#  elif defined(_UWIN)
#    define HAVE_MODE_T
#    define HAVE_STRUCT_TIMESPEC
#    define HAVE_SIGNAL_H
#  elif defined(__MINGW64__)
#    define HAVE_STRUCT_TIMESPEC
#    define HAVE_MODE_T
#  elif defined(__MINGW32__)
#    define HAVE_MODE_T
#  endif
#endif

#if !defined(_WIN32_WINNT)
# define _WIN32_WINNT 0x0400
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
     typedef _int64 int64_t;
     typedef unsigned _int64 uint64_t;
#  if defined (PTW32_CONFIG_MSVC6)
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

#if !defined(ETIMEDOUT)
#  define ETIMEDOUT 10060 /* Same as WSAETIMEDOUT */
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
#if  PTW32_VERSION_MAJOR > 2
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

#endif // RC_INVOKED

#endif	/* !PTW32_H */
