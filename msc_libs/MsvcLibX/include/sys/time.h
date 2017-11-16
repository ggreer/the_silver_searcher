/*****************************************************************************\
*                                                                             *
*   Filename	    sys/time.h						      *
*                                                                             *
*   Description     DOS/WIN32 port of standard C library's sys/time.h.	      *
*                                                                             *
*   Notes	    MUST be included before any direct or indirect inclusion  *
*		    of <windows.h>, as this sys/time.h file has a definition  *
*		    of struct timeval conflicting with that in <winsock.h>,   *
*		    and as a workaround it defines _WINSOCKAPI_ to prevent    *
*		    <windows.h> from loading <winsock.h>.		      *
*                                                                             *
*		    The same conflict exists with <winsock2.h>, but this one  *
*		    is not loaded automatically, and hence should not be      *
*		    visible to Unix apps built with the MsvcLibX library.     *
*                                                                             *
*   History								      *
*    2014-02-11 JFL Created this file.                                        *
*    2014-05-30 JFL Added the workaround for the conflict with <winsock.h>.   *
*    2014-06-03 JFL Moved struct timespec definition from sys/stat.h.         *
*		    Added macros TIMEVAL_TO_TIMESPEC & TIMESPEC_TO_TIMEVAL.   *
*    2015-12-04 JFL Bug fix: With VS14/VC19, the UCRT defines timespec.       *
*    2016-07-06 JFL Avoid error if winsocks2.h has been previously included.  *
*    2017-02-26 JFL Updated the 2015-12-04 fix if _CRT_NO_TIME_T is defined.  *
*		    							      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#include <time.h> /* for time_t definition */

#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#ifdef _MSDOS
/* MS-DOS only has a 2-second resolution on file times.
   Use the existence of macro _STRUCT_TIMEVAL to test if it's possible
   to use utimes(), else use utime(), which is supported by all OSs */
#endif

#ifdef _WIN32

/* There's a conflict with the timeval definition in winsock.h, which uses
   a long instead of a time_t for tv_sec. See:
   C:\Pgm64\Microsoft SDKs\Windows\v7.0\INCLUDE\winsock.h
*/
#define _WINSOCKAPI_   /* Prevent the inclusion of winsock.h in windows.h */
/* Could also #define WIN32_LEAN_AND_MEAN /* Avoid including many optional Windows features, including winsock.h  */

#include <windows.h>	/* For WCHAR */

#ifndef _STRUCT_TIMEVAL
#define _STRUCT_TIMEVAL       1

#ifndef _WINSOCK2API_	/* Prevent compilation errors if winsocks2.h has been previously included */
/* A time value with microsecond precision */
struct timeval {
  time_t tv_sec;	/* Seconds */
  int tv_usec;		/* Signed count of microseconds */
};
#endif /* _WINSOCK2API */

#endif /* !defined(_STRUCT_TIMEVAL) */

/* 2015-12-04 JFL Bug fix: With VS14/VC19, the UCRT defines timespec, with a long tv_nsec */
#if (!defined(_UCRT)) || defined(_CRT_NO_TIME_T) /* The UCRT does not define it if _CRT_NO_TIME_T is defined */
#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC

/* A time value with nanosecond precision */
struct timespec {
  time_t tv_sec;	/* Seconds */
  int tv_nsec;		/* Signed count of nanoseconds */
};
#define _TIMESPEC_DEFINED /* Some packages test this before redefining it */

#endif /* !defined(_STRUCT_TIMESPEC) */
#endif /* !defined(_UCRT) */

#define	TIMEVAL_TO_TIMESPEC(ptv, pts) {					\
	(pts)->tv_sec = (ptv)->tv_sec;					\
	(pts)->tv_nsec = (ptv)->tv_usec * 1000;				\
}
#define	TIMESPEC_TO_TIMEVAL(ptv, pts) {					\
	(ptv)->tv_sec = (pts)->tv_sec;					\
	(ptv)->tv_usec = (pts)->tv_nsec / 1000;				\
}

/* Change the file access time to tvp[0] and its modification time to tvp[1]. */
int utimesA(const char *file, const struct timeval tvp[2]);
int utimesU(const char *file, const struct timeval tvp[2]);
int utimesW(const WCHAR *file, const struct timeval tvp[2]);

/* Same as 'utimes', but does not follow symbolic links. */
int lutimesA(const char *path, const struct timeval tvp[2]);
int lutimesU(const char *path, const struct timeval tvp[2]);
int lutimesW(const WCHAR *path, const struct timeval tvp[2]);

/* Same as 'utimes', but takes an open file descriptor instead of a name. */
int futimes(int fd, const struct timeval tvp[2]);

#if defined(_UTF8_SOURCE)
#define utimes utimesU
#define lutimes lutimesU
#else /* _ANSI_SOURCE */
#define utimes utimesA
#define lutimes lutimesA
#endif

/* Get the current date and time into a struct timeval */
int gettimeofday(struct timeval *ptv, void *pTimeZone);

#endif /* defined(_WIN32) */

#endif /* !defined(_SYS_TIME_H) */

