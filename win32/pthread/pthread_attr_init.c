/*
 * pthread_attr_init.c
 *
 * Description:
 * This translation unit implements operations on thread attribute objects.
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
pthread_attr_init (pthread_attr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Initializes a thread attributes object with default
      *      attributes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *
      * DESCRIPTION
      *      Initializes a thread attributes object with default
      *      attributes.
      *
      *      NOTES:
      *              1)      Used to define thread attributes
      *
      * RESULTS
      *              0               successfully initialized attr,
      *              ENOMEM          insufficient memory for attr.
      *
      * ------------------------------------------------------
      */
{
  pthread_attr_t attr_result;
  cpu_set_t cpuset;

  if (attr == NULL)
    {
      /* This is disallowed. */
      return EINVAL;
    }

  attr_result = (pthread_attr_t) malloc (sizeof (*attr_result));

  if (attr_result == NULL)
    {
      return ENOMEM;
    }

#if defined(_POSIX_THREAD_ATTR_STACKSIZE)
  /*
   * Default to zero size. Unless changed explicitly this
   * will allow Win32 to set the size to that of the
   * main thread.
   */
  attr_result->stacksize = 0;
#endif

#if defined(_POSIX_THREAD_ATTR_STACKADDR)
  /* FIXME: Set this to something sensible when we support it. */
  attr_result->stackaddr = NULL;
#endif

  attr_result->detachstate = PTHREAD_CREATE_JOINABLE;

#if defined(HAVE_SIGSET_T)
  memset (&(attr_result->sigmask), 0, sizeof (sigset_t));
#endif /* HAVE_SIGSET_T */

  /*
   * Win32 sets new threads to THREAD_PRIORITY_NORMAL and
   * not to that of the parent thread. We choose to default to
   * this arrangement.
   */
  attr_result->param.sched_priority = THREAD_PRIORITY_NORMAL;
  attr_result->inheritsched = PTHREAD_EXPLICIT_SCHED;
  attr_result->contentionscope = PTHREAD_SCOPE_SYSTEM;
  CPU_ZERO(&cpuset);
  attr_result->cpuset = ((_sched_cpu_set_vector_*)&cpuset)->_cpuset;
  attr_result->thrname = NULL;

  attr_result->valid =  __PTW32_ATTR_VALID;

  *attr = attr_result;

  return 0;
}
