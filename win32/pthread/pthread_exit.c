/*
 * pthread_exit.c
 *
 * Description:
 * This translation unit implements routines associated with exiting from
 * a thread.
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
#if !defined(_UWIN)
/*#   include <process.h> */
#endif

void
pthread_exit (void *value_ptr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function terminates the calling thread, returning
      *      the value 'value_ptr' to any joining thread.
      *
      * PARAMETERS
      *      value_ptr
      *              a generic data value (i.e. not the address of a value)
      *
      *
      * DESCRIPTION
      *      This function terminates the calling thread, returning
      *      the value 'value_ptr' to any joining thread.
      *      NOTE: thread should be joinable.
      *
      * RESULTS
      *              N/A
      *
      * ------------------------------------------------------
      */
{
  __ptw32_thread_t * sp;

  /*
   * Don't use pthread_self() to avoid creating an implicit POSIX thread handle
   * unnecessarily.
   */
  sp = (__ptw32_thread_t *) pthread_getspecific (__ptw32_selfThreadKey);

#if defined(_UWIN)
  if (--pthread_count <= 0)
    exit ((int) value_ptr);
#endif

  if (NULL == sp)
    {
      /*
       * A POSIX thread handle was never created. I.e. this is a
       * Win32 thread that has never called a pthreads-win32 routine that
       * required a POSIX handle.
       *
       * Implicit POSIX handles are cleaned up in __ptw32_throw() now.
       */

#if ! defined (__MINGW32__) || defined (__MSVCRT__)  || defined (__DMC__)
      _endthreadex ((unsigned) (size_t) value_ptr);
#else
      _endthread ();
#endif

      /* Never reached */
    }

  sp->exitStatus = value_ptr;

  __ptw32_throw  (__PTW32_EPS_EXIT);

  /* Never reached. */

}
