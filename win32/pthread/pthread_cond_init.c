/*
 * pthread_cond_init.c
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
pthread_cond_init (pthread_cond_t * cond, const pthread_condattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function initializes a condition variable.
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *      attr
      *              specifies optional creation attributes.
      *
      *
      * DESCRIPTION
      *      This function initializes a condition variable.
      *
      * RESULTS
      *              0               successfully created condition variable,
      *              EINVAL          'attr' is invalid,
      *              EAGAIN          insufficient resources (other than
      *                              memory,
      *              ENOMEM          insufficient memory,
      *              EBUSY           'cond' is already initialized,
      *
      * ------------------------------------------------------
      */
{
  int result;
  pthread_cond_t cv = NULL;

  if (cond == NULL)
    {
      return EINVAL;
    }

  if ((attr != NULL && *attr != NULL) &&
      ((*attr)->pshared == PTHREAD_PROCESS_SHARED))
    {
      /*
       * Creating condition variable that can be shared between
       * processes.
       */
      result = ENOSYS;
      goto DONE;
    }

  cv = (pthread_cond_t) calloc (1, sizeof (*cv));

  if (cv == NULL)
    {
      result = ENOMEM;
      goto DONE;
    }

  cv->nWaitersBlocked = 0;
  cv->nWaitersToUnblock = 0;
  cv->nWaitersGone = 0;

  if (sem_init (&(cv->semBlockLock), 0, 1) != 0)
    {
      result =  __PTW32_GET_ERRNO();
      goto FAIL0;
    }

  if (sem_init (&(cv->semBlockQueue), 0, 0) != 0)
    {
      result =  __PTW32_GET_ERRNO();
      goto FAIL1;
    }

  if ((result = pthread_mutex_init (&(cv->mtxUnblockLock), 0)) != 0)
    {
      goto FAIL2;
    }

  result = 0;

  goto DONE;

  /*
   * -------------
   * Failed...
   * -------------
   */
FAIL2:
  (void) sem_destroy (&(cv->semBlockQueue));

FAIL1:
  (void) sem_destroy (&(cv->semBlockLock));

FAIL0:
  (void) free (cv);
  cv = NULL;

DONE:
  if (0 == result)
    {
      __ptw32_mcs_local_node_t node;

      __ptw32_mcs_lock_acquire(&__ptw32_cond_list_lock, &node);

      cv->next = NULL;
      cv->prev = __ptw32_cond_list_tail;

      if (__ptw32_cond_list_tail != NULL)
	{
	  __ptw32_cond_list_tail->next = cv;
	}

      __ptw32_cond_list_tail = cv;

      if (__ptw32_cond_list_head == NULL)
	{
	  __ptw32_cond_list_head = cv;
	}

      __ptw32_mcs_lock_release(&node);
    }

  *cond = cv;

  return result;

}				/* pthread_cond_init */
