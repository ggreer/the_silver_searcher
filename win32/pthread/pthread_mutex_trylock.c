/*
 * pthread_mutex_trylock.c
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
pthread_mutex_trylock (pthread_mutex_t * mutex)
{
  /*
   * Let the system deal with invalid pointers.
   */
  pthread_mutex_t mx = *mutex;
  int kind;
  int result = 0;

  if (mx == NULL)
    {
      return EINVAL;
    }

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static mutex. We check
   * again inside the guarded section of __ptw32_mutex_check_need_init()
   * to avoid race conditions.
   */
  if (mx >= PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
    {
      if ((result = __ptw32_mutex_check_need_init (mutex)) != 0)
	{
	  return (result);
	}
      mx = *mutex;
    }

  kind = mx->kind;

  if (kind >= 0)
    {
      /* Non-robust */
      if (0 ==  (__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG (
		          (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
		          (__PTW32_INTERLOCKED_LONG) 1,
		          (__PTW32_INTERLOCKED_LONG) 0))
        {
          if (kind != PTHREAD_MUTEX_NORMAL)
	    {
	      mx->recursive_count = 1;
	      mx->ownerThread = pthread_self ();
	    }
        }
      else
        {
          if (kind == PTHREAD_MUTEX_RECURSIVE &&
	      pthread_equal (mx->ownerThread, pthread_self ()))
	    {
	      mx->recursive_count++;
	    }
          else
	    {
	      result = EBUSY;
	    }
        }
    }
  else
    {
      /*
       * Robust types
       * All types record the current owner thread.
       * The mutex is added to a per thread list when ownership is acquired.
       */
      pthread_t self;
      __ptw32_robust_state_t* statePtr = &mx->robustNode->stateInconsistent;

      if  ((__PTW32_INTERLOCKED_LONG)__PTW32_ROBUST_NOTRECOVERABLE ==
                   __PTW32_INTERLOCKED_EXCHANGE_ADD_LONG(
                     (__PTW32_INTERLOCKED_LONGPTR)statePtr,
                     (__PTW32_INTERLOCKED_LONG)0))
        {
          return ENOTRECOVERABLE;
        }

      self = pthread_self();
      kind = -kind - 1; /* Convert to non-robust range */

      if (0 ==  (__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG (
        	          (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
        	          (__PTW32_INTERLOCKED_LONG) 1,
        	          (__PTW32_INTERLOCKED_LONG) 0))
        {
          if (kind != PTHREAD_MUTEX_NORMAL)
            {
              mx->recursive_count = 1;
            }
          __ptw32_robust_mutex_add(mutex, self);
        }
      else
        {
          if (PTHREAD_MUTEX_RECURSIVE == kind &&
              pthread_equal (mx->ownerThread, pthread_self ()))
            {
              mx->recursive_count++;
            }
          else
            {
              if (EOWNERDEAD == (result = __ptw32_robust_mutex_inherit(mutex)))
                {
                  mx->recursive_count = 1;
                  __ptw32_robust_mutex_add(mutex, self);
                }
              else
                {
                  if (0 == result)
                    { 
	              result = EBUSY;
                    }
                }
	    }
        }
    }

  return (result);
}
