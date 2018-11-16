/*
 * pthread_attr_setname_np.c
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

#include <stdio.h>
#include <string.h>
#include "pthread.h"
#include "implement.h"

#if defined (__PTW32_COMPATIBILITY_BSD) || defined (__PTW32_COMPATIBILITY_TRU64)
int
pthread_attr_setname_np(pthread_attr_t * attr, const char *name, void *arg)
{
  int len;
  int result;
  char tmpbuf[PTHREAD_MAX_NAMELEN_NP];
  char * newname;
  char * oldname;

  /*
   * According to the MSDN description for snprintf()
   * where count is the second parameter:
   * If len < count, then len characters are stored in buffer, a null-terminator is appended, and len is returned.
   * If len = count, then len characters are stored in buffer, no null-terminator is appended, and len is returned.
   * If len > count, then count characters are stored in buffer, no null-terminator is appended, and a negative value is returned.
   *
   * This is different to the POSIX behaviour which returns the number of characters that would have been written in all cases.
   */
  len = snprintf(tmpbuf, PTHREAD_MAX_NAMELEN_NP-1, name, arg);
  tmpbuf[PTHREAD_MAX_NAMELEN_NP-1] = '\0';
  if (len < 0)
    {
      return EINVAL;
    }

  newname = _strdup(tmpbuf);

  oldname = (*attr)->thrname;
  (*attr)->thrname = newname;
  if (oldname)
    {
      free(oldname);
    }

  return 0;
}
#else
int
pthread_attr_setname_np(pthread_attr_t * attr, const char *name)
{
  char * newname;
  char * oldname;

  newname = _strdup(name);

  oldname = (*attr)->thrname;
  (*attr)->thrname = newname;
  if (oldname)
    {
      free(oldname);
    }

  return 0;
}
#endif
