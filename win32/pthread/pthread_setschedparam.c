/*
 * sched_setschedparam.c
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

int
pthread_setschedparam (pthread_t thread, int policy,
		       const struct sched_param *param)
{
  int result;

  /*
   * Validate the thread id. This method works for pthreads-win32 because
   * pthread_kill and pthread_t are designed to accommodate it, but the
   * method is not portable.
   */
  result = pthread_kill (thread, 0);
  if (0 != result)
    {
      return result;
    }

  /* Validate the scheduling policy. */
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      return EINVAL;
    }

  /* Ensure the policy is SCHED_OTHER. */
  if (policy != SCHED_OTHER)
    {
      return ENOTSUP;
    }

  return (__ptw32_setthreadpriority (thread, policy, param->sched_priority));
}


int
__ptw32_setthreadpriority (pthread_t thread, int policy, int priority)
{
  int prio;
  __ptw32_mcs_local_node_t threadLock;
  int result = 0;
  __ptw32_thread_t * tp = (__ptw32_thread_t *) thread.p;

  prio = priority;

  /* Validate priority level. */
  if (prio < sched_get_priority_min (policy) ||
      prio > sched_get_priority_max (policy))
    {
      return EINVAL;
    }

#if (THREAD_PRIORITY_LOWEST > THREAD_PRIORITY_NORMAL)
/* WinCE */
#else
/* Everything else */

  if (THREAD_PRIORITY_IDLE < prio && THREAD_PRIORITY_LOWEST > prio)
    {
      prio = THREAD_PRIORITY_LOWEST;
    }
  else if (THREAD_PRIORITY_TIME_CRITICAL > prio
	   && THREAD_PRIORITY_HIGHEST < prio)
    {
      prio = THREAD_PRIORITY_HIGHEST;
    }

#endif

  __ptw32_mcs_lock_acquire (&tp->threadLock, &threadLock);

  /* If this fails, the current priority is unchanged. */
  if (0 == SetThreadPriority (tp->threadH, prio))
    {
      result = EINVAL;
    }
  else
    {
      /*
       * Must record the thread's sched_priority as given,
       * not as finally adjusted.
       */
      tp->sched_priority = priority;
    }

  __ptw32_mcs_lock_release (&threadLock);

  return result;
}
