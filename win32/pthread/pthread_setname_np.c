/*
 * pthread_setname_np.c
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

#if defined(_MSC_VER)
#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType; // Must be 0x1000.
  LPCSTR szName; // Pointer to name (in user addr space).
  DWORD dwThreadID; // Thread ID (-1=caller thread).
  DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void
SetThreadName( DWORD dwThreadID, char* threadName)
{
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = threadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  __try
  {
    RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
  }
}
#endif

#if defined (__PTW32_COMPATIBILITY_BSD) || defined (__PTW32_COMPATIBILITY_TRU64)
int
pthread_setname_np(pthread_t thr, const char *name, void *arg)
{
  __ptw32_mcs_local_node_t threadLock;
  int len;
  int result;
  char tmpbuf[PTHREAD_MAX_NAMELEN_NP];
  char * newname;
  char * oldname;
  __ptw32_thread_t * tp;
#if defined(_MSC_VER)
  DWORD Win32ThreadID;
#endif

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

#if defined(_MSC_VER)
  Win32ThreadID = pthread_getw32threadid_np (thr);
  if (Win32ThreadID)
    {
      SetThreadName(Win32ThreadID, newname);
    }
#endif

  tp = (__ptw32_thread_t *) thr.p;

  __ptw32_mcs_lock_acquire (&tp->threadLock, &threadLock);

  oldname = tp->name;
  tp->name = newname;
  if (oldname)
    {
      free(oldname);
    }

  __ptw32_mcs_lock_release (&threadLock);

  return 0;
}
#else
int
pthread_setname_np(pthread_t thr, const char *name)
{
  __ptw32_mcs_local_node_t threadLock;
  int result;
  char * newname;
  char * oldname;
  __ptw32_thread_t * tp;
#if defined(_MSC_VER)
  DWORD Win32ThreadID;
#endif

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

  newname = _strdup(name);

#if defined(_MSC_VER)
  Win32ThreadID = pthread_getw32threadid_np (thr);

  if (Win32ThreadID)
    {
      SetThreadName(Win32ThreadID, newname);
    }
#endif

  tp = (__ptw32_thread_t *) thr.p;

  __ptw32_mcs_lock_acquire (&tp->threadLock, &threadLock);

  oldname = tp->name;
  tp->name = newname;
  if (oldname)
    {
      free(oldname);
    }

  __ptw32_mcs_lock_release (&threadLock);

  return 0;
}
#endif
