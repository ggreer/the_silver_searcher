/*
 * -------------------------------------------------------------
 *
 * Module: sem_post.c
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


int
sem_post (sem_t * sem)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function posts a wakeup to a semaphore.
 *
 * PARAMETERS
 *      sem
 *              pointer to an instance of sem_t
 *
 * DESCRIPTION
 *      This function posts a wakeup to a semaphore. If there
 *      are waiting threads (or processes), one is awakened;
 *      otherwise, the semaphore value is incremented by one.
 *
 * RESULTS
 *              0               successfully posted semaphore,
 *              -1              failed, error in errno
 * ERRNO
 *              EINVAL          'sem' is not a valid semaphore,
 *              ENOSYS          semaphores are not supported,
 *              ERANGE          semaphore count is too big
 *
 * ------------------------------------------------------
 */
{
  int result = 0;

  __ptw32_mcs_local_node_t node;
  sem_t s = *sem;

  __ptw32_mcs_lock_acquire(&s->lock, &node);
  if (s->value < SEM_VALUE_MAX)
    {
#if defined(NEED_SEM)
      if (++s->value <= 0
          && !SetEvent(s->sem))
        {
          s->value--;
          result = EINVAL;
        }
#else
      if (++s->value <= 0
          && !ReleaseSemaphore (s->sem, 1, NULL))
        {
          s->value--;
          result = EINVAL;
        }
#endif /* NEED_SEM */
    }
  else
    {
      result = ERANGE;
    }
  __ptw32_mcs_lock_release(&node);

  if (result != 0)
    {
       __PTW32_SET_ERRNO(result);
      return -1;
    }

  return 0;
}
