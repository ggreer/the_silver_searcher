/*
 * pthread.c
 *
 * Description:
 * This translation unit agregates pthreads-win32 translation units.
 * It is used for inline optimisation of the library,
 * maximising for speed at the expense of size.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2012 Pthreads-win32 contributors
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
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"

/* The following are ordered for inlining */

#include "ptw32_MCS_lock.c"
#include "ptw32_is_attr.c"
#include "ptw32_processInitialize.c"
#include "ptw32_processTerminate.c"
#include "ptw32_threadStart.c"
#include "ptw32_threadDestroy.c"
#include "ptw32_tkAssocCreate.c"
#include "ptw32_tkAssocDestroy.c"
#include "ptw32_callUserDestroyRoutines.c"
#include "ptw32_semwait.c"
#include "ptw32_timespec.c"
#include "ptw32_throw.c"
#include "ptw32_getprocessors.c"
#include "ptw32_calloc.c"
#include "ptw32_new.c"
#include "ptw32_reuse.c"
#include "ptw32_relmillisecs.c"
#include "ptw32_cond_check_need_init.c"
#include "ptw32_mutex_check_need_init.c"
#include "ptw32_rwlock_check_need_init.c"
#include "ptw32_rwlock_cancelwrwait.c"
#include "ptw32_spinlock_check_need_init.c"
#include "pthread_attr_init.c"
#include "pthread_attr_destroy.c"
#include "pthread_attr_getaffinity_np.c"
#include "pthread_attr_setaffinity_np.c"
#include "pthread_attr_getdetachstate.c"
#include "pthread_attr_setdetachstate.c"
#include "pthread_attr_getname_np.c"
#include "pthread_attr_setname_np.c"
#include "pthread_attr_getscope.c"
#include "pthread_attr_setscope.c"
#include "pthread_attr_getstackaddr.c"
#include "pthread_attr_setstackaddr.c"
#include "pthread_attr_getstacksize.c"
#include "pthread_attr_setstacksize.c"
#include "pthread_barrier_init.c"
#include "pthread_barrier_destroy.c"
#include "pthread_barrier_wait.c"
#include "pthread_barrierattr_init.c"
#include "pthread_barrierattr_destroy.c"
#include "pthread_barrierattr_setpshared.c"
#include "pthread_barrierattr_getpshared.c"
#include "pthread_setcancelstate.c"
#include "pthread_setcanceltype.c"
#include "pthread_testcancel.c"
#include "pthread_cancel.c"
#include "pthread_condattr_destroy.c"
#include "pthread_condattr_getpshared.c"
#include "pthread_condattr_init.c"
#include "pthread_condattr_setpshared.c"
#include "pthread_cond_destroy.c"
#include "pthread_cond_init.c"
#include "pthread_cond_signal.c"
#include "pthread_cond_wait.c"
#include "create.c"
#include "cleanup.c"
#include "dll.c"
#include "errno.c"
#include "pthread_exit.c"
#include "global.c"
#include "pthread_equal.c"
#include "pthread_getconcurrency.c"
#include "pthread_kill.c"
#include "pthread_once.c"
#include "pthread_self.c"
#include "pthread_setconcurrency.c"
#include "w32_CancelableWait.c"
#include "pthread_mutex_init.c"
#include "pthread_mutex_destroy.c"
#include "pthread_mutexattr_init.c"
#include "pthread_mutexattr_destroy.c"
#include "pthread_mutexattr_getpshared.c"
#include "pthread_mutexattr_setpshared.c"
#include "pthread_mutexattr_settype.c"
#include "pthread_mutexattr_gettype.c"
#include "pthread_mutexattr_setrobust.c"
#include "pthread_mutexattr_getrobust.c"
#include "pthread_mutex_lock.c"
#include "pthread_mutex_timedlock.c"
#include "pthread_mutex_unlock.c"
#include "pthread_mutex_trylock.c"
#include "pthread_mutex_consistent.c"
#include "pthread_mutexattr_setkind_np.c"
#include "pthread_mutexattr_getkind_np.c"
#include "pthread_getw32threadhandle_np.c"
#include "pthread_getunique_np.c"
#include "pthread_timedjoin_np.c"
#include "pthread_tryjoin_np.c"
#include "pthread_setaffinity.c"
#include "pthread_delay_np.c"
#include "pthread_num_processors_np.c"
#include "pthread_win32_attach_detach_np.c"
#include "pthread_timechange_handler_np.c"
#include "pthread_rwlock_init.c"
#include "pthread_rwlock_destroy.c"
#include "pthread_rwlockattr_init.c"
#include "pthread_rwlockattr_destroy.c"
#include "pthread_rwlockattr_getpshared.c"
#include "pthread_rwlockattr_setpshared.c"
#include "pthread_rwlock_rdlock.c"
#include "pthread_rwlock_timedrdlock.c"
#include "pthread_rwlock_wrlock.c"
#include "pthread_rwlock_timedwrlock.c"
#include "pthread_rwlock_unlock.c"
#include "pthread_rwlock_tryrdlock.c"
#include "pthread_rwlock_trywrlock.c"
#include "pthread_attr_setschedpolicy.c"
#include "pthread_attr_getschedpolicy.c"
#include "pthread_attr_setschedparam.c"
#include "pthread_attr_getschedparam.c"
#include "pthread_attr_setinheritsched.c"
#include "pthread_attr_getinheritsched.c"
#include "pthread_setschedparam.c"
#include "pthread_getschedparam.c"
#include "sched_get_priority_max.c"
#include "sched_get_priority_min.c"
#include "sched_setscheduler.c"
#include "sched_getscheduler.c"
#include "sched_yield.c"
#include "sched_setaffinity.c"
#include "sem_init.c"
#include "sem_destroy.c"
#include "sem_trywait.c"
#include "sem_timedwait.c"
#include "sem_wait.c"
#include "sem_post.c"
#include "sem_post_multiple.c"
#include "sem_getvalue.c"
#include "sem_open.c"
#include "sem_close.c"
#include "sem_unlink.c"
#include "pthread_spin_init.c"
#include "pthread_spin_destroy.c"
#include "pthread_spin_lock.c"
#include "pthread_spin_unlock.c"
#include "pthread_spin_trylock.c"
#include "pthread_detach.c"
#include "pthread_join.c"
#include "pthread_key_create.c"
#include "pthread_key_delete.c"
#include "pthread_getname_np.c"
#include "pthread_setname_np.c"
#include "pthread_setspecific.c"
#include "pthread_getspecific.c"
