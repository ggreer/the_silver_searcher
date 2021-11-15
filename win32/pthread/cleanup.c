/*
 * cleanup.c
 *
 * Description:
 * This translation unit implements routines associated
 * with cleaning up threads.
 *
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
 *
 * --------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"


/*
 * The functions __ptw32_pop_cleanup and __ptw32_push_cleanup
 * are implemented here for applications written in C with no
 * SEH or C++ destructor support. 
 */

__ptw32_cleanup_t *
__ptw32_pop_cleanup (int execute)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function pops the most recently pushed cleanup
      *      handler. If execute is nonzero, then the cleanup handler
      *      is executed if non-null.
      *
      * PARAMETERS
      *      execute
      *              if nonzero, execute the cleanup handler
      *
      *
      * DESCRIPTION
      *      This function pops the most recently pushed cleanup
      *      handler. If execute is nonzero, then the cleanup handler
      *      is executed if non-null.
      *      NOTE: specify 'execute' as nonzero to avoid duplication
      *                of common cleanup code.
      *
      * RESULTS
      *              N/A
      *
      * ------------------------------------------------------
      */
{
  __ptw32_cleanup_t *cleanup;

  cleanup = (__ptw32_cleanup_t *) pthread_getspecific (__ptw32_cleanupKey);

  if (cleanup != NULL)
    {
      if (execute && (cleanup->routine != NULL))
	{

	  (*cleanup->routine) (cleanup->arg);

	}

      pthread_setspecific (__ptw32_cleanupKey, (void *) cleanup->prev);

    }

  return (cleanup);

}				/* __ptw32_pop_cleanup */


void
__ptw32_push_cleanup (__ptw32_cleanup_t * cleanup,
		    __ptw32_cleanup_callback_t routine, void *arg)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function pushes a new cleanup handler onto the thread's stack
      *      of cleanup handlers. Each cleanup handler pushed onto the stack is
      *      popped and invoked with the argument 'arg' when
      *              a) the thread exits by calling 'pthread_exit',
      *              b) when the thread acts on a cancellation request,
      *              c) or when the thread calls pthread_cleanup_pop with a nonzero
      *                 'execute' argument
      *
      * PARAMETERS
      *      cleanup
      *              a pointer to an instance of pthread_cleanup_t,
      *
      *      routine
      *              pointer to a cleanup handler,
      *
      *      arg
      *              parameter to be passed to the cleanup handler
      *
      *
      * DESCRIPTION
      *      This function pushes a new cleanup handler onto the thread's stack
      *      of cleanup handlers. Each cleanup handler pushed onto the stack is
      *      popped and invoked with the argument 'arg' when
      *              a) the thread exits by calling 'pthread_exit',
      *              b) when the thread acts on a cancellation request,
      *              c) or when the thrad calls pthread_cleanup_pop with a nonzero
      *                 'execute' argument
      *      NOTE: pthread_push_cleanup, __ptw32_pop_cleanup must be paired
      *                in the same lexical scope.
      *
      * RESULTS
      *              pthread_cleanup_t *
      *                              pointer to the previous cleanup
      *
      * ------------------------------------------------------
      */
{
  cleanup->routine = routine;
  cleanup->arg = arg;

  cleanup->prev = (__ptw32_cleanup_t *) pthread_getspecific (__ptw32_cleanupKey);

  pthread_setspecific (__ptw32_cleanupKey, (void *) cleanup);

}				/* __ptw32_push_cleanup */
