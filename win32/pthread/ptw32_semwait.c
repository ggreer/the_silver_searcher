/*
 * ptw32_semwait.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
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

#if !defined(_UWIN)
/*#   include <process.h> */
#endif
#include "pthread.h"
#include "implement.h"


int
__ptw32_semwait (sem_t * sem)
/*
 * ------------------------------------------------------
 * DESCRIPTION
 *      This function waits on a POSIX semaphore. If the
 *      semaphore value is greater than zero, it decreases
 *      its value by one. If the semaphore value is zero, then
 *      the calling thread (or process) is blocked until it can
 *      successfully decrease the value.
 *
 *      Unlike sem_wait(), this routine is non-cancelable.
 *
 * RESULTS
 *              0               successfully decreased semaphore,
 *              -1              failed, error in errno.
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

  __ptw32_mcs_lock_acquire(&s->lock, &node);
  v = --s->value;
  __ptw32_mcs_lock_release(&node);

  if (v < 0)
    {
      /* Must wait */
      if (WaitForSingleObject (s->sem, INFINITE) == WAIT_OBJECT_0)
        {
#if defined(NEED_SEM)
          __ptw32_mcs_lock_acquire(&s->lock, &node);
          if (s->leftToUnblock > 0)
            {
              --s->leftToUnblock;
              SetEvent(s->sem);
            }
          __ptw32_mcs_lock_release(&node);
#endif
return 0;
        }
    }
  else
    {
      return 0;
    }

  if (result != 0)
    {
       __PTW32_SET_ERRNO(result);
      return -1;
    }

  return 0;

}				/* __ptw32_semwait */
