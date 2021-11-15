/*
 * ptw32_threadStart.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
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
#include <stdio.h>

#if defined(__PTW32_CLEANUP_C)
# include <setjmp.h>
#endif

#if defined(__PTW32_CLEANUP_SEH)

static DWORD
ExceptionFilter (EXCEPTION_POINTERS * ep, DWORD * ei)
{
  switch (ep->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_PTW32_SERVICES:
      {
	DWORD param;
	DWORD numParams = ep->ExceptionRecord->NumberParameters;

	numParams = (numParams > 3) ? 3 : numParams;

	for (param = 0; param < numParams; param++)
	  {
	    ei[param] = (DWORD) ep->ExceptionRecord->ExceptionInformation[param];
	  }

	return EXCEPTION_EXECUTE_HANDLER;
	break;
      }
    default:
      {
	/*
	 * A system unexpected exception has occurred running the user's
	 * routine. We need to cleanup before letting the exception
	 * out of thread scope.
	 */
	pthread_t self = pthread_self ();

	__ptw32_callUserDestroyRoutines (self);

	return EXCEPTION_CONTINUE_SEARCH;
	break;
      }
    }
}

#elif defined(__PTW32_CLEANUP_CXX)

#if defined(_MSC_VER)
# include <eh.h>
#elif defined(__WATCOMC__)
# include <eh.h>
# include <exceptio.h>
#else
# if defined(__GNUC__) && __GNUC__ < 3
#   include <new.h>
# else
#   include <new>
using
  std::terminate;
using
  std::set_terminate;
# endif
#endif

#endif /* __PTW32_CLEANUP_CXX */

/*
 * MSVC6 does not optimize __ptw32_threadStart() safely
 * (i.e. tests/context1.c fails with "abnormal program
 * termination" in some configurations), and there's no
 * point to optimizing this routine anyway
 */
#ifdef _MSC_VER
# pragma optimize("g", off)
# pragma warning( disable : 4748 )
#endif

#if ! defined (__MINGW32__) || (defined (__MSVCRT__) && ! defined (__DMC__))
unsigned
  __stdcall
#else
void
#endif
__ptw32_threadStart (void *vthreadParms)
{
  ThreadParms * threadParms = (ThreadParms *) vthreadParms;
  pthread_t self;
  __ptw32_thread_t * sp;
  void *  (__PTW32_CDECL *start) (void *);
  void * arg;

#if defined(__PTW32_CLEANUP_SEH)
  DWORD
  ei[] = { 0, 0, 0 };
#endif

#if defined(__PTW32_CLEANUP_C)
  int setjmp_rc;
#endif

  __ptw32_mcs_local_node_t stateLock;
  void * status = (void *) 0;

  self = threadParms->tid;
  sp = (__ptw32_thread_t *) self.p;
  start = threadParms->start;
  arg = threadParms->arg;

  free (threadParms);

#if ! defined (__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)
#else
  /*
   * _beginthread does not return the thread id and is running
   * before it returns us the thread handle, and so we do it here.
   */
  sp->thread = GetCurrentThreadId ();
#endif

  pthread_setspecific (__ptw32_selfThreadKey, sp);
  /*
   * Here we're using stateLock as a general-purpose lock
   * to make the new thread wait until the creating thread
   * has the new handle.
   */
  __ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);
  sp->state = PThreadStateRunning;
  __ptw32_mcs_lock_release (&stateLock);

#if defined(__PTW32_CLEANUP_SEH)

  __try
  {
    /*
     * Run the caller's routine;
     */
    status = sp->exitStatus = (*start) (arg);
    sp->state = PThreadStateExiting;

#if defined(_UWIN)
    if (--pthread_count <= 0)
      exit (0);
#endif

  }
  __except (ExceptionFilter (GetExceptionInformation (), ei))
  {
    switch (ei[0])
      {
        case  __PTW32_EPS_CANCEL:
          status = sp->exitStatus = PTHREAD_CANCELED;
#if defined(_UWIN)
          if (--pthread_count <= 0)
        	exit (0);
#endif
          break;
        case  __PTW32_EPS_EXIT:
          status = sp->exitStatus;
          break;
        default:
          status = sp->exitStatus = PTHREAD_CANCELED;
          break;
      }
  }

#else /* __PTW32_CLEANUP_SEH */

#if defined(__PTW32_CLEANUP_C)

  setjmp_rc = setjmp (sp->start_mark);

  if (0 == setjmp_rc)
    {
      /*
       * Run the caller's routine;
       */
      status = sp->exitStatus = (*start) (arg);
      sp->state = PThreadStateExiting;
    }
  else
    {
      switch (setjmp_rc)
        {
      	  case  __PTW32_EPS_CANCEL:
      		status = sp->exitStatus = PTHREAD_CANCELED;
      		break;
      	  case  __PTW32_EPS_EXIT:
      		status = sp->exitStatus;
      		break;
      	  default:
      		status = sp->exitStatus = PTHREAD_CANCELED;
      		break;
        }
    }

#else /* __PTW32_CLEANUP_C */

#if defined(__PTW32_CLEANUP_CXX)

  try
  {
    status = sp->exitStatus = (*start) (arg);
    sp->state = PThreadStateExiting;
  }
  catch (__ptw32_exception_cancel &)
  {
    /*
     * Thread was canceled.
     */
    status = sp->exitStatus = PTHREAD_CANCELED;
  }
  catch (__ptw32_exception_exit &)
  {
    /*
     * Thread was exited via pthread_exit().
     */
    status = sp->exitStatus;
  }
  catch (...)
  {
    /*
     * Some other exception occurred. Clean up while we have
     * the opportunity, and call the terminate handler.
     */
    (void) pthread_win32_thread_detach_np ();
    terminate ();
  }

#else

#error ERROR [__FILE__, line __LINE__]: Cleanup type undefined.

#endif /* __PTW32_CLEANUP_CXX */
#endif /* __PTW32_CLEANUP_C */
#endif /* __PTW32_CLEANUP_SEH */

#if defined (__PTW32_STATIC_LIB)
  /*
   * We need to cleanup the pthread now if we have
   * been statically linked, in which case the cleanup
   * in DllMain won't get done. Joinable threads will
   * only be partially cleaned up and must be fully cleaned
   * up by pthread_join() or pthread_detach().
   *
   * Note: if this library has been statically linked,
   * implicitly created pthreads (those created
   * for Win32 threads which have called pthreads routines)
   * must be cleaned up explicitly by the application
   * by calling pthread_exit().
   * For the dll, DllMain will do the cleanup automatically.
   */
  (void) pthread_win32_thread_detach_np ();
#endif

#if ! defined (__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)
  _endthreadex ((unsigned)(size_t) status);
#else
  _endthread ();
#endif

  /*
   * Never reached.
   */

#if ! defined (__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)
  return (unsigned)(size_t) status;
#endif

}				/* __ptw32_threadStart */

/*
 * Reset optimization
 */
#ifdef _MSC_VER
# pragma optimize("", on)
#endif

#if defined  (__PTW32_USES_SEPARATE_CRT) && (defined(__PTW32_CLEANUP_CXX) || defined(__PTW32_CLEANUP_SEH))
__ptw32_terminate_handler
pthread_win32_set_terminate_np(__ptw32_terminate_handler termFunction)
{
  return set_terminate(termFunction);
}
#endif
