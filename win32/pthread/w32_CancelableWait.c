/*
 * w32_CancelableWait.c
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


static INLINE int
__ptw32_cancelable_wait (HANDLE waitHandle, DWORD timeout)
     /*
      * -------------------------------------------------------------------
      * This provides an extra hook into the pthread_cancel
      * mechanism that will allow you to wait on a Windows handle and make it a
      * cancellation point. This function blocks until the given WIN32 handle is
      * signalled or pthread_cancel has been called. It is implemented using
      * WaitForMultipleObjects on 'waitHandle' and a manually reset WIN32
      * event used to implement pthread_cancel.
      * 
      * Given this hook it would be possible to implement more of the cancellation
      * points.
      * -------------------------------------------------------------------
      */
{
  int result;
  pthread_t self;
  __ptw32_thread_t * sp;
  HANDLE handles[2];
  DWORD nHandles = 1;
  DWORD status;

  handles[0] = waitHandle;

  self = pthread_self();
  sp = (__ptw32_thread_t *) self.p;

  if (sp != NULL)
    {
      /*
       * Get cancelEvent handle
       */
      if (sp->cancelState == PTHREAD_CANCEL_ENABLE)
	{

	  if ((handles[1] = sp->cancelEvent) != NULL)
	    {
	      nHandles++;
	    }
	}
    }
  else
    {
      handles[1] = NULL;
    }

  status = WaitForMultipleObjects (nHandles, handles,  __PTW32_FALSE, timeout);

  switch (status - WAIT_OBJECT_0)
    {
    case 0:
      /*
       * Got the handle.
       * In the event that both handles are signalled, the smallest index
       * value (us) is returned. As it has been arranged, this ensures that
       * we don't drop a signal that we should act on (i.e. semaphore,
       * mutex, or condition variable etc).
       */
      result = 0;
      break;

    case 1:
      /*
       * Got cancel request.
       * In the event that both handles are signalled, the cancel will
       * be ignored (see case 0 comment).
       */
      ResetEvent (handles[1]);

      if (sp != NULL)
	{
          __ptw32_mcs_local_node_t stateLock;
	  /*
	   * Should handle POSIX and implicit POSIX threads.
	   * Make sure we haven't been async-cancelled in the meantime.
	   */
	  __ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);
	  if (sp->state < PThreadStateCanceling)
	    {
	      sp->state = PThreadStateCanceling;
	      sp->cancelState = PTHREAD_CANCEL_DISABLE;
	      __ptw32_mcs_lock_release (&stateLock);
	      __ptw32_throw  (__PTW32_EPS_CANCEL);

	      /* Never reached */
	    }
	  __ptw32_mcs_lock_release (&stateLock);
	}

      /* Should never get to here. */
      result = EINVAL;
      break;

    default:
      if (status == WAIT_TIMEOUT)
	{
	  result = ETIMEDOUT;
	}
      else
	{
	  result = EINVAL;
	}
      break;
    }

  return (result);

}				/* CancelableWait */

int
pthreadCancelableWait (HANDLE waitHandle)
{
  return (__ptw32_cancelable_wait (waitHandle, INFINITE));
}

int
pthreadCancelableTimedWait (HANDLE waitHandle, DWORD timeout)
{
  return (__ptw32_cancelable_wait (waitHandle, timeout));
}
