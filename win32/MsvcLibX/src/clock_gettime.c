/*****************************************************************************\
*                                                                             *
*   Filename:	    clock_gettime.c					      *
*                                                                             *
*   Description:    WIN32 port of standard C library's clock_gettime().	      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-06-04 JFL Created this file.                                        *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#include "msvclibx.h"

#include <time.h>
#include <errno.h>


#ifdef _MSDOS

/* Check for the definition of _STRUCT_TIMESPEC before using clock_gettime().
   If it's not defined, use time() instead, which is supported by all OSs. */

#endif /* defined(_MSDOS) */


#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN /* Avoid lots of unnecessary inclusions */
#include <windows.h>

#include <sys/stat.h> /* For MsvcLibX's Filetime2Timespec */

int clock_gettime(clockid_t clock_id, struct timespec *pTS) {
  FILETIME ft;
  if (clock_id != CLOCK_REALTIME) {
    errno = EINVAL;
    return -1;
  }
  GetSystemTimeAsFileTime(&ft);
  Filetime2Timespec(&ft, pTS);
  return 0;
}

#endif /* defined(_WIN32) */
