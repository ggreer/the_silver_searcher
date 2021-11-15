/*
 * pthread_attr_setschedparam.c
 * 
 * Description:
 * POSIX thread functions that deal with thread scheduling.
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
#include "sched.h"

int
pthread_attr_setschedparam (pthread_attr_t * attr,
			    const struct sched_param *param)
{
  int priority;

  if (__ptw32_is_attr (attr) != 0 || param == NULL)
    {
      return EINVAL;
    }

  priority = param->sched_priority;

  /* Validate priority level. */
  if (priority < sched_get_priority_min (SCHED_OTHER) ||
      priority > sched_get_priority_max (SCHED_OTHER))
    {
      return EINVAL;
    }

  memcpy (&(*attr)->param, param, sizeof (*param));
  return 0;
}
