/*
 * pthread_mutexattr_getrobust.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
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
pthread_mutexattr_getrobust (const pthread_mutexattr_t * attr, int * robust)
     /*
      * ------------------------------------------------------
      *
      * DOCPUBLIC
      * The pthread_mutexattr_setrobust() and
      * pthread_mutexattr_getrobust() functions  respectively set and
      * get the mutex robust  attribute. This attribute is set in  the
      * robust parameter to these functions.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_mutexattr_t
      *
      *     robust 
      *              must be one of:
      *
      *                      PTHREAD_MUTEX_STALLED
      *
      *                      PTHREAD_MUTEX_ROBUST
      *
      * DESCRIPTION
      * The pthread_mutexattr_setrobust() and
      * pthread_mutexattr_getrobust() functions  respectively set and
      * get the mutex robust  attribute. This attribute is set in  the
      * robust  parameter to these functions. The default value of the
      * robust  attribute is  PTHREAD_MUTEX_STALLED.
      * 
      * The robustness of mutex is contained in the robustness attribute
      * of the mutex attributes. Valid mutex robustness values are:
      *
      * PTHREAD_MUTEX_STALLED
      * No special actions are taken if the owner of the mutex is
      * terminated while holding the mutex lock. This can lead to
      * deadlocks if no other thread can unlock the mutex.
      * This is the default value.
      * 
      * PTHREAD_MUTEX_ROBUST
      * If the process containing the owning thread of a robust mutex
      * terminates while holding the mutex lock, the next thread that
      * acquires the mutex shall be notified about the termination by
      * the return value [EOWNERDEAD] from the locking function. If the
      * owning thread of a robust mutex terminates while holding the mutex
      * lock, the next thread that acquires the mutex may be notified
      * about the termination by the return value [EOWNERDEAD]. The
      * notified thread can then attempt to mark the state protected by
      * the mutex as consistent again by a call to
      * pthread_mutex_consistent(). After a subsequent successful call to
      * pthread_mutex_unlock(), the mutex lock shall be released and can
      * be used normally by other threads. If the mutex is unlocked without
      * a call to pthread_mutex_consistent(), it shall be in a permanently
      * unusable state and all attempts to lock the mutex shall fail with
      * the error [ENOTRECOVERABLE]. The only permissible operation on such
      * a mutex is pthread_mutex_destroy().
      *
      * RESULTS
      *              0               successfully set attribute,
      *              EINVAL          'attr' or 'robust' is invalid,
      *
      * ------------------------------------------------------
      */
{
  int result = EINVAL;

  if ((attr != NULL && *attr != NULL && robust != NULL))
    {
      *robust = (*attr)->robustness;
      result = 0;
    }

  return (result);
}				/* pthread_mutexattr_getrobust */
