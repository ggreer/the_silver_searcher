/*
 * pthread_once.c
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

int
pthread_once (pthread_once_t * once_control, void  (__PTW32_CDECL *init_routine) (void))
{
  if (once_control == NULL || init_routine == NULL)
    {
      return EINVAL;
    }
  
  if  ((__PTW32_INTERLOCKED_LONG)__PTW32_FALSE ==
       (__PTW32_INTERLOCKED_LONG)__PTW32_INTERLOCKED_EXCHANGE_ADD_LONG ((__PTW32_INTERLOCKED_LONGPTR)&once_control->done,
                                                                   (__PTW32_INTERLOCKED_LONG)0)) /* MBR fence */
    {
      __ptw32_mcs_local_node_t node;

      __ptw32_mcs_lock_acquire((__ptw32_mcs_lock_t *)&once_control->lock, &node);

      if (!once_control->done)
	{

#if defined (__PTW32_CONFIG_MSVC7)
#pragma inline_depth(0)
#endif

	  pthread_cleanup_push(__ptw32_mcs_lock_release, &node);
	  (*init_routine)();
	  pthread_cleanup_pop(0);

#if defined (__PTW32_CONFIG_MSVC7)
#pragma inline_depth()
#endif

	  once_control->done =  __PTW32_TRUE;
	}

      __ptw32_mcs_lock_release(&node);
    }

  return 0;

}				/* pthread_once */
