###############################################################################
#									      #
#   File name:	    arm.mak						      #
#									      #
#   Description:    A NMake makefile to build ARM programs.		      #
#									      #
#   Notes:	    Warning:						      #
#		    This is an experimental make file for building ARM	      #
#		    applications. Currently this does not work, as the	      #
#		    compiler fails with the error message: "Compiling Desktop # 
#		    applications for the ARM platform is not supported."      #
#		    Apparently this requires an "ARM Desktop SDK", which      #
#		    (As of fall 2015) Microsoft never publicly released.      #
#		    							      #
#		    Use with make.bat, which defines the necessary variables. #
#		    Usage: make -f arm.mak [definitions] [targets]	      #
#		    							      #
#		    Targets:						      #
#		    clean	      Erase all files in ARM subdirectory.    #
#		    {prog}.exe	      Build ARM[\DEBUG]\{prog}.exe.	      #
#		    {prog}.obj	      Build ARM[\DEBUG]\OBJ\{prog}.obj.       #
#		    {prog}.res	      Build ARM[\DEBUG]\OBJ\{prog}.res.       #
#		    ARM\{prog}.exe       Build the ARM release version.       #
#		    ARM\DEBUG\{prog}.exe Build the ARM debug version.         #
#		    ARM\OBJ\{prog}.obj       Compile the release version.     #
#		    ARM\DEBUG\OBJ\{prog}.obj Compile the debug version.       #
#		    ARM\OBJ\{prog}.res       Compile release resources.       #
#		    ARM\DEBUG\OBJ\{prog}.res Compile debug resources.	      #
#									      #
#		    Command-line definitions:				      #
#		    DEBUG=0	 Build the release ver. (<=> target ARM)      #
#		    DEBUG=1	 Build the debug ver. (<=> target ARM\DEBUG)  #
#		    PROGRAM=name Set the output file base name		      #
#									      #
#		    If a specific target [path\]{prog}.exe is specified,      #
#		    includes the corresponding {prog}.mak if it exists.       #
#		    This make file, defines the files to use beyond the       #
#		    default {prog}.c/{prog}.obj; Compiler options; etc.       #
#		    SOURCES	Source files to compile.		      #
#		    OBJECTS	Object files to link. Optional.		      #
#		    PROGRAM	The node name of the program to build. Opt.   #
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
#		    ARM_AS	    	32-bits Assembler		      #
#		    ARM_CC    		32-bits C compiler		      #
#		    ARM_INCPATH  	32-bits include files paths	      #
#		    ARM_LK	  	32-bits Linker			      #
#		    ARM_LIBPATH		32-bits libraries paths		      #
#		    ARM_LB	   	32-bits librarian     		      #
#		    ARM_PATH  		32-bits programs paths		      #
#		    ARM_RC    		32-bits Resource compiler	      #
#		    ARM_MT    		32-bits Manifest Tool		      #
#		    TMP		    	Temporary directory	 	      #
#									      #
#  History:								      #
#    2015-11-13 JFL Created this file.					      #
#    2016-09-15 JFL Added WSDKINCLUDE definition.                             #
#    2016-10-11 JFL Adapted for use in SysToolsLib global C include dir.      #
#									      #
#         © Copyright 2016 Hewlett Packard Enterprise Development LP          #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

T=ARM				# Target OS

!IF !DEFINED(DISPATCH_OS)

T_VARS=1	# Make sure OS-type-specific variables are defined only once

CC=$(ARM_CC)    		# 32-bits C compiler
AS=$(ARM_AS)	    		# 32-bits Assembler
LK=$(ARM_LK)		  	# 32-bits Linker			
LB=$(ARM_LB)	  	 	# 32-bits librarian     		
RC=$(ARM_RC)    		# 32-bits Resource compiler	
MT=$(ARM_MT)    		# 32-bits Manifest Tool		

PATH=$(ARM_PATH)  		# 32-bits programs paths		
INCPATH=$(ARM_INCPATH)		# 32-bits include files paths	
MSVCINCLUDE=$(ARM_VCINC:\=/)	# Path of MSVC compiler include files, without quotes, and with forward slashes
UCRTINCLUDE=$(ARM_CRTINC:\=/)	# Path of MSVC CRT library include files, without quotes, and with forward slashes
WSDKINCLUDE=$(ARM_WINSDKINC:\=/) # Path of Windows SDK include files, without quotes, and with forward slashes
LIB=$(ARM_LIBPATH)		# 32-bits libraries paths		

WINVER=6.2			# Target Windows 8
MACHINE=ARM			# Target CPU = 32-bits ARM
USEDOSSTUB=0			# Do not use an MS-DOS stub

!ENDIF # !DEFINED(DISPATCH_OS)

# Get common WIN32 definitions and rules from win32.mak
!INCLUDE <win32.mak>
