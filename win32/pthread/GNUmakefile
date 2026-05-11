#
# --------------------------------------------------------------------------
#
#      pthreads-win32 - POSIX Threads Library for Win32
#      Copyright(C) 1998 John E. Bossom
#      Copyright(C) 1999-2021 pthreads-win32 / pthreads4w contributors
# 
#      The current list of contributors is contained
#      in the file CONTRIBUTORS included with the source
#      code distribution. The list can also be seen at the
#      following World Wide Web location:
#      http://sources.redhat.com/pthreads-win32/contributors.html
# 
#      This library is free software; you can redistribute it and/or
#      modify it under the terms of the GNU Lesser General Public
#      License as published by the Free Software Foundation; either
#      version 2 of the License, or (at your option) any later version.
# 
#      This library is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#      Lesser General Public License for more details.
# 
#      You should have received a copy of the GNU Lesser General Public
#      License along with this library in the file COPYING.LIB;
#      if not, write to the Free Software Foundation, Inc.,
#      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
#
# --------------------------------------------------------------------------
#

DLL_VER	= 2$(EXTRAVERSION)

# See pthread.h and README for the description of version numbering.
DLL_VERD= $(DLL_VER)d

DESTROOT	= ../PTHREADS-BUILT
DEST_LIB_NAME = libpthread.a
DLLDEST	= $(DESTROOT)/bin
LIBDEST	= $(DESTROOT)/lib
HDRDEST	= $(DESTROOT)/include

# If Running MsysDTK
RM	= rm -f
MV	= mv -f
CP	= cp -f
MKDIR	= mkdir -p
ECHO	= echo
TESTNDIR = test ! -d
TESTFILE = test -f
AND	= &&

# If not.
#RM	= erase
#MV	= rename
#CP	= copy
#MKDIR	= mkdir
#ECHO	= echo
#TESTNDIR = if exist
#TESTFILE = if exist
# AND = 

# For cross compiling use e.g.
# make CROSS=x86_64-w64-mingw32- clean GC-inlined
CROSS	= 

AR		= $(CROSS)ar
DLLTOOL = $(CROSS)dlltool
CC      = $(CROSS)gcc
CXX     = $(CROSS)g++
RANLIB  = $(CROSS)ranlib
RC		= $(CROSS)windres
OD_PRIVATE	= $(CROSS)objdump -p

# Build for non-native architecture. E.g. "-m64" "-m32" etc.
# Not tested fully, needs gcc built with "--enable-multilib"
# Check your "gcc -v" output for the options used to build your gcc.
# You can set this as a shell variable or on the make comand line.
# You don't need to uncomment it here unless you want to hardwire
# a value.
#ARCH	= 

#
# Look for targets that $(RC) (usually windres) supports then look at any object
# file just built to see which target the compiler used and set the $(RC) target
# to match it.
#
SUPPORTED_TARGETS	= $(filter pe-% pei-% elf32-% elf64-% srec symbolsrec verilog tekhex binary ihex,$(shell $(RC) --help))
RC_TARGET			= --target $(firstword $(filter $(SUPPORTED_TARGETS),$(shell $(OD_PRIVATE) *.$(OBJEXT))))

OPT		=  $(CLEANUP) -O3 # -finline-functions -findirect-inlining
XOPT	= 

RCFLAGS	= --include-dir=.
LFLAGS	= $(ARCH)
# Uncomment this if config.h defines RETAIN_WSALASTERROR
#LFLAGS		+= -lws2_32
#
# Uncomment this next to link the GCC/C++ runtime libraries statically
# (Be sure to read about these options and their associated caveats
# at http://gcc.gnu.org/onlinedocs/gcc/Link-Options.html)
#
# NOTE 1: Doing this appears to break GCE:pthread_cleanup_*(), which
# relies on C++ class destructors being called when leaving scope.
#
# NOTE 2: If you do this DO NOT distribute your pthreads DLLs with
# the official filenaming, i.e. pthreadVC2.dll, etc. Instead, change DLL_VER
# above to "2slgcc" for example, to build "pthreadGC2slgcc.dll", etc.
#
#LFLAGS		+= -static-libgcc -static-libstdc++

