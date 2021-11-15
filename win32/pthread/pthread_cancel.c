/*
 * pthread_cancel.c
 *
 * Description:
 * POSIX thread functions related to thread cancellation.
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
#include "context.h"

static void
__ptw32_cancel_self (void)
{
  __ptw32_throw  (__PTW32_EPS_CANCEL);

  /* Never reached */
}

static void CALLBACK
__ptw32_cancel_callback (ULONG_PTR unused)
{
  __ptw32_throw  (__PTW32_EPS_CANCEL);

  /* Never reached */
}

/*
 * __ptw32_Registercancellation() -
 * Must have args of same type as QueueUserAPCEx because this function
 * is a substitute for QueueUserAPCEx if it's not available.
 */
DWORD
__ptw32_Registercancellation (PAPCFUNC unused1, HANDLE threadH, DWORD unused2)
{
  CONTEXT context;

  context.ContextFlags = CONTEXT_CONTROL;
  GetThreadContext (threadH, &context);
   __PTW32_PROGCTR (context) = (DWORD_PTR) __ptw32_cancel_self;
  SetThreadContext (threadH, &context);
  return 0;
}

int
pthread_cancel (pthread_t thread)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function requests cancellation of 'thread'.
      *
      * PARAMETERS
      *      thread
      *              reference to an instance of pthread_t
      *
      *
      * DESCRIPTION
      *      This function requests cancellation of 'thread'.
      *      NOTE: cancellation is asynchronous; use pthread_join to
      *                wait for termination of 'thread' if necessary.
      *
      * RESULTS
      *              0               successfully requested cancellation,
      *              ESRCH           no thread found corresponding to 'thread',
      *              ENOMEM          implicit self thread create failed.
      * ------------------------------------------------------
      */
{
  int result;
  int cancel_self;
  pthread_t self;
  __ptw32_thread_t * tp;
  __ptw32_mcs_local_node_t stateLock;

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

  if ((self = pthread_self ()).p == NULL)
    {
      return ENOMEM;
    };

  /*
   * For self cancellation we need to ensure that a thread can't
   * deadlock itself trying to cancel itself asynchronously
   * (pthread_cancel is required to be an async-cancel
   * safe function).
   */
  cancel_self = pthread_equal (thread, self);

  tp = (__ptw32_thread_t *) thread.p;

  /*
   * Lock for async-cancel safety.
   */
  __ptw32_mcs_lock_acquire (&tp->stateLock, &stateLock);

  if (tp->cancelType == PTHREAD_CANCEL_ASYNCHRONOUS
      && tp->cancelState == PTHREAD_CANCEL_ENABLE
      && tp->state < PThreadStateCanceling)
    {
      if (cancel_self)
	{
	  tp->state = PThreadStateCanceling;
	  tp->cancelState = PTHREAD_CANCEL_DISABLE;

	  __ptw32_mcs_lock_release (&stateLock);
	  __ptw32_throw  (__PTW32_EPS_CANCEL);

	  /* Never reached */
	}
      else
	{
	  HANDLE threadH = tp->threadH;

	  SuspendThread (threadH);

	  if (WaitForSingleObject (threadH, 0) == WAIT_TIMEOUT)
	    {
	      tp->state = PThreadStateCanceling;
	      tp->cancelState = PTHREAD_CANCEL_DISABLE;
	      /*
	       * If alertdrv and QueueUserAPCEx is available then the following
	       * will result in a call to QueueUserAPCEx with the args given, otherwise
	       * this will result in a call to __ptw32_Registercancellation and only
	       * the threadH arg will be used.
	       */
	      __ptw32_register_cancellation ((PAPCFUNC)__ptw32_cancel_callback, threadH, 0);
	      __ptw32_mcs_lock_release (&stateLock);
	      ResumeThread (threadH);
	    }
	}
    }
  else
    {
      /*
       * Set for deferred cancellation.
       */
      if (tp->state < PThreadStateCancelPending)
	{
	  tp->state = PThreadStateCancelPending;
	  if (!SetEvent (tp->cancelEvent))
	    {
	      result = ESRCH;
	    }
	}
      else if (tp->state >= PThreadStateCanceling)
	{
	  result = ESRCH;
	}

      __ptw32_mcs_lock_release (&stateLock);
    }

  return (result);
}
