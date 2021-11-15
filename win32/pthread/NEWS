RELEASE 3.0.0
--------------
(2018-08-08)

General
-------
Note that this is a new major release. The major version increment
introduces two ABI changes along with other naming changes that will
require recompilation of linking applications and possibly some textual
changes to compile-time macro references in configuration and source
files, e.g. PTW32_* changes to __PTW32_*, ptw32_* to __ptw32_*, etc.

License Change
--------------
With the agreement of all substantial relevant contributors Pthreads4w
version 3, with the exception of four files, is being released under the
terms of the Apache License v2.0. The APLv2 is compatible with the GPLv3
and LGPLv3 licenses and therefore this code may continue to be legally
included within GPLv3 and LGPLv3 projects.

A substantial relevant contributor was defined as one who has contributed
original code that implements a capability present in the releases going
forward. This excludes several contributors who have contributed code
that has been obsoleted, or have provided patches that fix bugs,
reorganise code for aesthetic or practical purposes, or improve build
processes. This distinction was necessary in order to move forward in the
likelyhood that not all contributors would be contactable. All
contributors are listed in the file CONTRIBUTORS.

The four files that will remain LGPL but change to v3 are files used to
configure the GNU environment builds:

	aclocal.m4
	configure.ac
	GNUmakefile.in
	tests/GNUmakefile.in

Contributors who have either requested this change or agreed to it when
consulted are:

John Bossom
Alexander Terekhov
Vladimir Kliatchko
Ross Johnson

Pthreads4w version 2 releases will remain LGPL but version 2.11 and later
will be released under v3 of that license so that any additions to
pthreads4w version 3 code that is backported to v2 will not pollute that
code.

Backporting and Support of Legacy Windows Releases
--------------------------------------------------
Some changes from 2011-02-26 onward may not be compatible with pre
Windows 2000 systems.

New bug fixes in all releases since 2.8.0 have NOT been applied to the
1.x.x series.

Testing and verification
------------------------
The MSVC, MinGW and MinGW64 builds have been tested on SMP architecture
(Intel x64 Hex Core) by completing the included test suite, as well as the
stress and bench tests.

Be sure to run your builds against the test suite. If you see failures
then please consider how your toolchains might be contributing to the
failure. See the README file for more detailed descriptions of the
toolchains and test systems that we have used to get the tests to pass
successfully.

We recommend MinGW64 over MinGW for both 64 and 32 bit GNU CC builds
only because the MinGW DWARF2 exception handling with C++ builds causes some
problems with thread cancelation.

MinGW64 also includes its own native pthreads implementation, which you may
prefer to use. If you wish to build our library you will need to select the
Win32 native threads option at install time. We recommend also selecting the
SJLJ exception handling method for MinGW64-w32 builds. For MinGW64-w64 builds
either the SJLJ or SEH exception handling method should work.

New Features
------------
Other than the following, this release is feature-equivalent to v2.11.0.

This release introduces a change to pthread_t and pthread_once_t that will
affect applications that link with the library.

pthread_t: remains a struct but extends the reuse counter from 32 bits to 64
bits. On 64 bit machines the overall size of the object will not increase, we
simply put 4 bytes of padding to good use reducing the risk that the counter
could wrap around in very long-running applications from small to, effectively,
zero. The 64 bit reuse counter extends risk-free run time from months
(assuming an average thread lifetime of 1ms) to centuries (assuming an
average thread lifetime of 1ns).

pthread_once_t: removes two long-obsoleted elements and reduces it's size.


RELEASE 2.11.0
--------------
(2018-08-08)

General
-------
New bug fixes in all releases since 2.8.0 have NOT been applied to the
1.x.x series.

Some changes from 2011-02-26 onward may not be compatible with
pre Windows 2000 systems.

License Change to LGPL v3
-------------------------
Pthreads4w version 2.11 and all future 2.x versions will be released
under the Lesser GNU Public License version 3 (LGPLv3).

Planned Release Under the Apache License v2
-------------------------------------------
The next major version of this software (version 3) will be released
under the Apache License version 2.0 (ALv2). Releasing 2.11 under LGPLv3
will allow modifications to version 3 of this software to be backported
to version 2 going forward. Further to this, any GPL projects currently
using this library will be able to continue to use either version 2 or 3
of this code in their projects.

For more information please see:
https://www.apache.org/licenses/GPL-compatibility.html

In order to remain consistent with this change, from this point on
modifications to this library will only be accepted against version 3
of this software under the terms of the ALv2. They will then, where
appropriate, be backported to version 2.

We hope to release version 3 at the same time as we release version 2.11.

Testing and verification
------------------------
This version has been tested on SMP architecture (Intel x64 Hex Core)
by completing the included test suite, as well as the stress and bench
tests.

Be sure to run your builds against the test suite. If you see failures
then please consider how your toolchains might be contributing to the
failure. See the README file for more detailed descriptions of the
toolchains and test systems that we have used to get the tests to pass
successfully. We recommend MinGW64 over MinGW32 for both 64 and 32 bit
GNU CC builds. MinGW64 also includes its own independent pthreads
implementation, which you may prefer to use.

New Features or Changes
-----------------------
For Microsoft toolchain builds:
(1) Static linking requires both this library and any linking
libraries or applications to be compiled with /MT consistently.

(2) Static libraries have been renamed as libpthreadV*.lib
to differentiate them from DLL import libs pthreadV*.lib.

(3) If you are using mixed linkage, e.g. linking the static /MT version
of the library to an application linked with /MD you may be able to use
GetLastError() to interrogate the error code because the library sets
both errno (via _set_errno()) and SetLastError().

Bug Fixes
---------
Remove the attempt to set PTW32_USES_SEPARATE_CRT in the headers which
can cause unexpected results. In certain situations a user may want to
define it explicitly in their environment to invoke it's effects, either
when buidling the library or an application or both. See README.NONPORTABLE.
-- Ross Johnson

The library should be more reliable under fully statically linked
scenarios. Note: we have removed the PIMAGE_TLS_CALLBACK code and
reverted to the earlier method that appears to be more reliable
across all compiler editions.
- Mark Pizzolato

Various corrections to GNUmakefile. Although this file has been removed, 
for completeness the changes have been recorded as commits to the
repository.
- Kyle Schwarz

MinGW64-w64 defines pid_t as __int64. sched.h now reflects that.
- Kyle Schwarz

