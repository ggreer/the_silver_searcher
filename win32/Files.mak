###############################################################################
#									      #
#   File name	    Files.mak						      #
#									      #
#   Description     Declare the subdirectories to build recursively	      #
#									      #
#   Notes	    							      #
#									      #
#   History								      #
#    2017-02-24 JFL Created this file.                                        #
#    2026-04-01 JFL No need for a special case for pthread anymore.           #
#									      #
###############################################################################

# List of subdirectories to build
DIRS=MsvcLibX pcre pthread zlib

# Extra directories that need cleaning
CLEAN_DIRS=include
