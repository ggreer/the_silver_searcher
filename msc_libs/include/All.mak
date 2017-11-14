###############################################################################
#									      #
#   File name:	    All.mak						      #
#									      #
#   Description:    NMake file to build all MS OS versions of a program.      #
#									      #
#   Notes:	    Depends on a series of target-OS-specific make files,     #
#		    that know how to build a program for that target OS.      #
#		    Ex: BIOS.mak, DOS.mak, WIN32.mak, WIN64.mak		      #
#		    By convention, each such make file outputs the files it   #
#		    builds into a directory tree with that base name.	      #
#		    Ex: BIOS\*, DOS\*, WIN32\*, WIN64\*			      #
#		    							      #
#		    Use with configure.bat and make.bat, which define the     #
#		    necessary variables.				      #
#		    							      #
#		    Usage: make.bat [options] [definitions] [targets]	      #
#									      #
#		    Sample targets:					      #
#		    clean	Erase all output files built for all targets. #
#		    {prog}.com	           Build BIOS and DOS {prog}.com.     #
#		    Debug\{prog}.com	   Build the debug versions of the 2. #
#		    {prog}.exe	     Build DOS, WIN32, and WIN64 {prog}.exe.  #
#		    Debug\{prog}.exe	   Build the debug versions of the 3. #
#		    BIOS\{prog}.com        Build the BIOS release version.    #
#		    BIOS\Debug\{prog}.com  Build the BIOS debug version.      #
#		    DOS\{prog}.com         Build the DOS release version.     #
#		    DOS\Debug\{prog}.com   Build the DOS debug version.	      #
#		    DOS\{prog}.exe         Build the DOS release version.     #
#		    DOS\Debug\{prog}.exe   Build the DOS debug version.	      #
#		    WIN32\{prog}.exe       Build the WIN32 release version.   #
#		    WIN32\Debug\{prog}.exe Build the WIN32 debug version.     #
#		    WIN64\{prog}.exe       Build the WIN64 release version.   #
#		    WIN64\Debug\{prog}.exe Build the WIN64 debug version.     #
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
#		    generic targets is used, then the default Dirs.mak or     #
#		    Files.mak are used instead. Same definitions.	      #
#									      #
#		    Note that these sub-make files are designed to be	      #
#		    OS-independant. The goal is to reuse them to build	      #
#		    the same program under Unix/Linux too. So for example,    #
#		    all paths must contain forward slashes. Also avoid using  #
#		    nmake-specific !conditional preprocessing directives.     #
#									      #
#		    Another design goal is to use that same All.mak	      #
#		    in complex 1-project environments (One Files.mak defines  #
#		    all project components); And in simple multiple-project   #
#		    environments (No Files.mak; Most programs have a single   #
#		    source file, and use default compiler options).	      #
#		    							      #
#		    To add support for a new OS or processor:                 #
#		    - Choose a unique name for that OS/Proc pair. Ex: CPM86   #
#		    - Update configure.bat to locate the necessary build      #
#		      tools, and set the necessary variables. Ex: CPM86_CC, ..#
#		    - Create a new CPM86.mak file that knows how to build for #
#		      that OS/proc pair. Use WIN95.mak as a sample for a make #
#		      file that is derived from a generic one (WIN32.mak).    #
#		    - Manage below the OS=CPM86 value;                        #
#		      Add definitions for DOCPM86 and IFCPM86.                #
#		    - Add below inference rules for building CPM86 targets.   #
#		    - Add below a set of dispatching rules for these targets. #
#		      DISPATCH_OS=CPM86                                       #
#		      !INCLUDE "Dispatch.mak"                                 #
#		    - Update the help in the end to show the new capabilities.#
#		    							      #
#  History:								      #
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
#    2010-03-26 JFL Restructured macros w.more generic 16/32/64 bits versions.#
#    2010-04-07 JFL Added dynamic generation of OBJECTS by src2objs.bat.      #
#		    Split in 4: DosWin.mak dos.mak win32.mak win64.mak        #
#    2012-10-04 JFL Added rules for the case where we just have a .mak file   #
#		    that matches the target names.			      #
#    2012-10-17 JFL Changed the output directories structure to:	      #
#		    [DOS[\$(MEM)]|WIN32|WIN64][\Debug][\OBJ|\LIST]	      #
#		    Removed the special handling of MultiOS.lib.	      #
#    2015-01-06 JFL Target prog.exe also makes the WIN64 version.	      #
#		    Added target debug\prog.exe, to make the 3 debug versions.#
#		    The all target now makes the 6 normal and debug versions. #
#    2015-01-16 JFL Added variable OS for specifying the target OSs list.     #
#		    Pass selected cmd-line definitions thru to sub-make files.#
#    2015-10-21 JFL If PROGRAMS is defined, build $(PROGRAMS) by default.     #
#    2015-10-27 JFL Added support for .com targets.			      #
#		    Added support for BIOS OS targets.			      #
#		    Fixed the OS variable default value handling.             #
#    2015-11-05 JFL Added WINVER variable to force the target OS version.     #
#    2015-11-06 JFL Removed all OS-specific inference rules, and use          #
#		    Dispatch.mak to get them from the OS-specific make files. #
#		    Renamed as All.mak to reflect new generic capabilities.   #
#    2015-12-15 JFL Added dynamic checking of prerequisites set in Files.mak. #
#    2016-04-11 JFL Renamed NODOSLIB as BIOSLIB.                              #
#    2016-04-22 JFL Renamed the MULTIOS library as SYSLIB.		      #
#    2016-09-28 JFL Avoid having the word "Error" in the log unnecessarily.   #
#		    Added support for the optional OUTDIR.		      #
#		    Rewrote the all rule handling to record errors, and	      #
#		    report them in the end.				      #
#    2016-10-03 JFL Added target list_programs.                               #
#		    Fixed errors comparing the WIN95 and WIN32 C compilers.   #
#    2016-10-04 JFL Updated fix comparing the WIN95 and WIN32 C compilers.    #
#		    Use the shell PID to generate unique temp file names.     #
#		    Display messages only if variable MESSAGES is defined.    #
#    2016-10-04 JFL Target distclean must delete config.*.bat files.	      #
#    2016-10-11 JFL Adapted for use in SysToolsLib global C include dir.      #
#    2016-10-20 JFL Added missing inference rules to build .asm programs.     #
#    2016-11-07 JFL Do not hide any command that's part of a build.           #
#    2017-02-22 JFL Added mechanism to build subprojects defined in Files.mak.#
#		    Avoid building OS targets with no corresp. make file.     #
#                   Allow building a makefile-defined executable.	      #
#    2017-02-28 JFL Bug fix: Enclose all $(MAKEPATH) references in "quotes".  #
#    2017-03-02 JFL Added the CLEAN_DIRS and CLEAN_FILES variables.           #
#    2017-03-13 JFL Fix build if environment variable OS is not defined.      #
#    2017-08-29 JFL Bugfix: The help target did output a "1 file copied" msg. #
#    2017-10-22 JFL Changed OUTDIR default to the bin subdirectory.           #
#    2017-10-30 JFL Corrected a typo in the help message.                     #
#    2017-11-13 JFL Added inference rules to build a DLL.		      #
#		    							      #
#       © Copyright 2016-2017 Hewlett Packard Enterprise Development LP       #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

