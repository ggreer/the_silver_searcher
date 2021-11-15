###############################################################################
#									      #
#   File name:	    win95.mak						      #
#									      #
#   Description:    A NMake makefile to build WIN95 programs.		      #
#									      #
#   Notes:	    Use with make.bat, which defines the necessary variables. #
#		    Usage: make -f win95.mak [definitions] [targets]	      #
#									      #
#		    Targets:						      #
#		    clean	      Erase all files in WIN95 subdirectory.  #
#		    {prog}.exe	      Build WIN95[\DEBUG]\{prog}.exe.	      #
#		    {prog}.obj	      Build WIN95[\DEBUG]\OBJ\{prog}.obj.     #
#		    {prog}.res	      Build WIN95[\DEBUG]\OBJ\{prog}.res.     #
#		    WIN95\{prog}.exe       Build the WIN95 release version.   #
#		    WIN95\DEBUG\{prog}.exe Build the WIN95 debug version.     #
#		    WIN95\OBJ\{prog}.obj       Compile the release version.   #
#		    WIN95\DEBUG\OBJ\{prog}.obj Compile the debug version.     #
#		    WIN95\OBJ\{prog}.res       Compile release resources.     #
#		    WIN95\DEBUG\OBJ\{prog}.res Compile debug resources.	      #
#									      #
#		    Command-line definitions:				      #
#		    DEBUG=0	 Build the release ver. (<=> target WIN95)    #
#		    DEBUG=1	 Build the debug ver. (<=> target WIN95\DEBUG)#
#		    OUTDIR=path  Output to path\WIN95\. Default: To bin\WIN95\#
#		    PROGRAM=name Set the output file base name		      #
#									      #
#		    If a specific target [path\]{prog}.exe is specified,      #
#		    includes the corresponding {prog}.mak if it exists.       #
#		    This make file, defines the files to use beyond the       #
#		    default {prog}.c/{prog}.obj; Compiler options; etc.       #
#		    SOURCES	Source files to compile.		      #
#		    OBJECTS	Object files to link. Optional.		      #
#		    PROGRAM	The node name of the program to build. Opt.   #
#		    EXENAME	The file name of the program to build. Opt.   #
#		    SKIP_THIS	Message explaining why NOT to build. Opt.     #
#									      #
#		    In the absence of a {prog}.mak file, or if one of the     #
#		    generic targets is used, then the default Files.mak is    #
#		    used instead. Same definitions.			      #
#									      #
#		    Note that these sub-make files are designed to be	      #
#		    OS-independant. The goal is to reuse them to build	      #
#		    the same program under Unix/Linux too. So for example,    #
#		    all paths must contain forward slashes.		      #
#									      #
#		    Another design goal is to use that same win95.mak	      #
#		    in complex 1-project environments (One Files.mak defines  #
#		    all project components); And in simple multiple-project   #
#		    environments (No Files.mak; Most programs have a single   #
#		    source file, and use default compiler options).	      #
#									      #
#		    The following macros / environment variables must be      #
#		    predefined. This allows to use the same makefile on       #
#		    machines with various locations for the build tools.      #
#									      #
#		    WIN95_AS	    	32-bits Assembler		      #
#		    WIN95_CC    	32-bits C compiler		      #
#		    WIN95_INCPATH  	32-bits include files paths	      #
#		    WIN95_LK	  	32-bits Linker			      #
#		    WIN95_LIBPATH       32-bits libraries paths		      #
#		    WIN95_LB	   	32-bits librarian     		      #
#		    WIN95_PATH  	32-bits programs paths		      #
#		    WIN95_RC    	32-bits Resource compiler	      #
#		    WIN95_MT    	32-bits Manifest Tool		      #
#		    TMP		    	Temporary directory	 	      #
#									      #
#  History:								      #
#    2015-11-05 JFL Created this file.					      #
#    2016-09-15 JFL Added WSDKINCLUDE definition.                             #
#    2016-10-11 JFL Adapted for use in SysToolsLib global C include dir.      #
#    2016-10-21 JFL Use the more recent WIN32 assembler, w. more instructions.#
#    2018-02-28 JFL Added $(LSX) Library SuffiX definition.		      #
#									      #
#      © Copyright 2016-2018 Hewlett Packard Enterprise Development LP        #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

T=WIN95				# Target OS

!IF !DEFINED(DISPATCH_OS)

T_VARS=1	# Make sure OS-type-specific variables are defined only once

CC=$(WIN95_CC)    		# 32-bits C compiler
# AS=$(WIN95_AS)    		# 32-bits Assembler
AS=$(WIN32_AS)	    		# Actually use this one, which is likely to support more instructions
LK=$(WIN95_LK)		  	# 32-bits Linker
LB=$(WIN95_LB)	  	 	# 32-bits librarian
RC=$(WIN95_RC)    		# 32-bits Resource compiler
MT=$(WIN95_MT)    		# 32-bits Manifest Tool

PATH=$(WIN95_PATH)  		# 32-bits programs paths
INCPATH=$(WIN95_INCPATH)	# 32-bits include files paths
MSVCINCLUDE=$(WIN95_VCINC:\=/)  # Path of MSVC compiler include files, without quotes, and with forward slashes
UCRTINCLUDE=$(WIN95_CRTINC:\=/) # Path of MSVC CRT library include files, without quotes, and with forward slashes
WSDKINCLUDE=$(WIN95_WINSDKINC:\=/) # Path of Windows SDK include files, without quotes, and with forward slashes
LIB=$(WIN95_LIBPATH)		# 32-bits libraries paths

!IF !DEFINED(AFLAGS)
AFLAGS=/Cx /Sn /Zim /D_WIN95 "/D_MODEL=flat,stdcall"	# Contrary to the C compiler, the assembler does not define _WINxx variables
!ENDIF # !DEFINED(AFLAGS)

WINVER=4.0			# Target Windows 95 and NT4
MACHINE=X86			# Target CPU = Intel 32-bits x86
USEDOSSTUB=1			# Use an MS-DOS stub

# Library SuffiX. For storing multiple versions of the same library in a single directory.
LSX=w95

!ENDIF # !DEFINED(DISPATCH_OS)

# Get common WIN32 definitions and rules from win32.mak
!INCLUDE <win32.mak>
