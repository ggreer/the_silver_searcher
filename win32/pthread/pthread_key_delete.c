/*
 * pthread_key_delete.c
 *
 * Description:
 * POSIX thread functions which implement thread-specific data (TSD).
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
pthread_key_delete (pthread_key_t key)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function deletes a thread-specific data key. This
 *      does not change the value of the thread specific data key
 *      for any thread and does not run the key's destructor
 *      in any thread so it should be used with caution.
 *
 * PARAMETERS
 *      key
 *              pointer to an instance of pthread_key_t
 *
 *
 * DESCRIPTION
 *      This function deletes a thread-specific data key. This
 *      does not change the value of the thread specific data key
 *      for any thread and does not run the key's destructor
 *      in any thread so it should be used with caution.
 *
 * RESULTS
 *              0               successfully deleted the key,
 *              EINVAL          key is invalid,
 *
 * ------------------------------------------------------
 */
{
  __ptw32_mcs_local_node_t keyLock;
  int result = 0;

  if (key != NULL)
    {
      if (key->threads != NULL && key->destructor != NULL)
        {
          ThreadKeyAssoc *assoc;
          __ptw32_mcs_lock_acquire (&(key->keyLock), &keyLock);
          /*
           * Run through all Thread<-->Key associations
           * for this key.
           *
           * While we hold at least one of the locks guarding
           * the assoc, we know that the assoc pointed to by
           * key->threads is valid.
           */
          while ((assoc = (ThreadKeyAssoc *) key->threads) != NULL)
            {
              __ptw32_mcs_local_node_t threadLock;
              __ptw32_thread_t * thread = assoc->thread;

              if (assoc == NULL)
                {
                  /* Finished */
                  break;
                }

              __ptw32_mcs_lock_acquire (&(thread->threadLock), &threadLock);
              /*
               * Since we are starting at the head of the key's threads
               * chain, this will also point key->threads at the next assoc.
               * While we hold key->keyLock, no other thread can insert
               * a new assoc for this key via pthread_setspecific.
               */
              __ptw32_tkAssocDestroy (assoc);
              __ptw32_mcs_lock_release (&threadLock);
            }
          __ptw32_mcs_lock_release (&keyLock);
        }

      TlsFree (key->key);
      if (key->destructor != NULL)
        {
          /* A thread could be holding the keyLock */
          __ptw32_mcs_lock_acquire (&(key->keyLock), &keyLock);
          __ptw32_mcs_lock_release (&keyLock);
        }

#if defined( _DEBUG )
      memset ((char *) key, 0, sizeof (*key));
#endif
      free (key);
    }

  return (result);
}