Several tests have been fixed that were seen to fail on machines under
load. Other tests that used similar crude mechanisms to synchronise
threads (these are unit tests) had the same improvements applied:
semaphore5.c recognises that sem_destroy can legitimately return
EBUSY; mutex6*.c, mutex7*.c and mutex8*.c all replaced a single
Sleep() with a polling loop.
- Ross Johnson


RELEASE 2.10.0
--------------
(2016-09-18)

General
-------
New bug fixes in all releases since 2.8.0 have NOT been applied to the
1.x.x series.

Some changes from 2011-02-26 onward may not be compatible with
pre Windows 2000 systems.

Testing and verification
------------------------
This version has been tested on SMP architecture (Intel x64 Hex Core)
by completing the included test suite, as well as the stress and bench
tests.

Be sure to run your builds against the test suite. If you see failures
then please consider how your toolchains might be contributing to the
failure. See the README file for more detailed descriptions of the
toolchains and test systems that we have used to get the tests to pass
successfully. We recommend MinGW64 over MinGW32 for both 64 and 32 bit
GNU CC builds. MinGW64 also includes its own independent pthreads
implementation, which you may prefer to use.

New Features
------------
New routines:
pthread_timedjoin_np()
pthread_tryjoin_np()
 - added for compatibility with Linux.
sched_getaffinity()
sched_setaffinity()
pthread_getaffinity_np()
pthread_setaffinity_np()
pthread_attr_getaffinity_np()
pthread_attr_setaffinity_np()
 - added for compatibility with Linux and other libgcc-based systems.
   The macros to manipulate cpu_set_t objects (the cpu affinity mask
   vector) are also defined: CPU_ZERO, CPU_CLR, CPU_SET, CPU_EQUAL,
   CPU_AND, CPU_OR, CPU_XOR, CPU_COUNT, CPU_ISSET.
pthread_getname_np()
pthread_setname_np()
pthread_attr_getname_np()
pthread_attr_setname_np()
 - added for compatibility with other POSIX implementations. Because
   some implementations use different *_setname_np() prototypes
   you can define one of the following macros when building the library:
      __PTW32_COMPATIBILITY_BSD (compatibility with NetBSD, FreeBSD)
      __PTW32_COMPATIBILITY_TRU64
   If not defined then compatibility is with Linux and other equivalents.
   We don't impose a strict limit on the length of the thread name for the
   default compatibility case. Unlike Linux, no default thread name is set.
   For MSVC builds, the thread name if set is made available for use by the
   MSVS debugger, i.e. it should be displayed within the debugger to
   identify the thread in place of/as well as a threadID.
pthread_win32_getabstime_np()
 - Return the current time plus an optional offset in a platform-aware way
   that is compatible with POSIX timed calls (returns the struct timespec
   address which is the first argument). Intended primarily to make it
   easier to write tests but may be useful for applications generally. 
GNU compiler environments (MinGW32 and MinGW64) now have the option of using
autoconf to automatically configure the build.

Builds:
New makefile targets have been added and existing targets modified or
removed. For example, targets to build and test all of the possible
configurations of both dll and static libs.

GNU compiler builds are now explicitly using ISO C and C++ 2011 standards
compatibility. If your GNU compiler doesn't support this please consider
updating. Auto configuration is now possible via 'configure' script. The
script must be generated using autoconf - see the README file. Thanks to
Keith Marshall from the MinGW project.

Static linking:
The autostatic functionality has been moved to dll.c, and extended so
that builds using MSVC8 and later no longer require apps to call
pthread_win32_thread_detach_np(). That is, all of the DllMain
functionality is now automatic for static linking for these builds.

Some nmake static linking targets have been disabled:
Due to an issue with TLS behaviour, the V*-small-static* nmake targets
in Makefile have been disabled. The issue is exposed by tests/semaphore3.c
where the pthread_self() call inside the thread fails to return the
correct POSIX thread handle but returns a new "implicit" POSIX thread
handle instead. Implicit pthread handles have detached thread status, which
causes the pthread_detach() call inside the thread to return EINVAL. The
V*-static* targets appear to be not affected. The primary difference is
that the latter are generated from a single compilation unit.

Bug Fixes
---------
Small object file static linking now works (MinGW). The autostatic code
is required but nothing explicitly referenced this code so was getting
optimised out.
- Daniel Richard G.

sem_getvalue() could return the errno value instead of setting errno
and returning -1.
- Ross Johnson

Errno values were being lost if the library is statically linked
with the runtime library, meaning also that the application used a
separate runtime instance. This is still the case except a build
switch has been added that allows more robust error status to be
incorporated, i.e. allow the return code to be retrieved via
GetLastError().
- Daniel Richard G.

Identified the cause of significant failures around cancelation
and pthread_exit() for the GCE (GNU C++) build configuration as
coming from Mingw32. Not sure if this is general or just when
building 32 bit libraries and apps that run on 64 bit systems.
These failures do not arise with Mingw64 32 bit builds (GCC built
with multilib enabled) running on 64 bit systems.
- Daniel Richard G. and Ross Johnson

pthread_key_delete() bug introduced in release 2.9.x caused this
routine to fail in a way that the test suite was not detecting. A
new test has been added to confirm that this routine behaves
correctly, particularly when keys with destructors are deleted
before threads exit.
- Stephane Clairet

pthread_win32_process_attach_np() fix potential failure/security around
finding and loading of QUSEREX.DLL.
- Jason Baker

_POSIX_THREAD_ATTR_STACKADDR is now set equal to -1 in pthread.h. As a
consequence pthread_attr_setstackaddr() now returns ENOSYS. Previously
the value was stored and could be retrieved but was otherwise unused.
pthread_attr_getstackaddr() returns ENOSYS correspondingly.
- Ross Johnson

Fixed a potential memory leak in pthread_mutex_init(). The leak would
only occur if the mutex initialisation failed (extremely rare if ever).
- Jaeeun Choi

Fixed sub-millisecond timeouts, which caused the library to busy wait.
- Mark Smith

Fix a race condition and crash in MCS locks. The waiter queue management
code in __ptw32_mcs_lock_acquire was racing with the queue management code
in __ptw32_mcs_lock_release and causing a segmentation fault.
- Anurag Sharma
- Jonathan Brown (also reported this bug and provided a fix)

RELEASE 2.9.1
-------------
(2012-05-27)

General
-------
New bug fixes in this release since 2.8.0 have NOT been applied to the
1.x.x series.

This release replaces an extremely brief 2.9.0 release and adds
some last minute non-code changes were made to embed better
descriptive properties in the dlls to indicate target architecture
and build environments.

Some changes post 2011-02-26 in CVS may not be compatible with pre
Windows 2000 systems.

