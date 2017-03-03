###############################################################################
#									      #
#   File name	    pcre.mak						      #
#									      #
#   Description     Specific rules for building the pcre library	      #
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
CFLAGS=$(CFLAGS) /D_CRT_SECURE_NO_WARNINGS /DHAVE_CONFIG_H

!IF DEFINED(_DEBUG) || "$(DEBUG)"=="1"
CFLAGS=$(CFLAGS) /DPCRE_DEBUG
!ENDIF

#-----------------------------------------------------------------------------#
#			Include files dependencies			      #
#-----------------------------------------------------------------------------#

pcre_byte_order.c:	config.h

pcre_chartables.c::	config.h

pcre_compile.c:		config.h

pcre_config.c:		config.h

pcre_dfa_exec.c:	config.h

pcre_exec.c:		config.h

pcre_fullinfo.c:	config.h

pcre_get.c:		config.h

pcre_globals.c:		config.h

pcre_jit_compile.c:	config.h

pcre_maketables.c:	config.h

pcre_newline.c:		config.h

pcre_ord2utf8.c:	config.h

pcre_refcount.c:	config.h

pcre_string_utils.c:	config.h

pcre_study.c:		config.h

pcre_tables.c:		config.h

pcre_ucd.c:		config.h

pcre_valid_utf8.c:	config.h

pcre_version.c:		config.h

pcre_xclass.c:		config.h
