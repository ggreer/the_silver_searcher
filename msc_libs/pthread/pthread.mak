###############################################################################
#									      #
#   File name	    pthread.mak						      #
#									      #
#   Description     Specific rules for building the pthread library	      #
#									      #
#   Notes	    The main rules are in all.mak, dos.mak, win32.mak, etc.   #
#		    							      #
#   History								      #
#    2017-02-15 JFL Created this file.                                        #
#									      #
###############################################################################

# Get the list of SOURCES files to compile, and ALL targets to build
!INCLUDE Files.mak

# Extra definitions for the C compilation

# Pthread's own Makefile defines:
#   XCFLAGS = /W3 /MD /nologo
#   CFLAGS  = /O2 /Ob2 $(XCFLAGS)
#   CFLAGSD = /Z7 $(XCFLAGS)
# Change SysToolsLib's CFLAGS to match pthread's. Keep all those that define output directories, etc.
# For info about /MT vs /MD, see https://msdn.microsoft.com/en-us/library/abx4dbyh(v=vs.80).aspx
CFLAGS=$(CFLAGS:/W4 =) /W3 /MT # Use /MT for multithread libs (libcmt.lib), not /MD for DLL libs (msvcrt.lib) */
CFLAGS=$(CFLAGS:/Zp =)
!IF DEFINED(_DEBUG) || "$(DEBUG)"=="1"
CFLAGS=$(CFLAGS:/Zi =) /Z7
!ELSE
CFLAGS=$(CFLAGS:/Os =) /O2 /Ob2
!ENDIF
CFLAGS=$(CFLAGS) /D_CRT_SECURE_NO_WARNINGS /DPTW32_STATIC_LIB /DPTW32_BUILD_INLINED /D__CLEANUP_C /DHAVE_CONFIG_H

# Work around for the conflicting definitions of struct timespec in Visual C++ >= 2015 ucrt\time.h, and that in pthread.h
!IF "$(WIN32_CRTINC)" != "$(WIN32_VCINC)" # They start differing with the introduction of the UCRT.
CFLAGS=$(CFLAGS) /DHAVE_STRUCT_TIMESPEC
!ENDIF

#-----------------------------------------------------------------------------#
#			Include files dependencies			      #
#-----------------------------------------------------------------------------#

