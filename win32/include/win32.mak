###############################################################################
#									      #
#   File name:	    win32.mak						      #
#									      #
#   Description:    A NMake makefile to build WIN32 programs.		      #
#									      #
#   Notes:	    Use with make.bat, which defines the necessary variables. #
#		    Usage: make -f win32.mak [definitions] [targets]	      #
#									      #
#		    Targets:						      #
#		    clean	      Erase all files in WIN32 subdirectory.  #
#		    {prog}.exe	      Build WIN32[\DEBUG]\{prog}.exe.	      #
#		    {prog}.obj	      Build WIN32[\DEBUG]\OBJ\{prog}.obj.     #
#		    {prog}.res	      Build WIN32[\DEBUG]\OBJ\{prog}.res.     #
#		    WIN32\{prog}.exe       Build the Win32 release version.   #
#		    WIN32\DEBUG\{prog}.exe Build the Win32 debug version.     #
#		    WIN32\OBJ\{prog}.obj       Compile the release version.   #
#		    WIN32\DEBUG\OBJ\{prog}.obj Compile the debug version.     #
#		    WIN32\OBJ\{prog}.res       Compile release resources.     #
#		    WIN32\DEBUG\OBJ\{prog}.res Compile debug resources.	      #
#									      #
#		    Command-line definitions:				      #
#		    DEBUG=0	 Build the release ver. (<=> target WIN32)    #
#		    DEBUG=1	 Build the debug ver. (<=> target WIN32\DEBUG)#
#		    OUTDIR=path  Output to path\WIN32\. Default: To bin\WIN32\#
#		    PROGRAM=name Set the output file base name		      #
#		    WINVER=M.m   4.0=Win95/NT4 5.1=XP 6.0=Vista 6.1=7 6.2=8   #
#		    		 Converted to WINVER=0xM0n for windows.h      #
#		    		 Converted to SUBSYSVER=M.0n for link.exe     #
#		    							      #
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
#		    Another design goal is to use that same win32.mak	      #
#		    in complex 1-project environments (One Files.mak defines  #
#		    all project components); And in simple multiple-project   #
#		    environments (No Files.mak; Most programs have a single   #
#		    source file, and use default compiler options).	      #
#									      #
#		    The following macros / environment variables must be      #
#		    predefined. This allows to use the same makefile on       #
#		    machines with various locations for the build tools.      #
#									      #
#		    WIN32_AS	    	32-bits Assembler		      #
#		    WIN32_CC    	32-bits C compiler		      #
#		    WIN32_INCPATH  	32-bits include files paths	      #
#		    WIN32_LK	  	32-bits Linker			      #
#		    WIN32_LIBPATH	32-bits libraries paths		      #
#		    WIN32_LB	   	32-bits librarian     		      #
#		    WIN32_RC    	32-bits Resource compiler	      #
#		    WIN32_MT    	32-bits Manifest Tool		      #
#		    TMP		    	Temporary directory	 	      #
#									      #
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
#    2010-03-26 JFL Restrucured macros w. more generic 16/32/64 bits versions.#
#    2010-04-07 JFL Added dynamic generation of OBJECTS by src2objs.bat.      #
#		    Split in 4: DosWin.mak dos.mak win32.mak win64.mak        #
#    2012-10-17 JFL Changed the output directories structure to:	      #
#		    WIN32\Debug][\OBJ|\LIST]				      #
#		    Removed the special handling of MultiOS.lib.	      #
#    2014-03-05 JFL Made the pthreads library optional.			      #
#    2014-06-24 JFL Added display include file trees in verbose mode.         #
#    2015-01-16 JFL Pass selected cmd-line definitions thru to sub-make files.#
#    2015-11-03 JFL Added rules to build a library from a .mak file.          #
#    2015-11-13 JFL Use new generic environment definition variables.         #
#    2015-12-07 JFL Added support for a base output directory other than .\   #
#    2016-01-07 JFL Correctly process predefined CFLAGS.                      #
#    2016-04-13 JFL Forward library detections to the C compiler.	      #
#    2016-08-24 JFL Added scripts for removing the UTF-8 BOM from C sources.  #
#    2016-09-15 JFL Added WSDKINCLUDE definition, and pass it to the compiler.#
#    2016-09-28 JFL Also pass WSDKINCLUDE definition to the resource compiler.#
#		    Display FAILED messages when compilation or link fails.   #
#		    Avoid having the word "Error" in the log unnecessarily.   #
#    2016-10-04 JFL Display messages only if variable MESSAGES is defined.    #
#    2016-10-11 JFL Adapted for use in SysToolsLib global C include dir.      #
#    2016-10-20 JFL Added missing inference rules to build .asm programs.     #
#    2016-10-21 JFL Allow having distinct flags for ML.EXE and ML64.EXE.      #
#    2017-02-26 JFL Store the list of object files to use for making libraries#
#		    in a *.lst file. This avoids having too long line issues. #
#    2017-03-02 JFL Added the actual commands for building $(B)\$(EXENAME).   #
#		    Fixed issues when the current directory contains spaces.  #
#		    Fixed src2objs.bat and use it indirectly via src2objs.mak.#
#    2017-03-10 JFL If WINVER is defined but empty, use SDK & linker defaults.#
#    2017-05-12 JFL Use a default asInvoker.manifest for all win32 builds.    #
#    2017-08-29 JFL Bugfix: The help target did output a "1 file copied" msg. #
#    2017-10-22 JFL Changed OUTDIR default to the bin subdirectory.           #
#    2017-11-13 JFL Added rules to build a DLL.				      #
#    2017-11-13 JFL Renamed *.LNK as *.link, and *.LST as *.list.	      #
#		    Added an optional $(POST_LINK_CMD).			      #
#    2018-01-12 JFL Added $(LIBRARIES) to the $(PROGRAM).* dependency list,   #
#		    so that it is relinked if one of its libraries changes.   #
#		    The LIBRARIES variable is generated by lib2libs.mak/.bat  #
#		    based on the LIB and LIBS variables.		      #
#    2018-02-22 JFL Fixed the last change to avoid linking libraries twice.   #
#    2018-02-28 JFL Added $(LSX) Library SuffiX definition.		      #
#    2018-03-01 JFL Always define OBJECTS before invoking lib2libs.mak.       #
#    2018-03-02 JFL Added variable SKIP_THIS, to prevent building specific    #
#		    versions.						      #
#    2019-03-18 JFL Fixed the DOS stub location, based on OUTDIR.	      #
#    2019-11-03 JFL Removed /Zp from CFLAGS as this breaks new SDK 10 builds. #
#    2019-11-13 JFL Added CXXFlags for C++ compilation: Fixes builds w. Boost.#
#		    							      #
#      � Copyright 2016-2018 Hewlett Packard Enterprise Development LP        #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

