# This makefile is compatible with MS nmake
# 
# The variables $DLLDEST and $LIBDEST hold the destination directories for the
# dll and the lib, respectively. Probably all that needs to change is $DEVROOT.

# PTW32_VER:
# See pthread.h and README for the description of version numbering.
PTW32_VER	= 3$(EXTRAVERSION)
PTW32_VER_DEBUG= $(PTW32_VER)d

DESTROOT	= ..\PTHREADS-BUILT

DLLDEST	= $(DESTROOT)\bin
LIBDEST	= $(DESTROOT)\lib
HDRDEST	= $(DESTROOT)\include

DLLS					= pthreadVCE$(PTW32_VER).dll pthreadVSE$(PTW32_VER).dll pthreadVC$(PTW32_VER).dll \
						  pthreadVCE$(PTW32_VER_DEBUG).dll pthreadVSE$(PTW32_VER_DEBUG).dll pthreadVC$(PTW32_VER_DEBUG).dll
INLINED_STATIC_STAMPS	= pthreadVCE$(PTW32_VER).inlined_static_stamp pthreadVSE$(PTW32_VER).inlined_static_stamp \
						  pthreadVC$(PTW32_VER).inlined_static_stamp pthreadVCE$(PTW32_VER_DEBUG).inlined_static_stamp \
						  pthreadVSE$(PTW32_VER_DEBUG).inlined_static_stamp pthreadVC$(PTW32_VER_DEBUG).inlined_static_stamp
SMALL_STATIC_STAMPS		= pthreadVCE$(PTW32_VER).small_static_stamp pthreadVSE$(PTW32_VER).small_static_stamp \
						  pthreadVC$(PTW32_VER).small_static_stamp pthreadVCE$(PTW32_VER_DEBUG).small_static_stamp \
						  pthreadVSE$(PTW32_VER_DEBUG).small_static_stamp pthreadVC$(PTW32_VER_DEBUG).small_static_stamp

CC	= cl /errorReport:none /nologo
CPPFLAGS = /I. /DHAVE_CONFIG_H
XCFLAGS = 
CFLAGS	= /W3 /O2 /Ob2 $(XCFLAGS)
CFLAGSD	= /W3 /Z7 $(XCFLAGS)

# Uncomment this if config.h defines RETAIN_WSALASTERROR
#XLIBS = wsock32.lib

# Default cleanup style
CLEANUP	= __PTW32_CLEANUP_C

# C++ Exceptions
# (Note: If you are using Microsoft VC++6.0, the library needs to be built
# with /EHa instead of /EHs or else cancellation won't work properly.)
VCEFLAGS	= /EHs /TP $(CPPFLAGS) $(CFLAGS)
VCEFLAGSD	= /EHs /TP $(CPPFLAGS) $(CFLAGSD)
#Structured Exceptions
VSEFLAGS	= $(CPPFLAGS) $(CFLAGS)
VSEFLAGSD	= $(CPPFLAGS) $(CFLAGSD)
#C cleanup code
VCFLAGS		= $(CPPFLAGS) $(CFLAGS)
VCFLAGSD	= $(CPPFLAGS) $(CFLAGSD)

OBJEXT	= obj
OEXT	= o
RESEXT	= res
 
include common.mk

DLL_OBJS			= $(DLL_OBJS) $(RESOURCE_OBJS)
STATIC_OBJS			= $(STATIC_OBJS) $(RESOURCE_OBJS)
STATIC_OBJS_SMALL	= $(STATIC_OBJS_SMALL) $(RESOURCE_OBJS)

help:
	@ echo To just build all possible versions and install them in $(DESTROOT)
	@ echo nmake all install
	@ echo ------------------------------------------
	@ echo Or run one of the following command lines:
	@ echo nmake clean all-tests
	@ echo nmake -DEXHAUSTIVE clean all-tests
	@ echo nmake clean all-tests-md
	@ echo nmake clean all-tests-mt
	@ echo nmake clean VC
	@ echo nmake clean VC-debug
	@ echo nmake clean VC-static
	@ echo nmake clean VC-static-debug
	@ echo nmake clean VCE
	@ echo nmake clean VCE-debug
	@ echo nmake clean VCE-static
	@ echo nmake clean VCE-static-debug
	@ echo nmake clean VSE
	@ echo nmake clean VSE-debug
	@ echo nmake clean VSE-static
	@ echo nmake clean VSE-static-debug

all:
	$(MAKE) /E clean VC-static
	$(MAKE) /E clean VCE-static
	$(MAKE) /E clean VSE-static
	$(MAKE) /E clean VC-static-debug
	$(MAKE) /E clean VCE-static-debug
	$(MAKE) /E clean VSE-static-debug
	$(MAKE) /E clean VC
	$(MAKE) /E clean VCE
	$(MAKE) /E clean VSE
	$(MAKE) /E clean VC-debug
	$(MAKE) /E clean VCE-debug
	$(MAKE) /E clean VSE-debug
	$(MAKE) /E clean

