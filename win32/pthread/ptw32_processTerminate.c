/*
 * ptw32_processTerminate.c
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


void
__ptw32_processTerminate (void)
     /*
      * ------------------------------------------------------
      * DOCPRIVATE
      *      This function performs process wide termination for
      *      the pthread library.
      *
      * PARAMETERS
      *      N/A
      *
      * DESCRIPTION
      *      This function performs process wide termination for
      *      the pthread library.
      *      This routine sets the global variable
      *      __ptw32_processInitialized to FALSE
      *
      * RESULTS
      *              N/A
      *
      * ------------------------------------------------------
      */
{
  if (__ptw32_processInitialized)
    {
      __ptw32_thread_t * tp, * tpNext;
      __ptw32_mcs_local_node_t node;

      if (__ptw32_selfThreadKey != NULL)
	{
	  /*
	   * Release __ptw32_selfThreadKey
	   */
	  pthread_key_delete (__ptw32_selfThreadKey);

	  __ptw32_selfThreadKey = NULL;
	}

      if (__ptw32_cleanupKey != NULL)
	{
	  /*
	   * Release __ptw32_cleanupKey
	   */
	  pthread_key_delete (__ptw32_cleanupKey);

	  __ptw32_cleanupKey = NULL;
	}

      __ptw32_mcs_lock_acquire(&__ptw32_thread_reuse_lock, &node);

      tp = __ptw32_threadReuseTop;
      while (tp !=  __PTW32_THREAD_REUSE_EMPTY)
	{
	  tpNext = tp->prevReuse;
	  free (tp);
	  tp = tpNext;
	}

      __ptw32_mcs_lock_release(&node);

      __ptw32_processInitialized =  __PTW32_FALSE;
    }

}				/* processTerminate */
