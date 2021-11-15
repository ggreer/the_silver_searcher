/*
 * ptw32_processInitialize.c
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

  /*  Visual Studio   &&   compiled with /MT, not /MD  */
#if defined(_MSC_VER) && defined(_MT) && !defined(_DLL)
  __PTW32_BEGIN_C_DECLS
#if defined(_UCRT) /* Names changed in VS 14 / 2015 with the new UCRT */
  #define _crtheap __acrt_heap
  #define _heap_init __acrt_initialize_heap
#endif /* defined(_UCRT) */
  extern HANDLE _crtheap;
  /*
   * Visual studio versions up to 9 / 2008 use an argument for their _heap_init() routine, to enable Multi-Treading support.
   * Versions up to 12 / 2013 have no argument. They'll ignore the one passed. Multi-Threading is not an option anymore.
   * Versions 14 / 2015 and above use a redesigned CRT, split into a VCRuntime and UCRT. The functions were renamed.
   */
  extern int __cdecl _heap_init(int mtflag);
  __PTW32_END_C_DECLS
#endif // defined(_MSC_VER) ...


int
__ptw32_processInitialize (void)
     /*
      * ------------------------------------------------------
      * DOCPRIVATE
      *      This function performs process wide initialization for
      *      the pthread library.
      *
      * PARAMETERS
      *      N/A
      *
      * DESCRIPTION
      *      This function performs process wide initialization for
      *      the pthread library.
      *      If successful, this routine sets the global variable
      *      __ptw32_processInitialized to TRUE.
      *
      * RESULTS
      *              TRUE    if successful,
      *              FALSE   otherwise
      *
      * ------------------------------------------------------
      */
{
  if (__ptw32_processInitialized)
    {
      return  __PTW32_TRUE;
    }

  /*
   * Explicitly initialise all variables from global.c
   */
  __ptw32_threadReuseTop =  __PTW32_THREAD_REUSE_EMPTY;
  __ptw32_threadReuseBottom =  __PTW32_THREAD_REUSE_EMPTY;
  __ptw32_selfThreadKey = NULL;
  __ptw32_cleanupKey = NULL;
  __ptw32_cond_list_head = NULL;
  __ptw32_cond_list_tail = NULL;

  __ptw32_concurrency = 0;

  /* What features have been auto-detected */
  __ptw32_features = 0;

  /*
   * Global [process wide] thread sequence Number
   */
  __ptw32_threadSeqNumber = 0;

  /*
   * Function pointer to QueueUserAPCEx if it exists, otherwise
   * it will be set at runtime to a substitute routine which cannot unblock
   * blocked threads.
   */
  __ptw32_register_cancellation = NULL;

  /*
   * Global lock for managing pthread_t struct reuse.
   */
  __ptw32_thread_reuse_lock = 0;

  /*
   * Global lock for testing internal state of statically declared mutexes.
   */
  __ptw32_mutex_test_init_lock = 0;

  /*
   * Global lock for testing internal state of PTHREAD_COND_INITIALIZER
   * created condition variables.
   */
  __ptw32_cond_test_init_lock = 0;

  /*
   * Global lock for testing internal state of PTHREAD_RWLOCK_INITIALIZER
   * created read/write locks.
   */
  __ptw32_rwlock_test_init_lock = 0;

  /*
   * Global lock for testing internal state of PTHREAD_SPINLOCK_INITIALIZER
   * created spin locks.
   */
  __ptw32_spinlock_test_init_lock = 0;

  /*
   * Global lock for condition variable linked list. The list exists
   * to wake up CVs when a WM_TIMECHANGE message arrives. See
   * w32_TimeChangeHandler.c.
   */
  __ptw32_cond_list_lock = 0;

  #if defined(_UWIN)
  /*
   * Keep a count of the number of threads.
   */
  pthread_count = 0;
  #endif

  __ptw32_processInitialized =  __PTW32_TRUE;

  /*
   * Initialize the CRT library memory heap
   * This routine may run before main().
   * For fully static builds compiled with MSVC /MT option, it may even run
   * before the static CRT library is fully initialized.
   * To make sure that the forthcoming memory allocations succeed, it is
   * necessary to make sure that the CRT library memory heap is initialized.
   */
  /*   Visual Studio  &&  Compiled with /MT, not /MD   */
#if defined(_MSC_VER) && defined(_MT) && !defined(_DLL)
  if (_crtheap == 0) _heap_init(_MT);
#endif // defined(_MSC_VER) ...

  /*
   * Initialize Keys
   */
  if ((pthread_key_create (&__ptw32_selfThreadKey, NULL) != 0) ||
      (pthread_key_create (&__ptw32_cleanupKey, NULL) != 0))
    {

      __ptw32_processTerminate ();
    }

  return (__ptw32_processInitialized);

}				/* processInitialize */
