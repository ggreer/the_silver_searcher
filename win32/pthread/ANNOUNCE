PTHREADS4W RELEASE 3.0.0 (2017-01-01)
--------------------------------------
Web Site:	https://sourceforge.net/projects/pthreads4w/
Repository:	https://sourceforge.net/p/pthreads4w/code
Releases:	https://sourceforge.net/projects/pthreads4w/files
Maintainer: Ross Johnson <firstname dot lastname at loungebythelake dot net>


We are pleased to announce the availability of a new release of Pthreads4w
(a.k.a. Pthreads-win32), an Open Source Software implementation of
the Threads component of the SUSV3 Standard for Microsoft's Windows
(x86 and x64). Some relevant functions from other sections of SUSV3 are
also supported including semaphores and scheduling functions. See the
Conformance section below for details.

Some common non-portable functions are also implemented for
additional compatibility, as are a few functions specific
to pthreads4w for easier integration with Windows applications.

Pthreads4w is free software. With the exception of four files noted later,
Version 3.0.0 is distributed under the Apache License version 2.0 (APLv2).
The APLv2 is compatible with the GPLv3 and LGPLv3 licenses and therefore
this code may continue to be legally included within GPLv3 and LGPLv3
projects.

All version 1 and 2 releases will remain LGPL but version 2.11 will be
released under v3 of that license so that any modifications to pthreads4w
version 3 code that we backport to v2 will not pollute that code.

The four files that will remain LGPL but change to v3 are files used to
configure the GNU environment builds:

	aclocal.m4
	configure.ac
	GNUmakefile.in
	tests/GNUmakefile.in

For those who want to try the most recent changes, the SourceForge Git
repository is the one to use. The Sourceware CVS repository is synchronised
much less often and may be abandoned altogether.

Release 2.9.1 was probably the last to provide pre-built libraries. The
supported compilers are now all available free for personal use. The MSVS
version should build out of the box using nmake. The GCC versions now make
use of GNU autoconf to generate a configure script which in turn creates a
custom config.h for the environment you use: MinGW or MinGW64, etc.


Acknowledgements
----------------
This library is based originally on a Win32 pthreads
implementation contributed by John Bossom.

The implementation of Condition Variables uses algorithms developed
by Alexander Terekhov and Louis Thomas.

The implementation of POSIX mutexes was improved by Thomas Pfaff
and later by Alexander Terekhov.

The implementation of Spinlocks and Barriers was contributed
by Ross Johnson.

The implementation of read/write locks was contributed by
Aurelio Medina and improved (replaced) by Alexander Terekhov.

An implementation of MCS queue-based locks (used internally) was contributed
by Vladimir Kliatchko.

Many others have contributed significant time and effort to solve crucial
problems in order to make the library workable, robust and reliable.

Thanks to Xavier Leroy for granting permission to use and modify his
LinuxThreads manual pages.

Thanks to The Open Group for making the Single Unix Specification
publicly available - many of the manual pages included in the package
were extracted from it.

There is also a separate CONTRIBUTORS file. This file and others are
on the web site:

	https://sourceforge.net/p/pthreads4w/wiki/Contributors/

As much as possible, the ChangeLog file acknowledges contributions to the
code base in more detail.


Changes since the last release
------------------------------
These are now documented in the NEWS file.
See the ChangeLog file also.


Known Bugs
----------
These are now documented in the BUGS file.


Level of standards conformance
------------------------------

The following POSIX options are defined and set to 200809L:

      _POSIX_THREADS
      _POSIX_THREAD_SAFE_FUNCTIONS
      _POSIX_THREAD_ATTR_STACKSIZE
      _POSIX_THREAD_PRIORITY_SCHEDULING
      _POSIX_SEMAPHORES
      _POSIX_READER_WRITER_LOCKS
      _POSIX_SPIN_LOCKS
      _POSIX_BARRIERS

The following POSIX options are defined and set to -1:

      _POSIX_THREAD_ATTR_STACKADDR
      _POSIX_THREAD_PRIO_INHERIT
      _POSIX_THREAD_PRIO_PROTECT
      _POSIX_THREAD_PROCESS_SHARED


The following POSIX limits are defined and set:

      _POSIX_THREAD_THREADS_MAX
      _POSIX_SEM_VALUE_MAX
      _POSIX_SEM_NSEMS_MAX
      _POSIX_THREAD_KEYS_MAX
      _POSIX_THREAD_DESTRUCTOR_ITERATIONS
      PTHREAD_STACK_MIN
      PTHREAD_THREADS_MAX
      SEM_VALUE_MAX
      SEM_NSEMS_MAX
      PTHREAD_KEYS_MAX
      PTHREAD_DESTRUCTOR_ITERATIONS


