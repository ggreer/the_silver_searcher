###############################################################################
#									      #
#   File name	    ag.mak						      #
#									      #
#   Description     Specific rules for building ag.exe.			      #
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
CFLAGS=$(CFLAGS) /MT /D_CRT_SECURE_NO_WARNINGS /D_UTF8_SOURCE /DPCRE_STATIC /DPTW32_STATIC_LIB

# Work around for the conflicting definitions of struct timespec in Visual C++ >= 2015 ucrt\time.h, and that in pthread.h
!IF "$(WIN32_CRTINC)" != "$(WIN32_VCINC)" # They start differing with the introduction of the UCRT.
CFLAGS=$(CFLAGS) /DHAVE_STRUCT_TIMESPEC
!ENDIF

# Extra libraries to link with ag.exe
LIBS=$(LIBS) pcre.lib pthread.lib zlib.lib

#-----------------------------------------------------------------------------#
#			Include files dependencies			      #
#-----------------------------------------------------------------------------#

decompress.c: decompress.h

ignore.c: ignore.h log.h options.h scandir.h util.h

lang.c: lang.h util.h

log.c: log.h util.h

main.c: config.h log.h options.h search.h util.h

options.c: config.h ignore.h lang.h log.h options.h print.h util.h

print.c: ignore.h log.h options.h print.h search.h util.h

print_w32.c: print.h

scandir.c: scandir.h util.h

search.c: search.h print.h scandir.h

util.c: config.h util.h
