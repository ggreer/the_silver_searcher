/*
 * pthread_tryjoin_np.c
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
pthread_tryjoin_np (pthread_t thread, void **value_ptr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function checks if 'thread' has terminated and
      *      returns the thread's exit value if 'value_ptr' is not
      *      NULL or until 'abstime' passes and returns an
      *      error. If the thread has not exited the function returns
      *      immediately. This function detaches the thread on successful
      *      completion.
      *
      * PARAMETERS
      *      thread
      *              an instance of pthread_t
      *
      *      value_ptr
      *              pointer to an instance of pointer to void
      *
      *
      * DESCRIPTION
      *      This function checks if 'thread' has terminated and
      *      returns the thread's exit value if 'value_ptr' is not
      *      NULL or until 'abstime' passes and returns an
      *      error. If the thread has not exited the function returns
      *      immediately. This function detaches the thread on successful
      *      completion.
      *      NOTE:   Detached threads cannot be joined or canceled.
      *              In this implementation 'abstime' will be
      *              resolved to the nearest millisecond.
      *
      * RESULTS
      *              0               'thread' has completed
      *              EBUSY           'thread' is still live
      *              EINVAL          thread is not a joinable thread,
      *              ESRCH           no thread could be found with ID 'thread',
      *              ENOENT          thread couldn't find it's own valid handle,
      *              EDEADLK         attempt to join thread with self
      *
      * ------------------------------------------------------
      */
{
  int result;
  pthread_t self;
  __ptw32_thread_t * tp = (__ptw32_thread_t *) thread.p;
  __ptw32_mcs_local_node_t node;

  __ptw32_mcs_lock_acquire(&__ptw32_thread_reuse_lock, &node);

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
      result = 0;
    }

  __ptw32_mcs_lock_release(&node);

  if (result == 0)
    {
      /*
       * The target thread is joinable and can't be reused before we join it.
       */
      self = pthread_self();

      if (NULL == self.p)
        {
          result = ENOENT;
        }
      else if (pthread_equal (self, thread))
        {
          result = EDEADLK;
        }
      else
        {
          /*
           * Pthread_join is a cancellation point.
           * If we are canceled then our target thread must not be
           * detached (destroyed). This is guaranteed because
           * pthreadCancelableTimedWait will not return if we
           * are canceled.
           */
          result = pthreadCancelableTimedWait (tp->threadH, 0);

          if (0 == result)
            {
              if (value_ptr != NULL)
                {
                  *value_ptr = tp->exitStatus;
                }

              /*
               * The result of making multiple simultaneous calls to
               * pthread_join(), pthread_timedjoin_np(), pthread_tryjoin_np()
               * or pthread_detach() specifying the same target is undefined.
               */
              result = pthread_detach (thread);
            }
          else if (ETIMEDOUT == result)
            {
              result = EBUSY;
            }
          else
            {
        	  result = ESRCH;
            }
        }
    }

  return (result);

}
