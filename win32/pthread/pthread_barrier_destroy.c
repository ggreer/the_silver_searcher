/*
 * pthread_barrier_destroy.c
 *
 * Description:
 * This translation unit implements barrier primitives.
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
pthread_barrier_destroy (pthread_barrier_t * barrier)
{
  int result = 0;
  pthread_barrier_t b;
  __ptw32_mcs_local_node_t node;

  if (barrier == NULL || *barrier == (pthread_barrier_t)  __PTW32_OBJECT_INVALID)
    {
      return EINVAL;
    }

  if (0 != __ptw32_mcs_lock_try_acquire(&(*barrier)->lock, &node))
    {
      return EBUSY;
    }

  b = *barrier;

  if (b->nCurrentBarrierHeight < b->nInitialBarrierHeight)
    {
      result = EBUSY;
    }
  else
	{
      if (0 == (result = sem_destroy (&(b->semBarrierBreeched))))
        {
          *barrier = (pthread_barrier_t)  __PTW32_OBJECT_INVALID;
          /*
           * Release the lock before freeing b.
           *
           * FIXME: There may be successors which, when we release the lock,
           * will be linked into b->lock, which will be corrupted at some
           * point with undefined results for the application. To fix this
           * will require changing pthread_barrier_t from a pointer to
           * pthread_barrier_t_ to an instance. This is a change to the ABI
           * and will require a major version number increment.
           */
          __ptw32_mcs_lock_release(&node);
	  (void) free (b);
	  return 0;
	}
      else
        {
          /*
           * This should not ever be reached.
           * Restore the barrier to working condition before returning.
           */
          (void) sem_init (&(b->semBarrierBreeched), b->pshared, 0);
        }

      if (result != 0)
        {
          /*
           * The barrier still exists and is valid
           * in the event of any error above.
           */
          result = EBUSY;
        }
    }

  __ptw32_mcs_lock_release(&node);
  return (result);
}