Use of other than the "C" version of the library is now discouraged.
That is, the "C++" version fails some tests and does not provide any
additional functionality.

Testing and verification
------------------------
This version has been tested on SMP architecture (Intel x64 Hex Core)
by completing the included test suite, stress and bench tests.

New Features
------------
DLL properties now properly includes the target architecture, i.e.
right-click on the file pthreadVC2.dll in explorer and choose the Detail
tab will show the compiler and architecture in the description field, e.g.
"MS C x64" or "MS C x86".
- Ross Johnson

(MSC and GNU builds) The statically linked library now automatically
initialises and cleans up on program start/exit, i.e. statically linked
applications need not call the routines pthread_win32_process_attach_np()
and pthread_win32_process_detach_np() explicitly. The per-thread routine
pthread_win32_thread_detach_np() is also called at program exit to cleanup
POSIX resources acquired by the primary Windows native thread, if I (RJ)
understand the process correctly. Other Windows native threads that call
POSIX API routines may need to call the thread detach routine on thread
exit if the application depends on reclaimed POSIX resources or running
POSIX TSD (TLS) destructors.
See README.NONPORTABLE for descriptions of these routines.
- Ramiro Polla

Robust mutexes are implemented within the PROCESS_PRIVATE scope. NOTE that
pthread_mutex_* functions may return different error codes for robust
mutexes than they otherwise do in normal usage, e.g.  pthread_mutex_unlock
is required to check ownership for all mutex types when the mutex is
robust, whereas this does not occur for the "normal" non-robust mutex type.
- Ross Johnson

pthread_getunique_np is implemented for source level compatibility
with some other implementations. This routine returns a 64 bit
sequence number that is uniquely associated with a thread. It can be
used by applications to order or hash POSIX thread handles.
- Ross Johnson

Bug fixes
---------
Many more changes for 64 bit systems.
- Kai Tietz

Various modifications and fixes to build and test for WinCE.
- Marcel Ruff, Sinan Kaya

Fix pthread_cond_destroy() - should not be a cancellation point. Other
minor build problems fixed.
- Romano Paolo Tenca

Remove potential deadlock condition from pthread_cond_destroy().
- Eric Berge

Various modifications to build and test for Win64.
- Kip Streithorst

Various fixes to the QueueUserAPCEx async cancellation helper DLL
(this is a separate download) and pthreads code cleanups.
- Sebastian Gottschalk

Removed potential NULL pointer reference.
- Robert Kindred

Removed the requirement that applications restrict the number of threads
calling pthread_barrier_wait to just the barrier count. Also reduced the
contention between barrier_wait and barrier_destroy. This change will have
slowed barriers down slightly but halves the number of semaphores consumed
per barrier to one.
- Ross Johnson

Fixed a handle leak in sched_[gs]etscheduler.
- Mark Pizzolato

Removed all of the POSIX re-entrant function compatibility macros from pthread.h.
Some were simply not semanticly correct.
- Igor Lubashev

Threads no longer attempt to pass uncaught exceptions out of thread scope (C++
and SEH builds only). Uncaught exceptions now cause the thread to exit with
the return code PTHREAD_CANCELED.
- Ross Johnson

Lots of casting fixes particularly for x64, Interlocked fixes and reworking
for x64.
- Daniel Richard G., John Kamp

Other changes
-------------
Dependence on the winsock library is now discretionary via
#define RETAIN_WSALASTERROR in config.h. It is undefined by default unless
WINCE is defined (because RJ is unsure of the dependency there).
- Ramiro Polla

Several static POSIX mutexes used for internal management were replaced by
MCS queue-based locks to reduce resource consumption, in particular use of Win32
objects.
- Ross Johnson

For security, the QuserEx.dll if used must now be installed in the Windows System
folder.
- Ross Johnson

New tests
---------
robust[1-5].c - Robust mutexes
sequence1.c - per-thread unique sequence numbers

Modified tests and benchtests
-----------------------------
All mutex*.c tests wherever appropriate have been modified to also test
robust mutexes under the same conditions.
Added robust mutex benchtests to benchtest*.c wherever appropriate.


RELEASE 2.8.0
-------------
(2006-12-22)

General
-------
New bug fixes in this release since 2.7.0 have not been applied to the
version 1.x.x series. It is probably time to drop version 1.

Testing and verification
------------------------
This release has not yet been tested on SMP architechtures. All tests pass
on a uni-processor system.

Bug fixes
---------
Sem_destroy could return EBUSY even though no threads were waiting on the 
semaphore. Other races around invalidating semaphore structs (internally)
have been removed as well.

New tests
---------
semaphore5.c - tests the bug fix referred to above.


RELEASE 2.7.0
-------------
(2005-06-04)

General
-------
All new features in this release have been back-ported in release 1.11.0,
including the incorporation of MCS locks in pthread_once, however, versions
1 and 2 remain incompatible even though they are now identical in
performance and functionality.

Testing and verification
------------------------
This release has been tested (passed the test suite) on both uni-processor
and multi-processor systems.
- Tim Theisen

Bug fixes
---------
Pthread_once has been re-implemented to remove priority boosting and other
complexity to improve robustness. Races for Win32 handles that are not
recycle-unique have been removed. The general form of pthread_once is now
the same as that suggested earlier by Alexander Terekhov, but instead of the
'named mutex', a queue-based lock has been implemented which has the required
properties of dynamic self initialisation and destruction. This lock is also
efficient. The ABI is unaffected in as much as the size of pthread_once_t has
not changed and PTHREAD_ONCE_INIT has not changed, however, applications that
peek inside pthread_once_t, which is supposed to be opaque, will break.
- Vladimir Kliatchko

New features
------------
* Support for Mingw cross development tools added to GNUmakefile.
Mingw cross tools allow building the libraries on Linux.
- Mikael Magnusson


RELEASE 2.6.0
-------------
(2005-05-19)

General
-------
All of the bug fixes and new features in this release have been
back-ported in release 1.10.0.

Testing and verification
------------------------
This release has been tested (passed the test suite) on both uni-processor
and multi-processor systems. Thanks to Tim Theisen at TomoTherapy for
exhaustively running the MP tests and for providing crutial observations
and data when faults are detected.

Bugs fixed
----------

* pthread_detach() now reclaims remaining thread resources if called after
the target thread has terminated. Previously, this routine did nothing in
this case.

New tests
---------

* detach1.c - tests that pthread_detach properly invalidates the target
thread, which indicates that the thread resources have been reclaimed.


RELEASE 2.5.0
-------------
(2005-05-09)

General
-------

