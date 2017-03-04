###############################################################################
#									      #
#   File name	    Files.mak						      #
#									      #
#   Description	    Define files included in pthread.lib.		      #
#									      #
#   Notes	    Intended to be shared between Windows and Unix make files.#
#		    Not applicable to pthread's version which is Windows only.#
#		    => We can use nmake.exe conditional directives.           #
#		    							      #
#   History								      #
#    2017-02-14 JFL Created this file.                                        #
#    2017-03-03 JFL Automatically detect and use the new common.mk if present.#
#									      #
###############################################################################

# SOURCES=List of source files to compile and include in the pthread library

# Using this all-features-included file allows for a simple and quick compilation,
# at the cost of wasted space.
ONE_SOURCE =	\
    pthread.c

# The list of files to include in the static library, for pthread version 2.09
ALL_SOURCES_209 =	\
    private.c		\
    attr.c		\
    barrier.c		\
    cancel.c		\
    cleanup.c		\
    condvar.c		\
    create.c		\
#   dll.c		\
    autostatic.c	\
    errno.c		\
    exit.c		\
    fork.c		\
    global.c		\
    misc.c		\
    mutex.c		\
    nonportable.c	\
    rwlock.c		\
    sched.c		\
    semaphore.c		\
    signal.c		\
    spin.c		\
    sync.c		\
    tsd.c		\

# The list of sources in pthread version 2.10 RC
# Made obsolete by the common.mk mechanism, see below. To be removed eventually.
ALL_SOURCES_210 =	\
    cleanup.c				\
    create.c				\
