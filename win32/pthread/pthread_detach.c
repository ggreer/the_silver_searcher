/*
 * pthread_detach.c
 *
 * Description:
 * This translation unit implements functions related to thread
 * synchronisation.
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

/*
 * Not needed yet, but defining it should indicate clashes with build target
 * environment that should be fixed.
 */
#if !defined(WINCE)
#  include <signal.h>
#endif


int
pthread_detach (pthread_t thread)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function detaches the given thread.
      *
      * PARAMETERS
      *      thread
      *              an instance of a pthread_t
      *
      *
      * DESCRIPTION
      *      This function detaches the given thread. You may use it to
      *      detach the main thread or to detach a joinable thread.
      *      NOTE:   detached threads cannot be joined;
      *              storage is freed immediately on termination.
      *
      * RESULTS
      *              0               successfully detached the thread,
      *              EINVAL          thread is not a joinable thread,
      *              ENOSPC          a required resource has been exhausted,
      *              ESRCH           no thread could be found for 'thread',
      *
      * ------------------------------------------------------
      */
{
  int result;
  BOOL destroyIt =  __PTW32_FALSE;
  __ptw32_thread_t * tp = (__ptw32_thread_t *) thread.p;
  __ptw32_mcs_local_node_t reuseLock;

  __ptw32_mcs_lock_acquire(&__ptw32_thread_reuse_lock, &reuseLock);

  if (NULL == tp
      || thread.x != tp->ptHandle.x)
    {
      result = ESRCH;
    }
  else if (PTHREAD_CREATE_DETACHED == tp->detachState)
    {
      result = EINVAL;
    }
  else
    {
      __ptw32_mcs_local_node_t stateLock;
      /*
       * Joinable __ptw32_thread_t structs are not scavenged until
       * a join or detach is done. The thread may have exited already,
       * but all of the state and locks etc are still there.
       */
      result = 0;

      __ptw32_mcs_lock_acquire (&tp->stateLock, &stateLock);
      if (tp->state < PThreadStateLast)
        {
          tp->detachState = PTHREAD_CREATE_DETACHED;
          if (tp->state == PThreadStateExiting)
          {
            destroyIt = __PTW32_TRUE;
          }
        }
      else if (tp->detachState != PTHREAD_CREATE_DETACHED)
        {
          /*
           * Thread is joinable and has exited or is exiting.
           */
          destroyIt =  __PTW32_TRUE;
        }
      __ptw32_mcs_lock_release (&stateLock);
    }

  __ptw32_mcs_lock_release(&reuseLock);

  if (result == 0)
    {
      /* Thread is joinable */

      if (destroyIt)
	{
	  /* The thread has exited or is exiting but has not been joined or
	   * detached. Need to wait in case it's still exiting.
	   */
	  (void) WaitForSingleObject(tp->threadH, INFINITE);
	  __ptw32_threadDestroy (thread);
	}
    }

  return (result);

}				/* pthread_detach */