.SUFFIXES: # Clear the predefined suffixes list.
.SUFFIXES: .com .exe .sys .obj .asm .c .r .cpp .res .rc .def .manifest .mak

!IFNDEF TMP
!IFDEF TEMP
TMP=$(TEMP)
!ELSE
TMP=.
!ENDIF
!ENDIF

!IF !DEFINED(OUTDIR)
OUTDIR=bin
OD=bin\			# Output directory - In the default bin subdirectory
!ELSEIF "$(OUTDIR)"=="."
OD=			# Output directory - In the current directory
!ELSE # It's defined and not empty
OD=$(OUTDIR)\		# Output directory - In the specified directory
!ENDIF

###############################################################################
#									      #
#			        Definitions				      #
#									      #
###############################################################################

# Command-line definitions that need carrying through to sub-make instances
# Note: Cannot redefine MAKEFLAGS, so defining an alternate variable instead.
MAKEDEFS=
!IF DEFINED(WINVER)	# Windows target version. 4.0=Win95/NT4 5.1=XP 6.0=Vista ...
MAKEDEFS=$(MAKEDEFS) "WINVER=$(WINVER)"
!ENDIF
!IF DEFINED(MEM)	# Memory model for DOS compilation. T|S|C|D|L|H. Default=S.
MAKEDEFS=$(MAKEDEFS) "MEM=$(MEM)"
!ENDIF

