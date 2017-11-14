/*
 * ptw32_mutex_check_need_init.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2012 Pthreads-win32 contributors
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
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"

static struct pthread_mutexattr_t_ ptw32_recursive_mutexattr_s =
  {PTHREAD_PROCESS_PRIVATE, PTHREAD_MUTEX_RECURSIVE, PTHREAD_MUTEX_STALLED};
static struct pthread_mutexattr_t_ ptw32_errorcheck_mutexattr_s =
  {PTHREAD_PROCESS_PRIVATE, PTHREAD_MUTEX_ERRORCHECK, PTHREAD_MUTEX_STALLED};
static pthread_mutexattr_t ptw32_recursive_mutexattr = &ptw32_recursive_mutexattr_s;
static pthread_mutexattr_t ptw32_errorcheck_mutexattr = &ptw32_errorcheck_mutexattr_s;


INLINE int
ptw32_mutex_check_need_init (pthread_mutex_t * mutex)
{
  register int result = 0;
  register pthread_mutex_t mtx;
  ptw32_mcs_local_node_t node;

  ptw32_mcs_lock_acquire(&ptw32_mutex_test_init_lock, &node);

  /*
   * We got here possibly under race
   * conditions. Check again inside the critical section
   * and only initialise if the mutex is valid (not been destroyed).
   * If a static mutex has been destroyed, the application can
   * re-initialise it only by calling pthread_mutex_init()
   * explicitly.
   */
  mtx = *mutex;

  if (mtx == PTHREAD_MUTEX_INITIALIZER)
    {
      result = pthread_mutex_init (mutex, NULL);
    }
  else if (mtx == PTHREAD_RECURSIVE_MUTEX_INITIALIZER)
    {
      result = pthread_mutex_init (mutex, &ptw32_recursive_mutexattr);
    }
  else if (mtx == PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
    {
      result = pthread_mutex_init (mutex, &ptw32_errorcheck_mutexattr);
    }
  else if (mtx == NULL)
    {
      /*
       * The mutex has been destroyed while we were waiting to
       * initialise it, so the operation that caused the
       * auto-initialisation should fail.
       */
      result = EINVAL;
    }

  ptw32_mcs_lock_release(&node);

  return (result);
}
