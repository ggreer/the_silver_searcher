/*
 * pthread_timedjoin_np.c
 *
 * Description:
 * This translation unit implements functions related to thread
 * synchronisation.
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

/*
 * Not needed yet, but defining it should indicate clashes with build target
 * environment that should be fixed.
 */
#if !defined(WINCE)
#  include <signal.h>
#endif


int
pthread_timedjoin_np (pthread_t thread, void **value_ptr, const struct timespec *abstime)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function waits for 'thread' to terminate and
      *      returns the thread's exit value if 'value_ptr' is not
      *      NULL or until 'abstime' passes and returns an
      *      error. If 'abstime' is NULL then the function waits
      *      forever, i.e. reverts to pthread_join behaviour.
      *      This function detaches the thread on successful
      *      completion.
      *
      * PARAMETERS
      *      thread
      *              an instance of pthread_t
      *
      *      value_ptr
      *              pointer to an instance of pointer to void
      *
      *      abstime
      *              pointer to an instance of struct timespec
      *              representing an absolute time value
      *
      *
      * DESCRIPTION
      *      This function waits for 'thread' to terminate and
      *      returns the thread's exit value if 'value_ptr' is not
      *      NULL or until 'abstime' passes and returns an
      *      error. If 'abstime' is NULL then the function waits
      *      forever, i.e. reverts to pthread_join behaviour.
      *      This function detaches the thread on successful
      *      completion.
      *      NOTE:   Detached threads cannot be joined or canceled.
      *              In this implementation 'abstime' will be
      *              resolved to the nearest millisecond.
      *
      * RESULTS
      *              0               'thread' has completed
      *              ETIMEDOUT       abstime passed
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
  DWORD milliseconds;
  ptw32_thread_t * tp = (ptw32_thread_t *) thread.p;
  ptw32_mcs_local_node_t node;

  if (abstime == NULL)
    {
      milliseconds = INFINITE;
    }
  else
    {
      /*
       * Calculate timeout as milliseconds from current system time.
       */
      milliseconds = ptw32_relmillisecs (abstime);
    }

  ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);

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

  ptw32_mcs_lock_release(&node);

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
          result = pthreadCancelableTimedWait (tp->threadH, milliseconds);

          if (0 == result)
            {
              if (value_ptr != NULL)
                {
                  *value_ptr = tp->exitStatus;
                }

              /*
               * The result of making multiple simultaneous calls to
               * pthread_join() or pthread_timedjoin_np() or pthread_detach()
               * specifying the same target is undefined.
               */
              result = pthread_detach (thread);
            }
          else if (ETIMEDOUT != result)
            {
              result = ESRCH;
            }
        }
    }

  return (result);

}