MAKEPATH=.
!IF (!EXIST("$(All.mak)")) && EXIST("$(STINCLUDE)\$(All.mak)")
MAKEPATH=$(STINCLUDE)
!ENDIF

# $(OS) = List of target operating systems to build for, separated by spaces
# Note: The OS variable here conflicts with Windows' %OS%, defaulting to Windows_NT
!IF !DEFINED(OS) || "$(OS)"=="Windows_NT" # ie. if OS is not specified on the command line
OS=/		# Initialize with a recognizable string that we'll remove later
!IF DEFINED(DOS_CC) && EXIST("$(MAKEPATH)\DOS.mak")
OS=$(OS) DOS
!ENDIF
!IF DEFINED(WIN95_CC) && DEFINED(WIN32_CC) && EXIST("$(MAKEPATH)\WIN95.mak") # Do not combine with next line, else there's a syntax error if WIN95_CC is not defined.
!IF ($(WIN95_CC) != $(WIN32_CC)) # CC paths have "quotes" already
OS=$(OS) WIN95
!ENDIF
!ENDIF
!IF DEFINED(WIN32_CC) && EXIST("$(MAKEPATH)\WIN32.mak")
OS=$(OS) WIN32
!ENDIF
!IF DEFINED(WIN64_CC) && EXIST("$(MAKEPATH)\WIN64.mak")
OS=$(OS) WIN64
!ENDIF
# Note: Don't attempt to build IA64 or ARM versions by default
OS=$(OS:/ =)	# Remove the initial / and the first following space
OS=$(OS:/=)	# Again, in the unlikely case that none of the default OSs matched
!ENDIF

!IF "$(OS)"=="all"
OS=
!IF DEFINED(DOS_CC) && EXIST("$(MAKEPATH)\bios.mak")
OS=$(OS) BIOS
!ENDIF
!IF DEFINED(DOS_CC) && EXIST("$(MAKEPATH)\dos.mak")
OS=$(OS) DOS
!ENDIF
!IF DEFINED(WIN95_CC) && DEFINED(WIN32_CC) && EXIST("$(MAKEPATH)\WIN95.mak") # Do not combine with next line, else there's a syntax error if WIN95_CC is not defined.
!IF ($(WIN95_CC) != $(WIN32_CC)) # CC paths have "quotes" already
OS=$(OS) WIN95
!ENDIF
!ENDIF
!IF DEFINED(WIN32_CC) && EXIST("$(MAKEPATH)\WIN32.mak")
OS=$(OS) WIN32
!ENDIF
!IF DEFINED(IA64_CC) && EXIST("$(MAKEPATH)\IA64.mak")
OS=$(OS) IA64
!ENDIF
!IF DEFINED(WIN64_CC) && EXIST("$(MAKEPATH)\WIN64.mak")
OS=$(OS) WIN64
!ENDIF
!IF DEFINED(ARM_CC) && EXIST("$(MAKEPATH)\ARM.mak")
OS=$(OS) ARM
!ENDIF
!ENDIF

