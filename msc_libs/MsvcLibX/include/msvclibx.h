/*****************************************************************************\
*                                                                             *
*   Filename	    MsvcLibX.h						      *
*                                                                             *
*   Description     MsvcLibX-specific definitions			      *
*                                                                             *
*   Notes	    Generates a library search record to load MsvcLibX.lib.   *
*                                                                             *
*   History:								      *
*    2013       JFL Created this file.                                        *
*    2014-05-30 JFL Added macros to work around the lack of a #include_next.  *
*    2015-11-15 JFL Added macro UCRT_INCLUDE_FILE for Visual Studio 2015.     *
*    2016-09-15 JFL Added macro WINSDK_INCLUDE_FILE for Windows SDK.	      *
*    2016-09-20 JFL Added workaround preventing warnings in WIN95 builds.     *
*    2016-09-28 JFL Can also be included by MS' Resource Compiler.            *
*    2017-02-05 JFL Changed the UTF-8 programs initialization method.         *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

/* Generate a library search record to load MsvcLibX.lib */

#ifndef _MSVCLIBX_H_
#define _MSVCLIBX_H_

#if !(defined(_MSC_VER) || defined(RC_INVOKED)) 
#error The msvclibx library is designed for use with the Microsoft Visual C/C++ tools only.
#endif

/* Compute the OS-specific suffix */
#if defined(_WIN64)
#  define _MSVCLIBX_LIB_OS_SUFFIX "w64"
#elif defined(_WIN95)
#  define _MSVCLIBX_LIB_OS_SUFFIX "w95"
#elif defined(_WIN32)
#  define _MSVCLIBX_LIB_OS_SUFFIX "w32"
#elif defined(_MSDOS)
#  if defined(_M_I86TM)
#    define _MSVCLIBX_LIB_OS_SUFFIX "dt"
#  elif defined(_M_I86SM)
#    define _MSVCLIBX_LIB_OS_SUFFIX "ds"
#  elif defined(_M_I86LM)
#    define _MSVCLIBX_LIB_OS_SUFFIX "dl"
#  else
#    error No msvclibx.lib version yet for this DOS memory model.
#  endif
#else
#  error No msvclibx.lib version for this target OS.
#endif

/* Compute the debug-mode-specific suffix */
#if defined(_DEBUG)
#  define _MSVCLIBX_LIB_DBG_SUFFIX "d"
#else
#  define _MSVCLIBX_LIB_DBG_SUFFIX ""
#endif

/* Generate the OS-and-debug-mode-specific library name */
#define _MSVCLIBX_LIB "MsvcLibX" _MSVCLIBX_LIB_OS_SUFFIX _MSVCLIBX_LIB_DBG_SUFFIX ".lib"
#pragma message("Adding pragma comment(lib, \"" _MSVCLIBX_LIB "\")")
#pragma comment(lib, _MSVCLIBX_LIB)

/* Library-specific routine used internally by many standard routines */
#if defined(_WIN32)
extern int Win32ErrorToErrno(); /* Converts the last WIN32 error to a Posix error code */
#ifndef ELOOP	/* Defined in VS10's errno.h, but not in VS9 */
#define ELOOP           114
#endif
/* Convert an ANSI or UTF-8 or OEM pathname to a Unicode string. Defined in mb2wpath.c. */
typedef unsigned int UINT;  /* Defined in windef.h */
typedef const char* LPCSTR; /* Defined in winnt.h */
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t; /* Defined in crtdefs.h */
#define _WCHAR_T_DEFINED
#endif
typedef wchar_t* LPWSTR;    /* Defined in winnt.h */
extern int MultiByteToWidePath(UINT nCodePage, LPCSTR pszName, LPWSTR pwszName, int nWideBufSize);
#endif
/* Count the number of elements in an array */
#define COUNTOF(array) (sizeof(array)/sizeof(array[0]))

/* Workaround for missing __pragma() directive in old versions of Visual Studio */
#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER <= 1400) /* For Visual C++ versions up to Visual Studio 2005 */
#define __pragma(x)
#endif /* (_MSC_VER <= 1400) */

/* Macros for working around the lack of a #include_next directive */
#define MSVCLIBX_CONCAT1(a,b) a##b /* Concatenate the raw arguments */
#define MSVCLIBX_CONCAT(a,b) MSVCLIBX_CONCAT1(a,b) /* Substitute the arguments, then concatenate the values */
#define MSVCLIBX_STRINGIZE1(x) #x /* Convert the raw argument to a string */
#define MSVCLIBX_STRINGIZE(x) MSVCLIBX_STRINGIZE1(x) /* Substitute the argument, then convert its value to a string */
/* Up to VS2013, both kinds of include files were in the same directory. Then in VS2015, they were split in two dirs. */
#define MSVC_INCLUDE_FILE(relpath) MSVCLIBX_STRINGIZE(MSVCLIBX_CONCAT(MSVCINCLUDE,MSVCLIBX_CONCAT(/,relpath))) /* C compiler include files */
#define UCRT_INCLUDE_FILE(relpath) MSVCLIBX_STRINGIZE(MSVCLIBX_CONCAT(UCRTINCLUDE,MSVCLIBX_CONCAT(/,relpath))) /* C runtime library include files */
#define WINSDK_INCLUDE_FILE(relpath) MSVCLIBX_STRINGIZE(MSVCLIBX_CONCAT(WSDKINCLUDE,MSVCLIBX_CONCAT(/,relpath))) /* Windows SDK include files */

/* Support for external linker symbols */
#if defined(_WIN64)
#define PUBLIC_SYMBOL_NAME(s) s
#else /* _MSDOS or _WIN32 */
#define PUBLIC_SYMBOL_NAME(s) _##s
#endif

/* Support for UTF-8 command lines */
#if defined(_WIN32)
#if defined(_UTF8_SOURCE) || defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
/* Force linking in MsvcLibX' UTF-8 initialization module */
#pragma comment(linker, "/include:" MSVCLIBX_STRINGIZE(PUBLIC_SYMBOL_NAME(_initU)))
#endif /* defined(_UTF8_SOURCE) ... */
#endif /* defined(_WIN32) */

/* Prevent an incompatibility with <winsock.h>. See MsvcLibX' "sys/time.h" for explanations. */
#define _WINSOCKAPI_   /* Prevent the inclusion of winsock.h in windows.h */

#endif /*  _MSVCLIBX_H_ */

