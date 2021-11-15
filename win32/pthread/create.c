/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
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
#if ! defined(_UWIN) && ! defined(WINCE)
#include <process.h>
#endif

int
pthread_create (pthread_t * tid,
    const pthread_attr_t * attr,
    void * (__PTW32_CDECL *start) (void *), void *arg)
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *      This function creates a thread running the start function,
 *      passing it the parameter value, 'arg'. The 'attr'
 *      argument specifies optional creation attributes.
 *      The identity of the new thread is returned
 *      via 'tid', which should not be NULL.
 *
 * PARAMETERS
 *      tid
 *              pointer to an instance of pthread_t
 *
 *      attr
 *              optional pointer to an instance of pthread_attr_t
 *
 *      start
 *              pointer to the starting routine for the new thread
 *
 *      arg
 *              optional parameter passed to 'start'
 *
 *
 * DESCRIPTION
 *      This function creates a thread running the start function,
 *      passing it the parameter value, 'arg'. The 'attr'
 *      argument specifies optional creation attributes.
 *      The identity of the new thread is returned
 *      via 'tid', which should not be the NULL pointer.
 *
 * RESULTS
 *              0               successfully created thread,
 *              EINVAL          attr invalid,
 *              EAGAIN          insufficient resources.
 *
 * ------------------------------------------------------
 */
{
  pthread_t thread;
  __ptw32_thread_t * tp;
  __ptw32_thread_t * sp;
  register pthread_attr_t a;
  HANDLE threadH = 0;
  int result = EAGAIN;
  int run =  __PTW32_TRUE;
  ThreadParms *parms = NULL;
  unsigned int stackSize;
  int priority;

  /*
   * Before doing anything, check that tid can be stored through
   * without invoking a memory protection error (segfault).
   * Make sure that the assignment below can't be optimised out by the compiler.
   * This is assured by conditionally assigning *tid again at the end.
   */
  tid->x = 0;

  if (NULL == (sp = (__ptw32_thread_t *)pthread_self().p))
    {
      goto FAIL0;
    }

  if (attr != NULL)
    {
      a = *attr;
    }
  else
    {
      a = NULL;
    }

  thread = __ptw32_new();
  if (thread.p == NULL)
    {
      goto FAIL0;
    }

  tp = (__ptw32_thread_t *) thread.p;

  priority = tp->sched_priority;

  if ((parms = (ThreadParms *) malloc (sizeof (*parms))) == NULL)
    {
      goto FAIL0;
    }

  parms->tid = thread;
  parms->start = start;
  parms->arg = arg;

  /*
   * Threads inherit their initial sigmask and CPU affinity from their creator thread.
   */
#if defined(HAVE_SIGSET_T)
  tp->sigmask = sp->sigmask;
#endif
#if defined(HAVE_CPU_AFFINITY)
  tp->cpuset = sp->cpuset;
#endif

  if (a != NULL)
    {
#if defined(HAVE_CPU_AFFINITY)
      cpu_set_t none;
      cpu_set_t attr_cpuset;
      ((_sched_cpu_set_vector_*)&attr_cpuset)->_cpuset = a->cpuset;

      CPU_ZERO(&none);
      if (! CPU_EQUAL(&attr_cpuset, &none))
        {
          tp->cpuset = a->cpuset;
        }
#endif
      stackSize = (unsigned int)a->stacksize;
      tp->detachState = a->detachstate;
      priority = a->param.sched_priority;
      if (a->thrname != NULL)
        tp->name = _strdup(a->thrname);

#if (THREAD_PRIORITY_LOWEST > THREAD_PRIORITY_NORMAL)
      /* WinCE */
#else
      /* Everything else */

      /*
       * Thread priority must be set to a valid system level
       * without altering the value set by pthread_attr_setschedparam().
       */

      /*
       * PTHREAD_EXPLICIT_SCHED is the default because Win32 threads
       * don't inherit their creator's priority. They are started with
       * THREAD_PRIORITY_NORMAL (win32 value). The result of not supplying
       * an 'attr' arg to pthread_create() is equivalent to defaulting to
       * PTHREAD_EXPLICIT_SCHED and priority THREAD_PRIORITY_NORMAL.
       */
      if (PTHREAD_INHERIT_SCHED == a->inheritsched)
        {
          /*
           * If the thread that called pthread_create() is a Win32 thread
           * then the inherited priority could be the result of a temporary
           * system adjustment. This is not the case for POSIX threads.
           */
          priority = sp->sched_priority;
        }

#endif

    }
  else
    {
      /*
       * Default stackSize
       */
      stackSize = PTHREAD_STACK_MIN;
    }

  /*
   * State must be >= PThreadStateRunning before we return to the caller.
   * __ptw32_threadStart will set state to PThreadStateRunning.
   */
  tp->state = PThreadStateSuspended;

  tp->keys = NULL;

  /*
   * Threads must be started in suspended mode and resumed if necessary
   * after _beginthreadex returns us the handle. Otherwise we set up a
   * race condition between the creating and the created threads.
   * Note that we also retain a local copy of the handle for use
   * by us in case thread.p->threadH gets NULLed later but before we've
   * finished with it here.
   */

#if ! defined (__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)

  tp->threadH =
      threadH =
          (HANDLE) _beginthreadex ((void *) NULL,	/* No security info             */
              stackSize,		/* default stack size   */
              __ptw32_threadStart,
              parms,
              (unsigned)
              CREATE_SUSPENDED,
              (unsigned *) &(tp->thread));

  if (threadH != 0)
    {
      if (a != NULL)
        {
          (void) __ptw32_setthreadpriority (thread, SCHED_OTHER, priority);
        }

#if defined(HAVE_CPU_AFFINITY)

      SetThreadAffinityMask(tp->threadH, tp->cpuset);

#endif

      if (run)
        {
          ResumeThread (threadH);
        }
    }

#else

  {
    __ptw32_mcs_local_node_t stateLock;

    /*
     * This lock will force pthread_threadStart() to wait until we have
     * the thread handle and have set the priority.
     */
    __ptw32_mcs_lock_acquire(&tp->stateLock, &stateLock);

    tp->threadH =
        threadH =
            (HANDLE) _beginthread (__ptw32_threadStart, stackSize,	/* default stack size   */
                parms);

    /*
     * Make the return code match _beginthreadex's.
     */
    if (threadH == (HANDLE) - 1L)
      {
        tp->threadH = threadH = 0;
      }
    else
      {
        if (!run)
          {
            /*
             * beginthread does not allow for create flags, so we do it now.
             * Note that beginthread itself creates the thread in SUSPENDED
             * mode, and then calls ResumeThread to start it.
             */
            SuspendThread (threadH);
          }

        if (a != NULL)
          {
            (void) __ptw32_setthreadpriority (thread, SCHED_OTHER, priority);
          }

#if defined(HAVE_CPU_AFFINITY)

        SetThreadAffinityMask(tp->threadH, tp->cpuset);

#endif

      }

    __ptw32_mcs_lock_release (&stateLock);
  }
#endif

  result = (threadH != 0) ? 0 : EAGAIN;

  /*
   * Fall Through Intentionally
   */

  /*
   * ------------
   * Failure Code
   * ------------
   */

  FAIL0:
  if (result != 0)
    {

      __ptw32_threadDestroy (thread);
      tp = NULL;

      if (parms != NULL)
        {
          free (parms);
        }
    }
  else
    {
      *tid = thread;
    }

#if defined(_UWIN)
  if (result == 0)
    pthread_count++;
#endif
  return (result);
}				/* pthread_create */
