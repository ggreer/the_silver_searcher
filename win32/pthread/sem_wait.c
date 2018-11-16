/*
 * -------------------------------------------------------------
 *
 * Module: sem_wait.c
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


static void  __PTW32_CDECL
__ptw32_sem_wait_cleanup(void * sem)
{
  sem_t s = (sem_t) sem;
  __ptw32_mcs_local_node_t node;

  __ptw32_mcs_lock_acquire(&s->lock, &node);
  /*
   * If sema is destroyed do nothing, otherwise:-
   * If the sema is posted between us being canceled and us locking
   * the sema again above then we need to consume that post but cancel
   * anyway. If we don't get the semaphore we indicate that we're no
   * longer waiting.
   */
  if (*((sem_t *)sem) != NULL && !(WaitForSingleObject(s->sem, 0) == WAIT_OBJECT_0))
    {
      ++s->value;
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
#endif /* NEED_SEM */
    }
  __ptw32_mcs_lock_release(&node);
}

int
sem_wait (sem_t * sem)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function  waits on a semaphore.
 *
 * PARAMETERS
 *      sem
 *              pointer to an instance of sem_t
 *
 * DESCRIPTION
 *      This function waits on a semaphore. If the
 *      semaphore value is greater than zero, it decreases
 *      its value by one. If the semaphore value is zero, then
 *      the calling thread (or process) is blocked until it can
 *      successfully decrease the value or until interrupted by
 *      a signal.
 *
 * RESULTS
 *              0               successfully decreased semaphore,
 *              -1              failed, error in errno
 * ERRNO
 *              EINVAL          'sem' is not a valid semaphore,
 *              ENOSYS          semaphores are not supported,
 *              EINTR           the function was interrupted by a signal,
 *              EDEADLK         a deadlock condition was detected.
 *
 * ------------------------------------------------------
 */
{
  __ptw32_mcs_local_node_t node;
  int v;
  int result = 0;
  sem_t s = *sem;

  pthread_testcancel();

  __ptw32_mcs_lock_acquire(&s->lock, &node);
  v = --s->value;
  __ptw32_mcs_lock_release(&node);

  if (v < 0)
    {
#if defined (__PTW32_CONFIG_MSVC7)
#pragma inline_depth(0)
#endif
      /* Must wait */
      pthread_cleanup_push(__ptw32_sem_wait_cleanup, (void *) s);
      result = pthreadCancelableWait (s->sem);
      /* Cleanup if we're canceled or on any other error */
      pthread_cleanup_pop(result);
#if defined (__PTW32_CONFIG_MSVC7)
#pragma inline_depth()
#endif
    }
#if defined(NEED_SEM)

  if (!result)
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

  if (result != 0)
    {
       __PTW32_SET_ERRNO(result);
      return -1;
    }

  return 0;

}				/* sem_wait */
