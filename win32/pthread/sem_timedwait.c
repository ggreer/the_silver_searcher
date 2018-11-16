/*
 * -------------------------------------------------------------
 *
 * Module: sem_timedwait.c
 *
 * Purpose:
 *	Semaphores aren't actually part of the PThreads standard.
 *	They are defined by the POSIX Standard:
 *
 *		POSIX 1003.1b-1993	(POSIX.1b)
 *
 * -------------------------------------------------------------
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
#include "semaphore.h"
#include "implement.h"


typedef struct {
  sem_t sem;
  int * resultPtr;
} sem_timedwait_cleanup_args_t;


static void  __PTW32_CDECL
__ptw32_sem_timedwait_cleanup (void * args)
{
  __ptw32_mcs_local_node_t node;
  sem_timedwait_cleanup_args_t * a = (sem_timedwait_cleanup_args_t *)args;
  sem_t s = a->sem;

  __ptw32_mcs_lock_acquire(&s->lock, &node);
  /*
   * We either timed out or were cancelled.
   * If someone has posted between then and now we try to take the semaphore.
   * Otherwise the semaphore count may be wrong after we
   * return. In the case of a cancellation, it is as if we
   * were cancelled just before we return (after taking the semaphore)
   * which is ok.
   */
  if (WaitForSingleObject(s->sem, 0) == WAIT_OBJECT_0)
    {
      /* We got the semaphore on the second attempt */
      *(a->resultPtr) = 0;
    }
  else
    {
      /* Indicate we're no longer waiting */
      s->value++;
#if defined(NEED_SEM)
      if (s->value > 0)
        {
          s->leftToUnblock = 0;
        }
#else
      /*
       * Don't release the W32 sema, it doesn't need adjustment
       * because it doesn't record the number of waiters.
       */
#endif
    }
  __ptw32_mcs_lock_release(&node);
}


int
sem_timedwait (sem_t * sem, const struct timespec *abstime)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function waits on a semaphore possibly until
 *      'abstime' time.
 *
 * PARAMETERS
 *      sem
 *              pointer to an instance of sem_t
 *
 *      abstime
 *              pointer to an instance of struct timespec
 *
 * DESCRIPTION
 *      This function waits on a semaphore. If the
 *      semaphore value is greater than zero, it decreases
 *      its value by one. If the semaphore value is zero, then
 *      the calling thread (or process) is blocked until it can
 *      successfully decrease the value or until interrupted by
 *      a signal.
 *
 *      If 'abstime' is a NULL pointer then this function will
 *      block until it can successfully decrease the value or
 *      until interrupted by a signal.
 *
 * RESULTS
 *              0               successfully decreased semaphore,
 *              -1              failed, error in errno
 * ERRNO
 *              EINVAL          'sem' is not a valid semaphore,
 *              ENOSYS          semaphores are not supported,
 *              EINTR           the function was interrupted by a signal,
 *              EDEADLK         a deadlock condition was detected.
 *              ETIMEDOUT       abstime elapsed before success.
 *
 * ------------------------------------------------------
 */
{
  __ptw32_mcs_local_node_t node;
  DWORD milliseconds;
  int v;
  int result = 0;
  sem_t s = *sem;

  pthread_testcancel();

  if (abstime == NULL)
    {
      milliseconds = INFINITE;
    }
  else
    {
      /*
       * Calculate timeout as milliseconds from current system time.
       */
      milliseconds = __ptw32_relmillisecs (abstime);
    }

  __ptw32_mcs_lock_acquire(&s->lock, &node);
  v = --s->value;
  __ptw32_mcs_lock_release(&node);

  if (v < 0)
    {
#if defined(NEED_SEM)
      int timedout;
#endif
      sem_timedwait_cleanup_args_t cleanup_args;

      cleanup_args.sem = s;
      cleanup_args.resultPtr = &result;

#if defined (__PTW32_CONFIG_MSVC7)
#pragma inline_depth(0)
#endif
      /* Must wait */
      pthread_cleanup_push(__ptw32_sem_timedwait_cleanup, (void *) &cleanup_args);
#if defined(NEED_SEM)
      timedout =
#endif
          result = pthreadCancelableTimedWait (s->sem, milliseconds);
      pthread_cleanup_pop(result);
#if defined (__PTW32_CONFIG_MSVC7)
#pragma inline_depth()
#endif

#if defined(NEED_SEM)

      if (!timedout)
        {
          __ptw32_mcs_lock_acquire(&s->lock, &node);
          if (s->leftToUnblock > 0)
            {
              --s->leftToUnblock;
              SetEvent(s->sem);
            }
          __ptw32_mcs_lock_release(&node);
        }

#endif /* NEED_SEM */

    }

  if (result != 0)
    {

       __PTW32_SET_ERRNO(result);
      return -1;

    }

  return 0;

}				/* sem_timedwait */
