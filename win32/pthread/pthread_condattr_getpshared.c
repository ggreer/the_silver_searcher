/*
 * pthread_condattr_getpshared.c
 *
 * Description:
 * This translation unit implements condition variables and their primitives.
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
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"


int
pthread_condattr_getpshared (const pthread_condattr_t * attr, int *pshared)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Determine whether condition variables created with 'attr'
      *      can be shared between processes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_condattr_t
      *
      *      pshared
      *              will be set to one of:
      *
      *                      PTHREAD_PROCESS_SHARED
      *                              May be shared if in shared memory
      *
      *                      PTHREAD_PROCESS_PRIVATE
      *                              Cannot be shared.
      *
      *
      * DESCRIPTION
      *      Condition Variables created with 'attr' can be shared
      *      between processes if pthread_cond_t variable is allocated
      *      in memory shared by these processes.
      *      NOTES:
      *      1)      pshared condition variables MUST be allocated in
      *              shared memory.
      *
      *      2)      The following macro is defined if shared mutexes
      *              are supported:
      *                      _POSIX_THREAD_PROCESS_SHARED
      *
      * RESULTS
      *              0               successfully retrieved attribute,
      *              EINVAL          'attr' or 'pshared' is invalid,
      *
      * ------------------------------------------------------
      */
{
  int result;

  if ((attr != NULL && *attr != NULL) && (pshared != NULL))
    {
      *pshared = (*attr)->pshared;
      result = 0;
    }
  else
    {
      result = EINVAL;
    }

  return result;

}				/* pthread_condattr_getpshared */
