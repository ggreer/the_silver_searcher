###############################################################################
#									      #
#   File name:	    win64.mak						      #
#									      #
#   Description:    A NMake makefile to build WIN64 programs.		      #
#									      #
#   Notes:	    Use with make.bat, which defines the necessary variables. #
#		    Usage: make -f win64.mak [targets]			      #
#									      #
#		    Targets:						      #
#		    clean	      Erase all files in WIN64 subdirectory.  #
#		    {prog}.exe	      Build WIN64[\DEBUG]\{prog}.exe.	      #
#		    {prog}.obj	      Build WIN64[\DEBUG]\OBJ\{prog}.obj.     #
#		    {prog}.res	      Build WIN64[\DEBUG]\OBJ\{prog}.res.     #
#		    WIN64\{prog}.exe       Build the Win64 release version.   #
#		    WIN64\DEBUG\{prog}.exe Build the Win64 debug version.     #
#		    WIN64\OBJ\{prog}.obj       Compile the release version.   #
#		    WIN64\DEBUG\OBJ\{prog}.obj Compile the debug version.     #
#		    WIN64\OBJ\{prog}.res       Compile release resources.     #
#		    WIN64\DEBUG\OBJ\{prog}.res Compile debug resources.	      #
#									      #
#		    Command-line definitions:				      #
#		    DEBUG=0	 Build the release ver. (<=> target WIN64)    #
#		    DEBUG=1	 Build the debug ver. (<=> target WIN64\DEBUG)#
#		    OUTDIR=path  Output to path\WIN64\. Default: To bin\WIN64\#
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
#		    Another design goal is to use that same win64.mak	      #
#		    in complex 1-project environments (One Files.mak defines  #
#		    all project components); And in simple multiple-project   #
#		    environments (No Files.mak; Most programs have a single   #
#		    source file, and use default compiler options).	      #
#									      #
#		    The following macros / environment variables must be      #
#		    predefined. This allows to use the same makefile on       #
#		    machines with various locations for the build tools.      #
#									      #
#		    WIN64_AS	    	64-bits Assembler		      #
#		    WIN64_CC    	64-bits C compiler		      #
#		    WIN64_INCPATH  	64-bits include files paths	      #
#		    WIN64_LK	  	64-bits Linker			      #
#		    WIN64_LIBPATH	64-bits libraries paths		      #
#		    WIN64_LB	   	64-bits librarian     		      #
#		    WIN64_RC    	64-bits Resource compiler	      #
#		    WIN64_MT    	64-bits Manifest Tool		      #
#		    TMP	    	Temporary directory	 		      #
#									      #
#  History:								      #
#									      #
#    2000-09-21 JFL Adapted from earlier projects.			      #
#    2001-01-11 JFL Added generation of 32-bit EXE.			      #
#    2001-01-11 JFL Generalized for use on multiple programs.		      #
#    2002-04-08 JFL Use debug versions of the MultiOS libraries.	      #
#    2002-05-21 JFL Build either a debug or a release version.		      #
#    2002-11-29 JFL Give the same name to the 16-bits EXE as to the 32-bits   #
#		    version, but put it in a different directory.	      #
#    2002-12-18 JFL And name these directories DOS and WIN32 repectively.     #
#    2003-03-21 JFL Move file dependancies to a sub-makefile called Files.mak.#
#		    Restructure directories as DOS, DOS\OBJ, DOS\LIST, etc.   #
#    2003-03-31 JFL Renamed as DosWin32.mak, and coupled with new make.bat.   #
#    2003-04-15 JFL Added inference rules for making {OS_NAME}\{prog}.exe     #
#		     targets.						      #
#    2003-06-16 JFL Fixed bound DOS+Win32 builds, broken in last change.      #
#    2003-06-16 JFL Fixed problem with files.mak, which must NOT be present   #
#                    if we don't mean to use it.			      #
#    2010-03-19 JFL Added support for building 64-bits Windows programs.      #
#    2010-03-26 JFL Restrucured macros w. more generic 16/32/64 bits versions.#
#    2010-04-07 JFL Added dynamic generation of OBJECTS by src2objs.bat.      #
#		    Split in 4: DosWin.mak dos.mak win32.mak win64.mak        #
#    2012-10-17 JFL Changed the output directories structure to:	      #
#		    WIN64\Debug][\OBJ|\LIST]				      #
#		    Removed the special handling of MultiOS.lib.	      #
#    2015-01-16 JFL Pass selected cmd-line definitions thru to sub-make files.#
#    2015-11-03 JFL Added rules to build a library from a .mak file.          #
#    2015-11-13 JFL Use new generic environment definition variables.         #
#    2015-12-07 JFL Added support for a base output directory other than .\   #
#    2015-12-10 JFL Keep win64-specific definitions, and include win32.mak.   #
#    2016-01-07 JFL Do not use /Zp to avoid alignment issues.                 #
#    2016-09-15 JFL Added WSDKINCLUDE definition.                             #
#    2016-10-11 JFL Adapted for use in SysToolsLib global C include dir.      #
#    2016-10-21 JFL Define ML64.EXE default flags.			      #
#    2018-02-28 JFL Added $(LSX) Library SuffiX definition.		      #
#									      #
#      © Copyright 2016-2018 Hewlett Packard Enterprise Development LP        #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

T=WIN64				# Target OS

!IF !DEFINED(DISPATCH_OS)

T_VARS=1	# Make sure OS-type-specific variables are defined only once

CC=$(WIN64_CC)    		# 64-bits C compiler
AS=$(WIN64_AS)	    		# 64-bits Assembler
LK=$(WIN64_LK)		  	# 64-bits Linker			
LB=$(WIN64_LB)	  	 	# 64-bits librarian     		
RC=$(WIN64_RC)    		# 64-bits Resource compiler	
MT=$(WIN64_MT)    		# 64-bits Manifest Tool		

PATH=$(WIN64_PATH)  		# 64-bits programs paths		
INCPATH=$(WIN64_INCPATH)	# 64-bits include files paths	
MSVCINCLUDE=$(WIN64_VCINC:\=/)  # Path of MSVC compiler include files, without quotes, and with forward slashes
UCRTINCLUDE=$(WIN64_CRTINC:\=/) # Path of MSVC CRT library include files, without quotes, and with forward slashes
WSDKINCLUDE=$(WIN64_WINSDKINC:\=/) # Path of Windows SDK include files, without quotes, and with forward slashes
LIB=$(WIN64_LIBPATH)		# 64-bits libraries paths		

!IF !DEFINED(CFLAGS)
CFLAGS=/FAsc /Os /W4 /Zi # Do not use /Zp to avoid alignment issues
!ENDIF # !DEFINED(CFLAGS)

!IF !DEFINED(AFLAGS)
AFLAGS=/Cx /Sn /Zi /D_WIN64	# Contrary to the C compiler, the assembler does not define _WINxx variables
!ENDIF # !DEFINED(AFLAGS)

WINVER=5.2			# First AMD64 Windows is not XP, but XP/64 ~= Server 2003 = Windows 5.2
MACHINE=X64			# Target CPU = AMD 64-bits X64
USEDOSSTUB=0			# Do not use an MS-DOS stub

# Library SuffiX. For storing multiple versions of the same library in a single directory.
LSX=w64

!ENDIF # !DEFINED(DISPATCH_OS)

# Get common WIN32 definitions and rules from win32.mak
!INCLUDE <win32.mak>