.SUFFIXES: # Clear the predefined suffixes list.
.SUFFIXES: .exe .obj .asm .c .r .cpp .res .rc .def .manifest .mak .bsc .sbr

###############################################################################
#									      #
#			        Definitions				      #
#									      #
###############################################################################

!IF !DEFINED(T)
T=WIN32				# Target OS
!ENDIF
!IF DEFINED(MESSAGES)
!MESSAGE Started $(T).mak in $(MAKEDIR) # Display this file name, or the caller's name
!ENDIF

# Command-line definitions that need carrying through to sub-make instances
# Note: Cannot redefine MAKEFLAGS, so defining an alternate variable instead.
MAKEDEFS=
!IF DEFINED(WINVER) # Windows target version. 4.0=Win95/NT4 5.1=XP 6.0=Vista ...
MAKEDEFS=$(MAKEDEFS) "WINVER=$(WINVER)"
!ENDIF

THIS_MAKEFILE=win32.mak		# This very make file name
MAKEFILE=$(T).mak		# The OS-specific make file name
!IF (!EXIST("$(MAKEFILE)")) && EXIST("$(STINCLUDE)\$(MAKEFILE)")
MAKEFILE=$(STINCLUDE)\$(MAKEFILE)
THIS_MAKEFILE=$(STINCLUDE)\$(THIS_MAKEFILE)
!ENDIF

# Debug-mode-specific definitions
!IF DEFINED(_DEBUG) || "$(DEBUG)"=="1"
DM=debug			# Debug mode. For information only
DEBUG=1
_DEBUG=				# MS tools define this in debug mode.
DD=/D_DEBUG			# Debug flag definition of the compiler
DS=\Debug			# Debug suffix to append to output paths
!ELSE
DM=release			# Debug mode. For information only
DEBUG=0
NDEBUG=				# MS tools define this in release mode.
DD=/DNDEBUG
DS=
!ENDIF

# Define directories
S=.				# Where to find source files
!IF !DEFINED(OUTDIR)
OUTDIR=bin
R=$(OUTDIR)\$(T)		# Root output path - In the default bin subdirectory
!ELSEIF "$(OUTDIR)"=="."
R=$(T)				# Root output path - In the current directory
!ELSE # It's defined and not empty
R=$(OUTDIR)\$(T)		# Root output path - In the specified directory
!ENDIF
B=$(R)$(DS)			# Where to store binary executable files
O=$(B)\OBJ			# Where to store object files
L=$(B)\LIST			# Where to store listing files

RP=$(R)\			# Idem, with the OS-specific path separator
SP=$(S)\			#
OP=$(O)\			#
BP=$(B)\			#
LP=$(L)\			#

BR=$(T)$(DS)			# Idem, relative to sources

!IFNDEF TMP
!IFDEF TEMP
TMP=$(TEMP)
!ELSE
TMP=.
!ENDIF
!ENDIF

!IF !DEFINED(DISPATCH_OS)
!IF !DEFINED(T_VARS)
T_VARS=1	# Make sure OS-type-specific variables are defined only once
# Tools and options
CC=$(WIN32_CC)
AS=$(WIN32_AS)
LK=$(WIN32_LK)
LB=$(WIN32_LB)
RC=$(WIN32_RC)
MT=$(WIN32_MT)

PATH=$(WIN32_PATH)
INCPATH=$(WIN32_INCPATH)
MSVCINCLUDE=$(WIN32_VCINC:\=/) # Path of MSVC compiler include files, without quotes, and with forward slashes
UCRTINCLUDE=$(WIN32_CRTINC:\=/) # Path of MSVC CRT library include files, without quotes, and with forward slashes
WSDKINCLUDE=$(WIN32_WINSDKINC:\=/) # Path of Windows SDK include files, without quotes, and with forward slashes
LIB=$(WIN32_LIBPATH)

MACHINE=X86			# Target CPU = Intel 32-bits X86
USEDOSSTUB=1			# Use an MS-DOS stub
!ENDIF # !DEFINED(T_VARS)

