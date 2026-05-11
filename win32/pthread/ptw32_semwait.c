/*
 * ptw32_semwait.c
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

#if !defined(_UWIN)
/*#   include <process.h> */
#endif
#include "pthread.h"
#include "implement.h"


int
ptw32_semwait (sem_t * sem)
/*
 * ------------------------------------------------------
 * DESCRIPTION
 *      This function waits on a POSIX semaphore. If the
 *      semaphore value is greater than zero, it decreases
 *      its value by one. If the semaphore value is zero, then
 *      the calling thread (or process) is blocked until it can
 *      successfully decrease the value.
 *
 *      Unlike sem_wait(), this routine is non-cancelable.
 *
 * RESULTS
 *              0               successfully decreased semaphore,
 *              -1              failed, error in errno.
 * ERRNO
 *              EINVAL          'sem' is not a valid semaphore,
 *              ENOSYS          semaphores are not supported,
 *              EINTR           the function was interrupted by a signal,
 *              EDEADLK         a deadlock condition was detected.
 *
 * ------------------------------------------------------
 */
{
  ptw32_mcs_local_node_t node;
  int v;
  int result = 0;
  sem_t s = *sem;

  ptw32_mcs_lock_acquire(&s->lock, &node);
  v = --s->value;
  ptw32_mcs_lock_release(&node);

  if (v < 0)
    {
      /* Must wait */
      if (WaitForSingleObject (s->sem, INFINITE) == WAIT_OBJECT_0)
        {
#if defined(NEED_SEM)
          ptw32_mcs_lock_acquire(&s->lock, &node);
          if (s->leftToUnblock > 0)
            {
              --s->leftToUnblock;
              SetEvent(s->sem);
            }
          ptw32_mcs_lock_release(&node);
#endif
return 0;
        }
    }
  else
    {
      return 0;
    }

  if (result != 0)
    {
      PTW32_SET_ERRNO(result);
      return -1;
    }

  return 0;

}				/* ptw32_semwait */
