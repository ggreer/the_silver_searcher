/*
 * pthread_mutex_lock.c
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

#if !defined(_UWIN)
/*#   include <process.h> */
#endif
#include "pthread.h"
#include "implement.h"

int
pthread_mutex_lock (pthread_mutex_t * mutex)
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
      if (PTHREAD_MUTEX_NORMAL == kind)
        {
          if  ((__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG(
		        (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
		        (__PTW32_INTERLOCKED_LONG) 1) != 0)
	    {
	      while  ((__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG(
                               (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
			       (__PTW32_INTERLOCKED_LONG) -1) != 0)
	        {
	          if (WAIT_OBJECT_0 != WaitForSingleObject (mx->event, INFINITE))
	            {
	              result = EINVAL;
		      break;
	            }
	        }
	    }
        }
      else
        {
          pthread_t self = pthread_self();

          if  ((__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG(
                        (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
		        (__PTW32_INTERLOCKED_LONG) 1,
		        (__PTW32_INTERLOCKED_LONG) 0) == 0)
	    {
	      mx->recursive_count = 1;
	      mx->ownerThread = self;
	    }
          else
	    {
	      if (pthread_equal (mx->ownerThread, self))
	        {
	          if (kind == PTHREAD_MUTEX_RECURSIVE)
		    {
		      mx->recursive_count++;
		    }
	          else
		    {
		      result = EDEADLK;
		    }
	        }
	      else
	        {
	          while  ((__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG(
                                   (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
			           (__PTW32_INTERLOCKED_LONG) -1) != 0)
		    {
	              if (WAIT_OBJECT_0 != WaitForSingleObject (mx->event, INFINITE))
		        {
	                  result = EINVAL;
		          break;
		        }
		    }

	          if (0 == result)
		    {
		      mx->recursive_count = 1;
		      mx->ownerThread = self;
		    }
	        }
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
      __ptw32_robust_state_t* statePtr = &mx->robustNode->stateInconsistent;

      if  ((__PTW32_INTERLOCKED_LONG)__PTW32_ROBUST_NOTRECOVERABLE ==  __PTW32_INTERLOCKED_EXCHANGE_ADD_LONG(
                                                  (__PTW32_INTERLOCKED_LONGPTR)statePtr,
                                                  (__PTW32_INTERLOCKED_LONG)0))
        {
          result = ENOTRECOVERABLE;
        }
      else
        {
          pthread_t self = pthread_self();

          kind = -kind - 1; /* Convert to non-robust range */
    
          if (PTHREAD_MUTEX_NORMAL == kind)
            {
              if  ((__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG(
                            (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
                            (__PTW32_INTERLOCKED_LONG) 1) != 0)
                {
                  while (0 == (result = __ptw32_robust_mutex_inherit(mutex))
                           &&  (__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG(
                                        (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
                                        (__PTW32_INTERLOCKED_LONG) -1) != 0)
                    {
                      if (WAIT_OBJECT_0 != WaitForSingleObject (mx->event, INFINITE))
                        {
                          result = EINVAL;
                          break;
                        }
                      if  ((__PTW32_INTERLOCKED_LONG)__PTW32_ROBUST_NOTRECOVERABLE ==
                                   __PTW32_INTERLOCKED_EXCHANGE_ADD_LONG(
                                     (__PTW32_INTERLOCKED_LONGPTR)statePtr,
                                     (__PTW32_INTERLOCKED_LONG)0))
                        {
                          /* Unblock the next thread */
                          SetEvent(mx->event);
                          result = ENOTRECOVERABLE;
                          break;
                        }
                    }
                }
              if (0 == result || EOWNERDEAD == result)
                {
                  /*
                   * Add mutex to the per-thread robust mutex currently-held list.
                   * If the thread terminates, all mutexes in this list will be unlocked.
                   */
                  __ptw32_robust_mutex_add(mutex, self);
                }
            }
          else
            {
              if  ((__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG(
                            (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
                            (__PTW32_INTERLOCKED_LONG) 1,
                            (__PTW32_INTERLOCKED_LONG) 0) == 0)
                {
                  mx->recursive_count = 1;
                  /*
                   * Add mutex to the per-thread robust mutex currently-held list.
                   * If the thread terminates, all mutexes in this list will be unlocked.
                   */
                  __ptw32_robust_mutex_add(mutex, self);
                }
              else
                {
                  if (pthread_equal (mx->ownerThread, self))
                    {
                      if (PTHREAD_MUTEX_RECURSIVE == kind)
                        {
                          mx->recursive_count++;
                        }
                      else
                        {
                          result = EDEADLK;
                        }
                    }
                  else
                    {
                      while (0 == (result = __ptw32_robust_mutex_inherit(mutex))
                               &&  (__PTW32_INTERLOCKED_LONG)  __PTW32_INTERLOCKED_EXCHANGE_LONG(
                                            (__PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
                                            (__PTW32_INTERLOCKED_LONG) -1) != 0)
                        {
                          if (WAIT_OBJECT_0 != WaitForSingleObject (mx->event, INFINITE))
                            {
                              result = EINVAL;
                              break;
                            }
                          if  ((__PTW32_INTERLOCKED_LONG)__PTW32_ROBUST_NOTRECOVERABLE ==
                                       __PTW32_INTERLOCKED_EXCHANGE_ADD_LONG(
                                         (__PTW32_INTERLOCKED_LONGPTR)statePtr,
                                         (__PTW32_INTERLOCKED_LONG)0))
                            {
                              /* Unblock the next thread */
                              SetEvent(mx->event);
                              result = ENOTRECOVERABLE;
                              break;
                            }
                        }

                      if (0 == result || EOWNERDEAD == result)
                        {
                          mx->recursive_count = 1;
                          /*
                           * Add mutex to the per-thread robust mutex currently-held list.
                           * If the thread terminates, all mutexes in this list will be unlocked.
                           */
                          __ptw32_robust_mutex_add(mutex, self);
                        }
                    }
	        }
            }
        }
    }

  return (result);
}