The following functions are implemented:

      ---------------------------
      PThreads
      ---------------------------
      pthread_attr_init
      pthread_attr_destroy
      pthread_attr_getdetachstate
      pthread_attr_getstackaddr
      pthread_attr_getstacksize
      pthread_attr_setdetachstate
      pthread_attr_setstackaddr
      pthread_attr_setstacksize

      pthread_create
      pthread_detach
      pthread_equal
      pthread_exit
      pthread_join
      pthread_once
      pthread_self

      pthread_cancel
      pthread_cleanup_pop
      pthread_cleanup_push
      pthread_setcancelstate
      pthread_setcanceltype
      pthread_testcancel

      ---------------------------
      Thread Specific Data
      ---------------------------
      pthread_key_create
      pthread_key_delete
      pthread_setspecific
      pthread_getspecific

      ---------------------------
      Mutexes
      ---------------------------
      pthread_mutexattr_init
      pthread_mutexattr_destroy
      pthread_mutexattr_getpshared
      pthread_mutexattr_setpshared
      pthread_mutexattr_gettype
      pthread_mutexattr_settype (types: PTHREAD_MUTEX_DEFAULT
										PTHREAD_MUTEX_NORMAL
										PTHREAD_MUTEX_ERRORCHECK
										PTHREAD_MUTEX_RECURSIVE  )
      pthread_mutexattr_getrobust
      pthread_mutexattr_setrobust (values: PTHREAD_MUTEX_STALLED
                                           PTHREAD_MUTEX_ROBUST)
      pthread_mutex_init
      pthread_mutex_destroy
      pthread_mutex_lock
      pthread_mutex_trylock
      pthread_mutex_timedlock
      pthread_mutex_unlock
      pthread_mutex_consistent

      ---------------------------
      Condition Variables
      ---------------------------
      pthread_condattr_init
      pthread_condattr_destroy
      pthread_condattr_getpshared
      pthread_condattr_setpshared

      pthread_cond_init
      pthread_cond_destroy
      pthread_cond_wait
      pthread_cond_timedwait
      pthread_cond_signal
      pthread_cond_broadcast

      ---------------------------
      Read/Write Locks
      ---------------------------
      pthread_rwlock_init
      pthread_rwlock_destroy
      pthread_rwlock_tryrdlock
      pthread_rwlock_trywrlock
      pthread_rwlock_rdlock
      pthread_rwlock_timedrdlock
      pthread_rwlock_rwlock
      pthread_rwlock_timedwrlock
      pthread_rwlock_unlock
      pthread_rwlockattr_init
      pthread_rwlockattr_destroy
      pthread_rwlockattr_getpshared
      pthread_rwlockattr_setpshared

      ---------------------------
      Spin Locks
      ---------------------------
      pthread_spin_init
      pthread_spin_destroy
      pthread_spin_lock
      pthread_spin_unlock
      pthread_spin_trylock

      ---------------------------
      Barriers
      ---------------------------
      pthread_barrier_init
      pthread_barrier_destroy
      pthread_barrier_wait
      pthread_barrierattr_init
      pthread_barrierattr_destroy
      pthread_barrierattr_getpshared
      pthread_barrierattr_setpshared

      ---------------------------
      Semaphores
      ---------------------------
      sem_init
      sem_destroy
      sem_post
      sem_wait
      sem_trywait
      sem_timedwait
      sem_getvalue	     (# free if +ve, # of waiters if -ve)
      sem_open		     (returns an error ENOSYS)
      sem_close 	     (returns an error ENOSYS)
      sem_unlink	     (returns an error ENOSYS)

      ---------------------------
      RealTime Scheduling
      ---------------------------
      pthread_attr_getschedparam
      pthread_attr_setschedparam
      pthread_attr_getinheritsched
      pthread_attr_setinheritsched
      pthread_attr_getschedpolicy (only supports SCHED_OTHER)
      pthread_attr_setschedpolicy (only supports SCHED_OTHER)
      pthread_getschedparam
      pthread_setschedparam
      pthread_getconcurrency
      pthread_setconcurrency
      pthread_attr_getscope
      pthread_attr_setscope  (only supports PTHREAD_SCOPE_SYSTEM)
      sched_get_priority_max
      sched_get_priority_min
      sched_rr_get_interval  (returns an error ENOTSUP)
      sched_getaffinity
      sched_setaffinity
      sched_setscheduler     (only supports SCHED_OTHER)
      sched_getscheduler     (only supports SCHED_OTHER)
      sched_yield

      ---------------------------
      Signals
      ---------------------------
      pthread_sigmask
      pthread_kill           (only supports zero sig value,
                              for thread validity checking)

      ---------------------------
      Non-portable routines
      ---------------------------
      (See the README.NONPORTABLE file or HTML manual pages for usage.)
      pthread_attr_getname_np
      pthread_attr_setname_np
      pthread_getname_np
      pthread_setname_np
      pthread_timedjoin_np
      pthread_tryjoin_np
      pthread_getw32threadhandle_np
      pthread_timechange_handler_np
      pthread_delay_np
      pthread_getunique_np
      pthread_attr_getaffinity_np
      pthread_attr_setaffinity_np
      pthread_getaffinity_np
      pthread_setaffinity_np
      pthread_mutexattr_getkind_np
      pthread_mutexattr_setkind_np	(types: PTHREAD_MUTEX_FAST_NP,
						PTHREAD_MUTEX_ERRORCHECK_NP,
						PTHREAD_MUTEX_RECURSIVE_NP,
						PTHREAD_MUTEX_ADAPTIVE_NP,
						PTHREAD_MUTEX_TIMED_NP)
      pthread_num_processors_np
      pthread_win32_getabstime_np
      (The following four routines should no longer be required.)
      pthread_win32_process_attach_np
      pthread_win32_process_detach_np
      pthread_win32_thread_attach_np
      pthread_win32_thread_detach_np

      ---------------------------
      Static Initializers
      ---------------------------
      PTHREAD_ONCE_INIT
      PTHREAD_MUTEX_INITIALIZER
      PTHREAD_RECURSIVE_MUTEX_INITIALIZER
      PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
      PTHREAD_ERRORCHECK_MUTEX_INITIALIZER
      PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
      PTHREAD_COND_INITIALIZER
      PTHREAD_RWLOCK_INITIALIZER
      PTHREAD_SPINLOCK_INITIALIZER

      ---------------------------
      CPU Affinity Mask Support
      ---------------------------
      CPU_ZERO
      CPU_EQUAL
      CPU_COUNT
      CPU_SET
      CPU_CLR
      CPU_ISSET
      CPU_AND
      CPU_OR
      CPU_XOR


The library includes two non-API functions for creating cancellation
points in applications and libraries:
      
      pthreadCancelableWait
      pthreadCancelableTimedWait

      
The following functions are not implemented:

      ---------------------------
      RealTime Scheduling
      ---------------------------
      pthread_mutex_getprioceiling
      pthread_mutex_setprioceiling
      pthread_mutex_attr_getprioceiling
      pthread_mutex_attr_getprotocol
      pthread_mutex_attr_setprioceiling
      pthread_mutex_attr_setprotocol

      ---------------------------
      Fork Handlers
      ---------------------------
      pthread_atfork

      ---------------------------
      Stdio
      --------------------------- 
      flockfile
      ftrylockfile
      funlockfile
      getc_unlocked
      getchar_unlocked	
      putc_unlocked
      putchar_unlocked

      ---------------------------
      Thread-Safe C Runtime Library
      ---------------------------
      readdir_r
      getgrgid_r
      getgrnam_r
      getpwuid_r
      getpwnam_r
      
      ---------------------------
      Signals
      ---------------------------
      sigtimedwait
      sigwait
      sigwaitinfo

      ---------------------------
      General
      ---------------------------
      sysconf      

      ---------------------------
      Thread-Safe C Runtime Library (macros)
      ---------------------------
      strtok_r
      asctime_r
      ctime_r
      gmtime_r
      localtime_r
      rand_r


Application Development Environments
------------------------------------

See the README file for more information.


Documentation
-------------

For the authoritative reference, see the online POSIX
standard reference at:

       http://www.OpenGroup.org

For POSIX Thread API programming, several reference books are
available:

       Programming with POSIX Threads
       David R. Butenhof
       Addison-Wesley (pub)

       Pthreads Programming
       By Bradford Nichols, Dick Buttlar & Jacqueline Proulx Farrell
       O'Reilly (pub)

Enjoy!

Ross Johnson