# Report start options
!IF DEFINED(MESSAGES)
!MESSAGE Started All.mak in $(MAKEDIR). OS=$(OS)
!ENDIF

# Convert that text list to boolean variables, one for each OS.
DOBIOS=0
DODOS=0
DOWIN95=0
DOWIN32=0
DOIA64=0
DOWIN64=0
DOARM=0
!IF [for %o in ($(OS)) do @if /i "%o"=="BIOS" exit 1]
DOBIOS=1
!ENDIF
!IF [for %o in ($(OS)) do @if /i "%o"=="DOS" exit 1]
DODOS=1
!ENDIF
!IF [for %o in ($(OS)) do @if /i "%o"=="WIN95" exit 1]
DOWIN95=1
!ENDIF
!IF [for %o in ($(OS)) do @if /i "%o"=="WIN32" exit 1]
DOWIN32=1
!ENDIF
!IF [for %o in ($(OS)) do @if /i "%o"=="IA64" exit 1]
DOIA64=1
!ENDIF
!IF [for %o in ($(OS)) do @if /i "%o"=="WIN64" exit 1]
DOWIN64=1
!ENDIF
!IF [for %o in ($(OS)) do @if /i "%o"=="ARM" exit 1]
DOARM=1
!ENDIF
# !MESSAGE DOBIOS=$(DOBIOS) DODOS=$(DODOS) DOWIN32=$(DOWIN32) DOWIN64=$(DOWIN64)
# Generate guard macros for each OS
IFBIOS=rem
IFDOS=rem
IFWIN95=rem
IFWIN32=rem
IFIA64=rem
IFWIN64=rem
IFARM=rem
!IF $(DOBIOS)
IFBIOS=
!ENDIF
!IF $(DODOS)
IFDOS=
!ENDIF
!IF $(DOWIN95)
IFWIN95=
!ENDIF
!IF $(DOWIN32)
IFWIN32=
!ENDIF
!IF $(DOIA64)
IFIA64=
!ENDIF
!IF $(DOWIN64)
IFWIN64=
!ENDIF
!IF $(DOARM)
IFARM=
!ENDIF
# !MESSAGE IFBIOS=$(IFBIOS) IFDOS=$(IFDOS) IFWIN32=$(IFWIN32) IFIA64=$(IFIA64) IFWIN64=$(IFWIN64) IFARM=$(IFARM)

MSG=>con echo		# Command for writing a progress message on the console
HEADLINE=$(MSG).&$(MSG)	# Output a blank line, then a message
REPORT_FAILURE=$(MSG) ... FAILED. & exit /b # Report that a build failed, and forward the error code.
SUBMAKE=$(MAKE) /$(MAKEFLAGS) /F "$(MAKEFILE)" $(MAKEDEFS) # Recursive call to this make file

###############################################################################
#									      #
#			       Inference rules				      #
#									      #
###############################################################################

# Inference rule to build a simple program. Build BIOS, DOS, Win32, and Win64 versions.
.c.com:
    @echo Applying inference rule .c.com:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $@

.c.exe:
    @echo Applying inference rule .c.exe:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $@

.cpp.com:
    @echo Applying inference rule .cpp.com:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $@

.cpp.exe:
    @echo Applying inference rule .cpp.exe:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $@

.asm.exe:
    @echo Applying inference rule .asm.exe:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $@

# Inference rule to build a makefile-defined executable. Build BIOS, DOS, Win32, and Win64 versions.
.mak.exe:
    @echo Applying inference rule .mak.exe:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $@

# Inference rule to build a makefile-defined library. Build BIOS, DOS, Win32, and Win64 versions.
.mak.lib:
    @echo Applying inference rule .mak.lib:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $@

# Inference rule to build a makefile-defined DLL. Build BIOS, DOS, Win32, and Win64 versions.
.mak.dll:
    @echo Applying inference rule .mak.dll:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $@