# ----------------------------------------------------------------------
# The library can be built with some alternative behaviour to
# facilitate development of applications on Win32 that will be ported
# to other POSIX systems. Nothing definable here will make the library
# non-compliant, but applications that make assumptions that POSIX
# does not garrantee may fail or misbehave under some settings.
#
# PTW32_THREAD_ID_REUSE_INCREMENT
# Purpose:
# POSIX says that applications should assume that thread IDs can be
# recycled. However, Solaris and some other systems use a [very large]
# sequence number as the thread ID, which provides virtual uniqueness.
# pthreads-win32 provides pseudo-unique IDs when the default increment
# (1) is used, but pthread_t is not a scalar type like Solaris's.
#
# Usage:
# Set to any value in the range: 0 <= value <= 2^wordsize
#
# Examples:
# Set to 0 to emulate non recycle-unique behaviour like Linux or *BSD.
# Set to 1 for recycle-unique thread IDs (this is the default).
# Set to some other +ve value to emulate smaller word size types
# (i.e. will wrap sooner).
#
#PTW32_FLAGS	= "-DPTW32_THREAD_ID_REUSE_INCREMENT=0"
#
# ----------------------------------------------------------------------

GC_CFLAGS	= $(PTW32_FLAGS) 
GCE_CFLAGS	= $(PTW32_FLAGS) -mthreads

## Mingw
#MAKE		?= make
CFLAGS	= $(OPT) $(XOPT) $(ARCH) -I. -DHAVE_CONFIG_H -DMINGW_HAS_SECURE_API -Wall

OBJEXT = o
RESEXT = o
 
include common.mk

DLL_OBJS += $(RESOURCE_OBJS)
STATIC_OBJS += $(RESOURCE_OBJS)

GCE_DLL	= pthreadGCE$(DLL_VER).dll
GCED_DLL= pthreadGCE$(DLL_VERD).dll
GCE_LIB	= libpthreadGCE$(DLL_VER).a
GCED_LIB= libpthreadGCE$(DLL_VERD).a

GC_DLL 	= pthreadGC$(DLL_VER).dll
GCD_DLL	= pthreadGC$(DLL_VERD).dll
GC_LIB	= libpthreadGC$(DLL_VER).a
GCD_LIB	= libpthreadGC$(DLL_VERD).a
GC_INLINED_STATIC_STAMP = libpthreadGC$(DLL_VER).inlined_static_stamp
GCD_INLINED_STATIC_STAMP = libpthreadGC$(DLL_VERD).inlined_static_stamp
GCE_INLINED_STATIC_STAMP = libpthreadGCE$(DLL_VER).inlined_static_stamp
GCED_INLINED_STATIC_STAMP = libpthreadGCE$(DLL_VERD).inlined_static_stamp
GC_SMALL_STATIC_STAMP = libpthreadGC$(DLL_VER).small_static_stamp
GCD_SMALL_STATIC_STAMP = libpthreadGC$(DLL_VERD).small_static_stamp
GCE_SMALL_STATIC_STAMP = libpthreadGCE$(DLL_VER).small_static_stamp
GCED_SMALL_STATIC_STAMP = libpthreadGCE$(DLL_VERD).small_static_stamp

PTHREAD_DEF	= pthread.def

help:
	@ echo "Run one of the following command lines:"
	@ echo "$(MAKE) clean all                      (build targets GC, GCE, GC-static, GCE-static)"
	@ echo "$(MAKE) clean all-tests                (build and test all non-debug targets below)"
	@ echo "$(MAKE) clean GC                       (to build the GNU C dll with C cleanup code)"
	@ echo "$(MAKE) clean GC-debug                 (to build the GNU C debug dll with C cleanup code)"
	@ echo "$(MAKE) clean GCE                      (to build the GNU C dll with C++ exception handling)"
	@ echo "$(MAKE) clean GCE-debug                (to build the GNU C debug dll with C++ exception handling)"
	@ echo "$(MAKE) clean GC-static                (to build the GNU C static lib with C cleanup code)"
	@ echo "$(MAKE) clean GC-static-debug          (to build the GNU C static debug lib with C cleanup code)"
	@ echo "$(MAKE) clean GCE-static               (to build the GNU C++ static lib with C++ cleanup code)"
	@ echo "$(MAKE) clean GCE-static-debug         (to build the GNU C++ static debug lib with C++ cleanup code)"
	@ echo "$(MAKE) clean GC-small-static          (to build the GNU C static lib with C cleanup code)"
	@ echo "$(MAKE) clean GC-small-static-debug    (to build the GNU C static debug lib with C cleanup code)"
	@ echo "$(MAKE) clean GCE-small-static         (to build the GNU C++ static lib with C++ cleanup code)"
	@ echo "$(MAKE) clean GCE-small-static-debug   (to build the GNU C++ static debug lib with C++ cleanup code)"

