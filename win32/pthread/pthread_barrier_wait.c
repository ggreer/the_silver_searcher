/*
 * pthread_barrier_wait.c
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
pthread_barrier_wait (pthread_barrier_t * barrier)
{
  int result;
  pthread_barrier_t b;

  __ptw32_mcs_local_node_t node;

  if (barrier == NULL || *barrier == (pthread_barrier_t)  __PTW32_OBJECT_INVALID)
    {
      return EINVAL;
    }

  __ptw32_mcs_lock_acquire(&(*barrier)->lock, &node);

  b = *barrier;
  if (--b->nCurrentBarrierHeight == 0)
    {
      /*
       * We are the last thread to arrive at the barrier before it releases us.
       * Move our MCS local node to the global scope barrier handle so that the
       * last thread out (not necessarily us) can release the lock.
       */
      __ptw32_mcs_node_transfer(&b->proxynode, &node);

      /*
       * Any threads that have not quite entered sem_wait below when the
       * multiple_post has completed will nevertheless continue through
       * the semaphore (barrier).
       */
      result = (b->nInitialBarrierHeight > 1
                ? sem_post_multiple (&(b->semBarrierBreeched),
				     b->nInitialBarrierHeight - 1) : 0);
    }
  else
    {
      __ptw32_mcs_lock_release(&node);
      /*
       * Use the non-cancelable version of sem_wait().
       *
       * It is possible that all nInitialBarrierHeight-1 threads are
       * at this point when the last thread enters the barrier, resets
       * nCurrentBarrierHeight = nInitialBarrierHeight and leaves.
       * If pthread_barrier_destroy is called at that moment then the
       * barrier will be destroyed along with the semas.
       */
      result = __ptw32_semwait (&(b->semBarrierBreeched));
    }

  if  ((__PTW32_INTERLOCKED_LONG)__PTW32_INTERLOCKED_INCREMENT_LONG ((__PTW32_INTERLOCKED_LONGPTR)&b->nCurrentBarrierHeight)
		  ==  (__PTW32_INTERLOCKED_LONG)b->nInitialBarrierHeight)
    {
      /*
       * We are the last thread to cross this barrier
       */
      __ptw32_mcs_lock_release(&b->proxynode);
      if (0 == result)
        {
          result = PTHREAD_BARRIER_SERIAL_THREAD;
        }
    }

  return (result);
}