# Inference rule to build a simple program. Build BIOS, DOS, Win32, and Win64 debug versions.
{.\}.c{Debug\}.com:
    @echo Applying inference rule {.\}.c{Debug\}.com:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $(OD)BIOS\$@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $(OD)DOS\$@

{.\}.c{Debug\}.exe:
    @echo Applying inference rule {.\}.c{Debug\}.exe:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $(OD)BIOS\$@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $(OD)DOS\$@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $(OD)WIN95\$@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $(OD)WIN32\$@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $(OD)IA64\$@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $(OD)WIN64\$@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $(OD)ARM\$@

{.\}.cpp{Debug\}.com:
    @echo Applying inference rule {.\}.cpp{Debug\}.com:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $(OD)BIOS\$@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $(OD)DOS\$@

{.\}.cpp{Debug\}.exe:
    @echo Applying inference rule {.\}.cpp{Debug\}.exe:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $(OD)BIOS\$@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $(OD)DOS\$@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $(OD)WIN95\$@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $(OD)WIN32\$@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $(OD)IA64\$@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $(OD)WIN64\$@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $(OD)ARM\$@

{.\}.asm{Debug\}.exe:
    @echo Applying inference rule {.\}.asm{Debug\}.exe:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $(OD)BIOS\$@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $(OD)DOS\$@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $(OD)WIN95\$@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $(OD)WIN32\$@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $(OD)IA64\$@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $(OD)WIN64\$@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $(OD)ARM\$@

# Inference rule to build a makefile-defined executable. Build BIOS, DOS, Win32, and Win64 versions.
{.\}.mak{Debug\}.exe:
    @echo Applying inference rule {.\}.mak{Debug\}.exe:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $(OD)BIOS\$@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $(OD)DOS\$@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $(OD)WIN95\$@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $(OD)WIN32\$@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $(OD)IA64\$@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $(OD)WIN64\$@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $(OD)ARM\$@

# Inference rule to build a makefile-defined library. Build BIOS, DOS, Win32, and Win64 versions.
{.\}.mak{Debug\}.lib:
    @echo Applying inference rule {.\}.mak{Debug\}.lib:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $(OD)BIOS\$@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $(OD)DOS\$@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $(OD)WIN95\$@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $(OD)WIN32\$@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $(OD)IA64\$@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $(OD)WIN64\$@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $(OD)ARM\$@

# Inference rule to build a makefile-defined DLL. Build BIOS, DOS, Win32, and Win64 versions.
{.\}.mak{Debug\}.dll:
    @echo Applying inference rule {.\}.mak{Debug\}.dll:
    $(IFBIOS)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\BIOS.mak"  $(MAKEDEFS) $(OD)BIOS\$@
    $(IFDOS)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\DOS.mak"   $(MAKEDEFS) $(OD)DOS\$@
    $(IFWIN95) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN95.mak" $(MAKEDEFS) $(OD)WIN95\$@
    $(IFWIN32) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN32.mak" $(MAKEDEFS) $(OD)WIN32\$@
    $(IFIA64)  $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\IA64.mak"  $(MAKEDEFS) $(OD)IA64\$@
    $(IFWIN64) $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\WIN64.mak" $(MAKEDEFS) $(OD)WIN64\$@
    $(IFARM)   $(MAKE) /$(MAKEFLAGS) /f "$(MAKEPATH)\ARM.mak"   $(MAKEDEFS) $(OD)ARM\$@

# Inference rules to build something for DOS, WIN32 and WIN64 respectively
# Get them from their respective DOS.mak, WIN32.mak, WIN64.mak make files, etc.

