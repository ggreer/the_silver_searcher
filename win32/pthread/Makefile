# This makefile is compatible with MS nmake
# 
# The variables $DLLDEST and $LIBDEST hold the destination directories for the
# dll and the lib, respectively. Probably all that needs to change is $DEVROOT.

# DLL_VER:
# See pthread.h and README for the description of version numbering.
DLL_VER	= 2$(EXTRAVERSION)
DLL_VERD= $(DLL_VER)d

DESTROOT	= ..\PTHREADS-BUILT
DEST_LIB_NAME = pthread.lib

DLLDEST	= $(DESTROOT)\bin
LIBDEST	= $(DESTROOT)\lib
HDRDEST	= $(DESTROOT)\include

DLLS					= pthreadVCE$(DLL_VER).dll pthreadVSE$(DLL_VER).dll pthreadVC$(DLL_VER).dll \
						  pthreadVCE$(DLL_VERD).dll pthreadVSE$(DLL_VERD).dll pthreadVC$(DLL_VERD).dll
INLINED_STATIC_STAMPS	= pthreadVCE$(DLL_VER).inlined_static_stamp pthreadVSE$(DLL_VER).inlined_static_stamp \
						  pthreadVC$(DLL_VER).inlined_static_stamp pthreadVCE$(DLL_VERD).inlined_static_stamp \
						  pthreadVSE$(DLL_VERD).inlined_static_stamp pthreadVC$(DLL_VERD).inlined_static_stamp
SMALL_STATIC_STAMPS		= pthreadVCE$(DLL_VER).small_static_stamp pthreadVSE$(DLL_VER).small_static_stamp \
						  pthreadVC$(DLL_VER).small_static_stamp pthreadVCE$(DLL_VERD).small_static_stamp \
						  pthreadVSE$(DLL_VERD).small_static_stamp pthreadVC$(DLL_VERD).small_static_stamp

CC	= cl
CPPFLAGS = /I. /DHAVE_CONFIG_H
XCFLAGS = /W3 /MD /nologo
CFLAGS	= /O2 /Ob2 $(XCFLAGS)
CFLAGSD	= /Z7 $(XCFLAGS)

# Uncomment this if config.h defines RETAIN_WSALASTERROR
#XLIBS = wsock32.lib

# Default cleanup style
CLEANUP	= __CLEANUP_C

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

OBJEXT = obj
RESEXT = res
 
include common.mk

DLL_OBJS	= $(DLL_OBJS) $(RESOURCE_OBJS)
STATIC_OBJS	= $(STATIC_OBJS) $(RESOURCE_OBJS)

help:
	@ echo Run one of the following command lines:
	@ echo nmake clean all-tests
	@ echo nmake -DEXHAUSTIVE clean all-tests 
	@ echo nmake clean VC
	@ echo nmake clean VC-debug
	@ echo nmake clean VC-static
	@ echo nmake clean VC-static-debug
#	@ echo nmake clean VC-small-static
#	@ echo nmake clean VC-small-static-debug
	@ echo nmake clean VCE
	@ echo nmake clean VCE-debug
	@ echo nmake clean VCE-static
	@ echo nmake clean VCE-static-debug
#	@ echo nmake clean VCE-small-static
#	@ echo nmake clean VCE-small-static-debug
	@ echo nmake clean VSE
	@ echo nmake clean VSE-debug
	@ echo nmake clean VSE-static
	@ echo nmake clean VSE-static-debug
#	@ echo nmake clean VSE-small-static
#	@ echo nmake clean VSE-small-static-debug

all:
	$(MAKE) /E clean VCE
	$(MAKE) /E clean VSE
	$(MAKE) /E clean VC
	$(MAKE) /E clean VCE-debug
	$(MAKE) /E clean VSE-debug
	$(MAKE) /E clean VC-debug

TEST_ENV = CFLAGS="$(CFLAGS) /DNO_ERROR_DIALOGS"

all-tests:
#	$(MAKE) /E realclean VC-small-static$(XDBG)
#	cd tests && $(MAKE) /E clean VC-small-static$(XDBG) $(TEST_ENV) && $(MAKE) /E clean VCX-small-static$(XDBG) $(TEST_ENV)
#	$(MAKE) /E realclean VCE-small-static$(XDBG)
#	cd tests && $(MAKE) /E clean VCE-small-static$(XDBG) $(TEST_ENV)
#	$(MAKE) /E realclean VSE-small-static$(XDBG)
#	cd tests && $(MAKE) /E clean VSE-small-static$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VC$(XDBG)
	cd tests && $(MAKE) /E clean VC$(XDBG) $(TEST_ENV) && $(MAKE) /E clean VCX$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VCE$(XDBG)
	cd tests && $(MAKE) /E clean VCE$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VSE$(XDBG)
	cd tests && $(MAKE) /E clean VSE$(XDBG) $(TEST_ENV)
#!IF DEFINED(EXHAUSTIVE)
	$(MAKE) /E realclean VC-static$(XDBG)
	cd tests && $(MAKE) /E clean VC-static$(XDBG) $(TEST_ENV) && $(MAKE) /E clean VCX-static$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VCE-static$(XDBG)
	cd tests && $(MAKE) /E clean VCE-static$(XDBG) $(TEST_ENV)
	$(MAKE) /E realclean VSE-static$(XDBG)
	cd tests && $(MAKE) /E clean VSE-static$(XDBG) $(TEST_ENV)
#!ENDIF
	$(MAKE) realclean
	@ echo $@ completed successfully.

all-tests-cflags:
	@ -$(SETENV)
	$(MAKE) all-tests XCFLAGS="/W3 /WX /MD /nologo"
	$(MAKE) all-tests XCFLAGS="/W3 /WX /MT /nologo"