The package now includes a reference documentation set consisting of
HTML formatted Unix-style manual pages that have been edited for
consistency with Pthreads-w32. The set can also be read online at:
https://sourceforge.net/projects/pthreads4w/manual/index.html

Thanks again to Tim Theisen for running the test suite pre-release
on an MP system.

All of the bug fixes and new features in this release have been
back-ported in release 1.9.0.

Bugs fixed
----------

* Thread Specific Data (TSD) key management has been ammended to
eliminate a source of (what was effectively) resource leakage (a HANDLE
plus memory for each key destruct routine/thread association). This was
not a true leak because these resources were eventually reclaimed when
pthread_key_delete was run AND each thread referencing the key had exited.
The problem was that these two conditions are often not met until very
late, and often not until the process is about to exit.

The ammended implementation avoids the need for the problematic HANDLE
and reclaims the memory as soon as either the key is deleted OR the
thread exits, whichever is first.

Thanks to Richard Hughes at Aculab for identifying and locating the leak.

* TSD key destructors are now processed up to PTHREAD_DESTRUCTOR_ITERATIONS
times instead of just once. PTHREAD_DESTRUCTOR_ITERATIONS has been
defined in pthread.h for some time but not used.

* Fix a semaphore accounting race between sem_post/sem_post_multiple
and sem_wait cancellation. This is the same issue as with
sem_timedwait that was fixed in the last release.

* sem_init, sem_post, and sem_post_multiple now check that the
semaphore count never exceeds _POSIX_SEM_VALUE_MAX.

* Although sigwait() is nothing more than a no-op, it should at least
be a cancellation point to be consistent with the standard.

New tests
---------

* stress1.c - attempts to expose problems in condition variable
and semaphore timed wait logic. This test was inspired by Stephan
Mueller's sample test code used to identify the sem_timedwait bug
from the last release. It's not a part of the regular test suite
because it can take awhile to run. To run it:
nmake clean VC-stress

* tsd2.c - tests that key destructors are re-run if the tsd key value is
not NULL after the destructor routine has run. Also tests that
pthread_setspecific() and pthread_getspecific() are callable from
destructors.


RELEASE 2.4.0
-------------
(2005-04-26)

General
-------

There is now no plan to release a version 3.0.0 to fix problems in
pthread_once(). Other possible implementations of pthread_once
will still be investigated for a possible future release in an attempt
to reduce the current implementation's complexity.

All of the bug fixes and new features in this release have been
back-ported for release 1.8.0.

Bugs fixed
----------

* Fixed pthread_once race (failures on an MP system). Thanks to
Tim Theisen for running exhaustive pre-release testing on his MP system
using a range of compilers:
  VC++ 6
  VC++ 7.1
  Intel C++ version 8.0
All tests passed.
Some minor speed improvements were also done.

* Fix integer overrun error in pthread_mutex_timedlock() - missed when
sem_timedwait() was fixed in release 2.2.0. This routine no longer returns
ENOTSUP when NEED_SEM is defined - it is supported (NEED_SEM is only
required for WinCE versions prior to 3.0).

* Fix timeout bug in sem_timedwait().
- Thanks to Stephan Mueller for reporting, providing diagnostic output
and test code.

* Fix several problems in the NEED_SEM conditionally included code.
NEED_SEM included code is provided for systems that don't implement W32
semaphores, such as WinCE prior to version 3.0. An alternate implementation
of POSIX semaphores is built using W32 events for these systems when
NEED_SEM is defined. This code has been completely rewritten in this
release to reuse most of the default POSIX semaphore code, and particularly,
to implement all of the sem_* routines supported by Pthreads4w. Tim
Theisen also run the test suite over the NEED_SEM code on his MP system. All
tests passed.

* The library now builds without errors for the Borland Builder 5.5 compiler.

New features
------------

* pthread_mutex_timedlock() and all sem_* routines provided by
Pthreads4w are now implemented for WinCE versions prior to 3.0. Those
versions did not implement W32 semaphores. Define NEED_SEM in config.h when
building the library for these systems.

Known issues in this release
----------------------------

* pthread_once is too complicated - but it works as far as testing can
determine..

* The Borland version of the dll fails some of the tests with a memory read
exception. The cause is not yet known but a compiler bug has not been ruled
out.


RELEASE 2.3.0
-------------
(2005-04-12)

General
-------

Release 1.7.0 is a backport of features and bug fixes new in
this release. See earlier notes under Release 2.0.0/General.

Bugs fixed
----------

* Fixed pthread_once potential for post once_routine cancellation
hanging due to starvation. See comments in pthread_once.c.
Momentary priority boosting is used to ensure that, after a
once_routine is cancelled, the thread that will run the
once_routine is not starved by higher priority waiting threads at
critical times. Priority boosting occurs only AFTER a once_routine 
cancellation, and is applied only to that once_control. The
once_routine is run at the thread's normal base priority.

New tests
---------

* once4.c: Aggressively tests pthread_once() under realtime
conditions using threads with varying priorities. Windows'
random priority boosting does not occur for threads with realtime
priority levels.


RELEASE 2.2.0
-------------
(2005-04-04)

General
-------

* Added makefile targets to build static link versions of the library.
Both MinGW and MSVC. Please note that this does not imply any change
to the LGPL licensing, which still imposes psecific conditions on
distributing software that has been statically linked with this library.

* There is a known bug in pthread_once(). Cancellation of the init_routine
exposes a potential starvation (i.e. deadlock) problem if a waiting thread
has a higher priority than the initting thread. This problem will be fixed
in version 3.0.0 of the library.

Bugs fixed
----------

* Fix integer overrun error in sem_timedwait().
Kevin Lussier

* Fix preprocessor directives for static linking.
Dimitar Panayotov


RELEASE 2.1.0
-------------
(2005-03-16)

Bugs fixed
----------

* Reverse change to pthread_setcancelstate() in 2.0.0.


RELEASE 2.0.0
-------------
(2005-03-16)

General
-------

This release represents an ABI change and the DLL version naming has
incremented from 1 to 2, e.g. pthreadVC2.dll.

Version 1.4.0 back-ports the new functionality included in this
release. Please distribute DLLs built from that version with updates
to applications built on pthreads-win32 version 1.x.x.

The package naming has changed, replacing the snapshot date with 
the version number + descriptive information. E.g. this
release is "pthreads-w32-2-0-0-release".

Bugs fixed
----------

* pthread_setcancelstate() no longer checks for a pending
async cancel event if the library is using alertable async
cancel. See the README file (Prerequisites section) for info
on adding alertable async cancellation.