all:
	@ $(MAKE) clean GC
	@ $(MAKE) clean GCE
	@ $(MAKE) clean GC-static
	@ $(MAKE) clean GCE-static

TEST_ENV = PTW32_FLAGS="$(PTW32_FLAGS) -DNO_ERROR_DIALOGS" DLL_VER=$(DLL_VER) ARCH="$(ARCH)"

all-tests:
	$(MAKE) realclean GC-small-static
	cd tests && $(MAKE) clean GC-small-static $(TEST_ENV) && $(MAKE) clean GCX-small-static $(TEST_ENV)
	$(MAKE) realclean GCE-small-static
	cd tests && $(MAKE) clean GCE-small-static $(TEST_ENV)
	@ $(ECHO) "$@ completed successfully."
	$(MAKE) realclean GC
	cd tests && $(MAKE) clean GC $(TEST_ENV) && $(MAKE) clean GCX $(TEST_ENV)
	$(MAKE) realclean GCE
	cd tests && $(MAKE) clean GCE $(TEST_ENV)
	$(MAKE) realclean GC-static
	cd tests && $(MAKE) clean GC-static $(TEST_ENV) && $(MAKE) clean GCX-static $(TEST_ENV)
	$(MAKE) realclean GCE-static
	cd tests && $(MAKE) clean GCE-static $(TEST_ENV)
	$(MAKE) realclean

all-tests-cflags:
	$(MAKE) all-tests PTW32_FLAGS="-Wall -Wextra"
	@ $(ECHO) "$@ completed successfully."

GC:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_OBJS)" $(GC_DLL)

GC-debug:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCD_DLL)

GCE:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED" CC=$(CXX) CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(DLL_OBJS)" $(GCE_DLL)

GCE-debug:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED" CC=$(CXX) CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(DLL_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_CXX -g -O0" $(GCED_DLL)

GC-static:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED -DPTW32_STATIC_LIB" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_OBJS)" $(GC_INLINED_STATIC_STAMP)

GC-static-debug:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED -DPTW32_STATIC_LIB" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCD_INLINED_STATIC_STAMP)

GC-small-static:
		$(MAKE) XOPT="-DPTW32_STATIC_LIB" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(STATIC_OBJS)" $(GC_SMALL_STATIC_STAMP)

GC-small-static-debug:
		$(MAKE) XOPT="-DPTW32_STATIC_LIB" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(STATIC_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCD_SMALL_STATIC_STAMP)

GCE-static:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED -DPTW32_STATIC_LIB" CC=$(CXX) CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(DLL_OBJS)" $(GCE_INLINED_STATIC_STAMP)

GCE-static-debug:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED -DPTW32_STATIC_LIB" CC=$(CXX) CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(DLL_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCED_INLINED_STATIC_STAMP)

GCE-small-static:
		$(MAKE) XOPT="-DPTW32_STATIC_LIB" CC=$(CXX) CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(STATIC_OBJS)" $(GCE_SMALL_STATIC_STAMP)

GCE-small-static-debug:
		$(MAKE) XOPT="-DPTW32_STATIC_LIB" CC=$(CXX) CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(STATIC_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCED_SMALL_STATIC_STAMP)

tests:
	@ cd tests
	@ $(MAKE) auto

