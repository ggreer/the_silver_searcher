/*
 * sched_get_priority_max.c
 * 
 * Description:
 * POSIX thread functions that deal with thread scheduling.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads4w - POSIX Threads for Windows
 *      Copyright 1998 John E. Bossom
 *      Copyright 1999-2018, Pthreads4w contributors
 *
 *      Homepage: https://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *
 *      https://sourceforge.net/p/pthreads4w/wiki/Contributors/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"
#include "sched.h"

/*
 * On Windows98, THREAD_PRIORITY_LOWEST is (-2) and 
 * THREAD_PRIORITY_HIGHEST is 2, and everything works just fine.
 * 
 * On WinCE 3.0, it so happen that THREAD_PRIORITY_LOWEST is 5
 * and THREAD_PRIORITY_HIGHEST is 1 (yes, I know, it is funny:
 * highest priority use smaller numbers) and the following happens:
 * 
 * sched_get_priority_min() returns 5
 * sched_get_priority_max() returns 1
 *
 * The following table shows the base priority levels for combinations
 * of priority class and priority value in Win32.
 *
 *   Process Priority Class               Thread Priority Level
 *   -----------------------------------------------------------------
 *   1 IDLE_PRIORITY_CLASS                THREAD_PRIORITY_IDLE
 *   1 BELOW_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_IDLE
 *   1 NORMAL_PRIORITY_CLASS              THREAD_PRIORITY_IDLE
 *   1 ABOVE_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_IDLE
 *   1 HIGH_PRIORITY_CLASS                THREAD_PRIORITY_IDLE
 *   2 IDLE_PRIORITY_CLASS                THREAD_PRIORITY_LOWEST
 *   3 IDLE_PRIORITY_CLASS                THREAD_PRIORITY_BELOW_NORMAL
 *   4 IDLE_PRIORITY_CLASS                THREAD_PRIORITY_NORMAL
 *   4 BELOW_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_LOWEST
 *   5 IDLE_PRIORITY_CLASS                THREAD_PRIORITY_ABOVE_NORMAL
 *   5 BELOW_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_BELOW_NORMAL
 *   5 Background NORMAL_PRIORITY_CLASS   THREAD_PRIORITY_LOWEST
 *   6 IDLE_PRIORITY_CLASS                THREAD_PRIORITY_HIGHEST
 *   6 BELOW_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_NORMAL
 *   6 Background NORMAL_PRIORITY_CLASS   THREAD_PRIORITY_BELOW_NORMAL
 *   7 BELOW_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_ABOVE_NORMAL
 *   7 Background NORMAL_PRIORITY_CLASS   THREAD_PRIORITY_NORMAL
 *   7 Foreground NORMAL_PRIORITY_CLASS   THREAD_PRIORITY_LOWEST
 *   8 BELOW_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_HIGHEST
 *   8 NORMAL_PRIORITY_CLASS              THREAD_PRIORITY_ABOVE_NORMAL
 *   8 Foreground NORMAL_PRIORITY_CLASS   THREAD_PRIORITY_BELOW_NORMAL
 *   8 ABOVE_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_LOWEST
 *   9 NORMAL_PRIORITY_CLASS              THREAD_PRIORITY_HIGHEST
 *   9 Foreground NORMAL_PRIORITY_CLASS   THREAD_PRIORITY_NORMAL
 *   9 ABOVE_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_BELOW_NORMAL
 *  10 Foreground NORMAL_PRIORITY_CLASS   THREAD_PRIORITY_ABOVE_NORMAL
 *  10 ABOVE_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_NORMAL
 *  11 Foreground NORMAL_PRIORITY_CLASS   THREAD_PRIORITY_HIGHEST
 *  11 ABOVE_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_ABOVE_NORMAL
 *  11 HIGH_PRIORITY_CLASS                THREAD_PRIORITY_LOWEST
 *  12 ABOVE_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_HIGHEST
 *  12 HIGH_PRIORITY_CLASS                THREAD_PRIORITY_BELOW_NORMAL
 *  13 HIGH_PRIORITY_CLASS                THREAD_PRIORITY_NORMAL
 *  14 HIGH_PRIORITY_CLASS                THREAD_PRIORITY_ABOVE_NORMAL
 *  15 HIGH_PRIORITY_CLASS                THREAD_PRIORITY_HIGHEST
 *  15 HIGH_PRIORITY_CLASS                THREAD_PRIORITY_TIME_CRITICAL
 *  15 IDLE_PRIORITY_CLASS                THREAD_PRIORITY_TIME_CRITICAL
 *  15 BELOW_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_TIME_CRITICAL
 *  15 NORMAL_PRIORITY_CLASS              THREAD_PRIORITY_TIME_CRITICAL
 *  15 ABOVE_NORMAL_PRIORITY_CLASS        THREAD_PRIORITY_TIME_CRITICAL
 *  16 REALTIME_PRIORITY_CLASS            THREAD_PRIORITY_IDLE
 *  17 REALTIME_PRIORITY_CLASS            -7
 *  18 REALTIME_PRIORITY_CLASS            -6
 *  19 REALTIME_PRIORITY_CLASS            -5
 *  20 REALTIME_PRIORITY_CLASS            -4
 *  21 REALTIME_PRIORITY_CLASS            -3
 *  22 REALTIME_PRIORITY_CLASS            THREAD_PRIORITY_LOWEST
 *  23 REALTIME_PRIORITY_CLASS            THREAD_PRIORITY_BELOW_NORMAL
 *  24 REALTIME_PRIORITY_CLASS            THREAD_PRIORITY_NORMAL
 *  25 REALTIME_PRIORITY_CLASS            THREAD_PRIORITY_ABOVE_NORMAL
 *  26 REALTIME_PRIORITY_CLASS            THREAD_PRIORITY_HIGHEST
 *  27 REALTIME_PRIORITY_CLASS             3
 *  28 REALTIME_PRIORITY_CLASS             4
 *  29 REALTIME_PRIORITY_CLASS             5
 *  30 REALTIME_PRIORITY_CLASS             6
 *  31 REALTIME_PRIORITY_CLASS            THREAD_PRIORITY_TIME_CRITICAL
 *
 * Windows NT:  Values -7, -6, -5, -4, -3, 3, 4, 5, and 6 are not supported.
 */


int
sched_get_priority_max (int policy)
{
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
       __PTW32_SET_ERRNO(EINVAL);
      return -1;
    }

#if (THREAD_PRIORITY_LOWEST > THREAD_PRIORITY_NORMAL)
  /* WinCE? */
  return  __PTW32_MAX (THREAD_PRIORITY_IDLE, THREAD_PRIORITY_TIME_CRITICAL);
#else
  /* This is independent of scheduling policy in Win32. */
  return  __PTW32_MAX (THREAD_PRIORITY_IDLE, THREAD_PRIORITY_TIME_CRITICAL);
#endif
}
