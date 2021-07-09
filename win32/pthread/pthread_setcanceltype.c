/*
 * pthread_setcanceltype.c
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


int
pthread_setcanceltype (int type, int *oldtype)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function atomically sets the calling thread's
      *      cancelability type to 'type' and returns the previous
      *      cancelability type at the location referenced by
      *      'oldtype'
      *
      * PARAMETERS
      *      type,
      *      oldtype
      *              PTHREAD_CANCEL_DEFERRED
      *                      only deferred cancellation is allowed,
      *
      *              PTHREAD_CANCEL_ASYNCHRONOUS
      *                      Asynchronous cancellation is allowed
      *
      *
      * DESCRIPTION
      *      This function atomically sets the calling thread's
      *      cancelability type to 'type' and returns the previous
      *      cancelability type at the location referenced by
      *      'oldtype'
      *
      *      NOTES:
      *      1)      Use with caution; most code is not safe for use
      *              with asynchronous cancelability.
      *
      * COMPATIBILITY ADDITIONS
      *      If 'oldtype' is NULL then the previous type is not returned
      *      but the function still succeeds. (Solaris)
      *
      * RESULTS
      *              0               successfully set cancelability type,
      *              EINVAL          'type' is invalid
      *
      * ------------------------------------------------------
      */
{
  __ptw32_mcs_local_node_t stateLock;
  int result = 0;
  pthread_t self = pthread_self ();
  __ptw32_thread_t * sp = (__ptw32_thread_t *) self.p;

  if (sp == NULL
      || (type != PTHREAD_CANCEL_DEFERRED
	  && type != PTHREAD_CANCEL_ASYNCHRONOUS))
    {
      return EINVAL;
    }

  /*
   * Lock for async-cancel safety.
   */
  __ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);

  if (oldtype != NULL)
    {
      *oldtype = sp->cancelType;
    }

  sp->cancelType = type;

  /*
   * Check if there is a pending asynchronous cancel
   */
  if (sp->cancelState == PTHREAD_CANCEL_ENABLE
      && type == PTHREAD_CANCEL_ASYNCHRONOUS
      && WaitForSingleObject (sp->cancelEvent, 0) == WAIT_OBJECT_0)
    {
      sp->state = PThreadStateCanceling;
      sp->cancelState = PTHREAD_CANCEL_DISABLE;
      ResetEvent (sp->cancelEvent);
      __ptw32_mcs_lock_release (&stateLock);
      __ptw32_throw  (__PTW32_EPS_CANCEL);

      /* Never reached */
    }

  __ptw32_mcs_lock_release (&stateLock);

  return (result);

}				/* pthread_setcanceltype */
