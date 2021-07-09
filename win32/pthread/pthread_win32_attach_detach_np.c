/*
 * pthread_win32_attach_detach_np.c
 *
 * Description:
 * This translation unit implements non-portable thread functions.
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
#include <tchar.h>
#if ! (defined(__GNUC__) || defined (__PTW32_CONFIG_MSVC7) || defined(WINCE))
# include <stdlib.h>
#endif

/*
 * Handle to quserex.dll
 */
static HINSTANCE __ptw32_h_quserex;

BOOL
pthread_win32_process_attach_np ()
{
  TCHAR QuserExDLLPathBuf[1024];
  BOOL result = TRUE;

  result = __ptw32_processInitialize ();

#if defined(_UWIN)
  pthread_count++;
#endif

#if defined(__GNUC__)
  __ptw32_features = 0;
#else
  /*
   * This is obsolete now.
   */
  __ptw32_features =  __PTW32_SYSTEM_INTERLOCKED_COMPARE_EXCHANGE;
#endif

  /*
   * Load QUSEREX.DLL and try to get address of QueueUserAPCEx.
   * Because QUSEREX.DLL requires a driver to be installed we will
   * assume the DLL is in the system directory.
   *
   * This should take care of any security issues.
   */
#if defined(__GNUC__) || defined (__PTW32_CONFIG_MSVC7)
  if(GetSystemDirectory(QuserExDLLPathBuf, sizeof(QuserExDLLPathBuf)))
  {
    (void) strncat(QuserExDLLPathBuf,
                   "\\QUSEREX.DLL",
                   sizeof(QuserExDLLPathBuf) - strlen(QuserExDLLPathBuf) - 1);
    __ptw32_h_quserex = LoadLibrary(QuserExDLLPathBuf);
  }
#else
#  if ! defined(WINCE)
  if(GetSystemDirectory(QuserExDLLPathBuf, sizeof(QuserExDLLPathBuf)/sizeof(TCHAR)) &&
      0 == _tcsncat_s(QuserExDLLPathBuf, _countof(QuserExDLLPathBuf), TEXT("\\QUSEREX.DLL"), 12))
    {
      __ptw32_h_quserex = LoadLibrary(QuserExDLLPathBuf);
    }
#  endif
#endif

  if (__ptw32_h_quserex != NULL)
    {
      __ptw32_register_cancellation = (DWORD (*)(PAPCFUNC, HANDLE, DWORD))
#if defined(NEED_UNICODE_CONSTS)
	GetProcAddress (__ptw32_h_quserex,
			(const TCHAR *) TEXT ("QueueUserAPCEx"));
#else
	GetProcAddress (__ptw32_h_quserex, (LPCSTR) "QueueUserAPCEx");
#endif
    }

  if (NULL == __ptw32_register_cancellation)
    {
      __ptw32_register_cancellation = __ptw32_Registercancellation;

      if (__ptw32_h_quserex != NULL)
	{
	  (void) FreeLibrary (__ptw32_h_quserex);
	}
      __ptw32_h_quserex = 0;
    }
  else
    {
      /* Initialise QueueUserAPCEx */
      BOOL (*queue_user_apc_ex_init) (VOID);

      queue_user_apc_ex_init = (BOOL (*)(VOID))
#if defined(NEED_UNICODE_CONSTS)
	GetProcAddress (__ptw32_h_quserex,
			(const TCHAR *) TEXT ("QueueUserAPCEx_Init"));
#else
	GetProcAddress (__ptw32_h_quserex, (LPCSTR) "QueueUserAPCEx_Init");
#endif

      if (queue_user_apc_ex_init == NULL || !queue_user_apc_ex_init ())
	{
	  __ptw32_register_cancellation = __ptw32_Registercancellation;

	  (void) FreeLibrary (__ptw32_h_quserex);
	  __ptw32_h_quserex = 0;
	}
    }

  if (__ptw32_h_quserex)
    {
      __ptw32_features |=  __PTW32_ALERTABLE_ASYNC_CANCEL;
    }

  return result;
}


