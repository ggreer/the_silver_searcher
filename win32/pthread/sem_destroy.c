/*
 * -------------------------------------------------------------
 *
 * Module: sem_destroy.c
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
sem_destroy (sem_t * sem)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function destroys an unnamed semaphore.
 *
 * PARAMETERS
 *      sem
 *              pointer to an instance of sem_t
 *
 * DESCRIPTION
 *      This function destroys an unnamed semaphore.
 *
 * RESULTS
 *              0               successfully destroyed semaphore,
 *              -1              failed, error in errno
 * ERRNO
 *              EINVAL          'sem' is not a valid semaphore,
 *              ENOSYS          semaphores are not supported,
 *              EBUSY           threads (or processes) are currently
 *                                      blocked on 'sem'
 *
 * ------------------------------------------------------
 */
{
  int result = 0;
  sem_t s = NULL;

  if (sem == NULL || *sem == NULL)
    {
      result = EINVAL;
    }
  else
    {
      __ptw32_mcs_local_node_t node;
      s = *sem;

      if ((result = __ptw32_mcs_lock_try_acquire(&s->lock, &node)) == 0)
        {
          if (s->value < 0)
            {
              result = EBUSY;
            }
          else
            {
              /*
               * There are no threads currently blocked on this semaphore
               * however there could be threads about to wait behind us.
               * It is up to the application to ensure this is not the case.
               */
              if (!CloseHandle (s->sem))
                {
                  result = EINVAL;
                }
            }
          __ptw32_mcs_lock_release(&node);
        }
    }

  if (result != 0)
    {
       __PTW32_SET_ERRNO(result);
      return -1;
    }

  free (s);

  return 0;

}				/* sem_destroy */
