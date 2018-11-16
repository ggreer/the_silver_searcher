/*
 * -------------------------------------------------------------
 *
 * Module: sem_getvalue.c
 *
 * Purpose:
 *	Semaphores aren't actually part of PThreads.
 *	They are defined by the POSIX Standard:
 *
 *		POSIX 1003.1-2001
 *
 * -------------------------------------------------------------
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
#include "semaphore.h"
#include "implement.h"


int
sem_getvalue (sem_t * sem, int *sval)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function stores the current count value of the
 *      semaphore.
 * RESULTS
 *
 * Return value
 *
 *       0                  sval has been set.
 *      -1                  failed, error in errno
 *
 *  in global errno
 *
 *      EINVAL              'sem' is not a valid semaphore,
 *      ENOSYS              this function is not supported,
 *
 *
 * PARAMETERS
 *
 *      sem                 pointer to an instance of sem_t
 *
 *      sval                pointer to int.
 *
 * DESCRIPTION
 *      This function stores the current count value of the semaphore
 *      pointed to by sem in the int pointed to by sval.
 */
{
  int result = 0;

  __ptw32_mcs_local_node_t node;
  register sem_t s = *sem;

  __ptw32_mcs_lock_acquire(&s->lock, &node);
  *sval = s->value;
  __ptw32_mcs_lock_release(&node);

  if (result != 0)
    {
       __PTW32_SET_ERRNO(result);
      return -1;
    }

  return 0;
}				/* sem_getvalue */
