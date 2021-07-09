/*
 * pthread_spin_destroy.c
 *
 * Description:
 * This translation unit implements spin lock primitives.
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
pthread_spin_destroy (pthread_spinlock_t * lock)
{
  register pthread_spinlock_t s;
  int result = 0;

  if (lock == NULL || *lock == NULL)
    {
      return EINVAL;
    }

  if ((s = *lock) != PTHREAD_SPINLOCK_INITIALIZER)
    {
      if (s->interlock ==  __PTW32_SPIN_USE_MUTEX)
	{
	  result = pthread_mutex_destroy (&(s->u.mutex));
	}
      else if  ((__PTW32_INTERLOCKED_LONG)  __PTW32_SPIN_UNLOCKED !=
	        __PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG  ((__PTW32_INTERLOCKED_LONGPTR) &s->interlock,
						    (__PTW32_INTERLOCKED_LONG)  __PTW32_SPIN_INVALID,
						    (__PTW32_INTERLOCKED_LONG)  __PTW32_SPIN_UNLOCKED))
	{
	  result = EINVAL;
	}

      if (0 == result)
	{
	  /*
	   * We are relying on the application to ensure that all other threads
	   * have finished with the spinlock before destroying it.
	   */
	  *lock = NULL;
	  (void) free (s);
	}
    }
  else
    {
      /*
       * See notes in __ptw32_spinlock_check_need_init() above also.
       */
      __ptw32_mcs_local_node_t node;

      __ptw32_mcs_lock_acquire(&__ptw32_spinlock_test_init_lock, &node);

      /*
       * Check again.
       */
      if (*lock == PTHREAD_SPINLOCK_INITIALIZER)
	{
	  /*
	   * This is all we need to do to destroy a statically
	   * initialised spinlock that has not yet been used (initialised).
	   * If we get to here, another thread
	   * waiting to initialise this mutex will get an EINVAL.
	   */
	  *lock = NULL;
	}
      else
	{
	  /*
	   * The spinlock has been initialised while we were waiting
	   * so assume it's in use.
	   */
	  result = EBUSY;
	}

       __ptw32_mcs_lock_release(&node);
    }

  return (result);
}
