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
ALL=bin_link config.h ag.exe # debug/ag.exe

# Create a link from this bin dir to the silver searcher's main bin dir.
# Make.bat creates an empty directory here, that we now change to a junction to ..\bin, if not done already.
# Caution, if invoked in this subdirectory, make.bat also creates a log file in bin\. In this case, it's not possible to create the junction.
# We use a junction instead of a symlinkd, because, unlike junctions, Windows does not allow symlinkd creations by default.
# $(OUTDIR)=bin by default, but may be changed on some systems. (And it is on my battery of test VMs)
bin_link:
    for /f %o in ("$(OS)") do if not exist $(OUTDIR)\%o $(HEADLINE) Creating link $(OUTDIR) to ..\$(OUTDIR) & rd $(OUTDIR) & mklink /j $(OUTDIR) ..\$(OUTDIR)

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
    copy /y config.h.MsvcLibX config.h