BOOL
pthread_win32_process_detach_np ()
{
  if (__ptw32_processInitialized)
    {
      __ptw32_thread_t * sp = (__ptw32_thread_t *) pthread_getspecific (__ptw32_selfThreadKey);

      if (sp != NULL)
	{
	  /*
	   * Detached threads have their resources automatically
	   * cleaned up upon exit (others must be 'joined').
	   */
	  if (sp->detachState == PTHREAD_CREATE_DETACHED)
	    {
	      __ptw32_threadDestroy (sp->ptHandle);
	      if (__ptw32_selfThreadKey)
	        {
	    	  TlsSetValue (__ptw32_selfThreadKey->key, NULL);
	        }
	    }
	}

      /*
       * The DLL is being unmapped from the process's address space
       */
      __ptw32_processTerminate ();

      if (__ptw32_h_quserex)
	{
	  /* Close QueueUserAPCEx */
	  BOOL (*queue_user_apc_ex_fini) (VOID);

	  queue_user_apc_ex_fini = (BOOL (*)(VOID))
#if defined(NEED_UNICODE_CONSTS)
	    GetProcAddress (__ptw32_h_quserex,
			    (const TCHAR *) TEXT ("QueueUserAPCEx_Fini"));
#else
	    GetProcAddress (__ptw32_h_quserex, (LPCSTR) "QueueUserAPCEx_Fini");
#endif

	  if (queue_user_apc_ex_fini != NULL)
	    {
	      (void) queue_user_apc_ex_fini ();
	    }
	  (void) FreeLibrary (__ptw32_h_quserex);
	}
    }

  return TRUE;
}

BOOL
pthread_win32_thread_attach_np ()
{
  return TRUE;
}

BOOL
pthread_win32_thread_detach_np ()
{
  if (__ptw32_processInitialized)
    {
      /*
       * Don't use pthread_self() - to avoid creating an implicit POSIX thread handle
       * unnecessarily.
       */
      __ptw32_thread_t * sp = (__ptw32_thread_t *) pthread_getspecific (__ptw32_selfThreadKey);

      if (sp != NULL) // otherwise Win32 thread with no implicit POSIX handle.
	{
          __ptw32_mcs_local_node_t stateLock;
	  __ptw32_callUserDestroyRoutines (sp->ptHandle);

	  __ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);
	  sp->state = PThreadStateLast;
	  /*
	   * If the thread is joinable at this point then it MUST be joined
	   * or detached explicitly by the application.
	   */
	  __ptw32_mcs_lock_release (&stateLock);

          /*
           * Robust Mutexes
           */
          while (sp->robustMxList != NULL)
            {
              pthread_mutex_t mx = sp->robustMxList->mx;
              __ptw32_robust_mutex_remove(&mx, sp);
              (void)  __PTW32_INTERLOCKED_EXCHANGE_LONG(
                        (__PTW32_INTERLOCKED_LONGPTR)&mx->robustNode->stateInconsistent,
                        (__PTW32_INTERLOCKED_LONG)-1);
              /*
               * If there are no waiters then the next thread to block will
               * sleep, wake up immediately and then go back to sleep.
               * See pthread_mutex_lock.c.
               */
              SetEvent(mx->event);
            }


	  if (sp->detachState == PTHREAD_CREATE_DETACHED)
	    {
	      __ptw32_threadDestroy (sp->ptHandle);

	      if (__ptw32_selfThreadKey)
	        {
	    	  TlsSetValue (__ptw32_selfThreadKey->key, NULL);
	        }
	    }
	}
    }

  return TRUE;
}

BOOL
pthread_win32_test_features_np (int feature_mask)
{
  return ((__ptw32_features & feature_mask) == feature_mask);
}
