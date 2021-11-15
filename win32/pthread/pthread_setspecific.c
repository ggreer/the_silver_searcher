/*
 * pthread_setspecific.c
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
pthread_setspecific (pthread_key_t key, const void *value)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function sets the value of the thread specific
      *      key in the calling thread.
      *
      * PARAMETERS
      *      key
      *              an instance of pthread_key_t
      *      value
      *              the value to set key to
      *
      *
      * DESCRIPTION
      *      This function sets the value of the thread specific
      *      key in the calling thread.
      *
      * RESULTS
      *              0               successfully set value
      *              EAGAIN          could not set value
      *              ENOENT          SERIOUS!!
      *
      * ------------------------------------------------------
      */
{
  pthread_t self;
  int result = 0;

  if (key != __ptw32_selfThreadKey)
    {
      /*
       * Using pthread_self will implicitly create
       * an instance of pthread_t for the current
       * thread if one wasn't explicitly created
       */
      self = pthread_self ();
      if (self.p == NULL)
	{
	  return ENOENT;
	}
    }
  else
    {
      /*
       * Resolve catch-22 of registering thread with selfThread
       * key
       */
      __ptw32_thread_t * sp = (__ptw32_thread_t *) pthread_getspecific (__ptw32_selfThreadKey);

      if (sp == NULL)
        {
	  if (value == NULL)
	    {
	      return ENOENT;
	    }
          self = *((pthread_t *) value);
        }
      else
        {
	  self = sp->ptHandle;
        }
    }

  result = 0;

  if (key != NULL)
    {
      if (self.p != NULL && key->destructor != NULL && value != NULL)
	{
          __ptw32_mcs_local_node_t keyLock;
          __ptw32_mcs_local_node_t threadLock;
	  __ptw32_thread_t * sp = (__ptw32_thread_t *) self.p;
	  /*
	   * Only require associations if we have to
	   * call user destroy routine.
	   * Don't need to locate an existing association
	   * when setting data to NULL for WIN32 since the
	   * data is stored with the operating system; not
	   * on the association; setting assoc to NULL short
	   * circuits the search.
	   */
	  ThreadKeyAssoc *assoc;

	  __ptw32_mcs_lock_acquire(&(key->keyLock), &keyLock);
	  __ptw32_mcs_lock_acquire(&(sp->threadLock), &threadLock);

	  assoc = (ThreadKeyAssoc *) sp->keys;
	  /*
	   * Locate existing association
	   */
	  while (assoc != NULL)
	    {
	      if (assoc->key == key)
		{
		  /*
		   * Association already exists
		   */
		  break;
		}
		assoc = assoc->nextKey;
	    }

	  /*
	   * create an association if not found
	   */
	  if (assoc == NULL)
	    {
	      result = __ptw32_tkAssocCreate (sp, key);
	    }

	  __ptw32_mcs_lock_release(&threadLock);
	  __ptw32_mcs_lock_release(&keyLock);
	}

      if (result == 0)
	{
	  if (!TlsSetValue (key->key, (LPVOID) value))
	    {
	      result = EAGAIN;
	    }
	}
    }

  return (result);
}				/* pthread_setspecific */
