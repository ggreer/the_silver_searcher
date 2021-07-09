/*
 * pthread_rwlock_destroy.c
 *
 * Description:
 * This translation unit implements read/write lock primitives.
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

#include <limits.h>

#include "pthread.h"
#include "implement.h"

int
pthread_rwlock_destroy (pthread_rwlock_t * rwlock)
{
  pthread_rwlock_t rwl;
  int result = 0, result1 = 0, result2 = 0;

  if (rwlock == NULL || *rwlock == NULL)
    {
      return EINVAL;
    }

  if (*rwlock != PTHREAD_RWLOCK_INITIALIZER)
    {
      rwl = *rwlock;

      if (rwl->nMagic !=  __PTW32_RWLOCK_MAGIC)
	{
	  return EINVAL;
	}

      if ((result = pthread_mutex_lock (&(rwl->mtxExclusiveAccess))) != 0)
	{
	  return result;
	}

      if ((result =
	   pthread_mutex_lock (&(rwl->mtxSharedAccessCompleted))) != 0)
	{
	  (void) pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
	  return result;
	}

      /*
       * Check whether any threads own/wait for the lock (wait for ex.access);
       * report "BUSY" if so.
       */
      if (rwl->nExclusiveAccessCount > 0
	  || rwl->nSharedAccessCount > rwl->nCompletedSharedAccessCount)
	{
	  result = pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted));
	  result1 = pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
	  result2 = EBUSY;
	}
      else
	{
	  rwl->nMagic = 0;

	  if ((result =
	       pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted))) != 0)
	    {
	      pthread_mutex_unlock (&rwl->mtxExclusiveAccess);
	      return result;
	    }

	  if ((result =
	       pthread_mutex_unlock (&(rwl->mtxExclusiveAccess))) != 0)
	    {
	      return result;
	    }

	  *rwlock = NULL;	/* Invalidate rwlock before anything else */
	  result = pthread_cond_destroy (&(rwl->cndSharedAccessCompleted));
	  result1 = pthread_mutex_destroy (&(rwl->mtxSharedAccessCompleted));
	  result2 = pthread_mutex_destroy (&(rwl->mtxExclusiveAccess));
	  (void) free (rwl);
	}
    }
  else
    {
      __ptw32_mcs_local_node_t node;
      /*
       * See notes in __ptw32_rwlock_check_need_init() above also.
       */
      __ptw32_mcs_lock_acquire(&__ptw32_rwlock_test_init_lock, &node);

      /*
       * Check again.
       */
      if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
	{
	  /*
	   * This is all we need to do to destroy a statically
	   * initialised rwlock that has not yet been used (initialised).
	   * If we get to here, another thread
	   * waiting to initialise this rwlock will get an EINVAL.
	   */
	  *rwlock = NULL;
	}
      else
	{
	  /*
	   * The rwlock has been initialised while we were waiting
	   * so assume it's in use.
	   */
	  result = EBUSY;
	}

      __ptw32_mcs_lock_release(&node);
    }

  return ((result != 0) ? result : ((result1 != 0) ? result1 : result2));
}
