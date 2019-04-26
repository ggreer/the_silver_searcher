###############################################################################
#									      #
#   File name:	    arm64.mak						      #
#									      #
#   Description:    A NMake makefile to build ARM64 programs.		      #
#									      #
#   Notes:	    Use with make.bat, which defines the necessary variables. #
#		    Usage: make -f arm64.mak [definitions] [targets]	      #
#		    							      #
#		    Targets:						      #
#		    clean	      Erase all files in ARM64 subdirectory.  #
#		    {prog}.exe	      Build ARM64[\DEBUG]\{prog}.exe.	      #
#		    {prog}.obj	      Build ARM64[\DEBUG]\OBJ\{prog}.obj.     #
#		    {prog}.res	      Build ARM64[\DEBUG]\OBJ\{prog}.res.     #
#		    ARM64\{prog}.exe       Build the ARM64 release version.   #
#		    ARM64\DEBUG\{prog}.exe Build the ARM64 debug version.     #
#		    ARM64\OBJ\{prog}.obj       Compile the release version.   #
#		    ARM64\DEBUG\OBJ\{prog}.obj Compile the debug version.     #
#		    ARM64\OBJ\{prog}.res       Compile release resources.     #
#		    ARM64\DEBUG\OBJ\{prog}.res Compile debug resources.	      #
#									      #
#		    Command-line definitions:				      #
#		    DEBUG=0	 Build the release ver. (<=> target ARM64)    #
#		    DEBUG=1	 Build the debug ver. (<=> target ARM64\DEBUG)#
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
#		    Another design goal is to use that same arm64.mak	      #
#		    in complex 1-project environments (One Files.mak defines  #
#		    all project components); And in simple multiple-project   #
#		    environments (No Files.mak; Most programs have a single   #
#		    source file, and use default compiler options).	      #
#									      #
#		    The following macros / environment variables must be      #
#		    predefined. This allows to use the same makefile on       #
#		    machines with various locations for the build tools.      #
#									      #
#		    ARM64_AS	    	64-bits ARM Assembler		      #
#		    ARM64_CC    	64-bits ARM C compiler		      #
#		    ARM64_INCPATH  	64-bits ARM include files paths	      #
#		    ARM64_LK	  	64-bits ARM Linker		      #
#		    ARM64_LIBPATH	64-bits ARM libraries paths	      #
#		    ARM64_LB	   	64-bits ARM librarian     	      #
#		    ARM64_PATH  	64-bits ARM programs paths	      #
#		    ARM64_RC    	64-bits Resource compiler	      #
#		    ARM64_MT    	64-bits Manifest Tool		      #
#		    TMP		    	Temporary directory	 	      #
#									      #
#  History:								      #
#    2019-02-10 JFL Created this file.					      #
#									      #
#         © Copyright 2019 Hewlett Packard Enterprise Development LP          #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

T=ARM64				# Target OS

!IF !DEFINED(DISPATCH_OS)

T_VARS=1	# Make sure OS-type-specific variables are defined only once

CC=$(ARM64_CC)    		# 64-bits C compiler
AS=$(ARM64_AS)	    		# 64-bits Assembler
LK=$(ARM64_LK)		  	# 64-bits Linker			
LB=$(ARM64_LB)	  	 	# 64-bits librarian     		
RC=$(ARM64_RC)    		# 64-bits Resource compiler	
MT=$(ARM64_MT)    		# 64-bits Manifest Tool		

PATH=$(ARM64_PATH)  		# 64-bits programs paths		
INCPATH=$(ARM64_INCPATH)	# 64-bits include files paths	
MSVCINCLUDE=$(ARM64_VCINC:\=/)	# Path of MSVC compiler include files, without quotes, and with forward slashes
UCRTINCLUDE=$(ARM64_CRTINC:\=/)	# Path of MSVC CRT library include files, without quotes, and with forward slashes
WSDKINCLUDE=$(ARM64_WINSDKINC:\=/) # Path of Windows SDK include files, without quotes, and with forward slashes
LIB=$(ARM64_LIBPATH)		# 64-bits libraries paths		

WINVER=6.2			# Target Windows 8
MACHINE=ARM64			# Target CPU = 64-bits ARM64
USEDOSSTUB=0			# Do not use an MS-DOS stub

# Library SuffiX. For storing multiple versions of the same library in a single directory.
LSX=a64

!ENDIF # !DEFINED(DISPATCH_OS)

# Get common WIN32 definitions and rules from win32.mak
!INCLUDE <win32.mak>
