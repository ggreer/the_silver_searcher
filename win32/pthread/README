PTHREADS4W (a.k.a. PTHREADS-WIN32)
==================================

What is it?
-----------

Pthreads4w is an Open Source Software implementation of the Threads
component of the POSIX 1003.1c 1995 Standard (or later) for Microsoft's
Windows environment. Some functions from POSIX 1003.1b are also supported,
including semaphores. Other related functions include the set of read-write
lock functions. The library also supports some of the functionality of the
Open Group's Single Unix specification, namely mutex types, plus some common
and pthreads4w specific non-portable routines (see README.NONPORTABLE).

See the file "ANNOUNCE" for more information including standards
conformance details and the list of supported and unsupported
routines.


Prerequisites
-------------
MSVC or GNU C (MinGW or MinGW64 with AutoConf Tools)
	To build from source.

QueueUserAPCEx by Panagiotis E. Hadjidoukas
	To support any thread cancellation in C++ library builds or
	to support cancellation of blocked threads in any build.
	This library is not required otherwise.

	For true async cancellation of threads (including blocked threads).
	This is a DLL and Windows driver that provides pre-emptive APC
	by forcing threads into an alertable state when the APC is queued.
	Both the DLL and driver are provided with the pthreads4w.exe
	self-unpacking ZIP, and on the pthreads4w FTP site  (in source
	and pre-built forms). Currently this is a separate LGPL package to
	pthreads4w. See the README in the QueueUserAPCEx folder for
	installation instructions.

	pthreads4w will automatically detect if the QueueUserAPCEx DLL
	QuserEx.DLL is available and whether the driver AlertDrv.sys is
	loaded. If it is not available, pthreads4w will simulate async
	cancellation, which means that it can async cancel only threads that
	are runnable. The simulated async cancellation cannot cancel blocked
	threads.

    [FOR SECURITY] To be found Quserex.dll MUST be installed in the
	Windows System Folder. This is not an unreasonable constraint given a
	driver must also be installed and loaded at system startup.


Library naming
--------------

Because the library is being built using various exception
handling schemes and compilers - and because the library
may not work reliably if these are mixed in an application,
each different version of the library has it's own name.

Please do not distribute your own modified versions of the library
using names conforming to this description. You can use the
makefile variable "EXTRAVERSION" to append your own suffix to the
library names when building and testing your library.

Note 1: the incompatibility is really between EH implementations
of the different compilers. It should be possible to use the
standard C version from either compiler with C++ applications
built with a different compiler. If you use an EH version of
the library, then you must use the same compiler for the
application. This is another complication and dependency that
can be avoided by using only the standard C library version.

Note 2: if you use a standard C pthread*.dll with a C++
application, then any functions that you define that are
intended to be called via pthread_cleanup_push() must be
__cdecl.

Note 3: the intention was to also name either the VC or GC
version (it should be arbitrary) as pthread.dll, including
pthread.lib and libpthread.a as appropriate. This is no longer
likely to happen.

Note 4: the compatibility number (major version number) was
added so that applications can differentiate between binary
incompatible versions of the libs and dlls.

In general the naming format used is:
    pthread[VG]{SE,CE,C}[c][E].dll
    pthread[VG]{SE,CE,C}[c][E].lib

where:
    [VG] indicates the compiler
        V	- MS VC, or
        G	- GNU C

    {SE,CE,C} indicates the exception handling scheme
        SE	- Structured EH, or
        CE	- C++ EH, or
        C	- no exceptions - uses setjmp/longjmp

        c	- DLL major version number indicating ABI
              compatibility with applications built against
              a snapshot with the same major version number.
              See 'Version numbering' below.
        E	- EXTRAVERSION suffix.

The name may also be suffixed by a 'd' to indicate a debugging version
of the library. E.g. pthreadVC2d.lib. These will be created e.g. when
the *-debug makefile targets are used.

Examples:
	pthreadVC2.dll	    (MSVC/not dependent on exceptions - not binary
                         compatible with pthreadVC1.dll or pthreadVC.dll)
    pthreadGC2-w32.dll  (As built, e.g., by "make GC ARCH=-m32 EXTRAVERSION=-w32")
    pthreadVC2-w64.dll  (As built, e.g., by "nmake VC ARCH=-m64 EXTRAVERSION=-w64")

For information on ARCH (MinGW GNUmakefile) or TARGET_CPU (MSVS Makefile)
see the respective "Building with ..." sections below.

The GNU library archive file names have correspondingly changed, e.g.:

	libpthreadGCE2.a
	libpthreadGC2.a
	libpthreadGC2-w64.a


Version numbering
-----------------

See pthread.h and the resource file 'version.rc'.

Microsoft version numbers use 4 integers:

	0.0.0.0

