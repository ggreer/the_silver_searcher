/*
 * ptw32_tkAssocCreate.c
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


int
__ptw32_tkAssocCreate (__ptw32_thread_t * sp, pthread_key_t key)
     /*
      * -------------------------------------------------------------------
      * This routine creates an association that
      * is unique for the given (thread,key) combination.The association 
      * is referenced by both the thread and the key.
      * This association allows us to determine what keys the
      * current thread references and what threads a given key
      * references.
      * See the detailed description
      * at the beginning of this file for further details.
      *
      * Notes:
      *      1)      New associations are pushed to the beginning of the
      *              chain so that the internal __ptw32_selfThreadKey association
      *              is always last, thus allowing selfThreadExit to
      *              be implicitly called last by pthread_exit.
      *      2)      
      *
      * Parameters:
      *              thread
      *                      current running thread.
      *              key
      *                      key on which to create an association.
      * Returns:
      *       0              - if successful,
      *       ENOMEM         - not enough memory to create assoc or other object
      *       EINVAL         - an internal error occurred
      *       ENOSYS         - an internal error occurred
      * -------------------------------------------------------------------
      */
{
  ThreadKeyAssoc *assoc;

  /*
   * Have to create an association and add it
   * to both the key and the thread.
   *
   * Both key->keyLock and thread->threadLock are locked before
   * entry to this routine.
   */
  assoc = (ThreadKeyAssoc *) calloc (1, sizeof (*assoc));

  if (assoc == NULL)
    {
      return ENOMEM;
    }

  assoc->thread = sp;
  assoc->key = key;

  /*
   * Register assoc with key
   */
  assoc->prevThread = NULL;
  assoc->nextThread = (ThreadKeyAssoc *) key->threads;
  if (assoc->nextThread != NULL)
    {
      assoc->nextThread->prevThread = assoc;
    }
  key->threads = (void *) assoc;

  /*
   * Register assoc with thread
   */
  assoc->prevKey = NULL;
  assoc->nextKey = (ThreadKeyAssoc *) sp->keys;
  if (assoc->nextKey != NULL)
    {
      assoc->nextKey->prevKey = assoc;
    }
  sp->keys = (void *) assoc;

  return (0);

}				/* __ptw32_tkAssocCreate */
