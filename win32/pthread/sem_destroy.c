/*
 * -------------------------------------------------------------
 *
 * Module: sem_destroy.c
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
sem_destroy (sem_t * sem)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function destroys an unnamed semaphore.
 *
 * PARAMETERS
 *      sem
 *              pointer to an instance of sem_t
 *
 * DESCRIPTION
 *      This function destroys an unnamed semaphore.
 *
 * RESULTS
 *              0               successfully destroyed semaphore,
 *              -1              failed, error in errno
 * ERRNO
 *              EINVAL          'sem' is not a valid semaphore,
 *              ENOSYS          semaphores are not supported,
 *              EBUSY           threads (or processes) are currently
 *                                      blocked on 'sem'
 *
 * ------------------------------------------------------
 */
{
  int result = 0;
  sem_t s = NULL;

  if (sem == NULL || *sem == NULL)
    {
      result = EINVAL;
    }
  else
    {
      ptw32_mcs_local_node_t node;
      s = *sem;

      if ((result = ptw32_mcs_lock_try_acquire(&s->lock, &node)) == 0)
        {
          if (s->value < 0)
            {
              result = EBUSY;
            }
          else
            {
              /*
               * There are no threads currently blocked on this semaphore
               * however there could be threads about to wait behind us.
               * It is up to the application to ensure this is not the case.
               */
              if (!CloseHandle (s->sem))
                {
                  result = EINVAL;
                }
            }
          ptw32_mcs_lock_release(&node);
        }
    }

  if (result != 0)
    {
      PTW32_SET_ERRNO(result);
      return -1;
    }

  free (s);

  return 0;

}				/* sem_destroy */
