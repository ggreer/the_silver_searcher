/*
 * pthread_attr_getstackaddr.c
 *
 * Description:
 * This translation unit implements operations on thread attribute objects.
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

/* ignore warning "unreferenced formal parameter" */
#if defined(_MSC_VER)
#pragma warning( disable : 4100 )
#endif

int
pthread_attr_getstackaddr (const pthread_attr_t * attr, void **stackaddr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function determines the address of the stack
      *      on which threads created with 'attr' will run.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *      stackaddr
      *              pointer into which is returned the stack address.
      *
      *
      * DESCRIPTION
      *      This function determines the address of the stack
      *      on which threads created with 'attr' will run.
      *
      *      NOTES:
      *              1)      Function supported only if this macro is
      *                      defined:
      *
      *                              _POSIX_THREAD_ATTR_STACKADDR
      *
      *              2)      Create only one thread for each stack
      *                      address..
      *
      * RESULTS
      *              0               successfully retrieved stack address,
      *              EINVAL          'attr' is invalid
      *              ENOSYS          function not supported
      *
      * ------------------------------------------------------
      */
{
#if defined( _POSIX_THREAD_ATTR_STACKADDR ) && _POSIX_THREAD_ATTR_STACKADDR != -1

  if (ptw32_is_attr (attr) != 0)
    {
      return EINVAL;
    }

  *stackaddr = (*attr)->stackaddr;
  return 0;

#else

  return ENOSYS;

#endif /* _POSIX_THREAD_ATTR_STACKADDR */
}