New features
------------

* pthread_once() now supports init_routine cancellability.

New tests
---------

* Agressively test pthread_once() init_routine cancellability.


SNAPSHOT 2005-03-08
-------------------
Version 1.3.0

Bug reports (fixed)
-------------------

* Implicitly created threads leave Win32 handles behind after exiting.
- Dmitrii Semii

* pthread_once() starvation problem.
- Gottlob Frege

New tests
---------

* More intense testing of pthread_once().


SNAPSHOT 2005-01-25
-------------------
Version 1.2.0

Bug fixes
---------

* Attempted acquisition of a recursive mutex could cause waiting threads
to not be woken when the mutex was released.
- Ralf Kubis  <RKubis at mc.com>

* Various package omissions have been fixed.


SNAPSHOT 2005-01-03
-------------------
Version 1.1.0

Bug fixes
---------

* Unlocking recursive or errorcheck mutexes would sometimes
unexpectedly return an EPERM error (bug introduced in
snapshot-2004-11-03).
- Konstantin Voronkov  <beowinkle at yahoo.com>


SNAPSHOT 2004-11-22
-------------------
Version 1.0.0

This snapshot primarily fixes the condvar bug introduced in
snapshot-2004-11-03. DLL versioning has also been included to allow
applications to runtime check the Microsoft compatible DLL version
information, and to extend the DLL naming system for ABI and major
(non-backward compatible) API changes. See the README file for details.

Bug fixes
---------

* Condition variables no longer deadlock (bug introduced in
snapshot-2004-11-03).
- Alexander Kotliarov and Nicolas at saintmac

* DLL naming extended to avoid 'DLL hell' in the future, and to
accommodate the ABI change introduced in snapshot-2004-11-03. Snapshot
2004-11-03 will be removed from FTP sites.

New features
------------

* A Microsoft-style version resource has been added to the DLL for
applications that wish to check DLL compatibility at runtime.

* Pthreads4w DLL naming has been extended to allow incompatible DLL
versions to co-exist in the same filesystem. See the README file for details,
but briefly: while the version information inside the DLL will change with
each release from now on, the DLL version names will only change if the new
DLL is not backward compatible with older applications.

The versioning scheme has been borrowed from GNU Libtool, and the DLL
naming scheme is from Cygwin. Provided the Libtool-style numbering rules are
honoured, the Cygwin DLL naming scheme automatcally ensures that DLL name
changes are minimal and that applications will not load an incompatible
Pthreads4w DLL.

Those who use the pre-built DLLs will find that the DLL/LIB names have a new
suffix (1) in this snapshot. E.g. pthreadVC1.dll etc.

* The POSIX thread ID reuse uniqueness feature introduced in the last snapshot
has been kept as default, but the behaviour can now be controlled when the DLL
is built to effectively switch it off. This makes the library much more
sensitive to applications that assume that POSIX thread IDs are unique, i.e.
are not strictly compliant with POSIX. See the  __PTW32_THREAD_ID_REUSE_INCREMENT
macro comments in config.h for details.

Other changes
-------------
Certain POSIX macros have changed.

These changes are intended to conform to the Single Unix Specification version 3,
which states that, if set to 0 (zero) or not defined, then applications may use
sysconf() to determine their values at runtime. Pthreads4w does not
implement sysconf().

The following macros are no longer undefined, but defined and set to -1
(not implemented):

      _POSIX_THREAD_ATTR_STACKADDR
      _POSIX_THREAD_PRIO_INHERIT
      _POSIX_THREAD_PRIO_PROTECT
      _POSIX_THREAD_PROCESS_SHARED

The following macros are defined and set to 200112L (implemented):

      _POSIX_THREADS
      _POSIX_THREAD_SAFE_FUNCTIONS
      _POSIX_THREAD_ATTR_STACKSIZE
      _POSIX_THREAD_PRIORITY_SCHEDULING
      _POSIX_SEMAPHORES
      _POSIX_READER_WRITER_LOCKS
      _POSIX_SPIN_LOCKS
      _POSIX_BARRIERS

The following macros are defined and set to appropriate values:

      _POSIX_THREAD_THREADS_MAX
      _POSIX_SEM_VALUE_MAX
      _POSIX_SEM_NSEMS_MAX
      PTHREAD_DESTRUCTOR_ITERATIONS
      PTHREAD_KEYS_MAX
      PTHREAD_STACK_MIN
      PTHREAD_THREADS_MAX


SNAPSHOT 2004-11-03
-------------------

DLLs produced from this snapshot cannot be used with older applications without
recompiling the application, due to a change to pthread_t to provide unique POSIX
thread IDs.

Although this snapshot passes the extended test suite, many of the changes are
fairly major, and some applications may show different behaviour than previously,
so adopt with care. Hopefully, any changed behaviour will be due to the library
being better at it's job, not worse.

Bug fixes
---------

* pthread_create() no longer accepts NULL as the thread reference arg.
A segfault (memory access fault) will result, and no thread will be
created.

* pthread_barrier_wait() no longer acts as a cancellation point.

* Fix potential race condition in pthread_once()
- Tristan Savatier  <tristan at mpegtv.com>

* Changes to pthread_cond_destroy() exposed some coding weaknesses in several
test suite mini-apps because pthread_cond_destroy() now returns EBUSY if the CV
is still in use.

New features
------------

* Added for compatibility:
PTHREAD_RECURSIVE_MUTEX_INITIALIZER,
PTHREAD_ERRORCHECK_MUTEX_INITIALIZER,
PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP,
PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP

* Initial support for Digital Mars compiler
- Anuj Goyal  <anuj.goyal at gmail.com>

* Faster Mutexes. These have been been rewritten following a model provided by
Alexander Terekhov that reduces kernel space checks, and eliminates some additional
critical sections used to manage a race between timedlock expiration and unlock.
Please be aware that the new mutexes do not enforce strict absolute FIFO scheduling
of mutexes, however any out-of-order lock acquisition should be very rare.

* Faster semaphores. Following a similar model to mutexes above, these have been
rewritten to use preliminary users space checks.

* sem_getvalue() now returns the number of waiters.

* The POSIX thread ID now has much stronger uniqueness characteristics. The library
garrantees not to reuse the same thread ID for at least 2^(wordsize) thread
destruction/creation cycles.

New tests
---------

* semaphore4.c: Tests cancellation of the new sem_wait().

* semaphore4t.c: Likewise for sem_timedwait().

* rwlock8.c: Tests and times the slow execution paths of r/w locks, and the CVs,
mutexes, and semaphores that they're built on.


