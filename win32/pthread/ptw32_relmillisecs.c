/*
 * ptw32_relmillisecs.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "pthread.h"
#include "implement.h"

static const int64_t NANOSEC_PER_SEC = 1000000000;
static const int64_t NANOSEC_PER_MILLISEC = 1000000;
static const int64_t MILLISEC_PER_SEC = 1000;

#if defined(PTW32_BUILD_INLINED)
INLINE 
#endif /* PTW32_BUILD_INLINED */
DWORD
ptw32_relmillisecs (const struct timespec * abstime)
{
  DWORD milliseconds;
  int64_t tmpAbsNanoseconds;
  int64_t tmpCurrNanoseconds;

  struct timespec currSysTime;
  FILETIME ft;

# if defined(WINCE)
  SYSTEMTIME st;
#endif

  /*
   * Calculate timeout as milliseconds from current system time.
   */

  /*
   * subtract current system time from abstime in a way that checks
   * that abstime is never in the past, or is never equivalent to the
   * defined INFINITE value (0xFFFFFFFF).
   *
   * Assume all integers are unsigned, i.e. cannot test if less than 0.
   */
  tmpAbsNanoseconds = (int64_t)abstime->tv_nsec + ((int64_t)abstime->tv_sec * NANOSEC_PER_SEC);

  /* get current system time */

# if defined(WINCE)
  GetSystemTime(&st);
  SystemTimeToFileTime(&st, &ft);
# else
  GetSystemTimeAsFileTime(&ft);
# endif

  ptw32_filetime_to_timespec(&ft, &currSysTime);

  tmpCurrNanoseconds = (int64_t)currSysTime.tv_nsec + ((int64_t)currSysTime.tv_sec * NANOSEC_PER_SEC);

  if (tmpAbsNanoseconds > tmpCurrNanoseconds)
    {
      int64_t deltaNanoseconds = tmpAbsNanoseconds - tmpCurrNanoseconds;

      if (deltaNanoseconds >= ((int64_t)INFINITE * NANOSEC_PER_MILLISEC))
         {
           /* Timeouts must be finite */
           milliseconds = INFINITE - 1;
         }
       else
         {
           milliseconds = (DWORD)(deltaNanoseconds / NANOSEC_PER_MILLISEC);
         }
    }
  else
    {
      /* The abstime given is in the past */
      milliseconds = 0;
    }

  if (milliseconds == 0 && tmpAbsNanoseconds > tmpCurrNanoseconds) {
     /*
      * millisecond granularity was too small to represent the wait time.
      * return the minimum time in milliseconds.
      */
     milliseconds = 1;
 }

  return milliseconds;
}


/*
 * Return the first parameter "abstime" modified to represent the current system time.
 * If "relative" is not NULL it represents an interval to add to "abstime".
 */

struct timespec *
pthread_win32_getabstime_np (struct timespec * abstime, const struct timespec * relative)
{
  int64_t sec;
  int64_t nsec;

  struct timespec currSysTime;
  FILETIME ft;

  /* get current system time */

# if defined(WINCE)
  SYSTEMTIME st;
  GetSystemTime(&st);
  SystemTimeToFileTime(&st, &ft);
# else
  GetSystemTimeAsFileTime(&ft);
# endif

  ptw32_filetime_to_timespec(&ft, &currSysTime);

  sec = currSysTime.tv_sec;
  nsec = currSysTime.tv_nsec;

  if (NULL != relative)
    {
      nsec += relative->tv_nsec;
      if (nsec >= NANOSEC_PER_SEC)
	{
	  sec++;
	  nsec -= NANOSEC_PER_SEC;
	}
      sec += relative->tv_sec;
    }

  abstime->tv_sec = (time_t) sec;
  abstime->tv_nsec = (long) nsec;

  return abstime;
}
