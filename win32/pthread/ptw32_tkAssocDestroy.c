/*
 * ptw32_tkAssocDestroy.c
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


void
__ptw32_tkAssocDestroy (ThreadKeyAssoc * assoc)
     /*
      * -------------------------------------------------------------------
      * This routine releases all resources for the given ThreadKeyAssoc
      * once it is no longer being referenced
      * ie) either the key or thread has stopped referencing it.
      *
      * Parameters:
      *              assoc
      *                      an instance of ThreadKeyAssoc.
      * Returns:
      *      N/A
      * -------------------------------------------------------------------
      */
{

  /*
   * Both key->keyLock and thread->threadLock are locked before
   * entry to this routine.
   */
  if (assoc != NULL)
    {
      ThreadKeyAssoc * prev, * next;

      /* Remove assoc from thread's keys chain */
      prev = assoc->prevKey;
      next = assoc->nextKey;
      if (prev != NULL)
	{
	  prev->nextKey = next;
	}
      if (next != NULL)
	{
	  next->prevKey = prev;
	}

      if (assoc->thread->keys == assoc)
	{
	  /* We're at the head of the thread's keys chain */
	  assoc->thread->keys = next;
	}
      if (assoc->thread->nextAssoc == assoc)
	{
	  /*
	   * Thread is exiting and we're deleting the assoc to be processed next.
	   * Hand thread the assoc after this one.
	   */
	  assoc->thread->nextAssoc = next;
	}

      /* Remove assoc from key's threads chain */
      prev = assoc->prevThread;
      next = assoc->nextThread;
      if (prev != NULL)
	{
	  prev->nextThread = next;
	}
      if (next != NULL)
	{
	  next->prevThread = prev;
	}

      if (assoc->key->threads == assoc)
	{
	  /* We're at the head of the key's threads chain */
	  assoc->key->threads = next;
	}

      free (assoc);
    }

}				/* __ptw32_tkAssocDestroy */