# Does not work, due to late evaluation of nmake macros.
# So using make.bat instead to generate the correct make file.
# DISPATCH_OS=BIOS
# !INCLUDE "Dispatch.mak"
# 
# DISPATCH_OS=DOS
# !INCLUDE "Dispatch.mak"
# 
# DISPATCH_OS=WIN95
# !INCLUDE "Dispatch.mak"
# 
# DISPATCH_OS=WIN32
# !INCLUDE "Dispatch.mak"
# 
# DISPATCH_OS=IA64
# !INCLUDE "Dispatch.mak"
# 
# DISPATCH_OS=WIN64
# !INCLUDE "Dispatch.mak"
# 
# DISPATCH_OS=ARM
# !INCLUDE "Dispatch.mak"

###############################################################################
#									      #
#			        Specific rules				      #
#									      #
###############################################################################

default: all

!IF EXIST("Files.mak")
!  INCLUDE Files.mak	# Set variable DIRS, SOURCES, ALL, MODULE, CLEAN_DIRS, CLEAN_FILES, etc
!ENDIF

!IF DEFINED(DIRS)
# List sub-directories to build, one per line
list_dirs:
    for %%d in ($(DIRS)) do @echo %%~d

# Build individual modules in the specified subdirectories
$(DIRS): NUL
    rem &:# Build the module in dir $@, using the current log file
    $(BMAKE) -C $@ || $(MSG) $@ build failed. Error !ERRORLEVEL! & exit /b
!ENDIF

!IF DEFINED(MODULE) # Defined in Files.mak
module_name:
    @echo $(MODULE)
!ENDIF

!IF DEFINED(ALL) || DEFINED(DIRS)
all: $(REQS) $(DIRS) $(ALL)
!ELSE # Another scheme for defining all goals, using $(PROGRAMS)
all: $(REQS) # Having a batch file is necessary for dynamically updating the *FAILED variables.
    cmd /c <<"$(TMP)\build_all.$(PID).bat" || exit /b &:# Using the shell PID to generate a unique name, to avoid conflicts in case of // builds.
        @echo off
        setlocal EnableExtensions EnableDelayedExpansion
        set "PROGRAMS=$(PROGRAMS)"
        if defined PROGRAMS ( :# Build the list of programs defined in Files.mak 
	    echo Applying All.mak all rule with PROGRAMS=!PROGRAMS!
	) else ( :# As a last resort, try compiling all C and C++ files in the current directory
	    echo Applying All.mak all rule with PROGRAMS undefined
	    for %%f in (*.c *.cpp) do set "PROGRAMS=!PROGRAMS! %%~nf.exe"
	    set PROGRAMS=!PROGRAMS:~1!
	    echo Trying to compile all C and C++ files: set PROGRAMS=!PROGRAMS!
	)
        set "NFAILED=0"
        set "WHAT_FAILED="
        for %%f in (!PROGRAMS!) do (
	    $(HEADLINE) Building %%~f
	    $(SUBMAKE) "OS=$(OS)" "%%~f" "Debug\%%~f"
	    if errorlevel 1 (
		set /A "NFAILED+=1"
		set "WHAT_FAILED=!WHAT_FAILED! %%~f"
		echo All.mak: %%~f build failed
	    )
        )
        if defined WHAT_FAILED set "WHAT_FAILED=%WHAT_FAILED:~1%"
        echo NFAILED=%NFAILED% WHAT_FAILED=(%WHAT_FAILED%)
        if not %NFAILED%==0 $(HEADLINE) Error: Builds failed: %WHAT_FAILED%
        exit /b %NFAILED%
<<
!ENDIF

# Dummy targets for dynamically checking common prerequisites
ERRMSG=>&2 echo Error: # Use a variable, to avoid getting the word "Error" in the build log when there's no error
BiosLib_library:
    if not defined BIOSLIB %ERRMSG% The BiosLib library is not configured & exit /b 1
    if not exist %BIOSLIB%\clibdef.h %ERRMSG% The BiosLib library is not configured correctly & exit /b 1
    if not exist %BIOSLIB%\bios.lib %ERRMSG% The BiosLib library must be built first & exit /b 1