Pthreads4w uses the first 3 following the standard major.minor.micro
system. We had claimed to follow the Libtool convention but this has
not been the case with recent releases. Binary compatibility and
consequently library file naming has not changed over this time either
so it should not cause any problems.

NOTE: Changes to the platform ABI can cause the library ABI to change
and the current version numbering system does not account for this.

The fourth is commonly used for the build number, but will be reserved
for future use.

	major.minor.micro.0

The numbers are changed as follows:

1. If the general binary interface (ABI) has changed at all since the
   last update in a way that requires recompilation and relinking of
   applications, then increment Major, and set both minor and micro to 0.
   (`M:m:u' becomes `M+1:0:0')
2. If the general API has changed at all since the last update or
   there have been semantic/behaviour changes (bug fixes etc) but does
   not require recompilation of existing applications, then increment
   minor and set micro to 0.
   (`M:m:u' becomes `M:m+1:0')
3. If there have been no interface or semantic changes since the last
   public release but a new release is deemed necessary for some reason,
   then increment micro.
   (`M:m:u' becomes `M:m:u+1')


DLL compatibility numbering is an attempt to ensure that applications
always load a compatible pthreads4w DLL by using a DLL naming system
that is consistent with the version numbering system. It also allows
older and newer DLLs to coexist in the same filesystem so that older
applications can continue to be used. For pre .NET Windows systems,
this inevitably requires incompatible versions of the same DLLs to have
different names.

Pthreads4w has adopted the Cygwin convention of appending a single
integer number to the DLL name. The number used is simply the library's
major version number.

Consequently, DLL name/s will only change when the DLL's
backwards compatibility changes. Note that the addition of new
'interfaces' will not of itself change the DLL's compatibility for older
applications.


Which of the several dll versions to use?
-----------------------------------------
or,
---
What are all these pthread*.dll and pthread*.lib files?
-------------------------------------------------------

Simple, use either pthreadGCc.* if you use GCC, or pthreadVCc.* if you
use MSVC - where 'c' is the DLL versioning (compatibility) number.

Otherwise, you need to choose carefully and know WHY.

The most important choice you need to make is whether to use a
version that uses exceptions internally, or not. There are versions
of the library that use exceptions as part of the thread
cancellation and exit implementation. The default version uses
setjmp/longjmp.

If you use either pthreadVCE[2] or pthreadGCE[2]:

1. [See also the discussion in the FAQ file - Q2, Q4, and Q5]

If your application contains catch(...) blocks in your POSIX
threads then you will need to replace the "catch(...)" with the macro
"__PtW32Catch", eg.

	#ifdef __PtW32Catch
		__PtW32Catch {
			...
		}
	#else
		catch(...) {
			...
		}
	#endif

Otherwise neither pthreads cancellation nor pthread_exit() will work
reliably when using versions of the library that use C++ exceptions
for cancellation and thread exit.

NB: [lib]pthreadGCE[2] does not support asynchronous cancellation. Any
attempt to cancel a thread set for asynchronous cancellation using
this version of the library will cause the applicaton to terminate.
We believe this is due to the "unmanaged" context switch that is
disrupting the stack unwinding mechanism and which is used
to cancel blocked threads. See pthread_cancel.c


Other name changes
------------------

All snapshots prior to and including snapshot 2000-08-13
used "_pthread_" as the prefix to library internal
functions, and "_PTHREAD_" to many library internal
macros. These have now been changed to "__ptw32_" and "__PTW32_"
respectively so as to not conflict with the ANSI standard's
reservation of identifiers beginning with "_" and "__" for
use by compiler implementations only.

If you have written any applications and you are linking
statically with the pthreads4w library then you may have
included a call to _pthread_processInitialize. You will
now have to change that to __ptw32_processInitialize.


Cleanup code default style
--------------------------

Previously, if not defined, the cleanup style was determined automatically
from the compiler used, and one of the following was defined accordingly:

	__PTW32_CLEANUP_SEH	MSVC only
	__PTW32_CLEANUP_CXX	C++, including MSVC++, GNU G++
	__PTW32_CLEANUP_C	C, including GNU GCC, not MSVC

These defines determine the style of cleanup (see pthread.h) and,
most importantly, the way that cancellation and thread exit (via
pthread_exit) is performed (see the routine __ptw32_throw()).

In short, the exceptions versions of the library throw an exception
when a thread is canceled, or exits via pthread_exit(). This exception is
caught by a handler in the thread startup routine, so that the
the correct stack unwinding occurs regardless of where the thread
is when it's canceled or exits via pthread_exit().

In this snapshot, unless the build explicitly defines (e.g. via a
compiler option) __PTW32_CLEANUP_SEH, __PTW32_CLEANUP_CXX, or __PTW32_CLEANUP_C, then
the build NOW always defaults to __PTW32_CLEANUP_C style cleanup. This style
uses setjmp/longjmp in the cancellation and pthread_exit implementations,
and therefore won't do stack unwinding even when linked to applications
that have it (e.g. C++ apps). This is for consistency with most/all
commercial Unix POSIX threads implementations.

Although it was not clearly documented before, it is still necessary to
build your application using the same __PTW32_CLEANUP_* define as was
used for the version of the library that you link with, so that the
correct parts of pthread.h are included. That is, the possible
defines require the following library versions:

	__PTW32_CLEANUP_SEH	pthreadVSE.dll
	__PTW32_CLEANUP_CXX	pthreadVCE.dll or pthreadGCE.dll
	__PTW32_CLEANUP_C	pthreadVC.dll or pthreadGC.dll

It is recommended that you let pthread.h use it's default __PTW32_CLEANUP_C
for both library and application builds. That is, don't define any of
the above, and then link with pthreadVC.lib (MSVC or MSVC++) and
libpthreadGC.a (MinGW GCC or G++). The reason is explained below, but
another reason is that the prebuilt pthreadVCE.dll is currently broken.
Versions built with MSVC++ later than version 6 may not be broken, but I
can't verify this yet.

WHY ARE WE MAKING THE DEFAULT STYLE LESS EXCEPTION-FRIENDLY?
Because no commercial Unix POSIX threads implementation allows you to
choose to have stack unwinding. Therefore, providing it in pthread-win32
as a default is dangerous. We still provide the choice but unless
you consciously choose to do otherwise, your pthreads applications will
now run or crash in similar ways irrespective of the pthreads platform
you use. Or at least this is the hope.


Development Build Toolchains and Configurations
-----------------------------------------------

As of Release 2.10 all build configurations pass the full test suite
for the following toolchains and configurations:

All DLL and static library build targets enabled in the makefiles:
VC, VCE, VSE (DLL, inlined statics only)
GC, GCE (DLL, inlined and small statics)

MSVS:
Intel Core i7 (6 Core HT)
Windows 7 64 bit
MSVS 2010 Express with SDK 7.1 (using the SDK command shell TARGET_CPU = x64 or x86)
MSVS 2013 Express Cross Tools for x64 Command Prompt
MSVS 2013 Express Native Tools for x32 Command Prompt

GNU:
Intel Core i7 (6 Core HT)
Windows 7 64 bit
MinGW64 multilib enabled (ARCH = -m64 or -m32)
MinGW64 multilib disabled


Building with MS Visual Studio (C, VC++ using C++ EH, or Structured EH)
-----------------------------------------------------------------------

NOTE: A VS project/solution/whatever file is included as a contributed
work and is not used of maintained in development. All building and
testing is done using makefiles. We use the native make system for each
toolchain, which is 'nmake' in this case.

From the source directory run nmake without any arguments to list
help information. E.g.

$ nmake

As examples, as at Release 2.10 the pre-built DLLs and static libraries
can be built using one of the following command-lines:

[Note: "setenv" comes with the SDK which is not required to build the library.
I use it to build and test both 64 and 32 bit versions of the library.
"/2003" is used to override my build system which is Win7 (at the time of
writing) for backwards compatibility.]

$ setenv /x64 /2003 /Release
$ nmake realclean VC
$ nmake realclean VCE
$ nmake realclean VSE
$ nmake realclean VC-static
$ nmake realclean VCE-static
$ nmake realclean VSE-static
$ setenv /x86 /2003 /Release
$ nmake realclean VC
$ nmake realclean VCE
$ nmake realclean VSE
$ nmake realclean VC-static
$ nmake realclean VCE-static
$ nmake realclean VSE-static

If you want to differentiate or customise library naming you can use,
e.g.:

$ nmake realclean VC EXTRAVERSION="-w64"

The string provided via the variable EXTRAVERSION is appended to the dll
and .lib library names, e.g.:

pthreadVC2-w64.dll
pthreadVC2-w64.lib

To build and test all DLLs and static lib compatibility versions
(VC, VCE, VSE):

$ setenv /x64 /2003 /release
$ nmake all-tests

You can run the testsuite by changing to the "tests" directory and
running nmake. E.g.:

$ cd tests
$ nmake VC

Note: the EXTRAVERSION="..." option is passed to the tests Makefile
when you target "all-tests". If you build the library then change to the
tests directory to run the tests you will need to repeat the option
explicitly to the test "nmake" command-line.

For failure analysis etc. individual tests can be built
and run, e.g:

$ cd tests
$ nmake VC TESTS="foo bar"

This builds and runs all prerequisite tests as well as the individual
tests listed. Prerequisite tests are defined in tests\runorder.mk.

To build and run only the tests listed use:

$ cd tests
$ nmake VC NO_DEPS=1 TESTS="foo bar"


Building with MinGW
-------------------

NOTE: All building and testing is done using makefiles. We use the native
make system for each toolchain, which is 'make' in this case.

We have found that Mingw builds of the GCE library variants can fail when
run on 64 bit systems, believed to be due to the DWARF2 exception handling
being a 32 bit mechanism. The GC variants are fine. MinGW64 offers
SJLJ or SEH exception handling so choose one of those.

From the source directory:

run 'autoheader' to rewrite the config.h file
run 'autoconf' to rewrite the GNUmakefiles (library and tests)
run './configure' to create config.h and GNUmakefile.
run 'make' without arguments to list possible targets.

E.g.

$ autoheader
$ autoconf
$ ./configure
$ make realclean all-tests

With MinGW64 multilib installed the following variables can be defined
either on the make command line or in the shell environment:

ARCH
 - possible values are "-m64" and "-m32". You will probably recognise
   these as gcc flags however the GNUmakefile also converts these into
   the appropriate windres options when building version.o.

As examples, as at Release 2.10 the pre-built DLLs and static libraries
are built from the following command-lines:

$ nmake realclean GC ARCH=-m64
$ nmake realclean GC ARCH=-m32
$ nmake realclean GCE ARCH=-m64
$ nmake realclean GCE ARCH=-m32
$ nmake realclean GC-static ARCH=-m64
$ nmake realclean GC-static ARCH=-m32
$ nmake realclean GCE-static ARCH=-m64
$ nmake realclean GCE-static ARCH=-m32

If you want to differentiate between libraries by their names you can use,
e.g.:

$ make realclean GC ARCH="-m64" EXTRAVERSION="-w64"

The string provided via the variable EXTRAVERSION is appended to the dll
and .a library names, e.g.:

pthreadGC2-w64.dll
libpthreadGC2-w64.a

To build and test all DLLs and static lib compatibility variants (GC, GCE):

$ make all-tests
or, with MinGW64 (multilib enabled):
$ make all-tests ARCH=-m64
$ make all-tests ARCH=-m32

You can run the testsuite by changing to the "tests" directory and
running make. E.g.:

$ cd tests
$ make GC

Note that the ARCH="..." and/or EXTRAVERSION="..." options are passed to the
tests GNUmakefile when you target "all-tests". If you change to the tests
directory and run the tests you will need to repeat those options explicitly
to the test "make" command-line.

For failure analysis etc. individual tests can be built and run, e.g:

$ cd tests
$ make GC TESTS="foo bar"

This builds and runs all prerequisite tests as well as the individual
tests listed. Prerequisite tests are defined in tests\runorder.mk.

To build and run only those tests listed use:

$ cd tests
$ make GC NO_DEPS=1 TESTS="foo bar"


Building under Linux using the MinGW cross development tools
------------------------------------------------------------

You can build the library on Linux by using the MinGW cross development
toolchain. See http://www.libsdl.org/extras/win32/cross/ for tools and
info. The GNUmakefile contains some support for this, for example:

make CROSS=i386-mingw32msvc- clean GC

will build pthreadGCn.dll and libpthreadGCn.a (n=version#), provided your
cross-tools/bin directory is in your PATH (or use the cross-make.sh script
at the URL above).


Building the library as a statically linkable library
-----------------------------------------------------

General:  __PTW32_STATIC_LIB must be defined for both the library build and the
application build. The makefiles supplied and used by the following 'make'
command lines will define this for you.

MSVC (creates pthreadVCn.lib as a static link lib):

nmake clean VC-static


MinGW32 (creates libpthreadGCn.a as a static link lib):

make clean GC-static

Define  __PTW32_STATIC_LIB also when building your application.

Building the library under Cygwin
---------------------------------

Cygwin implements it's own POSIX threads routines and these
will be the ones to use if you develop using Cygwin.

Building applications
---------------------

The files you will need for your application build are:

The four header files:
	_ptw32.h
	pthread.h
	semaphore.h
	sched.h

The DLL library files that you built:
	pthread*.dll
	plus the matching *.lib (MSVS) or *.a file (GNU)

or, the static link library that you built:
	pthread*.lib (MSVS) or libpthread*.a (GNU)

Place them in the appropriate directories for your build, which may be the
standard compiler locations or, locations specific to your project (you
might have a separate third-party dependency tree for example).

Acknowledgements
----------------

See the ANNOUNCE file for acknowledgements.
See the 'CONTRIBUTORS' file for the list of contributors.

As much as possible, the ChangeLog file attributes
contributions and patches that have been incorporated
in the library to the individuals responsible.

Finally, thanks to all those who work on and contribute to the
POSIX and Single Unix Specification standards. The maturity of an
industry can be measured by it's open standards.

----
Ross Johnson
<ross.johnson@loungebythelake.net>
