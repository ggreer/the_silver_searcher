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
#									      #
###############################################################################

# List of subdirectories to build
DIRS=MsvcLibX pcre pthread zlib

# Special build rule for the pthread lib:
pthread:
    $(BMAKE) -C $@ VC-lib

# Extra directories that need cleaning
CLEAN_DIRS=include
