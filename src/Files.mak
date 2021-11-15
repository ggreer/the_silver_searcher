###############################################################################
#                                                                             #
#   File name       Files.mak                                                 #
#                                                                             #
#   Description     Define files included in ag.exe.                          #
#                                                                             #
#   Notes           Shared between the DOS/Windows and Unix make files.       #
#                   Do not use any OS-specific make syntax, such as           #
#                   conditional directives.                                   #
#                                                                             #
#   History                                                                   #
#    2017-02-15 JFL Created this file.                                        #
#    2018-01-11 JFL Added a message when creating config.h.		      #
#                                                                             #
###############################################################################

# List of source files to compile and include in The Silver Searcher
SOURCES = \
    decompress.c        \
    ignore.c            \
    lang.c              \
    log.c               \
    main.c              \
    options.c           \
    print.c             \
#   print_w32.c         \
    scandir.c           \
    search.c            \
    util.c              \

# Everything that needs to be built
ALL=config.h ag.exe # debug/ag.exe

# We use a junction instead of a symlinkd, because, unlike junctions, Windows does not allow symlinkd creations by default.
# $(OUTDIR)=bin by default, but may be changed on some systems. (And it is on my battery of test VMs)
# 2017-10-22 JFL The bin link creation can now be handled by defining MD_OUTDIR in a configure.xxx.bat file.

# Make file, and default log file, base name
MODULE=ag

# List of extra files to clean
CLEAN_FILES = \
    config.h            \

#-----------------------------------------------------------------------------#
#                   Rules for recreating the missing files                    #
#-----------------------------------------------------------------------------#

# Hand-made config.h for Visual C++ / MsvcLibX builds
config.h: config.h.MsvcLibX
    $(MSG) Creating $@
    copy /y config.h.MsvcLibX config.h

