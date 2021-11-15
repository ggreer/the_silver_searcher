/*
 * signal.c
 *
 * Description:
 * Thread-aware signal functions.
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

/*
 * Possible future strategy for implementing pthread_kill()
 * ========================================================
 *
 * Win32 does not implement signals.
 * Signals are simply software interrupts.
 * pthread_kill() asks the system to deliver a specified
 * signal (interrupt) to a specified thread in the same
 * process.
 * Signals are always asynchronous (no deferred signals).
 * Pthread-win32 has an async cancellation mechanism.
 * A similar system can be written to deliver signals
 * within the same process (on ix86 processors at least).
 *
 * Each thread maintains information about which
 * signals it will respond to. Handler routines
 * are set on a per-process basis - not per-thread.
 * When signalled, a thread will check it's sigmask
 * and, if the signal is not being ignored, call the
 * handler routine associated with the signal. The
 * thread must then (except for some signals) return to
 * the point where it was interrupted.
 *
 * Ideally the system itself would check the target thread's
 * mask before possibly needlessly bothering the thread
 * itself. This could be done by pthread_kill(), that is,
 * in the signaling thread since it has access to
 * all pthread_t structures. It could also retrieve
 * the handler routine address to minimise the target
 * threads response overhead. This may also simplify
 * serialisation of the access to the per-thread signal
 * structures.
 *
 * pthread_kill() eventually calls a routine similar to
 * __ptw32_cancel_thread() which manipulates the target
 * threads processor context to cause the thread to
 * run the handler launcher routine. pthread_kill() must
 * save the target threads current context so that the
 * handler launcher routine can restore the context after
 * the signal handler has returned. Some handlers will not
 * return, eg. the default SIGKILL handler may simply
 * call pthread_exit().
 *
 * The current context is saved in the target threads
 * pthread_t structure.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"

#if defined(HAVE_SIGSET_T)

static void
__ptw32_signal_thread ()
{
}

static void
__ptw32_signal_callhandler ()
{
}

int
pthread_sigmask (int how, sigset_t const *set, sigset_t * oset)
{
  pthread_t thread = pthread_self ();

  if (thread.p == NULL)
    {
      return ENOENT;
    }

  /* Validate the `how' argument. */
  if (set != NULL)
    {
      switch (how)
	{
	case SIG_BLOCK:
	  break;
	case SIG_UNBLOCK:
	  break;
	case SIG_SETMASK:
	  break;
	default:
	  /* Invalid `how' argument. */
	  return EINVAL;
	}
    }

  /* Copy the old mask before modifying it. */
  if (oset != NULL)
    {
      memcpy (oset, &(thread.p->sigmask), sizeof (sigset_t));
    }

  if (set != NULL)
    {
      unsigned int i;

      /* FIXME: this code assumes that sigmask is an even multiple of
         the size of a long integer. */

      unsigned long *src = (unsigned long const *) set;
      unsigned long *dest = (unsigned long *) &(thread.p->sigmask);

      switch (how)
	{
	case SIG_BLOCK:
	  for (i = 0; i < (sizeof (sigset_t) / sizeof (unsigned long)); i++)
	    {
	      /* OR the bit field longword-wise. */
	      *dest++ |= *src++;
	    }
	  break;
	case SIG_UNBLOCK:
	  for (i = 0; i < (sizeof (sigset_t) / sizeof (unsigned long)); i++)
	    {
	      /* XOR the bitfield longword-wise. */
	      *dest++ ^= *src++;
	    }
	case SIG_SETMASK:
	  /* Replace the whole sigmask. */
	  memcpy (&(thread.p->sigmask), set, sizeof (sigset_t));
	  break;
	}
    }

  return 0;
}

int
sigwait (const sigset_t * set, int *sig)
{
  /* This routine is a cancellation point */
  pthread_test_cancel();
}

int
sigaction (int signum, const struct sigaction *act, struct sigaction *oldact)
{
}

#endif /* HAVE_SIGSET_T */
