###############################################################################
#									      #
#   File name	    Files.mak						      #
#									      #
#   Description	    Define files included in zlib.lib.			      #
#									      #
#   Notes	    Shared between the DOS/Windows and Unix make files.	      #
#		    Do not use any OS-specific make syntax, such as           #
#		    conditional directives.				      #
#		    							      #
#   History								      #
#    2017-02-14 JFL Created this file.                                        #
#									      #
###############################################################################

# List of source files to compile and include in the zlib library
SOURCES = \
    adler32.c	\
    compress.c	\
    crc32.c	\
    deflate.c	\
    gzclose.c   \
    gzlib.c     \
    gzread.c    \
    gzwrite.c   \
    infback.c	\
    inffast.c	\
    inflate.c	\
    inftrees.c	\
    trees.c	\
    uncompr.c	\
    zutil.c	\

# List of all targets that must be built
ALL=zlib.lib # debug\zlib.lib
