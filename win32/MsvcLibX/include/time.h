/*****************************************************************************\
*                                                                             *
*   Filename:	    time.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to time.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-06-04 JFL Created this file.                                        *
*    2015-11-15 JFL Visual Studio 2015 moved this file to the Windows Kit UCRT.
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_TIME_H
#define	_MSVCLIBX_TIME_H	1

#include "msvclibx.h"

#include UCRT_INCLUDE_FILE(time.h) /* Include MSVC's own <time.h> file */


#ifdef _MSDOS

/* Check for the definition of _STRUCT_TIMESPEC before using clock_gettime().
   If it's not defined, use time() instead, which is supported by all OSs. */

#endif /* defined(_MSDOS) */


#ifdef _WIN32

#include "sys\time.h" /* for struct timespec */

typedef int clockid_t;
/* Supported values for clockid_t */
#define CLOCK_REALTIME 0

int clock_gettime(clockid_t clock_id, struct timespec *tp);

#endif /* defined(_WIN32) */


/* Defined in strptime.c */
char *strptime(const char *buf, const char *fmt, struct tm *tm);

#endif /* defined(_MSVCLIBX_TIME_H)  */