SNAPSHOT 2004-05-16
-------------------

Attempt to add Watcom to the list of compilers that can build the library.
This failed in the end due to it's non-thread-aware errno. The library
builds but the test suite fails. See README.Watcom for more details.

Bug fixes
---------
* Bug and memory leak in sem_init()
- Alex Blanco  <Alex.Blanco at motorola.com>

* __ptw32_getprocessors() now returns CPU count of 1 for WinCE.
- James Ewing  <james.ewing at sveasoft.com>

* pthread_cond_wait() could be canceled at a point where it should not
be cancelable. Fixed.
- Alexander Terekhov  <TEREKHOV at de.ibm.com>

* sem_timedwait() had an incorrect timeout calculation.
- Philippe Di Cristo  <philipped at voicebox.com>

* Fix a memory leak left behind after threads are destroyed.
- P. van Bruggen  <pietvb at newbridges.nl>

New features
------------
* Ported to AMD64.
- Makoto Kato  <raven at oldskool.jp>

* True pre-emptive asynchronous cancellation of threads. This is optional
and requires that Panagiotis E. Hadjidoukas's QueueUserAPCEx package be
installed. This package is included in the pthreads-win32 self-unpacking
Zip archive starting from this snapshot. See the README.txt file inside
the package for installation details.

Note: If you don't use async cancellation in your application, or don't need
to cancel threads that are blocked on system resources such as network I/O,
then the default non-preemptive async cancellation is probably good enough.
However, pthreads-win32 auto-detects the availability of these components
at run-time, so you don't need to rebuild the library from source if you
change your mind later.

All of the advice available in books and elsewhere on the undesirability
of using async cancellation in any application still stands, but this
feature is a welcome addition with respect to the library's conformance to
the POSIX standard.

SNAPSHOT 2003-09-18
-------------------

Cleanup of thread priority management. In particular, setting of thread
priority now attempts to map invalid Win32 values within the range returned
by sched_get_priority_min/max() to useful values. See README.NONPORTABLE
under "Thread priority".

Bug fixes
---------
* pthread_getschedparam() now returns the priority given by the most recent
call to pthread_setschedparam() or established by pthread_create(), as
required by the standard. Previously, pthread_getschedparam() incorrectly
returned the running thread priority at the time of the call, which may have
been adjusted or temporarily promoted/demoted.

* sched_get_priority_min() and sched_get_priority_max() now return -1 on error
and set errno. Previously, they incorrectly returned the error value directly.


SNAPSHOT 2003-09-04
-------------------

Bug fixes
---------
* __ptw32_cancelableWait() now allows cancellation of waiting implicit POSIX
threads.

New test
--------
* cancel8.c tests cancellation of Win32 threads waiting at a POSIX cancellation
point.


SNAPSHOT 2003-09-03
-------------------

Bug fixes
---------
* pthread_self() would free the newly created implicit POSIX thread handle if
DuplicateHandle failed instead of recycle it (very unlikely).

* pthread_exit() was neither freeing nor recycling the POSIX thread struct
for implicit POSIX threads.

New feature - cancellation of/by Win32 (non-POSIX) threads
---------------------------------------------------------
Since John Bossom's original implementation, the library has allowed non-POSIX
initialised threads (Win32 threads) to call Pthreads4w routines and
therefore interact with POSIX threads. This is done by creating an on-the-fly
POSIX thread ID for the Win32 thread that, once created, allows fully
reciprical interaction. This did not extend to thread cancellation (async or
deferred). Now it does.

Any thread can be canceled by any other thread (Win32 or POSIX) if the former
thread's POSIX pthread_t value is known. It's TSD destructors and POSIX
cleanup handlers will be run before the thread exits with an exit code of
PTHREAD_CANCELED (retrieved with GetExitCodeThread()).

This allows a Win32 thread to, for example, call POSIX CV routines in the same way
that POSIX threads would/should, with pthread_cond_wait() cancelability and
cleanup handlers (pthread_cond_wait() is a POSIX cancellation point).

By adding cancellation, Win32 threads should now be able to call all POSIX
threads routines that make sense including semaphores, mutexes, condition
variables, read/write locks, barriers, spinlocks, tsd, cleanup push/pop,
cancellation, pthread_exit, scheduling, etc.

Note that these on-the-fly 'implicit' POSIX thread IDs are initialised as detached
(not joinable) with deferred cancellation type. The POSIX thread ID will be created
automatically by any POSIX routines that need a POSIX handle (unless the routine
needs a pthread_t as a parameter of course). A Win32 thread can discover it's own
POSIX thread ID by calling pthread_self(), which will create the handle if
necessary and return the pthread_t value.

New tests
---------
Test the above new feature.


SNAPSHOT 2003-08-19
-------------------

This snapshot fixes some accidental corruption to new test case sources.
There are no changes to the library source code.


SNAPSHOT 2003-08-15
-------------------

Bug fixes
---------

* pthread.dsp now uses correct compile flags (/MD).
- Viv  <vcotirlea@hotmail.com>

* pthread_win32_process_detach_np() fixed memory leak.
- Steven Reddie  <Steven.Reddie@ca.com>

* pthread_mutex_destroy() fixed incorrect return code.
- Nicolas Barry  <boozai@yahoo.com>

* pthread_spin_destroy() fixed memory leak.
- Piet van Bruggen  <pietvb@newbridges.nl>

* Various changes to tighten arg checking, and to work with later versions of
MinGW32 and MsysDTK.

* pthread_getschedparam() etc, fixed dangerous thread validity checking.
- Nicolas Barry  <boozai@yahoo.com>

* POSIX thread handles are now reused and their memory is not freed on thread exit.
This allows for stronger thread validity checking.

New standard routine
--------------------

* pthread_kill() added to provide thread validity checking to applications.
It does not accept any non zero values for the signal arg.

New test cases
--------------

* New test cases to confirm validity checking, pthread_kill(), and thread reuse.


SNAPSHOT 2003-05-10
-------------------

Bug fixes
---------

* pthread_mutex_trylock() now returns correct error values.
pthread_mutex_destroy() will no longer destroy a recursively locked mutex.
pthread_mutex_lock() is no longer inadvertantly behaving as a cancellation point.
- Thomas Pfaff  <tpfaff@gmx.net>

* pthread_mutex_timedlock() no longer occasionally sets incorrect mutex
ownership, causing deadlocks in some applications.
- Robert Strycek <strycek@posam.sk> and Alexander Terekhov  <TEREKHOV@de.ibm.com>


SNAPSHOT 2002-11-04
-------------------

