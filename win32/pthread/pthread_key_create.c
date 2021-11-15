/*
 * pthread_key_create.c
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


/* TLS_OUT_OF_INDEXES not defined on WinCE */
#if !defined(TLS_OUT_OF_INDEXES)
#define TLS_OUT_OF_INDEXES 0xffffffff
#endif

int
pthread_key_create (pthread_key_t * key, void  (__PTW32_CDECL *destructor) (void *))
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function creates a thread-specific data key visible
      *      to all threads. All existing and new threads have a value
      *      NULL for key until set using pthread_setspecific. When any
      *      thread with a non-NULL value for key terminates, 'destructor'
      *      is called with key's current value for that thread.
      *
      * PARAMETERS
      *      key
      *              pointer to an instance of pthread_key_t
      *
      *
      * DESCRIPTION
      *      This function creates a thread-specific data key visible
      *      to all threads. All existing and new threads have a value
      *      NULL for key until set using pthread_setspecific. When any
      *      thread with a non-NULL value for key terminates, 'destructor'
      *      is called with key's current value for that thread.
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              EAGAIN          insufficient resources or PTHREAD_KEYS_MAX
      *                              exceeded,
      *              ENOMEM          insufficient memory to create the key,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_key_t newkey;

  if ((newkey = (pthread_key_t) calloc (1, sizeof (*newkey))) == NULL)
    {
      result = ENOMEM;
    }
  else if ((newkey->key = TlsAlloc ()) == TLS_OUT_OF_INDEXES)
    {
      result = EAGAIN;

      free (newkey);
      newkey = NULL;
    }
  else if (destructor != NULL)
    {
      /*
       * Have to manage associations between thread and key;
       * Therefore, need a lock that allows competing threads
       * to gain exclusive access to the key->threads list.
       *
       * The mutex will only be created when it is first locked.
       */
      newkey->keyLock = 0;
      newkey->destructor = destructor;
    }

  *key = newkey;

  return (result);
}
