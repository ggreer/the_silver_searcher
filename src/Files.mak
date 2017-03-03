###############################################################################
#									      #
#   File name	    Files.mak						      #
#									      #
#   Description     Define files included in ag.exe.			      #
#									      #
#   Notes	    Shared between the DOS/Windows and Unix make files.	      #
#		    Do not use any OS-specific make syntax, such as           #
#		    conditional directives.				      #
#		    							      #
#   History								      #
#    2017-02-15 JFL Created this file.                                        #
#									      #
###############################################################################

# List of source files to compile and include in The Silver Searcher
SOURCES = \
    decompress.c	\
    ignore.c		\
    lang.c		\
    log.c		\
    main.c		\
    options.c		\
    print.c		\
#   print_w32.c		\
    scandir.c		\
    search.c		\
    util.c		\

# Everything that needs to be built
ALL=config.h ag.exe # debug/ag.exe

# Make file, and default log file, base name
MODULE=ag

# List of extra files to clean
CLEAN_FILES = \
    config.h		\

#-----------------------------------------------------------------------------#
#		    Rules for recreating the missing files		      #
#-----------------------------------------------------------------------------#

# Hand-made config.h for Visual C++ / MsvcLibX builds
config.h: config.h.MsvcLibX
    copy /y config.h.MsvcLibX config.h

