/*
 * pthread_mutex_destroy.c
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

#include "pthread.h"
#include "implement.h"


int
pthread_mutex_destroy (pthread_mutex_t * mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  /*
   * Let the system deal with invalid pointers.
   */

  /*
   * Check to see if we have something to delete.
   */
  if (*mutex < PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
    {
      mx = *mutex;

      result = pthread_mutex_trylock (&mx);

      /*
       * If trylock succeeded and the mutex is not recursively locked it
       * can be destroyed.
       */
      if (0 == result || ENOTRECOVERABLE == result)
	{
	  if (mx->kind != PTHREAD_MUTEX_RECURSIVE || 1 == mx->recursive_count)
	    {
	      /*
	       * FIXME!!!
	       * The mutex isn't held by another thread but we could still
	       * be too late invalidating the mutex below since another thread
	       * may already have entered mutex_lock and the check for a valid
	       * *mutex != NULL.
	       */
	      *mutex = NULL;

	      result = (0 == result)?pthread_mutex_unlock(&mx):0;

	      if (0 == result)
		{
                  if (mx->robustNode != NULL)
                    {
                      free(mx->robustNode);
                    }
		  if (!CloseHandle (mx->event))
		    {
		      *mutex = mx;
		      result = EINVAL;
		    }
		  else
		    {
		      free (mx);
		    }
		}
	      else
		{
		  /*
		   * Restore the mutex before we return the error.
		   */
		  *mutex = mx;
		}
	    }
	  else			/* mx->recursive_count > 1 */
	    {
	      /*
	       * The mutex must be recursive and already locked by us (this thread).
	       */
	      mx->recursive_count--;	/* Undo effect of pthread_mutex_trylock() above */
	      result = EBUSY;
	    }
	}
    }
  else
    {
      __ptw32_mcs_local_node_t node;

      /*
       * See notes in __ptw32_mutex_check_need_init() above also.
       */

      __ptw32_mcs_lock_acquire(&__ptw32_mutex_test_init_lock, &node);

      /*
       * Check again.
       */
      if (*mutex >= PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
	{
	  /*
	   * This is all we need to do to destroy a statically
	   * initialised mutex that has not yet been used (initialised).
	   * If we get to here, another thread
	   * waiting to initialise this mutex will get an EINVAL.
	   */
	  *mutex = NULL;
	}
      else
	{
	  /*
	   * The mutex has been initialised while we were waiting
	   * so assume it's in use.
	   */
	  result = EBUSY;
	}
      __ptw32_mcs_lock_release(&node);
    }

  return (result);
}
