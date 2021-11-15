/*
 * pthread_getname_np.c
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
pthread_getname_np(pthread_t thr, char *name, int len)
{
  __ptw32_mcs_local_node_t threadLock;
  __ptw32_thread_t * tp;
  char * s, * d;
  int result;

  /*
   * Validate the thread id. This method works for pthreads-win32 because
   * pthread_kill and pthread_t are designed to accommodate it, but the
   * method is not portable.
   */
  result = pthread_kill (thr, 0);
  if (0 != result)
    {
      return result;
    }

  tp = (__ptw32_thread_t *) thr.p;

  __ptw32_mcs_lock_acquire (&tp->threadLock, &threadLock);

  for (s = tp->name, d = name; *s && d < &name[len - 1]; *d++ = *s++)
    {}

  *d = '\0';
  __ptw32_mcs_lock_release (&threadLock);

  return result;
}