TEST_ENV = CFLAGS="$(CFLAGS) /DNO_ERROR_DIALOGS"

all-tests:
	$(MAKE) all-tests-md all-tests-mt

all-tests-dll:
	$(MAKE) /E realclean VC$(XDBG)
	cd tests && $(MAKE) /E clean VC$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VCE$(XDBG)
	cd tests && $(MAKE) /E clean VCE$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VSE$(XDBG)
	cd tests && $(MAKE) /E clean VSE$(XDBG) $(TEST_ENV)

all-tests-static:
	$(MAKE) /E realclean VC-static$(XDBG)
	cd tests && $(MAKE) /E clean VC-static$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VCE-static$(XDBG)
	cd tests && $(MAKE) /E clean VCE-static$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VSE-static$(XDBG)
	cd tests && $(MAKE) /E clean VSE-static$(XDBG) $(TEST_ENV)
	$(MAKE) realclean
	@ echo $@ completed successfully.

all-tests-md:
	@ -$(SETENV)
	$(MAKE) all-tests-dll
!IF DEFINED(EXHAUSTIVE)
	$(MAKE) all-tests-dll XDBG="-debug"
!ENDIF
	@ echo $@ completed successfully.

all-tests-mt:
	@ -$(SETENV)
	$(MAKE) all-tests-static
!IF DEFINED(EXHAUSTIVE)
	$(MAKE) all-tests-static XDBG="-debug"
!ENDIF
	@ echo $@ completed successfully.

VCE:
	@ $(MAKE) /E /nologo XCFLAGS="/MD" EHFLAGS="$(VCEFLAGS) /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_CXX pthreadVCE$(PTW32_VER).dll

VCE-debug:
	@ $(MAKE) /E /nologo XCFLAGS="/MDd" EHFLAGS="$(VCEFLAGSD) /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_CXX pthreadVCE$(PTW32_VER_DEBUG).dll

VSE:
	@ $(MAKE) /E /nologo XCFLAGS="/MD" EHFLAGS="$(VSEFLAGS) /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_SEH pthreadVSE$(PTW32_VER).dll

VSE-debug:
	@ $(MAKE) /E /nologo XCFLAGS="/MDd" EHFLAGS="$(VSEFLAGSD) /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_SEH pthreadVSE$(PTW32_VER_DEBUG).dll

VC:
	@ $(MAKE) /E /nologo XCFLAGS="/MD" EHFLAGS="$(VCFLAGS) /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_C pthreadVC$(PTW32_VER).dll

VC-debug:
	@ $(MAKE) /E /nologo XCFLAGS="/MDd" EHFLAGS="$(VCFLAGSD) /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_C pthreadVC$(PTW32_VER_DEBUG).dll

#
# Static builds
#
#VCE-small-static:
#	@ $(MAKE) /E /nologo XCFLAGS="/MT" EHFLAGS="$(VCEFLAGS) /D__PTW32_STATIC_LIB" CLEANUP=__PTW32_CLEANUP_CXX pthreadVCE$(PTW32_VER).small_static_stamp

#VCE-small-static-debug:
#	@ $(MAKE) /E /nologo XCFLAGS="/MTd" EHFLAGS="$(VCEFLAGSD) /D__PTW32_STATIC_LIB" CLEANUP=__PTW32_CLEANUP_CXX pthreadVCE$(PTW32_VER_DEBUG).small_static_stamp

#VSE-small-static:
#	@ $(MAKE) /E /nologo XCFLAGS="/MT" EHFLAGS="$(VSEFLAGS) /D__PTW32_STATIC_LIB" CLEANUP=__PTW32_CLEANUP_SEH pthreadVSE$(PTW32_VER).small_static_stamp

#VSE-small-static-debug:
#	@ $(MAKE) /E /nologo XCFLAGS="/MTd" EHFLAGS="$(VSEFLAGSD) /D__PTW32_STATIC_LIB" CLEANUP=__PTW32_CLEANUP_SEH pthreadVSE$(PTW32_VER_DEBUG).small_static_stamp

#VC-small-static:
#	@ $(MAKE) /E /nologo XCFLAGS="/MT" EHFLAGS="$(VCFLAGS) /D__PTW32_STATIC_LIB" CLEANUP=__PTW32_CLEANUP_C pthreadVC$(PTW32_VER).small_static_stamp

#VC-small-static-debug:
#	@ $(MAKE) /E /nologo XCFLAGS="/MTd" EHFLAGS="$(VCFLAGSD) /D__PTW32_STATIC_LIB" CLEANUP=__PTW32_CLEANUP_C pthreadVC$(PTW32_VER_DEBUG).small_static_stamp

VCE-static:
	@ $(MAKE) /E /nologo XCFLAGS="/MT" EHFLAGS="$(VCEFLAGS) /D__PTW32_STATIC_LIB /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_CXX pthreadVCE$(PTW32_VER).inlined_static_stamp

