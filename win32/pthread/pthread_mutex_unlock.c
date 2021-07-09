/*
 * pthread_mutex_unlock.c
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
pthread_mutex_unlock (pthread_mutex_t * mutex)
{
  /*
   * Let the system deal with invalid pointers.
   */
  pthread_mutex_t mx = *mutex;
  int kind;
  int result = 0;

  /*
   * If the thread calling us holds the mutex then there is no
   * race condition. If another thread holds the
   * lock then we shouldn't be in here.
   */
  if (mx < PTHREAD_ERRORCHECK_MUTEX_INITIALIZER) // Remember, pointers are unsigned.
    {
      kind = mx->kind;

      if (kind >= 0)
        {
          if (kind == PTHREAD_MUTEX_NORMAL)
	    {
	      LONG idx;

	      idx = (LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG  ((__PTW32_INTERLOCKED_LONGPTR)&mx->lock_idx,
							     (__PTW32_INTERLOCKED_LONG)0);
	      if (idx != 0)
	        {
	          if (idx < 0)
		    {
		      /*
		       * Someone may be waiting on that mutex.
		       */
		      if (SetEvent (mx->event) == 0)
		        {
		          result = EINVAL;
		        }
		    }
	        }
	    }
          else
	    {
	      if (pthread_equal (mx->ownerThread, pthread_self()))
	        {
	          if (kind != PTHREAD_MUTEX_RECURSIVE
		      || 0 == --mx->recursive_count)
		    {
		      mx->ownerThread.p = NULL;

		      if ((LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG  ((__PTW32_INTERLOCKED_LONGPTR)&mx->lock_idx,
							           (__PTW32_INTERLOCKED_LONG)0) < 0L)
		        {
		          /* Someone may be waiting on that mutex */
		          if (SetEvent (mx->event) == 0)
			    {
			      result = EINVAL;
			    }
		        }
		    }
	        }
	      else
	        {
	          result = EPERM;
	        }
	    }
        }
      else
        {
          /* Robust types */
          pthread_t self = pthread_self();
          kind = -kind - 1; /* Convert to non-robust range */

          /*
           * The thread must own the lock regardless of type if the mutex
           * is robust.
           */
          if (pthread_equal (mx->ownerThread, self))
            {
               __PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG ((__PTW32_INTERLOCKED_LONGPTR) &mx->robustNode->stateInconsistent,
                                                       (__PTW32_INTERLOCKED_LONG)__PTW32_ROBUST_NOTRECOVERABLE,
                                                       (__PTW32_INTERLOCKED_LONG)__PTW32_ROBUST_INCONSISTENT);
              if (PTHREAD_MUTEX_NORMAL == kind)
                {
                  __ptw32_robust_mutex_remove(mutex, NULL);

                  if ((LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG ((__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
                                                              (__PTW32_INTERLOCKED_LONG) 0) < 0)
                    {
                      /*
                       * Someone may be waiting on that mutex.
                       */
                      if (SetEvent (mx->event) == 0)
                        {
                          result = EINVAL;
                        }
                    }
                }
              else
                {
                  if (kind != PTHREAD_MUTEX_RECURSIVE
                      || 0 == --mx->recursive_count)
                    {
                      __ptw32_robust_mutex_remove(mutex, NULL);

                      if ((LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG ((__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
                                                                  (__PTW32_INTERLOCKED_LONG) 0) < 0)
                        {
                          /*
                           * Someone may be waiting on that mutex.
                           */
                          if (SetEvent (mx->event) == 0)
                            {
                              result = EINVAL;
                            }
                        }
                    }
                }
            }
          else
            {
              result = EPERM;
            }
        }
    }
  else if (mx != PTHREAD_MUTEX_INITIALIZER)
    {
      /*
       * If mx is PTHREAD_ERRORCHECK_MUTEX_INITIALIZER or PTHREAD_RECURSIVE_MUTEX_INITIALIZER
       * we need to know we are doing something unexpected. For PTHREAD_MUTEX_INITIALIZER
       * (normal) mutexes we can just silently ignore it.
       */
      result = EINVAL;
    }

  return (result);
}