# Tools and options
!IF !DEFINED(AFLAGS)
AFLAGS=/Cx /Sn /Zim "/D_MODEL=flat,stdcall"	# Default flags for the 32-bits ML.EXE assembler
!ENDIF
AFLAGS=$(AFLAGS) "/D_WIN32"	# Both 32 and 64 bits builds should defined _WIN32 
RFLAGS=$(DD) "/DMACHINE=\"$(MACHINE)\""
!IF !DEFINED(CFLAGS)
CFLAGS=/FAsc /Os /W4 /Zi	# Removed /Zp as this breaks ARM builds, and Windows SDK 10.0.18362 C_ASSERT(TYPE_ALIGNMENT(LARGE_INTEGER) == 8)
!ENDIF # DEFINED(CFLAGS)
AFLAGS=$(AFLAGS) $(DD) /I$(O) /Fl$(L)\ /Fo$(O)\ # Include files generated by h2inc from $(O)
CFLAGS=$(CFLAGS) $(DD) /Fa$(L)\ /Fd$(B)\ /Fo$(O)\ /FR$(O)\ # Use make.bat verbose mode to trigger the /showIncludes mode (See further down)
!IF "$(MSVCINCLUDE)"!=""
CFLAGS=$(CFLAGS) "/DMSVCINCLUDE=$(MSVCINCLUDE)" # Path of MSVC compiler include files, without quotes, and with forward slashes
!ENDIF # "$(MSVCINCLUDE)"!=""
!IF "$(UCRTINCLUDE)"!=""
CFLAGS=$(CFLAGS) "/DUCRTINCLUDE=$(UCRTINCLUDE)" # Path of MSVC CRT library include files, without quotes, and with forward slashes
!ENDIF # "$(UCRTINCLUDE)"!=""
!IF "$(WSDKINCLUDE)"!=""
!IF EXIST("$(WSDKINCLUDE)\windows.h")
CFLAGS=$(CFLAGS) "/DWSDKINCLUDE=$(WSDKINCLUDE)" # Path of Windows SDK include files, without quotes, and with forward slashes
RFLAGS=$(RFLAGS) "/DWSDKINCLUDE=$(WSDKINCLUDE)" # Path of Windows SDK include files, without quotes, and with forward slashes
!ELSE IF EXIST("$(WSDKINCLUDE)\um\windows.h")                
CFLAGS=$(CFLAGS) "/DWSDKINCLUDE=$(WSDKINCLUDE)/um" # Path of Windows SDK include files, without quotes, and with forward slashes
RFLAGS=$(RFLAGS) "/DWSDKINCLUDE=$(WSDKINCLUDE)/um" # Path of Windows SDK include files, without quotes, and with forward slashes
!ENDIF # EXIST("$(WSDKINCLUDE)\windows.h")
!ENDIF # "$(WSDKINCLUDE)"!=""
!IF !DEFINED(LFLAGS)
LFLAGS=/NOLOGO /INCREMENTAL:NO /MACHINE:$(MACHINE) /MAP:$(L)\$(*B).map /FORCE:MULTIPLE
!ENDIF # DEFINED(LFLAGS)
LIBS=rpcrt4.lib Kernel32.lib User32.lib Advapi32.lib Winmm.lib Mpr.lib
GUILIBS=gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib 
!IF (("$(T)"=="WIN32") || ("$(T)"=="WIN95")) && DEFINED(PTHREADS) && EXIST("$(PTHREADS)\pthread.lib")
LIBS=$(LIBS) pthread.lib
!ENDIF

# Target subsystem version. XP=5.1, minimum supported by VS 2013 and later. Win95/NT4=4.0, only supported by VS 2005 and earlier.
!IF !DEFINED(WINVER)
WINVER=5.1
!ENDIF

# Define the Windows SDK target version. Used by Windows.h to only define functions available in that OS.
!IF DEFINED(WINVER) && ("$(WINVER)" != "")
C_WINVER=0x$(WINVER:.=0) # Windows.h WINVER format is 0xMMnn, with MM the major version, and mm the minor version
CFLAGS=$(CFLAGS) "/DWINVER=$(C_WINVER)"
RFLAGS=$(RFLAGS) "/DWINVER=$(C_WINVER)"
!ENDIF

# Special definitions for WIN95 builds
!IF "$(T)"=="WIN95"
CFLAGS=$(CFLAGS) "/D_$(T)="	# Define a _WIN95 flag for WIN95 builds
LIBS=unicows.lib $(LIBS)	# Microsoft Layer for Unicode (MSLU) for Windows 95
!ENDIF
!IF DEFINED(VERBOSE) && ("$(VERBOSE)" == "1")
CFLAGS=$(CFLAGS) /showIncludes
!ENDIF
!IFDEF _DEBUG
LFLAGS=$(LFLAGS) /debug
!ENDIF
!IF DEFINED(WINSDKVER) && (("$(WINSDKVER)" == "v7.1") || ("$(WINSDKVER)" == "v7.1A"))
# http://stackoverflow.com/questions/14363929/vs2012-c-warning-c4005-useheader-macro-redefinition
CFLAGS=$(CFLAGS) "/D_USING_V110_SDK71_=1" # Workaround for VisualStudio2012/WinSDK7.1 macros redefinitions
!ENDIF

# Tell the resource compiler about 64-bits builds
!IF "$(T)"=="WIN64" || "$(T)"=="ARM64" || "$(T)"=="IA64"
RFLAGS=$(RFLAGS) "/D_WIN64"
!ENDIF
# and about the DOS stub, if it'll be linked with the WIN32 .exe
!IF DEFINED(PROGRAM) && $(USEDOSSTUB) && EXIST($(OUTDIR)/DOS$(DS)/$(PROGRAM).EXE)
RFLAGS=$(RFLAGS) "/DWITH_DOS_STUB"
!ENDIF

!IF !DEFINED(SUBSYSTEM)
!IF DEFINED(WINVER) && ("$(WINVER)" != "")
SUBSYSTEM=CONSOLE,$(WINVER:.=.0) # Link.exe SUBSYSTEM version format is M.mm, with M the major version, and mm the minor version
!ELSE
SUBSYSTEM=CONSOLE
!ENDIF
!ENDIF

INCLUDE=$(S);$(O);$(STINCLUDE);$(INCPATH);$(USER_INCLUDE)
LIBS=$(LIBS) $(USER_LIBS)

# Forward library detections by configure.bat to the C compiler and assembler
CFLAGS=$(CFLAGS) $(HAS_SDK_FLAGS)
AFLAGS=$(AFLAGS) $(HAS_SDK_FLAGS)

# Forward user information from configure.bat to the C and RC compilers
!IF DEFINED(MY_FULLNAME)
CFLAGS=$(CFLAGS) "/DMY_FULLNAME=$(MY_FULLNAME)"
RFLAGS=$(RFLAGS) "/DMY_FULLNAME=$(MY_FULLNAME)"
AFLAGS=$(AFLAGS) "/DMY_FULLNAME=$(MY_FULLNAME)"
!ENDIF
!IF DEFINED(MY_EMAIL)
CFLAGS=$(CFLAGS) "/DMY_EMAIL=$(MY_EMAIL)"
RFLAGS=$(RFLAGS) "/DMY_EMAIL=$(MY_EMAIL)"
AFLAGS=$(AFLAGS) "/DMY_EMAIL=$(MY_EMAIL)"
!ENDIF
!IF DEFINED(PROGRAM)
CFLAGS=$(CFLAGS) "/DPROGRAM_VER_H=$(PROGRAM).ver.h"
RFLAGS=$(RFLAGS) "/DPROGRAM_VER_H=$(PROGRAM).ver.h"
AFLAGS=$(AFLAGS) "/DPROGRAM_VER_H=$(PROGRAM).ver.h"
!ENDIF

