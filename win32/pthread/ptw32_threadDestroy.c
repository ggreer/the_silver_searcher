/*
 * ptw32_threadDestroy.c
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
__ptw32_threadDestroy (pthread_t thread)
{
  __ptw32_thread_t * tp = (__ptw32_thread_t *) thread.p;

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
      __ptw32_threadReusePush (thread);

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
}				/* __ptw32_threadDestroy */

