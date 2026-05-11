/*
 * pthread_kill.c
 *
 * Description:
 * This translation unit implements the pthread_kill routine.
 *
 * --------------------------------------------------------------------------
 *
 *      pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999-2021 pthreads-win32 / pthreads4w contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * --------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "pthread.h"
#include "implement.h"

/*
 * Not needed yet, but defining it should indicate clashes with build target
 * environment that should be fixed.
 */
#if !defined(WINCE)
#  include <signal.h>
#endif

int
pthread_kill (pthread_t thread, int sig)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function requests that a signal be delivered to the
      *      specified thread. If sig is zero, error checking is
      *      performed but no signal is actually sent such that this
      *      function can be used to check for a valid thread ID.
      *
      * PARAMETERS
      *      thread  reference to an instances of pthread_t
      *      sig     signal. Currently only a value of 0 is supported.
      *
      *
      * DESCRIPTION
      *      This function requests that a signal be delivered to the
      *      specified thread. If sig is zero, error checking is
      *      performed but no signal is actually sent such that this
      *      function can be used to check for a valid thread ID.
      *
      * RESULTS
      *              ESRCH           the thread is not a valid thread ID,
      *              EINVAL          the value of the signal is invalid
      *                              or unsupported.
      *              0               the signal was successfully sent.
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  ptw32_thread_t * tp;
  ptw32_mcs_local_node_t node;

  ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);

  tp = (ptw32_thread_t *) thread.p;

  if (NULL == tp
      || thread.x != tp->ptHandle.x
      || NULL == tp->threadH)
    {
      result = ESRCH;
    }

  ptw32_mcs_lock_release(&node);

  if (0 == result && 0 != sig)
    {
      /*
       * Currently only supports direct thread termination via SIGABRT.
       */
      switch (sig)
      {
      default:
          result = EINVAL;
          break;
#ifdef SIGINT
      case SIGINT:
#endif      
#ifdef SIGTERM
      case SIGTERM:
#endif      
#ifdef SIGBREAK
      case SIGBREAK:
#endif            
#ifdef SIGABRT
      case SIGABRT:
#endif
#ifdef SIGABRT_COMPAT
      case SIGABRT_COMPAT:
#endif
      {
          ptw32_mcs_local_node_t stateLock;

          /*
           * Lock for async-cancel safety.
           */
          ptw32_mcs_lock_acquire(&tp->stateLock, &stateLock);

          // Only terminate the thread when it is still running:
          if (tp->state < PThreadStateLast)
          {
              tp->state = PThreadStateLast;
              tp->cancelState = PTHREAD_CANCEL_DISABLE;
              ptw32_mcs_lock_release(&stateLock);

              result = TerminateThread(tp->threadH, (DWORD)(ptrdiff_t)PTHREAD_CANCELED);
              result = (result != 0) ? 0 : EINVAL;

              // Set exit to CANCELED (KILLED) when it hasn't been set already.
              if (tp->exitStatus == NULL)
                  tp->exitStatus = PTHREAD_CANCELED;
          }
          else
          {
              ptw32_mcs_lock_release(&stateLock);

              // TODO: ? flag the call as not-necessary-any-more because thread has already terminated ?
              result = 0;
          }
      }
          break;
      }
    }

  return result;

}				/* pthread_kill */