CXXFLAGS=/EHsc $(CFLAGS)

# Files and scripts used for compilation
UTF8_BOM_FILE=$(O)\UTF8_BOM	# A file containing the UTF-8 Byte-Order Mark
REMOVE_UTF8_BOM=$(O)\RemBOM.bat	# Script for conditionally removing the UTF-8 BOM
CONV_SCRIPT=$(O)\MiniConv.bat	# Script emulating what conv.exe would do for us
!IF !DEFINED(CONV)
CONV=$(COMSPEC) /c $(CONV_SCRIPT)
!ENDIF

# Library SuffiX. For storing multiple versions of the same library in a single directory.
!IF !DEFINED(LSX)
LSX=w32
!ENDIF
!IF $(DEBUG)
LSX=$(LSX)d
!ENDIF

# Report start options
!IF DEFINED(MESSAGES)
!MESSAGE PROGRAM="$(PROGRAM)" Mode=$(DM).
!MESSAGE R="$(R)" B="$(B)" O="$(O)".
!MESSAGE PATH=$(PATH) # Default library paths
!MESSAGE INCLUDE=$(INCLUDE) # Target OS specific include paths
!MESSAGE LIB=$(LIB) # Default library paths
!MESSAGE LIBS=$(LIBS) # Default library names
!ENDIF

!ENDIF # !DEFINED(DISPATCH_OS)

MSG=>con echo		# Command for writing a progress message on the console
HEADLINE=$(MSG).&$(MSG)	# Output a blank line, then a message
REPORT_FAILURE=$(MSG) ... FAILED. & exit /b # Report that a build failed, and forward the error code.

# Add the /NOLOGO flags to MAKEFLAGS. But problem: MAKEFLAGS cannot be updated
MAKEFLAGS_=/$(MAKEFLAGS)# Also MAKEFLAGS does not contain the initial /
!IF "$(MAKEFLAGS_: =)"=="/" # And if no flag is provided, it still contains a dozen spaces
MAKEFLAGS_=		# In that case, clear the useless / and spaces
!ENDIF
MAKEFLAGS__=/_ /$(MAKEFLAGS) /_	# Temp variable to check if /NOLOGO is already there
!IF "$(MAKEFLAGS__: /NOLOGO =)"=="$(MAKEFLAGS__)"
MAKEFLAGS_=/NOLOGO $(MAKEFLAGS_)
!ENDIF
!UNDEF MAKEFLAGS__
SUBMAKE=$(MAKE) $(MAKEFLAGS_) /F "$(MAKEFILE)" $(MAKEDEFS) # Recursive call to this make file

###############################################################################
#									      #
#			       Inference rules				      #
#									      #
###############################################################################

# Inference rules to generate the required PROGRAM variable
!IF !DEFINED(PROGRAM)

!IF !DEFINED(DISPATCH_OS)
# Inference rules generating the output path, using the predefined debug mode.
.cpp.obj:
    @echo Applying $(T).mak inference rule .cpp.obj:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(O)\$(*F).obj

.c.obj:
    @echo Applying $(T).mak inference rule .c.obj:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(O)\$(*F).obj

.asm.obj:
    @echo Applying $(T).mak inference rule .asm.obj:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(O)\$(*F).obj

.rc.res:
    @echo Applying $(T).mak inference rule .rc.res:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(O)\$(*F).res

.cpp.exe:
    @echo Applying $(T).mak inference rule .cpp.exe:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(O)\$(*F).obj $(B)\$(*F).exe

.c.exe:
    @echo Applying $(T).mak inference rule .c.exe:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(O)\$(*F).obj $(B)\$(*F).exe

.asm.exe:
    @echo Applying $(T).mak inference rule .asm.exe:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(O)\$(*F).obj $(B)\$(*F).exe

.mak.exe:
    @echo Applying $(T).mak inference rule .mak.exe:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(B)\$(*F).exe

.mak.lib:
    @echo Applying $(T).mak inference rule .mak.lib:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(B)\$(*F).lib

.mak.dll:
    @echo Applying $(T).mak inference rule .mak.dll:
    $(HEADLINE) Building $(@F) $(T) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" dirs $(B)\$(*F).dll
!ENDIF # !DEFINED(DISPATCH_OS)

