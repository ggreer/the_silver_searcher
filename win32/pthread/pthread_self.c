/*
 * pthread_self.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
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

pthread_t
pthread_self (void)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function returns a reference to the current running
      *      thread.
      *
      * PARAMETERS
      *      N/A
      *
      *
      * DESCRIPTION
      *      This function returns a reference to the current running
      *      thread.
      *
      * RESULTS
      *              pthread_t       reference to the current thread
      *
      * ------------------------------------------------------
      */
{
  pthread_t self;
  pthread_t nil = {NULL, 0};
  ptw32_thread_t * sp;

  if (!ptw32_processInitialize())
	return nil;

#if defined(_UWIN)
  if (!ptw32_selfThreadKey)
    return nil;
#endif

  sp = (ptw32_thread_t *) pthread_getspecific (ptw32_selfThreadKey);

  if (sp != NULL)
    {
      self = sp->ptHandle;
    }
  else
    {
	  int fail = PTW32_FALSE;

      /*
       * Need to create an implicit 'self' for the currently
       * executing thread.
       */
      self = ptw32_new ();
      sp = (ptw32_thread_t *) self.p;

      if (sp != NULL)
        {
    	  /*
    	   * This is a non-POSIX thread which has chosen to call
    	   * a POSIX threads function for some reason. We assume that
    	   * it isn't joinable, but we do assume that it's
    	   * (deferred) cancelable.
    	   */
    	  sp->implicit = 1;
    	  sp->detachState = PTHREAD_CREATE_DETACHED;
    	  sp->thread = GetCurrentThreadId ();

#if defined(NEED_DUPLICATEHANDLE)
    	  /*
    	   * DuplicateHandle does not exist on WinCE.
    	   *
    	   * NOTE:
    	   * GetCurrentThread only returns a pseudo-handle
    	   * which is only valid in the current thread context.
    	   * Therefore, you should not pass the handle to
    	   * other threads for whatever purpose.
    	   */
    	  sp->threadH = GetCurrentThread ();
#else
    	  if (!DuplicateHandle (GetCurrentProcess (),
				GetCurrentThread (),
				GetCurrentProcess (),
				&sp->threadH,
				0, FALSE, DUPLICATE_SAME_ACCESS))
    	    {
    		  fail = PTW32_TRUE;
    	    }
#endif

    	  if (!fail)
    	    {

#if defined(HAVE_CPU_AFFINITY)

    	      /*
    	       * Get this threads CPU affinity by temporarily setting the threads
    	       * affinity to that of the process to get the old thread affinity,
    	       * then reset to the old affinity.
    	       */
	      DWORD_PTR vThreadMask, vProcessMask, vSystemMask;
    	      if (GetProcessAffinityMask(GetCurrentProcess(), &vProcessMask, &vSystemMask))
    	        {
    	          vThreadMask = SetThreadAffinityMask(sp->threadH, vProcessMask);
    	          if (vThreadMask)
    	            {
    	              if (SetThreadAffinityMask(sp->threadH, vThreadMask))
    	                {
    	                  sp->cpuset = (size_t) vThreadMask;
    	                }
    	              else fail = PTW32_TRUE;
    	            }
    	          else fail = PTW32_TRUE;
    	        }
    	      else fail = PTW32_TRUE;

#endif

    	      /*
    	       * No need to explicitly serialise access to sched_priority
    	       * because the new handle is not yet public.
    	       */
    	      sp->sched_priority = GetThreadPriority (sp->threadH);
    	      pthread_setspecific (ptw32_selfThreadKey, (void *) sp);
    	    }
        }

      if (fail)
        {
    	  /*
    	   * Thread structs are never freed but are reused so if this
    	   * continues to fail at least we don't leak memory.
    	   */
		  sp->threadH = 0;	// [i_a]
    	  ptw32_threadReusePush (self);
    	  /*
    	   * As this is a win32 thread calling us and we have failed,
    	   * return a value that makes sense to win32.
    	   */
    	  return nil;
        }
      else
	{
	  /*
	   * This implicit POSIX thread is running (it called us).
	   * No other thread can reference us yet because all API calls
	   * passing a pthread_t should recognise an invalid thread id
	   * through the reuse counter inequality.
	   */
    	  sp->state = PThreadStateRunning;
	}
    }

  return (self);
}
