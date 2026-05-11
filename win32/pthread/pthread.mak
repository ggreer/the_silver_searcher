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
#    2026-03-17 JFL Updated constants for pthread 3.0.3.                      #
#    2026-03-27 JFL Make it easy to build either ONE_SOURCE or PTHREAD_SRCS.  #
#		    							      #
###############################################################################

# Get the list of SOURCES files to compile in various cases
!INCLUDE Files.mak

!IF "$(TYPE)"=="DLL"
DEFINE_BUILD_TYPE=
SOURCES=$(ONE_SOURCE)		# The DLL builds much faster when we compile just pthread.c, which includes everything
ALIAS_TYPE=dll
!ELSEIF "$(TYPE)"=="LIB"
DEFINE_BUILD_TYPE=/DPTW32_STATIC_LIB
# Leave SOURCES undefined here, to compile all individual sources. In this case, SOURCES will be defined further down.
# SOURCES=$(ONE_SOURCE)		# The lib builds much faster with one source. But this puts lots of useless code in your app.
!IF "$(FAST)"=="1"		# Allow selecting the fast build mode by setting "FAST=1" on the make.bat command line.
SOURCES=$(ONE_SOURCE)
!ENDIF
ALIAS_TYPE=lib
!ENDIF

!IF "$(EH)"=="C"	    # C cleanup code
EHFLAGS =			# Exception Handling Flags
CLEANUP = PTW32_CLEANUP_C
ALIAS_SUFFIX=VC
!ELSEIF "$(EH)"=="CXX"	    # C++ Exceptions
# (Note: If you are using Microsoft VC++6.0, the library needs to be built
# with /EHa instead of /EHs or else cancellation won't work properly.)
EHFLAGS = /EHs /TP		# Exception Handling Flags
CLEANUP = PTW32_CLEANUP_CXX
ALIAS_SUFFIX=VCE
!ELSEIF "$(EH)"=="SEH"	    # Structured Exceptions
EHFLAGS =			# Exception Handling Flags
CLEANUP = PTW32_CLEANUP_SEH
ALIAS_SUFFIX=VSE
!ENDIF

!IF EXIST(common.mk)
OBJEXT = obj
RESEXT = res
!INCLUDE common.mk		# Defines PTHREAD_SRCS = Sources for versions >= 2.10 RC
# OBJECTS = $(STATIC_OBJS)	# src2obj will recreate with the object directory path prefix
!ELSEIF EXIST(pthread_exit.c)
PTHREAD_SRCS = $(SOURCES_210_BETA)	# Sources for versions 2.10 beta
!ELSE # !EXIST(common.mk)
PTHREAD_SRCS = $(SOURCES_209)	# Sources for versions <= 2.09
!ENDIF # EXIST(common.mk)
!IF !DEFINED(SOURCES)		# If not defined as $(ONE_SOURCE) in the "$(TYPE)"=="LIB" section above
SOURCES=$(PTHREAD_SRCS)		# Then compile in all individual sources
!ENDIF # !DEFINED(SOURCES)

# Change SysToolsLib's *.mak CFLAGS to match pthread's, but keep all those that define output directories, etc.
# Pthread's own Makefile defines:
#   XCFLAGS = /W3 /MD /nologo
#   CFLAGS  = /O2 /Ob2 $(XCFLAGS)
#   CFLAGSD = /Z7 $(XCFLAGS)
# For info about /MT vs /MD, see https://msdn.microsoft.com/en-us/library/abx4dbyh(v=vs.80).aspx
# It's supposed to be /MT for multithread libs (libcmt.lib), and /MD for DLL libs (msvcrt.lib) */
CFLAGS=$(CFLAGS:/W4 =) /W3 /MT	# ag.exe hanged with pthread.lib 2.10 release if compiled with /MT. Used /MD as a workaround.
!IF "$(TYPE)"=="DLL"
CFLAGS=$(CFLAGS: /MT=) /MD	# DLLs must use /MD anyway
!ENDIF
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

# pthreads version 3.0.3 requires this additional variable to compile in one-source mode
!IF "$(SOURCES)"=="$(ONE_SOURCE)"
CFLAGS=$(CFLAGS) /D__PTHREAD_JUMBO_BUILD__
!ENDIF

#-----------------------------------------------------------------------------#
#                         Include files dependencies                          #
#-----------------------------------------------------------------------------#

!IF "$(SOURCES)"=="$(ONE_SOURCE)"
$(ONE_SOURCE): $(PTHREAD_SRCS)
  for %%f in ($?) do if exist "$(O)\%%f" del "$(O)\%%f" &:# For all updated C sources, delete the de-BOM-ed copy.
  touch $@ || copy /b $@+NUL &:# Use touch.exe if available, else use internal DOS commands for doing the same, albeit less efficiently.
!ENDIF

