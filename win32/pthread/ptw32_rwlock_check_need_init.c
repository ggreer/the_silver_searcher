/*
 * pthread_rwlock_check_need_init.c
 *
 * Description:
 * This translation unit implements read/write lock primitives.
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

INLINE int
__ptw32_rwlock_check_need_init (pthread_rwlock_t * rwlock)
{
  int result = 0;
  __ptw32_mcs_local_node_t node;

  /*
   * The following guarded test is specifically for statically
   * initialised rwlocks (via PTHREAD_RWLOCK_INITIALIZER).
   */
  __ptw32_mcs_lock_acquire(&__ptw32_rwlock_test_init_lock, &node);

  /*
   * We got here possibly under race
   * conditions. Check again inside the critical section
   * and only initialise if the rwlock is valid (not been destroyed).
   * If a static rwlock has been destroyed, the application can
   * re-initialise it only by calling pthread_rwlock_init()
   * explicitly.
   */
  if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
    {
      result = pthread_rwlock_init (rwlock, NULL);
    }
  else if (*rwlock == NULL)
    {
      /*
       * The rwlock has been destroyed while we were waiting to
       * initialise it, so the operation that caused the
       * auto-initialisation should fail.
       */
      result = EINVAL;
    }

  __ptw32_mcs_lock_release(&node);

  return result;
}
