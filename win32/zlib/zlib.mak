###############################################################################
#									      #
#   File name	    zlib.mak						      #
#									      #
#   Description     Specific rules for building the zlib library	      #
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
CFLAGS=$(CFLAGS) /DWIN32 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE /D_UTF8_SOURCE /DHAVE_UNISTD_H /DHAVE_STDARG_H /D_LARGEFILE64_SOURCE=1
# Disable warning C4131: Function uses old-style declarator
CFLAGS=$(CFLAGS) /wd4131 

!IF DEFINED(_DEBUG) || "$(DEBUG)"=="1"
CFLAGS=$(CFLAGS) /DZLIB_DEBUG
!ENDIF

#-----------------------------------------------------------------------------#
#			Include files dependencies			      #
#-----------------------------------------------------------------------------#

adler32.c: zlib.h zconf.h

compress.c: zlib.h zconf.h

crc32.c: zlib.h zconf.h crc32.h

deflate.c: deflate.h zutil.h zlib.h zconf.h

gzclose.c: zlib.h zconf.h gzguts.h

gzlib.c: zlib.h zconf.h gzguts.h

gzread.c: zlib.h zconf.h gzguts.h

gzwrite.c: zlib.h zconf.h gzguts.h

infback.c: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h

inffast.c: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h

inflate.c: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h

inftrees.c: zutil.h zlib.h zconf.h inftrees.h

trees.c: zutil.h zlib.h zconf.h deflate.h trees.h

uncompr.c: zlib.h zconf.h

zutil.c: zutil.h zlib.h zconf.h