#   dll.c				\
    errno.c				\
    global.c				\
    pthread_attr_destroy.c		\
    pthread_attr_getaffinity_np.c	\
    pthread_attr_getdetachstate.c	\
    pthread_attr_getinheritsched.c	\
    pthread_attr_getname_np.c		\
    pthread_attr_getschedparam.c	\
    pthread_attr_getschedpolicy.c	\
    pthread_attr_getscope.c		\
    pthread_attr_getstackaddr.c		\
    pthread_attr_getstacksize.c		\
    pthread_attr_init.c			\
    pthread_attr_setaffinity_np.c	\
    pthread_attr_setdetachstate.c	\
    pthread_attr_setinheritsched.c	\
    pthread_attr_setname_np.c		\
    pthread_attr_setschedparam.c	\
    pthread_attr_setschedpolicy.c	\
    pthread_attr_setscope.c		\
    pthread_attr_setstackaddr.c		\
    pthread_attr_setstacksize.c		\
    pthread_barrier_destroy.c		\
    pthread_barrier_init.c		\
    pthread_barrier_wait.c		\
    pthread_barrierattr_destroy.c	\
    pthread_barrierattr_getpshared.c	\
    pthread_barrierattr_init.c		\
    pthread_barrierattr_setpshared.c	\
    pthread_cancel.c			\
    pthread_cond_destroy.c		\
    pthread_cond_init.c			\
    pthread_cond_signal.c		\
    pthread_cond_wait.c			\
    pthread_condattr_destroy.c		\
    pthread_condattr_getpshared.c	\
    pthread_condattr_init.c		\
    pthread_condattr_setpshared.c	\
    pthread_delay_np.c			\
    pthread_detach.c			\
    pthread_equal.c			\
    pthread_exit.c			\
    pthread_getconcurrency.c		\
    pthread_getname_np.c		\
    pthread_getschedparam.c		\
    pthread_getspecific.c		\
    pthread_getunique_np.c		\
    pthread_getw32threadhandle_np.c	\
    pthread_join.c			\
    pthread_timedjoin_np.c		\
    pthread_tryjoin_np.c		\
    pthread_key_create.c		\
    pthread_key_delete.c		\
    pthread_kill.c			\
    pthread_mutex_consistent.c		\
    pthread_mutex_destroy.c		\
    pthread_mutex_init.c		\
    pthread_mutex_lock.c		\
    pthread_mutex_timedlock.c		\
    pthread_mutex_trylock.c		\
    pthread_mutex_unlock.c		\
    pthread_mutexattr_destroy.c		\
    pthread_mutexattr_getkind_np.c	\
    pthread_mutexattr_getpshared.c	\
    pthread_mutexattr_getrobust.c	\
    pthread_mutexattr_gettype.c		\
    pthread_mutexattr_init.c		\
    pthread_mutexattr_setkind_np.c	\
    pthread_mutexattr_setpshared.c	\
    pthread_mutexattr_setrobust.c	\
    pthread_mutexattr_settype.c		\
    pthread_num_processors_np.c		\
    pthread_once.c			\
    pthread_rwlock_destroy.c		\
    pthread_rwlock_init.c		\
    pthread_rwlock_rdlock.c		\
    pthread_rwlock_timedrdlock.c	\
    pthread_rwlock_timedwrlock.c	\
    pthread_rwlock_tryrdlock.c		\
    pthread_rwlock_trywrlock.c		\
    pthread_rwlock_unlock.c		\
    pthread_rwlock_wrlock.c		\
    pthread_rwlockattr_destroy.c	\
    pthread_rwlockattr_getpshared.c	\
    pthread_rwlockattr_init.c		\
    pthread_rwlockattr_setpshared.c	\
    pthread_self.c			\
    pthread_setaffinity.c		\
    pthread_setcancelstate.c		\
    pthread_setcanceltype.c		\
    pthread_setconcurrency.c		\
    pthread_setname_np.c		\
    pthread_setschedparam.c		\
    pthread_setspecific.c		\
    pthread_spin_destroy.c		\
    pthread_spin_init.c			\
    pthread_spin_lock.c			\
    pthread_spin_trylock.c		\
    pthread_spin_unlock.c		\
    pthread_testcancel.c		\
    pthread_timechange_handler_np.c	\
    pthread_win32_attach_detach_np.c	\
    ptw32_MCS_lock.c			\
    ptw32_callUserDestroyRoutines.c	\
    ptw32_calloc.c			\
    ptw32_cond_check_need_init.c	\
    ptw32_getprocessors.c		\
    ptw32_is_attr.c			\
    ptw32_mutex_check_need_init.c	\
    ptw32_new.c				\
    ptw32_processInitialize.c		\
    ptw32_processTerminate.c		\
    ptw32_relmillisecs.c		\
    ptw32_reuse.c			\
    ptw32_rwlock_cancelwrwait.c		\
    ptw32_rwlock_check_need_init.c	\
    ptw32_semwait.c			\
    ptw32_spinlock_check_need_init.c	\
    ptw32_threadDestroy.c		\
    ptw32_threadStart.c			\
    ptw32_throw.c			\
    ptw32_timespec.c			\
    ptw32_tkAssocCreate.c		\
    ptw32_tkAssocDestroy.c		\
    sched_get_priority_max.c		\
    sched_get_priority_min.c		\
    sched_getscheduler.c		\
    sched_setaffinity.c			\
    sched_setscheduler.c		\
    sched_yield.c			\
    sem_close.c				\
    sem_destroy.c			\
    sem_getvalue.c			\
    sem_init.c				\
    sem_open.c				\
    sem_post.c				\
    sem_post_multiple.c			\
    sem_timedwait.c			\
    sem_trywait.c			\
    sem_unlink.c			\
    sem_wait.c				\
    w32_CancelableWait.c		\

# Pthread 2.10 RC introduces a new common.mk file, similar in purpose to SysToolLib's File.mak.
# Since the pthread-win32 library is by design for Windows only, we can bend the usual SysToolLib rules,
# and use nmake-specific conditional directives to detect common.mk, and use it if present.
!IF EXIST(common.mk)
!INCLUDE common.mk
SOURCES = $(PTHREAD_SRCS)
!ELSE
SOURCES = $(ALL_SOURCES_209)
!ENDIF

# List of all targets that must be built
ALL=pthread.lib # debug\pthread.lib

