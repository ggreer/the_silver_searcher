/*
 * pthread_rwlock_unlock.c
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
pthread_rwlock_unlock (pthread_rwlock_t * rwlock)
{
  int result, result1;
  pthread_rwlock_t rwl;

  if (rwlock == NULL || *rwlock == NULL)
    {
      return (EINVAL);
    }

  if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
    {
      /*
       * Assume any race condition here is harmless.
       */
      return 0;
    }

  rwl = *rwlock;

  if (rwl->nMagic !=  __PTW32_RWLOCK_MAGIC)
    {
      return EINVAL;
    }

  if (rwl->nExclusiveAccessCount == 0)
    {
      if ((result =
	   pthread_mutex_lock (&(rwl->mtxSharedAccessCompleted))) != 0)
	{
	  return result;
	}

      if (++rwl->nCompletedSharedAccessCount == 0)
	{
	  result = pthread_cond_signal (&(rwl->cndSharedAccessCompleted));
	}

      result1 = pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted));
    }
  else
    {
      rwl->nExclusiveAccessCount--;

      result = pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted));
      result1 = pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));

    }

  return ((result != 0) ? result : result1);
}
