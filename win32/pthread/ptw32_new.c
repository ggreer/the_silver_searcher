/*
 * ptw32_new.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
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


pthread_t
__ptw32_new (void)
{
  pthread_t t;
  pthread_t nil = {NULL, 0};
  __ptw32_thread_t * tp;

  /*
   * If there's a reusable pthread_t then use it.
   */
  t = __ptw32_threadReusePop ();

  if (NULL != t.p)
    {
      tp = (__ptw32_thread_t *) t.p;
    }
  else
    {
      /* No reuse threads available */
      tp = (__ptw32_thread_t *) calloc (1, sizeof(__ptw32_thread_t));

      if (tp == NULL)
	{
	  return nil;
	}

      /* ptHandle.p needs to point to it's parent __ptw32_thread_t. */
      t.p = tp->ptHandle.p = tp;
      t.x = tp->ptHandle.x = 0;
    }

  /* Set default state. */
  tp->seqNumber = ++__ptw32_threadSeqNumber;
  tp->sched_priority = THREAD_PRIORITY_NORMAL;
  tp->detachState = PTHREAD_CREATE_JOINABLE;
  tp->cancelState = PTHREAD_CANCEL_ENABLE;
  tp->cancelType = PTHREAD_CANCEL_DEFERRED;
  tp->stateLock = 0;
  tp->threadLock = 0;
  tp->robustMxListLock = 0;
  tp->robustMxList = NULL;
  tp->name = NULL;
#if defined(HAVE_CPU_AFFINITY)
  CPU_ZERO((cpu_set_t*)&tp->cpuset);
#endif
  tp->cancelEvent = CreateEvent (0, (int)  __PTW32_TRUE,	/* manualReset  */
				 (int)  __PTW32_FALSE,	/* setSignaled  */
				 NULL);

  if (tp->cancelEvent == NULL)
    {
      __ptw32_threadReusePush (tp->ptHandle);
      return nil;
    }

  return t;

}
