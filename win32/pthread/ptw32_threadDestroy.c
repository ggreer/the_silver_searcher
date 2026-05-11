/*
 * ptw32_threadDestroy.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
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


void
ptw32_threadDestroy (pthread_t thread)
{
  ptw32_thread_t * tp = (ptw32_thread_t *) thread.p;

  if (tp != NULL)
    {
      /*
       * Copy thread state so that the thread can be atomically NULLed.
       */
#if ! defined(__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)
      HANDLE threadH = tp->threadH;
#endif
      HANDLE cancelEvent = tp->cancelEvent;

      /*
       * Thread ID structs are never freed. They're NULLed and reused.
       * This also sets the thread state to PThreadStateInitial before
       * it is finally set to PThreadStateReuse.
       */
      ptw32_threadReusePush (thread);

      /* Now work on the copies as the `tp` instance has already been nulled. */
      if (cancelEvent != NULL)
	{
	  CloseHandle (cancelEvent);
	}

#if ! defined(__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)
      /*
       * See documentation for endthread vs endthreadex.
       */
      if (threadH != 0)
	{
	  CloseHandle (threadH);
	}
#endif

    }
}				/* ptw32_threadDestroy */

