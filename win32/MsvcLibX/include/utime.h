/*****************************************************************************\
*                                                                             *
*   Filename:	    utime.h						      *
*                                                                             *
*   Description:    DOS/WIN32 adaptation of standard C library's utime.h.     *
*                                                                             *
*   Notes:	    The Unix standard now standardizes utime.h location in    *
*		    the include directory itself.			      *
*                   Microsoft put it in the include/sys subdirectory.         *
*                                                                             *
*   History:								      *
*    2014-12-13 JFL Created this file.					      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _MSVXLIBX_UTIME_H
#define _MSVXLIBX_UTIME_H

/* In MS-DOS, simply use MSVC's sys/utime.h. */

#ifdef _MSDOS

#include <sys/utime.h>

#define lutime utime /* There are no links in MS-DOS, so lutime() is the same as utime() */

#endif /* _MSDOS */


/* In Windows, we use MSVC's sys/utime.h, but redefine the utime*() functions. */

#ifdef _WIN32

#include "msvclibx.h" /* Generate a library search record to load MsvcLibX.lib. */

/* Save the initial definition and value of __STDC__ */
#ifdef __STDC__
#define _UTIME__STDC__WAS_DEFINED 1
#pragma push_macro("__STDC__")
#undef __STDC__
#else
#define _UTIME__STDC__WAS_DEFINED 0
#endif

#define __STDC__ 1 /* Prevents <sys/utime.h> from defining structures and functions without _ */

#include <sys/utime.h>

#ifdef _USE_32BIT_TIME_T
#define utimbuf __utimbuf32
#define utime   _utime32x
#error "32-bits time_t not supported in MsvcLibX' utime."
#else
#define utimbuf __utimbuf64
#if defined(_UTF8_SOURCE)
#define utime utimeU
#define lutime lutimeU
#else /* _ANSI_SOURCE */
#define utime utimeA
#define lutime lutimeA
#endif
#endif

/* Our redefinition in utime.c */
int utimeA(const char * pszFilename, const struct utimbuf * pUtimbuf);
int lutimeA(const char * pszFilename, const struct utimbuf * pUtimbuf);
int utimeU(const char * pszFilename, const struct utimbuf * pUtimbuf);
int lutimeU(const char * pszFilename, const struct utimbuf * pUtimbuf);
int utimeW(const WCHAR * pszFilename, const struct utimbuf * pUtimbuf);
int lutimeW(const WCHAR * pszFilename, const struct utimbuf * pUtimbuf);
int futime(int fd, const struct utimbuf * pUtimbuf);

#undef __STDC__

/* Restore the initial definition and value of __STDC__ */
#if _UTIME__STDC__WAS_DEFINED
#pragma pop_macro("__STDC__")
#endif
#undef _UTIME__STDC__WAS_DEFINED

#endif /* _WIN32 */

#endif /* _MSVXLIBX_UTIME_H */