Bug fixes
---------

* sem_getvalue() now returns the correct value under Win NT and WinCE.
- Rob Fanner  <rfanner@stonethree.com>

* sem_timedwait() now uses tighter checks for unreasonable
abstime values - that would result in unexpected timeout values.

* __ptw32_cond_wait_cleanup() no longer mysteriously consumes
CV signals but may produce more spurious wakeups. It is believed
that the sem_timedwait() call is consuming a CV signal that it
shouldn't.
- Alexander Terekhov  <TEREKHOV@de.ibm.com>

* Fixed a memory leak in __ptw32_threadDestroy() for implicit threads.

* Fixed potential for deadlock in pthread_cond_destroy().
A deadlock could occur for statically declared CVs (PTHREAD_COND_INITIALIZER),
when one thread is attempting to destroy the condition variable while another
is attempting to dynamically initialize it.
- Michael Johnson  <michaelj@maine.rr.com>


SNAPSHOT 2002-03-02
-------------------

Cleanup code default style. (IMPORTANT)
----------------------------------------------------------------------
Previously, if not defined, the cleanup style was determined automatically
from the compiler/language, and one of the following was defined accordingly:

        __PTW32_CLEANUP_SEH   MSVC only
        __PTW32_CLEANUP_CXX   C++, including MSVC++, GNU G++
        __PTW32_CLEANUP_C             C, including GNU GCC, not MSVC

These defines determine the style of cleanup (see pthread.h) and,
most importantly, the way that cancellation and thread exit (via
pthread_exit) is performed (see the routine __ptw32_throw() in private.c).

In short, the exceptions versions of the library throw an exception
when a thread is canceled or exits (via pthread_exit()), which is
caught by a handler in the thread startup routine, so that the
the correct stack unwinding occurs regardless of where the thread
is when it's canceled or exits via pthread_exit().

In this and future snapshots, unless the build explicitly defines (e.g.
via a compiler option) __PTW32_CLEANUP_SEH, __PTW32_CLEANUP_CXX, or __PTW32_CLEANUP_C, then
the build NOW always defaults to __PTW32_CLEANUP_C style cleanup. This style
uses setjmp/longjmp in the cancellation and pthread_exit implementations,
and therefore won't do stack unwinding even when linked to applications
that have it (e.g. C++ apps). This is for consistency with most
current commercial Unix POSIX threads implementations. Compaq's TRU64
may be an exception (no pun intended) and possible future trend.

Although it was not clearly documented before, it is still necessary to
build your application using the same __PTW32_CLEANUP_* define as was
used for the version of the library that you link with, so that the
correct parts of pthread.h are included. That is, the possible
defines require the following library versions:

        __PTW32_CLEANUP_SEH   pthreadVSE.dll
        __PTW32_CLEANUP_CXX   pthreadVCE.dll or pthreadGCE.dll
        __PTW32_CLEANUP_C     pthreadVC.dll or pthreadGC.dll

E.g. regardless of whether your app is C or C++, if you link with
pthreadVC.lib or libpthreadGC.a, then you must define __PTW32_CLEANUP_C.


THE POINT OF ALL THIS IS: if you have not been defining one of these
explicitly, then the defaults as described at the top of this
section were being used.

THIS NOW CHANGES, as has been explained above, but to try to make this
clearer here's an example:

If you were building your application with MSVC++ i.e. using C++
exceptions and not explicitly defining one of __PTW32_CLEANUP_*, then
__PTW32_CLEANUP_C++ was automatically defined for you in pthread.h.
You should have been linking with pthreadVCE.dll, which does
stack unwinding.

If you now build your application as you had before, pthread.h will now
automatically set __PTW32_CLEANUP_C as the default style, and you will need to
link with pthreadVC.dll. Stack unwinding will now NOT occur when a thread
is canceled, or the thread calls pthread_exit().

Your application will now most likely behave differently to previous
versions, and in non-obvious ways. Most likely is that locally
instantiated objects may not be destroyed or cleaned up after a thread
is canceled.

If you want the same behaviour as before, then you must now define
__PTW32_CLEANUP_C++ explicitly using a compiler option and link with
pthreadVCE.dll as you did before.


WHY ARE WE MAKING THE DEFAULT STYLE LESS EXCEPTION-FRIENDLY?
Because no commercial Unix POSIX threads implementation allows you to
choose to have stack unwinding. Therefore, providing it in pthread-win32
as a default is dangerous. We still provide the choice but unless
you consciously choose to do otherwise, your pthreads applications will
now run or crash in similar ways irrespective of the threads platform
you use. Or at least this is the hope.


WHY NOT REMOVE THE EXCEPTIONS VERSIONS OF THE LIBRARY ALTOGETHER?
There are a few reasons:
- because there are well respected POSIX threads people who believe
  that POSIX threads implementations should be exceptions aware and
  do the expected thing in that context. (There are equally respected
  people who believe it should not be easily accessible, if it's there
  at all, for unconditional conformity to other implementations.)
- because Pthreads4w is one of the few implementations that has
  the choice, perhaps the only freely available one, and so offers
  a laboratory to people who may want to explore the effects;
- although the code will always be around somewhere for anyone who
  wants it, once it's removed from the current version it will not be
  nearly as visible to people who may have a use for it.


Source module splitting
-----------------------
In order to enable smaller image sizes to be generated
for applications that link statically with the library,
most routines have been separated out into individual
source code files.

