/*
 * pthread_mutex_unlock.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
 *
 * --------------------------------------------------------------------------
 *
 *      pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999-2021 pthreads-win32 / pthreads4w contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * --------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "pthread.h"
#include "implement.h"


int PTW32_CDECL
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

	      idx = (LONG) PTW32_INTERLOCKED_EXCHANGE_LONG ((PTW32_INTERLOCKED_LONGPTR)&mx->lock_idx,
							    (PTW32_INTERLOCKED_LONG)0);
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
	      else	/* [i_a] when unlocking an unlocked mutex, pthread_mutex_unlock() should always produce EPERM for any type of mutex -- see test mutex7.c */
	        {
	          result = EPERM;
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

		      if ((LONG) PTW32_INTERLOCKED_EXCHANGE_LONG ((PTW32_INTERLOCKED_LONGPTR)&mx->lock_idx,
							          (PTW32_INTERLOCKED_LONG)0) < 0L)
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
              PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG((PTW32_INTERLOCKED_LONGPTR) &mx->robustNode->stateInconsistent,
                                                      (PTW32_INTERLOCKED_LONG)PTW32_ROBUST_NOTRECOVERABLE,
                                                      (PTW32_INTERLOCKED_LONG)PTW32_ROBUST_INCONSISTENT);
              if (PTHREAD_MUTEX_NORMAL == kind)
                {
                  ptw32_robust_mutex_remove(mutex, NULL);

                  if ((LONG) PTW32_INTERLOCKED_EXCHANGE_LONG((PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
                                                             (PTW32_INTERLOCKED_LONG) 0) < 0)
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
                      ptw32_robust_mutex_remove(mutex, NULL);

                      if ((LONG) PTW32_INTERLOCKED_EXCHANGE_LONG((PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
                                                                 (PTW32_INTERLOCKED_LONG) 0) < 0)
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
