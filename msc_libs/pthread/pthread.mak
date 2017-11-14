###############################################################################
#									      #
#   File name	    pthread.mak						      #
#									      #
#   Description     Rules for building the pthread library with make.bat      #
#									      #
#   Notes	    The main rules are in all.mak, win32.mak, win64.mak, etc. #
#		    							      #
#   History								      #
#    2017-02-15 JFL Created this file.                                        #
#									      #
###############################################################################

# Get the list of SOURCES files to compile in various cases
!INCLUDE Files.mak

!IF "$(TYPE)"=="DLL"
DEFINE_BUILD_TYPE=
SOURCES=$(ONE_SOURCE)		# The DLL builds much faster when we compile just pthread.c, which includes everything
ALIAS_TYPE=dll
!ELSEIF "$(TYPE)"=="LIB"
DEFINE_BUILD_TYPE=/DPTW32_STATIC_LIB
ALIAS_TYPE=lib
!ENDIF

!IF "$(EH)"=="C"	    # C cleanup code
EHFLAGS =			# Exception Handling Flags
CLEANUP = __CLEANUP_C
ALIAS_SUFFIX=VC
!ELSEIF "$(EH)"=="CXX"	    # C++ Exceptions
# (Note: If you are using Microsoft VC++6.0, the library needs to be built
# with /EHa instead of /EHs or else cancellation won't work properly.)
EHFLAGS = /EHs /TP		# Exception Handling Flags
CLEANUP = __CLEANUP_CXX
ALIAS_SUFFIX=VCE
!ELSEIF "$(EH)"=="SEH"	    # Structured Exceptions
EHFLAGS =			# Exception Handling Flags
CLEANUP = __CLEANUP_SEH
ALIAS_SUFFIX=VSE
!ENDIF

!IF !DEFINED(SOURCES)
!IF EXIST(common.mk)
OBJEXT = obj
RESEXT = res
!INCLUDE common.mk
SOURCES = $(PTHREAD_SRCS)	# Sources for versions >= 2.10 RC
# OBJECTS = $(STATIC_OBJS)	# src2obj will recreate with the object directory path prefix
!ELSEIF EXIST(pthread_exit.c)
SOURCES = $(SOURCES_210_BETA)	# Sources for versions 2.10 beta
!ELSE
SOURCES = $(SOURCES_209)	# Sources for versions <= 2.09
!ENDIF
!ENDIF # !DEFINED(SOURCES)

# Change SysToolsLib's *.mak CFLAGS to match pthread's, but keep all those that define output directories, etc.
# Pthread's own Makefile defines:
#   XCFLAGS = /W3 /MD /nologo
#   CFLAGS  = /O2 /Ob2 $(XCFLAGS)
#   CFLAGSD = /Z7 $(XCFLAGS)
# For info about /MT vs /MD, see https://msdn.microsoft.com/en-us/library/abx4dbyh(v=vs.80).aspx
# It's supposed to be /MT for multithread libs (libcmt.lib), and /MD for DLL libs (msvcrt.lib) */
CFLAGS=$(CFLAGS:/W4 =) /W3 /MD # Yet ag.exe hangs with pthread.lib if linked with /MT. So use /MD always.
CFLAGS=$(CFLAGS:/Zp =)
!IF DEFINED(_DEBUG) || "$(DEBUG)"=="1"
CFLAGS=$(CFLAGS:/Zi =) /Z7
!ELSE
CFLAGS=$(CFLAGS:/Os =) /O2 /Ob2
!ENDIF
CFLAGS=$(CFLAGS) /D_CRT_SECURE_NO_WARNINGS $(EHFLAGS) $(DEFINE_BUILD_TYPE) /DPTW32_BUILD_INLINED /D$(CLEANUP) /DHAVE_CONFIG_H

# Work around for the conflicting definitions of struct timespec in Visual C++ >= 2015 ucrt\time.h, and that in pthread.h
!IF "$(WIN32_CRTINC)" != "$(WIN32_VCINC)" # They start differing with the introduction of the UCRT.
CFLAGS=$(CFLAGS) /DHAVE_STRUCT_TIMESPEC
!ENDIF

#-----------------------------------------------------------------------------#
#			Include files dependencies			      #
#-----------------------------------------------------------------------------#

# TBD
