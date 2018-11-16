/*
 * pthread_self.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
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
  __ptw32_thread_t * sp;

#if defined(_UWIN)
  if (!__ptw32_selfThreadKey)
    return nil;
#endif

  sp = (__ptw32_thread_t *) pthread_getspecific (__ptw32_selfThreadKey);

  if (sp != NULL)
    {
      self = sp->ptHandle;
    }
  else
    {
      int fail =  __PTW32_FALSE;

      /*
       * Need to create an implicit 'self' for the currently
       * executing thread.
       */
      self = __ptw32_new ();
      sp = (__ptw32_thread_t *) self.p;

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
    		  fail =  __PTW32_TRUE;
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
    	              else fail =  __PTW32_TRUE;
    	            }
    	          else fail =  __PTW32_TRUE;
    	        }
    	      else fail =  __PTW32_TRUE;

#endif

    	      sp->sched_priority = GetThreadPriority (sp->threadH);
    	      pthread_setspecific (__ptw32_selfThreadKey, (void *) sp);
    	    }
        }

      if (fail)
        {
    	  /*
    	   * Thread structs are never freed but are reused so if this
    	   * continues to fail at least we don't leak memory.
    	   */
    	  __ptw32_threadReusePush (self);
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