LoDosLib_library:
    if not defined LODOSLIB %ERRMSG% The LoDosLib library is not configured & exit /b 1
    if not exist %LODOSLIB%\lodos.h %ERRMSG% The LoDosLib library is not configured correctly & exit /b 1
    if not exist %LODOSLIB%\lodos.lib %ERRMSG% The LoDosLib library must be built first & exit /b 1

SysLib_library:
    if not defined SYSLIB %ERRMSG% The SysLib library is not configured & exit /b 1
    if not exist %SYSLIB%\oprintf.h %ERRMSG% The SysLib library is not configured correctly & exit /b 1
    if not exist %SYSLIB%\$(OD)lib\*.lib %ERRMSG% The SysLib library must be built first & exit /b 1

MsvcLibX_library:
    if not defined MSVCLIBX %ERRMSG% The MsvcLibX library is not configured & exit /b 1
    if not exist %MSVCLIBX%\include\msvclibx.h %ERRMSG% The MsvcLibX library is not configured correctly & exit /b 1
    if not exist %MSVCLIBX%\$(OD)lib\*.lib %ERRMSG% The MsvcLibX library must be built first & exit /b 1

PModeLib_library:
    if not defined PMODELIB %ERRMSG% The PModeLib library is not configured & exit /b 1
    if not exist %PMODELIB%\pmode.h %ERRMSG% The PModeLib library is not configured correctly & exit /b 1
    if not exist %PMODELIB%\pmode.lib %ERRMSG% The PModeLib library must be built first & exit /b 1

!IF !DEFINED(ZIPFILE)
ZIPFILE=sources.zip
ZIPSOURCES=*.c *.cpp *.h *.asm *.inc *Makefile *.mak *.bat *.rc *.def *.manifest
!ENDIF

$(ZIPFILE): $(ZIPSOURCES)
    $(MSG) Building $@ ...
    if exist $@ del $@
    set PATH=$(PATH);C:\Program Files\7-zip
    7z.exe a $@ $**
    $(MSG) ... done

dist zip: $(ZIPFILE)

# List PROGRAMS defined in Files.mak
list_programs:
    @set PROGRAMS=$(PROGRAMS)
    @if defined PROGRAMS echo $(PROGRAMS)

# Erase all output files
clean mostlyclean distclean:
!IF DEFINED(DIRS)
    for %%d in ($(DIRS)) do @$(BMAKE) -C %%d $@ 
!ENDIF
    if exist "$(MAKEPATH)\BIOS.mak"  $(MAKE) /$(MAKEFLAGS) /c /s /f "$(MAKEPATH)\BIOS.mak" clean
    if exist "$(MAKEPATH)\DOS.mak"   $(MAKE) /$(MAKEFLAGS) /c /s /f "$(MAKEPATH)\DOS.mak" clean
    if exist "$(MAKEPATH)\WIN95.mak" $(MAKE) /$(MAKEFLAGS) /c /s /f "$(MAKEPATH)\WIN95.mak" clean
    if exist "$(MAKEPATH)\WIN32.mak" $(MAKE) /$(MAKEFLAGS) /c /s /f "$(MAKEPATH)\WIN32.mak" clean
    if exist "$(MAKEPATH)\IA64.mak"  $(MAKE) /$(MAKEFLAGS) /c /s /f "$(MAKEPATH)\IA64.mak" clean
    if exist "$(MAKEPATH)\WIN64.mak" $(MAKE) /$(MAKEFLAGS) /c /s /f "$(MAKEPATH)\WIN64.mak" clean
    if exist "$(MAKEPATH)\ARM.mak"   $(MAKE) /$(MAKEFLAGS) /c /s /f "$(MAKEPATH)\ARM.mak" clean
!IF DEFINED(OUTDIR) && "$(OUTDIR)" != "" && "$(OUTDIR)" != "." && "$(OUTDIR)" != ".."
    -rd /S /Q $(OUTDIR)	>NUL 2>&1
