/*
 * ptw32_new.c
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


pthread_t
ptw32_new (void)
{
  pthread_t t;
  pthread_t nil = {NULL, 0};
  ptw32_thread_t * tp;

  /*
   * If there's a reusable pthread_t then use it.
   */
  t = ptw32_threadReusePop ();

  if (NULL != t.p)
    {
      tp = (ptw32_thread_t *) t.p;
    }
  else
    {
      /* No reuse threads available */
      tp = (ptw32_thread_t *) calloc (1, sizeof(ptw32_thread_t));

      if (tp == NULL)
	{
	  return nil;
	}

      /* ptHandle.p needs to point to it's parent ptw32_thread_t. */
      t.p = tp->ptHandle.p = tp;
      t.x = tp->ptHandle.x = 0;
    }

  /* Set default state. */
  tp->seqNumber = ++ptw32_threadSeqNumber;
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
  tp->cancelEvent = CreateEvent (0, (int) PTW32_TRUE,	/* manualReset  */
				 (int) PTW32_FALSE,	/* setSignaled  */
				 NULL);

  if (tp->cancelEvent == NULL)
    {
      ptw32_threadReusePush (tp->ptHandle);
      return nil;
    }

  return t;
}
