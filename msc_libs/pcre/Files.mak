###############################################################################
#									      #
#   File name	    Files.mak						      #
#									      #
#   Description	    Define files included in pcre.lib.			      #
#									      #
#   Notes	    Shared between the DOS/Windows and Unix make files.	      #
#		    Do not use any OS-specific make syntax, such as           #
#		    conditional directives.				      #
#		    							      #
#   History								      #
#    2017-02-14 JFL Created this file.                                        #
#									      #
###############################################################################

# List of source files to compile and include in the pcre library
SOURCES = \
    pcre_byte_order.c		\
    pcre_chartables.c		\
    pcre_compile.c		\
    pcre_config.c		\
    pcre_dfa_exec.c		\
    pcre_exec.c			\
    pcre_fullinfo.c		\
    pcre_get.c			\
    pcre_globals.c		\
    pcre_jit_compile.c		\
    pcre_maketables.c		\
    pcre_newline.c		\
    pcre_ord2utf8.c		\
    pcre_refcount.c		\
    pcre_string_utils.c		\
    pcre_study.c		\
    pcre_tables.c		\
    pcre_ucd.c			\
    pcre_valid_utf8.c		\
    pcre_version.c		\
    pcre_xclass.c		\

# List of all targets that must be built
ALL=config.h pcre.h pcre_chartables.c pcre.lib # debug\pcre.lib

# List of extra files to clean
CLEAN_FILES = \
    config.h		\
    pcre.h		\
    pcre_chartables.c	\

#-----------------------------------------------------------------------------#
#		    Rules for recreating the missing files		      #
#-----------------------------------------------------------------------------#

# Hand-made config.h for Visual C++ / MsvcLibX builds
config.h: config.h.MsvcLibX
    copy /y config.h.MsvcLibX config.h

# Rules for recreating missing source files from generic versions 
pcre.h: pcre.h.generic
    copy /y pcre.h.generic pcre.h

pcre_chartables.c:: pcre_chartables.c.dist
    copy /y pcre_chartables.c.dist pcre_chartables.c