This is being done in such a way as to be backward compatible.
The old source files are reused to congregate the individual
routine files into larger translation units (via a bunch of
# includes) so that the compiler can still optimise wherever
possible, e.g. through inlining, which can only be done
within the same translation unit.

It is also possible to build the entire library by compiling
the single file named "pthread.c", which just #includes all
the secondary congregation source files. The compiler
may be able to use this to do more inlining of routines.

Although the GNU compiler is able to produce libraries with
the necessary separation (the -ffunction-segments switch),
AFAIK, the MSVC and other compilers don't have this feature.

Finally, since I use makefiles and command-line compilation,
I don't know what havoc this reorganisation may wreak amongst
IDE project file users. You should be able to continue
using your existing project files without modification.


New non-portable functions
--------------------------
pthread_num_processors_np():
  Returns the number of processors in the system that are
  available to the process, as determined from the processor
  affinity mask.

pthread_timechange_handler_np():
  To improve tolerance against operator or time service initiated
  system clock changes.

  This routine can be called by an application when it
  receives a WM_TIMECHANGE message from the system. At present
  it broadcasts all condition variables so that waiting threads
  can wake up and re-evaluate their conditions and restart
  their timed waits if required.
  - Suggested by Alexander Terekhov


Platform dependence
-------------------
As Win95 doesn't provide one, the library now contains
it's own InterlockedCompareExchange() routine, which is used
whenever Windows doesn't provide it. InterlockedCompareExchange()
is used to implement spinlocks and barriers, and also in mutexes.
This routine relies on the CMPXCHG machine instruction which
is not available on i386 CPUs. This library (from snapshot
20010712 onwards) is therefore no longer supported on i386
processor platforms.


New standard routines
---------------------
For source code portability only - rwlocks cannot be process shared yet.

        pthread_rwlockattr_init()
        pthread_rwlockattr_destroy()
        pthread_rwlockattr_setpshared()
        pthread_rwlockattr_getpshared()

As defined in the new POSIX standard, and the Single Unix Spec version 3:

        sem_timedwait()
        pthread_mutex_timedlock()    - Alexander Terekhov and Thomas Pfaff
        pthread_rwlock_timedrdlock() - adapted from pthread_rwlock_rdlock()
        pthread_rwlock_timedwrlock() - adapted from pthread_rwlock_wrlock()


pthread.h no longer includes windows.h
--------------------------------------
[Not yet for G++]

This was done to prevent conflicts.

HANDLE, DWORD, and NULL are temporarily defined within pthread.h if
they are not already.


pthread.h, sched.h and semaphore.h now use dllexport/dllimport
--------------------------------------------------------------
Not only to avoid the need for the pthread.def file, but to
improve performance. Apparently, declaring functions with dllimport
generates a direct call to the function and avoids the overhead
of a stub function call.

Bug fixes
---------
* Fixed potential NULL pointer dereferences in pthread_mutexattr_init,
pthread_mutexattr_getpshared, pthread_barrierattr_init,
pthread_barrierattr_getpshared, and pthread_condattr_getpshared.
- Scott McCaskill <scott@magruder.org>

* Removed potential race condition in pthread_mutex_trylock and
pthread_mutex_lock;
- Alexander Terekhov <TEREKHOV@de.ibm.com>

* The behaviour of pthread_mutex_trylock in relation to
recursive mutexes was inconsistent with commercial implementations.
Trylock would return EBUSY if the lock was owned already by the
calling thread regardless of mutex type. Trylock now increments the
recursion count and returns 0 for RECURSIVE mutexes, and will
return EDEADLK rather than EBUSY for ERRORCHECK mutexes. This is
consistent with Solaris.
- Thomas Pfaff <tpfaff@gmx.net>

* Found a fix for the library and workaround for applications for
the known bug #2, i.e. where __PTW32_CLEANUP_CXX or __PTW32_CLEANUP_SEH is defined.
See the "Known Bugs in this snapshot" section below.

This could be made transparent to applications by replacing the macros that
define the current C++ and SEH versions of pthread_cleanup_push/pop
with the C version, but AFAIK cleanup handlers would not then run in the
correct sequence with destructors and exception cleanup handlers when
an exception occurs.

* cancellation once started in a thread cannot now be inadvertantly
double canceled. That is, once a thread begins it's cancellation run,
cancellation is disabled and a subsequent cancel request will
return an error (ESRCH).

* errno: An incorrect compiler directive caused a local version
of errno to be used instead of the Win32 errno. Both instances are
thread-safe but applications checking errno after a Pthreads4w
call would be wrong. Fixing this also fixed a bad compiler
option in the testsuite (/MT should have been /MD) which is
needed to link with the correct library MSVCRT.LIB.


SNAPSHOT 2001-07-12
-------------------

To be added


SNAPSHOT 2001-07-03
-------------------

To be added


SNAPSHOT 2000-08-13
-------------------

New:
-       Renamed DLL and LIB files:
                pthreadVSE.dll  (MS VC++/Structured EH)
                pthreadVSE.lib
                pthreadVCE.dll  (MS VC++/C++ EH)
                pthreadVCE.lib
                pthreadGCE.dll  (GNU G++/C++ EH)
                libpthreadw32.a

        Both your application and the pthread dll should use the
        same exception handling scheme.

Bugs fixed:
-       MSVC++ C++ exception handling.

Some new tests have been added.


SNAPSHOT 2000-08-10
-------------------

New:
-       asynchronous cancellation on X86 (Jason Nye)
-       Makefile compatible with MS nmake to replace
        buildlib.bat
-       GNUmakefile for Mingw32
-       tests/Makefile for MS nmake replaces runall.bat
-       tests/GNUmakefile for Mingw32

Bugs fixed:
-       kernel32 load/free problem
-       attempt to hide internel exceptions from application
        exception handlers (__try/__except and try/catch blocks)
-       Win32 thread handle leakage bug
        (David Baggett/Paul Redondo/Eyal Lebedinsky)

Some new tests have been added.


SNAPSHOT 1999-11-02
-------------------

Bugs fixed:
-       ctime_r macro had an incorrect argument (Erik Hensema),
-       threads were not being created 
        PTHREAD_CANCEL_DEFERRED. This should have
        had little effect as deferred is the only
        supported type. (Ross Johnson).

Some compatibility improvements added, eg.
-       pthread_setcancelstate accepts NULL pointer
        for the previous value argument. Ditto for
        pthread_setcanceltype. This is compatible
        with Solaris but should not affect
        standard applications (Erik Hensema)

Some new tests have been added.


SNAPSHOT 1999-10-17
-------------------

Bug fix - cancellation of threads waiting on condition variables
now works properly (Lorin Hochstein and Peter Slacik)


SNAPSHOT 1999-08-12
-------------------

Fixed exception stack cleanup if calling pthread_exit()
- (Lorin Hochstein and John Bossom).

Fixed bugs in condition variables - (Peter Slacik):
        - additional contention checks
        - properly adjust number of waiting threads after timed
          condvar timeout.


SNAPSHOT 1999-05-30
-------------------

Some minor bugs have been fixed. See the ChangeLog file for details.

Some more POSIX 1b functions are now included but ony return an
error (ENOSYS) if called. They are:

        sem_open
        sem_close
        sem_unlink
        sem_getvalue


SNAPSHOT 1999-04-07
-------------------

Some POSIX 1b functions which were internally supported are now
available as exported functions:

        sem_init
        sem_destroy
        sem_wait
        sem_trywait
        sem_post
        sched_yield
        sched_get_priority_min
        sched_get_priority_max

Some minor bugs have been fixed. See the ChangeLog file for details.


SNAPSHOT 1999-03-16
-------------------

Initial release.