# Inference rules to compile a C++ program, inferring the debug mode from the output path specified.
# (Define C++ inferences rules before C inferences rules, so that if both a .c and .cpp file are present, the .cpp is used preferably.)
{$(S)\}.cpp{$(R)\obj\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.cpp{$$(R)\obj\}.obj:
    $(MSG) Compiling the $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $@

{$(S)\}.cpp{$(R)\DEBUG\obj\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.cpp{$$(R)\DEBUG\obj\}.obj:
    $(MSG) Compiling the $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $@

# Inference rules to compile a C program, inferring the debug mode from the output path specified.
{$(S)\}.c{$(R)\obj\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.c{$$(R)\obj\}.obj:
    $(MSG) Compiling the $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $@

{$(S)\}.c{$(R)\DEBUG\obj\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.c{$$(R)\DEBUG\obj\}.obj:
    $(MSG) Compiling the $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $@

# Inference rules to assemble an assembler program, inferring the debug mode from the output path specified.
{$(S)\}.asm{$(R)\obj\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.asm{$$(R)\obj\}.obj:
    $(MSG) Assembling the $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $@

{$(S)\}.asm{$(R)\DEBUG\obj\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.asm{$$(R)\DEBUG\obj\}.obj:
    $(MSG) Assembling the $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $@

# Inference rules to compile a Windows resource file, inferring the debug mode from the output path specified.
{$(S)\}.rc{$(R)\obj\}.res:
    @echo Applying $(T).mak inference rule {$$(S)\}.rc{$$(R)\obj\}.res:
    $(MSG) Compiling the $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $@

{$(S)\}.rc{$(R)\DEBUG\obj\}.res:
    @echo Applying $(T).mak inference rule {$$(S)\}.rc{$$(R)\DEBUG\obj\}.res:
    $(MSG) Compiling the $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $@

# Inference rules to build a C++ program, inferring the debug mode from the output path specified.
# (Define C++ inferences rules before C inferences rules, so that if both a .c and .cpp file are present, the .cpp is used preferably.)
{$(S)\}.cpp{$(R)\}.exe:
    @echo Applying $(T).mak inference rule {$$(S)\}.cpp{$$(R)\}.exe:
    $(HEADLINE) Building $(@F) $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $(R)\OBJ\$(*F).obj $(R)\$(*F).exe

{$(S)\}.cpp{$(R)\DEBUG\}.exe:
    @echo Applying $(T).mak inference rule {$$(S)\}.cpp{$$(R)\DEBUG\}.exe:
    $(HEADLINE) Building $(@F) $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $(R)\DEBUG\OBJ\$(*F).obj $(R)\DEBUG\$(*F).exe

# Inference rules to build a C program, inferring the debug mode from the output path specified.
{$(S)\}.c{$(R)\}.exe:
    @echo Applying $(T).mak inference rule {$$(S)\}.c{$$(R)\}.exe:
    $(HEADLINE) Building $(@F) $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $(R)\OBJ\$(*F).obj $(R)\$(*F).exe

{$(S)\}.c{$(R)\DEBUG\}.exe:
    @echo Applying $(T).mak inference rule {$$(S)\}.c{$$(R)\DEBUG\}.exe:
    $(HEADLINE) Building $(@F) $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $(R)\DEBUG\OBJ\$(*F).obj $(R)\DEBUG\$(*F).exe

# Inference rules to build an assembler program, inferring the debug mode from the output path specified.
{$(S)\}.asm{$(R)\}.exe:
    @echo Applying $(T).mak inference rule {$$(S)\}.asm{$$(R)\}.exe:
    $(HEADLINE) Building $(@F) $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $(R)\OBJ\$(*F).obj $(R)\$(*F).exe

{$(S)\}.asm{$(R)\DEBUG\}.exe:
    @echo Applying $(T).mak inference rule {$$(S)\}.asm{$$(R)\DEBUG\}.exe:
    $(HEADLINE) Building $(@F) $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $(R)\DEBUG\OBJ\$(*F).obj $(R)\DEBUG\$(*F).exe

# Inference rules to build a makefile-defined program, inferring the debug mode from the output path specified.
{$(S)\}.mak{$(R)\}.exe:
    @echo Applying $(T).mak inference rule {$$(S)\}.mak{$$(R)\}.exe:
    $(HEADLINE) Building $(@F) $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $@

{$(S)\}.mak{$(R)\DEBUG\}.exe:
    @echo Applying $(T).mak inference rule {$$(S)\}.mak{$$(R)\DEBUG\}.exe:
    $(HEADLINE) Building $(@F) $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $@

# Inference rules to build a library, inferring the debug mode from the output path specified.
{$(S)\}.mak{$(R)\}.lib:
    @echo Applying $(T).mak inference rule {$$(S)\}.mak{$$(R)\}.lib:
    $(HEADLINE) Building $(@F) $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $@

{$(S)\}.mak{$(R)\DEBUG\}.lib:
    @echo Applying $(T).mak inference rule {$$(S)\}.mak{$$(R)\DEBUG\}.lib:
    $(HEADLINE) Building $(@F) $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $@

# Inference rules to build a DLL, inferring the debug mode from the output path specified.
{$(S)\}.mak{$(R)\}.dll:
    @echo Applying $(T).mak inference rule {$$(S)\}.mak{$$(R)\}.dll:
    $(HEADLINE) Building $(@F) $(T) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" dirs $@

{$(S)\}.mak{$(R)\DEBUG\}.dll:
    @echo Applying $(T).mak inference rule {$$(S)\}.mak{$$(R)\DEBUG\}.dll:
    $(HEADLINE) Building $(@F) $(T) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" dirs $@

!ELSE # if DEFINED(PROGRAM)
# Inference rule for C++ compilation
{$(S)\}.cpp{$(O)\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.cpp{$$(O)\}.obj:
    $(MSG) Compiling $(<F) ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(REMOVE_UTF8_BOM) $< $(O)\$(<F)
    $(CC) $(CXXFLAGS) /c $(TC) $(O)\$(<F) || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule for C compilation
{$(S)\}.c{$(O)\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.c{$$(O)\}.obj:
    $(MSG) Compiling $(<F) ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(REMOVE_UTF8_BOM) $< $(O)\$(<F)
    $(CC) $(CFLAGS) /c $(TC) $(O)\$(<F) || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule for Assembly language.
{$(S)\}.asm{$(O)\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.asm{$$(O)\}.obj:
    $(MSG) Assembling $(<F) ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(AS) $(AFLAGS) /c $< || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule to compile Windows resources
{$(S)\}.rc{$(O)\}.res:
    @echo Applying $(T).mak inference rule {$$(S)\}.rc{$$(O)\}.res:
    $(MSG) Compiling $(<F) resources ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(RC) /Fo$@ $(RFLAGS) /r $< || $(REPORT_FAILURE)
    $(MSG) ... done.

{$(O)\}.rc{$(O)\}.res:
    @echo Applying $(T).mak inference rule {$$(O)\}.rc{$$(O)\}.res:
    $(MSG) Compiling $(<F) resources ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(RC) /Fo$@ $(RFLAGS) /r $< || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule to link a program
!IF $(USEDOSSTUB) && EXIST($(OUTDIR)/DOS$(DS)/$(PROGRAM).EXE)
STUB=/STUB:$(OUTDIR)\DOS$(DS)\$(PROGRAM).EXE
!ELSE
STUB=
!ENDIF
{$(O)\}.obj{$(B)\}.exe:
    @echo Applying $(T).mak inference rule {$$(O)\}.obj{$$(B)\}.exe:
    $(MSG) Linking $(B)\$(@F) ...
    set LIB=$(LIB)
    set PATH=$(PATH)
    copy << $(L)\$(*B).link
$**
$(LIBS)
/OUT:$@
$(STUB)
/SUBSYSTEM:$(SUBSYSTEM)
$(LFLAGS)
<<NOKEEP
    @echo "	type $(L)\$(*B).link"
    @$(COMSPEC) /c "type $(L)\$(*B).link"
    $(LK) @$(L)\$(*B).link || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule to link a DLL
{$(O)\}.obj{$(B)\}.dll:
    @echo Applying $(T).mak inference rule {$$(O)\}.obj{$$(B)\}.dll:
    $(MSG) Linking $(B)\$(@F) ...
    set LIB=$(LIB)
    set PATH=$(PATH)
    copy << $(L)\$(*B).link
$**
$(LIBS)
/OUT:$@
/SUBSYSTEM:WINDOWS /DLL /IMPLIB:$(B)\$(*B).lib
$(LFLAGS)
<<NOKEEP
    @echo "	type $(L)\$(*B).link"
    @$(COMSPEC) /c "type $(L)\$(*B).link"
    $(LK) @$(L)\$(*B).link || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule for generating a bsc file
BSCFLAGS=/nologo 
{$(O)\}.sbr{$(O)\}.bsc:
    @echo Applying $(T).mak inference rule {$$(O)\}.sbr{$$(B)\}.bsc:
    set PATH=$(PATH)
    $(BSC) $(BSCFLAGS) /o$@ $**

# Inference rule to build a library
{$(O)\}.mak{$(B)\}.lib:
    @echo Applying $(T).mak inference rule {$$(O)\}.mak{$$(B)\}.lib:
    $(MSG) Creating $(B)\$(@F) ...
    copy <<$(L)\$(*B).list NUL
$(OBJECTS:+=)
<<KEEP
    if exist $@ del $@
    set PATH=$(PATH)
    $(LB) /OUT:$@ @$(L)\$(*B).list || $(REPORT_FAILURE)
    $(MSG) ... done.

###############################################################################
#									      #
#			        Specific rules				      #
#									      #
###############################################################################

# $(PROGRAM).mak and/or Files.mak may define macros SOURCES, OBJECTS, LIBRARIES, and PROGRAM.
# These make files are intended to be OS-independant, and be used in both Windows and Unix build environments. 
# These macros in turn allow the following rules to work, and build more complex programs with more than one source.
TMPMAK=$(TMP)\$(T)_vars.$(PID).mak # Using the shell PID to generate a unique name, to avoid conflicts in case of // builds.
!IF DEFINED(PROGRAM) && EXIST("$(PROGRAM).mak")
!  MESSAGE Getting specific rules from $(PROGRAM).mak.
!  INCLUDE $(PROGRAM).mak
!ELSE IF EXIST("Files.mak")
!  MESSAGE Getting specific rules from Files.mak.
!  INCLUDE Files.mak
!  IF DEFINED(PROGRAM) && ![$(STINCLUDE)\GetDefs.bat Files.mak $(PROGRAM) >"$(TMPMAK)" 2>NUL]
!    MESSAGE Getting specific definitions for $(PROGRAM) from Files.mak.
!    INCLUDE $(TMPMAK)
!  ENDIF
!ELSE
!  MESSAGE There are no specific rules.
EXENAME=_-_-_-_.exe	# An unlikely name, to prevent the $(EXENAME) dependency rule below from firing.
OBJECTS=
LIBRARIES=
!ENDIF

!IF !DEFINED(EXENAME)
EXENAME=$(PROGRAM).exe	# Both DOS and Windows expect this extension.
!ENDIF

# If needed, convert the SOURCES list to an OBJECTS list
!IF DEFINED(SOURCES) && !DEFINED(OBJECTS)
!  INCLUDE src2objs.mak # Convert the SOURCES list to an OBJECTS list
!ENDIF
# If we still don't have an OBJECTS list, use the PROGRAM basename 
!IF DEFINED(PROGRAM) && !DEFINED(OBJECTS)
OBJECTS=$(O)\$(PROGRAM).obj $(O)\$(PROGRAM).res # If there's no $(PROGRAM).rc file, we create one in $(O)
!ENDIF

# Generate LIBRARIES with full library pathnames, based on LIB & LIBS
!IF DEFINED(LIBS) && !DEFINED(LIBRARIES)
!  INCLUDE lib2libs.mak # Generate LIBRARIES based on LIB & LIBS
LIBS= # Avoid having them defined twice in the linker input list
!ENDIF

# Generic rule to build program
!IF DEFINED(PROGRAM) && (!EXIST("$(PROGRAM).manifest")) && EXIST("asInvoker.rc") # If no manifest is defined for PROGRAM
# Then use the default asInvoker.manifest, that flags the WIN32 exe as Vista-aware
# But no need to add /MANIFEST to $(LFLAGS), as it just outputs a useless copy next to the exe
OBJECTS=$(OBJECTS) $(O)\asInvoker.res # asInvoker.rc includes asInvoker.manifest in the executable 
!ENDIF
!MESSAGE OBJECTS=($(OBJECTS))
!IF !DEFINED(SKIP_THIS)
$(B)\$(EXENAME): $(OBJECTS:+=) $(LIBRARIES) # The dependency on libraries forces relinking if one of the libraries has changed
    @echo Applying $(T).mak build rule $$(B)\$$(EXENAME):
    $(MSG) Linking $(B)\$(@F) ...
    set LIB=$(LIB)
    set PATH=$(PATH)
    copy <<$(L)\$(*B).link NUL
$(OBJECTS:+=)
$(LIBRARIES) $(LIBS)
/OUT:$@
$(STUB)
/SUBSYSTEM:$(SUBSYSTEM)
$(LFLAGS)
<<KEEP
    @echo "	type $(L)\$(*B).link"
    @$(COMSPEC) /c "type $(L)\$(*B).link"
    $(LK) @$(L)\$(*B).link || $(REPORT_FAILURE)
    $(POST_LINK_CMD)
    $(MSG) ... done.

# Generic rule to build a static library
$(B)\$(PROGRAM).lib: $(OBJECTS:+=) $(LIBRARIES)
    @echo Applying $(T).mak build rule $$(B)\$$(PROGRAM).lib:
    $(MSG) Creating $@ ...
    copy <<$(L)\$(*B).list NUL
$(OBJECTS:+=)
<<KEEP
    if exist $@ del $@
    set PATH=$(PATH)
    $(LB) /OUT:$@ @$(L)\$(*B).list || $(REPORT_FAILURE)
    $(POST_LINK_CMD)
    $(MSG) ... done.

# Generic rule to build a Dynamic Link Library (DLL)
$(B)\$(PROGRAM).dll: $(OBJECTS:+=) $(LIBRARIES) # The dependency on libraries forces relinking if one of the libraries has changed
    @echo Applying $(T).mak build rule $$(B)\$$(PROGRAM).dll:
    $(MSG) Linking $@ ...
    set LIB=$(LIB)
    set PATH=$(PATH)
    copy <<$(L)\$(*B).link NUL
$(OBJECTS:+=)
$(LIBRARIES) $(LIBS)
/OUT:$@
/DLL /IMPLIB:$(B)\$(*B).lib
$(LFLAGS)
<<KEEP
    @echo "	type $(L)\$(*B).link"
    @$(COMSPEC) /c "type $(L)\$(*B).link"
    $(LK) @$(L)\$(*B).link || $(REPORT_FAILURE)
    $(POST_LINK_CMD)
    $(MSG) ... done.

!ELSE # DEFINED(SKIP_THIS)
$(OBJECTS) $(B)\$(EXENAME) $(B)\$(PROGRAM).lib $(B)\$(PROGRAM).dll: skip_this
    @rem This rem prevents inference rules from firing. Do not remove.
!ENDIF # if !DEFINED(SKIP_THIS)

!ENDIF # if DEFINED(PROGRAM)

# If SKIP_THIS is defined, build attempts will trigger this information message
skip_this:
    $(MSG) $(SKIP_THIS)

!IF !DEFINED(DISPATCH_OS)

$(B):
    if not exist $(B) $(MSG) Creating directory $(B)
    if not exist $(B) mkdir $(B)

$(O):
    if not exist $(O) $(MSG) Creating directory $(O)
    if not exist $(O) mkdir $(O)

$(L):
    if not exist $(L) $(MSG) Creating directory $(L)
    if not exist $(L) mkdir $(L)

!IF !DEFINED(SKIP_THIS)
dirs: $(B) $(O) $(L) files

files: $(UTF8_BOM_FILE) $(REMOVE_UTF8_BOM) $(CONV_SCRIPT)
!ELSE
dirs files: skip_this
    @rem This rem prevents inference rules from firing. Do not remove.
!ENDIF

$(UTF8_BOM_FILE): "$(THIS_MAKEFILE)"
    $(MSG) Generating file $@
    cscript //E:JScript //nologo << $@
	var args = WScript.Arguments;
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var WriteBinaryFile = function(fileName, data) {
	  var df = fso.OpenTextFile(fileName, 2, true, 0); // ForWriting, ASCII
	  df.write(data);
	  df.Close();
	}
	var szBOM = "\xEF\xBB\xBF";
	WriteBinaryFile(args(0), szBOM);
	WScript.Quit(0);
<<NOKEEP

$(REMOVE_UTF8_BOM): "$(THIS_MAKEFILE)"
    $(MSG) Generating script $@
    copy <<$@ NUL
	@echo off
	setlocal EnableExtensions EnableDelayedExpansion
	goto :main
	:# Subroutine to append an integer constant definition to an include file
	:append_int_def %1=name %2=value %3=pathname
	setlocal
	set "NAME=%~1"
	set "VALUE=%~2"
	set "PATHNAME=%~3"
	if not defined VALUE ( :# There's no value
	  >> "!PATHNAME!" echo #undef  !NAME! /* Value undefined */
	  endlocal & exit /b 1
	)
	if not "!VALUE:~5!"=="" ( :# It's too long. Show what the value is, but don't define it.
	  >> "!PATHNAME!" echo #undef  !NAME! /* Value "!VALUE!" longer than 5 digits */
	  endlocal & exit /b 1
	)
	set "ALL_DIGITS=0123456789"
	set "IS_ALL_DIGITS=1"
	for /l %%i in (0,1,4) do (
	  for %%c in ("!VALUE:~%%i,1!") do if not "%%~c"=="" (
	    rem echo Digit %%i is '%%~c'
	    if "!ALL_DIGITS:%%~c=!"=="!ALL_DIGITS!" set "IS_ALL_DIGITS=0"
	  )
	)
	if "!IS_ALL_DIGITS!"=="1" ( :# It's an integer. Add the C integer constant definition
	  >> "!PATHNAME!" echo #define !NAME! !VALUE!
	) else ( :# It's not an integer. Show what the value is, but don't define it.
	  >> "!PATHNAME!" echo #undef  !NAME! /* Value "!VALUE!" is not an integer */
	)
	endlocal & exit /b 0
	:main
	:# Main action: Remove the UTF-8 BOM, if present in the C source
	findstr /B /G:$(UTF8_BOM_FILE) <"%~1" >NUL
	if errorlevel 1 (
	  echo No UTF-8 BOM in "%~1". Copying the file.
	  copy /y "%~1" "%~2"
	) else ( rem :# Remove the BOM before compiling the source
	  echo UTF-8 BOM found in "%~1". Converting the file.
	  :# Must be compatible both with conv.exe and $(CONV_SCRIPT)
	  $(CONV) 8 8 "%~1" "%~2" -B 
	)
	:# Side action: Extract the version information into a PROGRAM.ver include file
	set "PROGRAM.ver=%~dpn2.ver.h"
	echo Generating !PROGRAM.ver!
	:# Extract the PROGRAM_* definitions from the C source
	findstr /C:"#define PROGRAM_" < "%~1" > "!PROGRAM.ver!" 2>NUL &:# Don't use /B to allow flagging other lines to copy with a /* #define PROGRAM_ */ comment
	:# Then split the program version into its MAJOR.MINOR.PATH.BUILD components
	set "PROGRAM_VERSION="
	for %%v in (PROGRAM_VERSION PROGRAM_DATE) do if not defined PROGRAM_VERSION ( :# Fall back to using PROGRAM_DATE if there's no PROGRAM_VERSION
	  for /f "delims=" %%l in ('findstr /B /C:"#define %%v" "!PROGRAM.ver!" 2^>NUL') do if not defined PROGRAM_VERSION (
	    set LINE=%%l
	    :# Convert the C definition to a batch definition
	    set "PROGRAM_VERSION=!LINE:*"=!
	    echo Extracting integer version components from !PROGRAM_VERSION! 
	    for /f "tokens=1,2,3,* delims=.-" %%a in ("!PROGRAM_VERSION!") do (
	      call :append_int_def PROGRAM_VERSION_MAJOR "%%a" "!PROGRAM.ver!"
	      call :append_int_def PROGRAM_VERSION_MINOR "%%b" "!PROGRAM.ver!"
	      call :append_int_def PROGRAM_VERSION_PATCH "%%c" "!PROGRAM.ver!"
	      call :append_int_def PROGRAM_VERSION_BUILD "%%d" "!PROGRAM.ver!"
	    )
	  )
	)
	set "PROGRAM_DESCRIPTION="
	for /f "tokens=1,2,*" %%a in ('findstr /B /C:"#define PROGRAM_DESCRIPTION" "!PROGRAM.ver!" 2^>NUL') do set "PROGRAM_DESCRIPTION=%%~c"
	:# If the C source contains PROGRAM property definitions, and if there's no corresponding .rc file, then create one
	set PROGRAM_ 2>NUL
	if defined PROGRAM_VERSION if defined PROGRAM_DESCRIPTION if not exist "%~n1.rc" (
	  echo Generating %~dpn2.rc
	  >"%~dpn2.rc" echo #include "%~dp1SysToolsLib.rc"
	)
	if not exist "%~n1.rc" if not exist "%~dpn2.rc" (
	  echo No PROGRAM_* definitions, or %~n1.rc exists. Generating empty %~dpn2.rc
	  type nul >"%~dpn2.rc" &rem Create an empty file
	)
	:# Record the build date
	for /f %%i in ('%WINDIR%\System32\wbem\wmic.exe OS GET LocalDateTime /value') do for /f "tokens=2 delims==" %%j in ("%%i") do set "dt=%%j"
        set "YEAR=%dt:~0,4%" & set "MONTH=%dt:~4,2%" & set "DAY=%dt:~6,2%" &:# set "HOUR=%dt:~8,2%" & set "MINUTE=%dt:~10,2%" & set "SECOND=%dt:~12,2%" & set "MS=%dt:~15,3%"
	>> "!PROGRAM.ver!" echo #define BUILD_DATE "!YEAR!-!MONTH!-!DAY!"
	call :append_int_def BUILD_YEAR  "!YEAR!"  "!PROGRAM.ver!"
	call :append_int_def BUILD_MONTH "!MONTH!" "!PROGRAM.ver!"
	call :append_int_def BUILD_DAY   "!DAY!"   "!PROGRAM.ver!"
	:# Delete the resource file, which depends on the above version information
	set "PROGRAM.res=%~dpn2.res"
	if exist "%PROGRAM.res%" echo Deleting "%PROGRAM.res%" & del "%PROGRAM.res%"
	exit /b 0
<<KEEP

$(CONV_SCRIPT): "$(THIS_MAKEFILE)"	# Poor man's version of conv.exe, limited to what this make file needs
    $(MSG) Generating script $@
    copy <<$@ NUL
	@if (@Language == @Batch) @then /* NOOP for Batch; Begins a comment for JScript.
	@echo off & cscript //E:JScript //nologo "%~f0" %* & exit /b
	:# End of the Batch section, and beginning of the JScript section */ @end
	var args = WScript.Arguments;
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var ReadBinaryFile = function(fileName) {
	  var sf = fso.OpenTextFile(fileName, 1, false, 0); // ForReading, ASCII
	  var data = sf.ReadAll();
	  sf.Close()
	  return data;
	}
	var WriteBinaryFile = function(fileName, data) {
	  var df = fso.OpenTextFile(fileName, 2, true, 0); // ForWriting, ASCII
	  df.write(data);
	  df.Close();
	}
	var szBOM = "\xEF\xBB\xBF";
	text = ReadBinaryFile(args(2));
	text = text.replace(szBOM, "");
	WriteBinaryFile(args(3), text);
	WScript.Quit(0);
<<KEEP

# Erase all output files
clean:
    -rd /S /Q $(R)	>NUL 2>&1
    -del /Q *.pdb	>NUL 2>&1
    -del /Q *.ncb	>NUL 2>&1
    -del /Q *.suo	>NUL 2>&1
    -del /Q *.bak	>NUL 2>&1
    -del /Q *~		>NUL 2>&1

# Help message describing the targets
help:
    type <<
Targets:
  clean                   Erase all files in the $(R) directory
  $(R)\{prog}.exe        Build {prog}.exe release version from {prog}.c/cpp
  $(R)\Debug\{prog}.exe  Build {prog}.exe debug version from {prog}.c/cpp
  $(R)\OBJ\{prog}.obj       Compile {prog}.obj release version from {prog}.c/cpp
  $(R)\Debug\OBJ\{prog}.obj Compile {prog}.obj debug version from {prog}.c/cpp
  {prog}.exe            Build $(R)[\Debug]\{prog}.exe from {prog}.c/cpp
  {prog}.obj            Compile $(R)[\Debug]\OBJ\{prog}.obj from {prog}.c/cpp
  {prog}.res            Compile $(R)[\Debug]\OBJ\{prog}.res from {prog}.rc

The debug mode is set based on the first definition found in...
 1) The nmake command line option "DEBUG=0|1"
 2) The target directory $(R)|$(R)\Debug
 3) The environment variable DEBUG=0|1
 4) Default: DEBUG=0
<<NOKEEP

!ENDIF # !DEFINED(DISPATCH_OS)