!IF DEFINED(MORE_EXHAUSTIVE)
	$(MAKE) all-tests XCFLAGS="/W3 /WX /MDd /nologo" XDBG="-debug"
	$(MAKE) all-tests XCFLAGS="/W3 /WX /MTd /nologo" XDBG="-debug"
!ENDIF
	@ echo $@ completed successfully.

VCE:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGS) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VER).dll

VCE-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGSD) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VERD).dll

VSE:
	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGS) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VER).dll

VSE-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGSD) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VERD).dll

VC:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGS) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VER).dll

VC-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGSD) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VERD).dll

#
# Static builds
#
#VCE-small-static:
#	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGS) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VER).small_static_stamp

#VCE-small-static-debug:
#	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGSD) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VERD).small_static_stamp

#VSE-small-static:
#	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGS) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VER).small_static_stamp

#VSE-small-static-debug:
#	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGSD) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VERD).small_static_stamp

#VC-small-static:
#	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGS) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VER).small_static_stamp

#VC-small-static-debug:
#	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGSD) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VERD).small_static_stamp

VCE-static:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGS) /DPTW32_STATIC_LIB /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VER).inlined_static_stamp

VCE-static-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGSD) /DPTW32_STATIC_LIB /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VERD).inlined_static_stamp

VSE-static:
	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGS) /DPTW32_STATIC_LIB /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VER).inlined_static_stamp

VSE-static-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGSD) /DPTW32_STATIC_LIB /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VERD).inlined_static_stamp

VC-static:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGS) /DPTW32_STATIC_LIB /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VER).inlined_static_stamp

VC-static-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGSD) /DPTW32_STATIC_LIB /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VERD).inlined_static_stamp


realclean: clean
	if exist *.dll del *.dll
	if exist *.lib del *.lib
	if exist *.a del *.a
	if exist *.manifest del *.manifest
	if exist *_stamp del *_stamp
	if exist make.log.txt del make.log.txt
	cd tests && $(MAKE) clean

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

# Very basic install. It assumes "realclean" was done just prior to build target if
# you want the installed $(DEVDEST_LIB_NAME) to match that build.
install:
	if not exist $(DLLDEST) mkdir $(DLLDEST)
	if not exist $(LIBDEST) mkdir $(LIBDEST)
	if not exist $(HDRDEST) mkdir $(HDRDEST)
	if exist pthreadV*.dll copy pthreadV*.dll $(DLLDEST)
	copy pthreadV*.lib $(LIBDEST)
	copy _ptw32.h $(HDRDEST)
	copy pthread.h $(HDRDEST)
	copy sched.h $(HDRDEST)
	copy semaphore.h $(HDRDEST)
	if exist pthreadVC$(DLL_VER).lib copy pthreadVC$(DLL_VER).lib $(LIBDEST)\$(DEST_LIB_NAME)
	if exist pthreadVC$(DLL_VERD).lib copy pthreadVC$(DLL_VERD).lib $(LIBDEST)\$(DEST_LIB_NAME)
	if exist pthreadVCE$(DLL_VER).lib copy pthreadVCE$(DLL_VER).lib $(LIBDEST)\$(DEST_LIB_NAME)
	if exist pthreadVCE$(DLL_VERD).lib copy pthreadVCE$(DLL_VERD).lib $(LIBDEST)\$(DEST_LIB_NAME)
	if exist pthreadVSE$(DLL_VER).lib copy pthreadVSE$(DLL_VER).lib $(LIBDEST)\$(DEST_LIB_NAME)
	if exist pthreadVSE$(DLL_VERD).lib copy pthreadVSE$(DLL_VERD).lib $(LIBDEST)\$(DEST_LIB_NAME)

$(DLLS): $(DLL_OBJS)
	$(CC) /LDd /Zi /nologo $(DLL_OBJS) /link /implib:$*.lib $(XLIBS) /out:$@

$(INLINED_STATIC_STAMPS): $(DLL_OBJS)
	if exist $*.lib del $*.lib
	lib $(DLL_OBJS) /out:$*.lib
	echo. >$@

$(SMALL_STATIC_STAMPS): $(STATIC_OBJS)
	if exist $*.lib del $*.lib
	lib $(STATIC_OBJS) /out:$*.lib
	echo. >$@

.c.obj:
	$(CC) $(EHFLAGS) /D$(CLEANUP) -c $<

# TARGET_CPU is an environment variable set by Visual Studio Command Prompt
# as provided by the SDK (VS 2010 Express plus SDK 7.1)
# PLATFORM is an environment variable that may be set in the VS 2013 Express x64 cross
# development environment
# On my HP Compaq PC running VS 10, PLATFORM was defined as "HPD" but PROCESSOR_ARCHITECTURE
# was defined as "x86"
.rc.res:
!IF DEFINED(PLATFORM)
!  IF DEFINED(PROCESSOR_ARCHITECTURE)
	  rc /dPTW32_ARCH$(PROCESSOR_ARCHITECTURE) /dPTW32_RC_MSC /d$(CLEANUP) $<
!  ELSE
	  rc /dPTW32_ARCH$(PLATFORM) /dPTW32_RC_MSC /d$(CLEANUP) $<
!  ENDIF
!ELSE IF DEFINED(TARGET_CPU)
	rc /dPTW32_ARCH$(TARGET_CPU) /dPTW32_RC_MSC /d$(CLEANUP) $<
!ELSE
	rc /dPTW32_ARCHx86 /dPTW32_RC_MSC /d$(CLEANUP) $<
!ENDIF

.c.i:
	$(CC) /P /O2 /Ob1 $(VCFLAGS) $<
