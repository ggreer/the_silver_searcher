/*
 * pthread_condattr_init.c
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
pthread_condattr_init (pthread_condattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Initializes a condition variable attributes object
      *      with default attributes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_condattr_t
      *
      *
      * DESCRIPTION
      *      Initializes a condition variable attributes object
      *      with default attributes.
      *
      *      NOTES:
      *              1)      Use to define condition variable types
      *              2)      It is up to the application to ensure
      *                      that it doesn't re-init an attribute
      *                      without destroying it first. Otherwise
      *                      a memory leak is created.
      *
      * RESULTS
      *              0               successfully initialized attr,
      *              ENOMEM          insufficient memory for attr.
      *
      * ------------------------------------------------------
      */
{
  pthread_condattr_t attr_result;
  int result = 0;

  attr_result = (pthread_condattr_t) calloc (1, sizeof (*attr_result));

  if (attr_result == NULL)
    {
      result = ENOMEM;
    }

  *attr = attr_result;

  return result;

}				/* pthread_condattr_init */