!ENDIF
    -del /Q *.bak	>NUL 2>&1
    -del /Q *~		>NUL 2>&1
    -del /Q *.log	>NUL 2>&1
    -if "$@"=="distclean" del /Q config.*.bat >NUL 2>&1
!IF DEFINED(CLEAN_DIRS) # Then clean each directory, and remove it if empty
    -for %%d in ($(CLEAN_DIRS)) do @pushd %%d & $(SUBMAKE) $@ & popd & rd %%d 2>NUL
!ENDIF
!IF DEFINED(CLEAN_FILES) # Then clean each file
    -for %%d in ($(CLEAN_FILES)) do @if exist %%d del %%d >NUL 2>&1
!ENDIF
!IF DEFINED(DISTCLEAN_FILES) # Then clean each file
    -if "$@"=="distclean" for %%d in ($(DISTCLEAN_FILES)) do @if exist %%d del %%d >NUL 2>&1
!ENDIF

# Convert sources from Windows to Unix formats
UNIXTEMP=$(TMP)\$(PROGRAM)
w2u:
    echo Converting sources into Unix format, in $(UNIXTEMP) >con
    -rd /S /Q $(UNIXTEMP) >nul
    md $(UNIXTEMP)
    for %%F in ($(SOURCES) *.h *Makefile *.mak go go.bat make.bat *.htm) do call w2u %%F $(UNIXTEMP)\%%~nxF

# Help message describing the targets
help:
    type <<
Usage: make.bat [options] [nmake_options] [macro_definitions] [targets] ...

Macro definitions:     (They must be quoted, else the = sign will be lost)
  "DEBUG=1"            Generate the debug version. <==> Target in a Debug\ dir.
  "MEM=L"              Build the DOS version w. large memory model. Dflt: T or S
  "OS=BIOS DOS WIN95 WIN32 WIN64"   List of target OSs to build for
  "WINVER=4.0"         Target OS version. 4.0=Win95/NT4, 5.1=WinXP, 6.1=Win7

Targets:
  all                    Build all available sources
  clean                  Erase all output files in BIOS, DOS, WIN32, WIN64
  distclean              Erase all output files and all configuration files
  {prog}.com             Build BIOS and DOS versions of {prog}.com
  {prog}.exe             Build DOS and all Windows versions of {prog}.exe
  Debug\{prog}.exe       Build BIOS and DOS versions of the same
  {prog}.exe             Build DOS, WIN32, and WIN64 versions of {prog}.exe
  Debug\{prog}.exe       Build DOS, WIN32, and WIN64 debug versions of the same
  BIOS\{prog}.com        Build the BIOS release version of {prog}.com
  BIOS\Debug\{prog}.com  Build the BIOS debug version of {prog}.com
  DOS\{prog}.com         Build the DOS release version of {prog}.com
  DOS\Debug\{prog}.com   Build the DOS debug version of {prog}.com
  DOS\{prog}.exe         Build the DOS release version of {prog}.exe
  DOS\Debug\{prog}.exe   Build the DOS debug version of {prog}.exe
  WIN95\{prog}.exe       Build the WIN95 release version of {prog}.exe
  WIN95\Debug\{prog}.exe Build the WIN95 debug version of {prog}.exe
  WIN32\{prog}.exe       Build the WIN32 release version of {prog}.exe
  WIN32\Debug\{prog}.exe Build the WIN32 debug version of {prog}.exe
  WIN64\{prog}.exe       Build the WIN64 release version of {prog}.exe
  WIN64\Debug\{prog}.exe Build the WIN64 debug version of {prog}.exe
  w2u                    Convert all sources to Unix format, into $(UNIXTEMP)
  zip                    Make a zip file with all sources

Also supports .obj and .res to compile C, C++, ASM, and Windows .RC files.
<<NOKEEP