# Very basic install. It assumes "realclean" was done just prior to build target if
# you want the installed $(DEVDEST_LIB_NAME) to match that build.
install:
	-$(TESTNDIR) $(DLLDEST) $(AND) $(MKDIR) $(DLLDEST)
	-$(TESTNDIR) $(LIBDEST) $(AND) $(MKDIR) $(LIBDEST)
	-$(TESTNDIR) $(HDRDEST) $(AND) $(MKDIR) $(HDRDEST)
	$(CP) pthreadGC*.dll $(DLLDEST)
	$(CP) libpthreadGC*.a $(LIBDEST)
	$(CP) pthread.h $(HDRDEST)
	$(CP) sched.h $(HDRDEST)
	$(CP) semaphore.h $(HDRDEST)
	-$(TESTFILE) libpthreadGC$(DLL_VER).a $(AND) $(CP) libpthreadGC$(DLL_VER).a $(LIBDEST)/$(DEST_LIB_NAME)
	-$(TESTFILE) libpthreadGC$(DLL_VERD).a $(AND) $(CP) libpthreadGC$(DLL_VERD).a $(LIBDEST)/$(DEST_LIB_NAME)
	-$(TESTFILE) libpthreadGCE$(DLL_VER).a $(AND) $(CP) libpthreadGCE$(DLL_VER).a $(LIBDEST)/$(DEST_LIB_NAME)
	-$(TESTFILE) libpthreadGCE$(DLL_VERD).a $(AND) $(CP) libpthreadGCE$(DLL_VERD).a $(LIBDEST)/$(DEST_LIB_NAME)

%.pre: %.c
	$(CC) -E -o $@ $(CFLAGS) $^

%.s: %.c
	$(CC) -c $(CFLAGS) -DPTW32_BUILD_INLINED -Wa,-ahl $^ > $@

%.o: %.rc
	$(RC) $(RC_TARGET) $(RCFLAGS) $(CLEANUP) -o $@ -i $<

.SUFFIXES: .dll .rc .c .o

.c.o:;		 $(CC) -c -o $@ $(CFLAGS) $(XC_FLAGS) $<


$(GC_DLL) $(GCD_DLL): $(DLL_OBJS)
	$(CC) $(OPT) -shared -o $@ $^ $(LFLAGS)
	$(DLLTOOL) -z pthread.def $^
	$(DLLTOOL) -k --dllname $@ --output-lib lib$(basename $@).a --def $(PTHREAD_DEF)

$(GCE_DLL) $(GCED_DLL): $(DLL_OBJS)
	$(CC) $(OPT) -mthreads -shared -o $@ $^ $(LFLAGS)
	$(DLLTOOL) -z pthread.def $^
	$(DLLTOOL) -k --dllname $@ --output-lib lib$(basename $@).a --def $(PTHREAD_DEF)

$(GC_INLINED_STATIC_STAMP) $(GCE_INLINED_STATIC_STAMP) $(GCD_INLINED_STATIC_STAMP) $(GCED_INLINED_STATIC_STAMP): $(DLL_OBJS)
	$(RM) $(basename $@).a
	$(AR) -rsv $(basename $@).a $^
	$(ECHO) touched > $@

$(GC_SMALL_STATIC_STAMP) $(GCE_SMALL_STATIC_STAMP) $(GCD_SMALL_STATIC_STAMP) $(GCED_SMALL_STATIC_STAMP): $(STATIC_OBJS)
	$(RM) $(basename $@).a
	$(AR) -rsv $(basename $@).a $^
	$(ECHO) touched > $@

clean:
	-$(RM) *~
	-$(RM) *.i
	-$(RM) *.s
	-$(RM) *.o
	-$(RM) *.obj
	-$(RM) *.exe
	-$(RM) *.manifest
	-$(RM) $(PTHREAD_DEF)

realclean: clean
	-$(RM) lib*.a
	-$(RM) *.lib
	-$(RM) pthread*.dll
	-$(RM) *_stamp
	-$(RM) make.log.txt
	-cd tests && $(MAKE) clean

var_check_list =

define var_check_target
var-check-$(1):
	@for src in $($(1)); do \
		fgrep -q "\"$$$$src\"" $(2) && continue; \
		echo "$$$$src is in \$$$$($(1)), but not in $(2)"; \
		exit 1; \
	done
	@grep '^# *include *".*\.c"' $(2) | cut -d'"' -f2 | while read src; do \
		echo " $($(1)) " | fgrep -q " $$$$src " && continue; \
		echo "$$$$src is in $(2), but not in \$$$$($(1))"; \
		exit 1; \
	done
	@echo "$(1) <-> $(2): OK"

var_check_list += var-check-$(1)
endef

$(eval $(call var_check_target,PTHREAD_SRCS,pthread.c))

srcs-vars-check: $(var_check_list)