VCE-static-debug:
	@ $(MAKE) /E /nologo XCFLAGS="/MTd" EHFLAGS="$(VCEFLAGSD) /D__PTW32_STATIC_LIB /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_CXX pthreadVCE$(PTW32_VER_DEBUG).inlined_static_stamp

VSE-static:
	@ $(MAKE) /E /nologo XCFLAGS="/MT" EHFLAGS="$(VSEFLAGS) /D__PTW32_STATIC_LIB /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_SEH pthreadVSE$(PTW32_VER).inlined_static_stamp

VSE-static-debug:
	@ $(MAKE) /E /nologo XCFLAGS="/MTd" EHFLAGS="$(VSEFLAGSD) /D__PTW32_STATIC_LIB /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_SEH pthreadVSE$(PTW32_VER_DEBUG).inlined_static_stamp

VC-static:
	@ $(MAKE) /E /nologo XCFLAGS="/MT" EHFLAGS="$(VCFLAGS) /D__PTW32_STATIC_LIB /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_C pthreadVC$(PTW32_VER).inlined_static_stamp

VC-static-debug:
	@ $(MAKE) /E /nologo XCFLAGS="/MTd" EHFLAGS="$(VCFLAGSD) /D__PTW32_STATIC_LIB /D__PTW32_BUILD_INLINED" CLEANUP=__PTW32_CLEANUP_C pthreadVC$(PTW32_VER_DEBUG).inlined_static_stamp


realclean: clean
	if exist *.dll del *.dll
	if exist *.lib del *.lib
	if exist *.a del *.a
	if exist *.manifest del *.manifest
	if exist make.log.txt del make.log.txt
	cd tests && $(MAKE) realclean

clean:
	if exist *.obj del *.obj
	if exist *.def del *.def
	if exist *.ilk del *.ilk
	if exist *.pdb del *.pdb
	if exist *.exp del *.exp
	if exist *.map del *.map
	if exist *.o del *.o
	if exist *.i del *.i
	if exist *.res del *.res
	if exist *_stamp del *_stamp
	cd tests && $(MAKE) clean

# Very basic install. It assumes "realclean" was done just prior to build target.
install:
	if not exist $(DLLDEST) mkdir $(DLLDEST)
	if not exist $(LIBDEST) mkdir $(LIBDEST)
	if not exist $(HDRDEST) mkdir $(HDRDEST)
	if exist pthreadV*.dll copy pthreadV*.dll $(DLLDEST)
	copy pthreadV*.lib $(LIBDEST)
	copy libpthreadV*.lib $(LIBDEST)
	copy _ptw32.h $(HDRDEST)
	copy pthread.h $(HDRDEST)
	copy sched.h $(HDRDEST)
	copy semaphore.h $(HDRDEST)

$(DLLS): $(DLL_OBJS)
	$(CC) /LDd /Zi $(DLL_OBJS) /link /implib:$*.lib $(XLIBS) /out:$@

$(INLINED_STATIC_STAMPS): $(STATIC_OBJS)
	if exist lib$*.lib del lib$*.lib
	lib $(STATIC_OBJS) /out:lib$*.lib
	echo. >$@

$(SMALL_STATIC_STAMPS): $(STATIC_OBJS_SMALL)
	if exist lib$*.lib del lib$*.lib
	lib $(STATIC_OBJS_SMALL) /out:lib$*.lib
	echo. >$@

.c.obj:
	$(CC) $(XCFLAGS) $(EHFLAGS) /D$(CLEANUP) -c $<

.c.o:
	$(CC) $(XCFLAGS) $(EHFLAGS) /D$(CLEANUP) /Fo$*.$(OEXT) -c $<

# TARGET_CPU is an environment variable set by Visual Studio Command Prompt
# as provided by the SDK (VS 2010 Express plus SDK 7.1)
# PLATFORM is an environment variable that may be set in the VS 2013 Express x64 cross
# development environment
# On my HP Compaq PC running VS 10, PLATFORM was defined as "HPD" but PROCESSOR_ARCHITECTURE
# was defined as "x86"
.rc.res:
!IF DEFINED(PLATFORM)
!  IF DEFINED(PROCESSOR_ARCHITECTURE)
	  rc /d__PTW32_ARCH$(PROCESSOR_ARCHITECTURE) /d__PTW32_RC_MSC /d$(CLEANUP) $<
!  ELSE
	  rc /d__PTW32_ARCH$(PLATFORM) /d__PTW32_RC_MSC /d$(CLEANUP) $<
!  ENDIF
!ELSE IF DEFINED(TARGET_CPU)
	rc /d__PTW32_ARCH$(TARGET_CPU) /d__PTW32_RC_MSC /d$(CLEANUP) $<
!ELSE
	rc /d__PTW32_ARCHx86 /d__PTW32_RC_MSC /d$(CLEANUP) $<
!ENDIF

.c.i:
	$(CC) /P /O2 /Ob1 $(VCFLAGS) $<
