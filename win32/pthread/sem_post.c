/*
 * -------------------------------------------------------------
 *
 * Module: sem_post.c
 *
 * Purpose:
 *	Semaphores aren't actually part of the PThreads standard.
 *	They are defined by the POSIX Standard:
 *
 *		POSIX 1003.1b-1993	(POSIX.1b)
 *
 * -------------------------------------------------------------
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
#include "semaphore.h"
#include "implement.h"


int
sem_post (sem_t * sem)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function posts a wakeup to a semaphore.
 *
 * PARAMETERS
 *      sem
 *              pointer to an instance of sem_t
 *
 * DESCRIPTION
 *      This function posts a wakeup to a semaphore. If there
 *      are waiting threads (or processes), one is awakened;
 *      otherwise, the semaphore value is incremented by one.
 *
 * RESULTS
 *              0               successfully posted semaphore,
 *              -1              failed, error in errno
 * ERRNO
 *              EINVAL          'sem' is not a valid semaphore,
 *              ENOSYS          semaphores are not supported,
 *              ERANGE          semaphore count is too big
 *
 * ------------------------------------------------------
 */
{
  int result = 0;

  ptw32_mcs_local_node_t node;
  sem_t s = *sem;

  ptw32_mcs_lock_acquire(&s->lock, &node);
  if (s->value < SEM_VALUE_MAX)
    {
#if defined(NEED_SEM)
      if (++s->value <= 0
          && !SetEvent(s->sem))
        {
          s->value--;
          result = EINVAL;
        }
#else
      if (++s->value <= 0
          && !ReleaseSemaphore (s->sem, 1, NULL))
        {
          s->value--;
          result = EINVAL;
        }
#endif /* NEED_SEM */
    }
  else
    {
      result = ERANGE;
    }
  ptw32_mcs_lock_release(&node);

  if (result != 0)
    {
      PTW32_SET_ERRNO(result);
      return -1;
    }

  return 0;
}
