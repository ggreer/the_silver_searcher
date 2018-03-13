###############################################################################
#									      #
#   File name:	    dos.mak						      #
#									      #
#   Description:    A NMake makefile to build DOS programs.		      #
#									      #
#   Notes:	    Use with make.bat, which defines the necessary variables. #
#		    Usage: make -f dos.mak [definitions] [targets]	      #
#									      #
#		    Targets:						      #
#		    clean	    Erase all files in the DOS subdirectory.  #
#		    {prog}.com	    Build DOS[\Debug]\{prog}.com.	      #
#		    {prog}.exe	    Build DOS[\Debug]\{prog}.exe.	      #
#		    {prog}.obj	    Build DOS[\Debug]\OBJ\$(MEM)\{prog}.obj.  #
#		    {prog}.res	    Build DOS[\Debug]\OBJ\{prog}.res.	      #
#		    DOS\{prog}.com       Build the DOS release version.       #
#		    DOS\Debug\{prog}.com Build the DOS debug version.         #
#		    DOS\{prog}.exe       Build the DOS release version.       #
#		    DOS\Debug\{prog}.exe Build the DOS debug version.         #
#		    DOS\OBJ\{prog}.obj       Compile the DOS release version. #
#		    DOS\Debug\OBJ\{prog}.obj Compile the DOS debug version.   #
#		    DOS\OBJ\{prog}.res       Compile WIN16 release resources. #
#		    DOS\Debug\OBJ\{prog}.res Compile WIN16 debug resources.   #
#		    DOS[\Debug][\OBJ]\T\*  Compile/Build the tiny version.    #
#		    DOS[\Debug][\OBJ]\S\*  Compile/Build the small version.   #
#		    DOS[\Debug][\OBJ]\L\*  Compile/Build the large version.   #
#									      #
#		    Command-line definitions:				      #
#		    DEBUG=0	 Build the release ver. (<=> program in DOS)  #
#		    DEBUG=1	 Build the debug ver. (<=> pgm in DOS\DEBUG)  #
#		    MEM=T	 Build the tiny ver.  (<=> objects in OBJ\T)  #
#		    MEM=S	 Build the small ver. (<=> objects in OBJ\S)  #
#		    MEM=L	 Build the large ver. (<=> objects in OBJ\L)  #
#		    OUTDIR=path  Output to path\DOS\. Default: To bin\DOS\    #
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
#		    Another design goal is to use that same dos.mak	      #
#		    in complex 1-project environments (One Files.mak defines  #
#		    all project components); And in simple multiple-project   #
#		    environments (No Files.mak; Most programs have a single   #
#		    source file, and use default compiler options).	      #
#									      #
#		    The following macros / environment variables must be      #
#		    predefined. This allows to use the same makefile on       #
#		    machines with various locations for the build tools.      #
#									      #
#		    AS	    	16-bits Assembler			      #
#		    CC16    	16-bits C compiler			      #
#		    INCPATH16  	16-bits include files paths		      #
#		    LINK16  	16-bits Linker				      #
#		    LIBPATH16   16-bits libraries paths			      #
#		    LIB16   	16-bits librarian     			      #
#		    RC16    	16-bits Resource compiler		      #
#		    MAPSYM	16-bits Linker .map file to .sym converter    #
#		    TMP	    	Temporary directory	 		      #
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
#    2012-10-16 JFL Added an optional subdir. level for each memory model.    #
#		    Renamed the variable specifying the memory model as MEM.  #
#    2012-10-17 JFL Changed the output directories structure to:	      #
#		    DOS[\$(MEM)][\Debug][\OBJ|\LIST]			      #
#		    Removed the special handling of MultiOS.lib.	      #
#    2014-03-05 JFL Fail immediately if CC16 or LINK16 aren't present.        #
#    2015-01-16 JFL Pass selected cmd-line definitions thru to sub-make files.#
#    2015-10-27 JFL Added support for .com targets.			      #
#    2012-10-17 JFL Changed the output directories structure to:	      #
#		    DOS[\Debug]\[BIN|OBJ|LIST]\$(MEM)			      #
#		    Then copy binary files to DOS[\Debug]		      #
#		    This makes the make file more orthogonal, and allows      #
#		    finding both .com and .exe programs in the same place.    #
#    2015-11-03 JFL Added rules to build a library from a .mak file.          #
#    2015-11-13 JFL Use new generic environment definition variables.         #
#		    Allow including this file from another, such as BIOS.mak. #
#    2015-12-07 JFL Added support for a base output directory other than .\   #
#    2016-04-01 JFL Do not change the PROGRAM value, once it has been set.    #
#		    Added an inference rule for compiling resident C modules. #
#    2016-04-14 JFL Forward HAS_<lib> flags to the C compiler.		      #
#    2016-08-24 JFL Added scripts for removing the UTF-8 BOM from C sources.  #
#    2016-09-21 JFL Fixed an issue that caused double definition warnings.    #
#    2016-09-28 JFL Display FAILED messages when compilation or link fails.   #
#		    Avoid having the word "Error" in the log unnecessarily.   #
#    2016-10-04 JFL Use the shell PID to generate unique temp file names.     #
#		    Display messages only if variable MESSAGES is defined.    #
#    2016-10-11 JFL Adapted for use in SysToolsLib global C include dir.      #
#    2016-10-21 JFL Added missing inference rules for assembly language.      #
#		    Define _MSDOS and _MODEL constants for the assembler.     #
#    2017-03-02 JFL Fixed src2objs.bat and use it indirectly via src2objs.mak.#
#    2017-08-29 JFL Bugfix: The help target did output a "1 file copied" msg. #
#    2017-10-22 JFL Changed OUTDIR default to the bin subdirectory.           #
#    2018-01-12 JFL Added $(LIBRARIES) to the $(PROGRAM).* dependency list,   #
#		    so that it is relinked if one of its libraries changes.   #
#		    The LIBRARIES variable is generated by lib2libs.mak/.bat  #
#		    based on the LIB and LIBS variables.		      #
#    2018-02-28 JFL Added $(LSX) Library SuffiX definition.		      #
#    2018-03-02 JFL Added variable SKIP_THIS, to prevent building specific    #
#		    versions.						      #
#		    							      #
#      © Copyright 2016-2018 Hewlett Packard Enterprise Development LP        #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

.SUFFIXES: # Clear the predefined suffixes list.
.SUFFIXES: .com .exe .sys .obj .asm .c .r .cpp .mak .res .rc

###############################################################################
#									      #
#			        Definitions				      #
#									      #
###############################################################################

!IF !DEFINED(T)
T=DOS				# Target OS
!ENDIF
!IF DEFINED(MESSAGES)
!MESSAGE Started $(T).mak in $(MAKEDIR) # Display this file name, or the caller's name
!ENDIF

# Command-line definitions that need carrying through to sub-make instances
# Note: Cannot redefine MAKEFLAGS, so defining an alternate variable instead.
MAKEDEFS=
!IF DEFINED(MEM)	# Memory model for 16-bits compilation. T|S|C|D|L|H.
MAKEDEFS=$(MAKEDEFS) "MEM=$(MEM)"
!ENDIF

THIS_MAKEFILE=dos.mak		# This very make file name
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

# If possible, load the 16-bits memory model definition from the make file for the current program.
# Do not load the rest of the make file, as it may contain more rules depending on other variables defined further down.
# Also loading it entirely here would cause warnings about goals defined twice.
TMPMAK=$(TMP)\$(T)_mem.$(PID).mak # Using the shell PID to generate a unique name, to avoid conflicts in case of // builds.
!IF DEFINED(PROGRAM) && EXIST("$(PROGRAM).mak") && ![findstr /R "^MEM=" "$(PROGRAM).mak" >"$(TMPMAK)" 2>NUL]
!  MESSAGE Getting memory model definition from $(PROGRAM).mak.
!  INCLUDE "$(TMPMAK)"
!ENDIF

# Memory model for 16-bit C compilation (T|S|C|D|L|H)
!IF !DEFINED(MEM)
MEM=S				# Memory model for C compilation
!IF DEFINED(MESSAGES)
!MESSAGE Using the default memory model $(MEM).
!ENDIF
!ELSE
!IF DEFINED(MESSAGES)
!MESSAGE Using the specified memory model $(MEM).
!ENDIF
!ENDIF

# Convert the memory model flag into a memory model name
!IF "$(MEM)"=="T"
MMN=tiny
LCMEM=t
!ELSEIF  "$(MEM)"=="S"
MMN=small
LCMEM=s
!ELSEIF  "$(MEM)"=="C"
MMN=code
LCMEM=c
!ELSEIF  "$(MEM)"=="D"
MMN=data
LCMEM=d
!ELSEIF  "$(MEM)"=="L"
MMN=large
LCMEM=l
!ELSEIF  "$(MEM)"=="H"
MMN=huge
LCMEM=h
!ELSE
!ERROR "Invalid memory model: $(MEM)"
!ENDIF

# Define directories
S=.				# Where to find source files
!IF !DEFINED(OUTDIR)
OUTDIR=bin
R=bin\$(T)			# Root output path - In the default bin subdirectory
!ELSEIF "$(OUTDIR)"=="."
R=$(T)				# Root output path - In the current directory
!ELSE # It's defined and not empty
R=$(OUTDIR)\$(T)		# Root output path - In the specified directory
!ENDIF
BD=$(R)$(DS)
B=$(BD)\BIN\$(MEM)		# Where to store binary executable files
O=$(BD)\OBJ\$(MEM)		# Where to store object files
L=$(BD)\LIST\$(MEM)		# Where to store listing files

RP=$(R)\			# Idem, with the OS-specific path separator
SP=$(S)\			#
OP=$(O)\			#
BP=$(B)\			#
LP=$(L)\			#

BR=$(T)$(DS)\BIN\$(MEM)		# Idem, relative to sources

BB=$(BD)			# Final destination of executable files

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
AFLAGS=/Cx $(DD) /I$(O) /Fl$(L)\ /Fo$(O)\ /San /Zdim /D_MSDOS "/D_MODEL=$(MMN)"
CFLAGS=/A$(MEM) $(DD) /Fc$(L)\ /Fd$(B)\ /Fo$(O)\ /G3 /Oaes /W4 /Zpi
!IF DEFINED(DOS_VCINC)
CFLAGS=$(CFLAGS) "/DMSVCINCLUDE=$(DOS_VCINC:\=/)" # Path of MSVC compiler include files, without quotes, and with forward slashes
!ENDIF
!IF DEFINED(DOS_CRTINC)
CFLAGS=$(CFLAGS) "/DUCRTINCLUDE=$(DOS_CRTINC:\=/)" # Path of MSVC CRT library include files, without quotes, and with forward slashes
!ENDIF
LFLAGS=/map /li /batch /noe /onerror:noexe
!IF "$(DEBUG)"=="1"
# Note: The MSVC 1.52 linker does not support the /debug option
LFLAGS=$(LFLAGS) /co
!ENDIF
RFLAGS=$(DD)

PATH=$(DOS_PATH)
CC=$(DOS_CC)
AS=$(DOS_AS)
INCLUDE=$(S);$(STINCLUDE);$(DOS_INCPATH);$(USER_INCLUDE)
LK=$(DOS_LK)
LIBS=$(DOS_LIBS) $(USER_LIBS)
LIB=$(DOS_LIBPATH)
LB=$(DOS_LB)
RC=$(DOS_RC)

STARTCOM=$(MSVC)\LIB\CRTCOM.LIB
STARTEXE=
!ENDIF # !DEFINED(T_VARS)

# Forward library detections by configure.bat to the C compiler and assembler
CFLAGS=$(CFLAGS) $(HAS_SDK_FLAGS)
AFLAGS=$(AFLAGS) $(HAS_SDK_FLAGS)

# Files and scripts used for compilation
UTF8_BOM_FILE=$(O)\UTF8_BOM	# A file containing the UTF-8 Byte-Order Mark
REMOVE_UTF8_BOM=$(O)\RemBOM.bat	# Tool for conditionally removing the UTF-8 BOM
CONV_SCRIPT=$(O)\MiniConv.bat	# Script emulating what conv.exe would do for us
COMPACT_PATHS=$(O)\CompactP.bat # Compact paths, to avoid passing the 256-char length limit that causes out-of-memory errors for the DOS C compiler
!IF !DEFINED(CONV)
CONV=$(COMSPEC) /c $(CONV_SCRIPT)
!ENDIF

# Library SuffiX. For storing multiple versions of the same library in a single directory.
!IF !DEFINED(LSX)
LSX=d$(LCMEM)
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
# We can't guess the memory model, except for .com targets.
# But we can generate the PROGRAM variable, and let the next make instance figure out from PROGRAM.mak what the memory model is.
.cpp.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .cpp.obj:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.c.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .c.obj:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.asm.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .asm.obj:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.rc.res:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .rc.res:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.cpp.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .cpp.exe:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.c.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .c.exe:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.asm.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .asm.exe:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.cpp.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .cpp.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" "MEM=T" dirs $(R)$(DS)\OBJ\T\$(*F).obj $(R)$(DS)\BIN\T\$(*F).com

.c.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .c.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" "MEM=T" dirs $(R)$(DS)\OBJ\T\$(*F).obj $(R)$(DS)\BIN\T\$(*F).com

.asm.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .asm.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" "MEM=T" dirs $(R)$(DS)\OBJ\T\$(*F).obj $(R)$(DS)\BIN\T\$(*F).com

.mak.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .mak.lib:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

{.\}.mak{Debug\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {.\}.mak{Debug\}.lib:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@
!ENDIF # !DEFINED(DISPATCH_OS)

# Inference rules to compile a C++ program, inferring the memory model and debug mode from the output path specified.
# (Define C++ inferences rules before C inferences rules, so that if both a .c and .cpp file are present, the .cpp is used preferably.)
#   First rules for a target with no memory model defined. Output directly into the $(R)[\Debug] directory.
{$(S)\}.cpp{$(R)\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\OBJ\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" $@

{$(S)\}.cpp{$(R)\Debug\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\OBJ\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

#   Rules for the tiny memory model. Output into the $(R)[\Debug]\OBJ\T directory.
{$(S)\}.cpp{$(R)\OBJ\T\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\OBJ\T\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" $@

{$(S)\}.cpp{$(R)\Debug\OBJ\T\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\OBJ\T\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" $@

#   Rules for the small memory model. Output into the $(R)[\Debug]\OBJ\S directory.
{$(S)\}.cpp{$(R)\OBJ\S\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\OBJ\S\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=S" $@

{$(S)\}.cpp{$(R)\Debug\OBJ\S\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\OBJ\S\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=S" $@

#   Rules for the large memory model. Output into the $(R)[\Debug]\OBJ\L directory.
{$(S)\}.cpp{$(R)\OBJ\L\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\OBJ\L\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=L" $@

{$(S)\}.cpp{$(R)\Debug\OBJ\L\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\OBJ\L\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=L" $@

# Inference rules to compile a C program, inferring the memory model and debug mode from the output path specified.
#   First rules for a target with no memory model defined. Output directly into the $(R)[\Debug] directory.
{$(S)\}.c{$(R)\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\OBJ\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" $@

{$(S)\}.c{$(R)\Debug\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\OBJ\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

#   Rules for the tiny memory model. Output into the $(R)[\Debug]\OBJ\T directory.
{$(S)\}.c{$(R)\OBJ\T\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\OBJ\T\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" $@

{$(S)\}.c{$(R)\Debug\OBJ\T\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\OBJ\T\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" $@

#   Rules for the small memory model. Output into the $(R)[\Debug]\OBJ\S directory.
{$(S)\}.c{$(R)\OBJ\S\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\OBJ\S\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=S" $@

{$(S)\}.c{$(R)\Debug\OBJ\S\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\OBJ\S\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=S" $@

#   Rules for the large memory model. Output into the $(R)[\Debug]\OBJ\L directory.
{$(S)\}.c{$(R)\OBJ\L\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\OBJ\L\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=L" $@

{$(S)\}.c{$(R)\Debug\OBJ\L\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\OBJ\L\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=L" $@

# Inference rules to compile a Windows 16-bits resource file, inferring the debug mode from the output path specified.
{$(S)\}.rc{$(R)\OBJ\}.res:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.rc{$$(R)\OBJ\}.res:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" $@

{$(S)\}.rc{$(R)\Debug\OBJ\}.res:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.rc{$$(R)\Debug\OBJ\}.res:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

# Inference rules to assemble an ASM program, inferring the memory model and debug mode from the output path specified.
#   First rules for a target with no memory model defined. Output directly into the $(R)[\Debug] directory.
{$(S)\}.asm{$(R)\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\OBJ\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" $@

{$(S)\}.asm{$(R)\Debug\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\OBJ\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

#   Rules for the tiny memory model. Output into the $(R)[\Debug]\OBJ\T directory.
{$(S)\}.asm{$(R)\OBJ\T\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\OBJ\T\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" $@

{$(S)\}.asm{$(R)\Debug\OBJ\T\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\OBJ\T\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" $@

#   Rules for the small memory model. Output into the $(R)[\Debug]\OBJ\S directory.
{$(S)\}.asm{$(R)\OBJ\S\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\OBJ\S\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=S" $@

{$(S)\}.asm{$(R)\Debug\OBJ\S\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\OBJ\S\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=S" $@

#   Rules for the large memory model. Output into the $(R)[\Debug]\OBJ\L directory.
{$(S)\}.asm{$(R)\OBJ\L\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\OBJ\L\}.obj:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=L" $@

{$(S)\}.asm{$(R)\Debug\OBJ\L\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\OBJ\L\}.obj:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=L" $@

# Inference rules to compile a Windows 16-bits resource file, inferring the debug mode from the output path specified.
{$(S)\}.rc{$(R)\OBJ\}.res:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.rc{$$(R)\OBJ\}.res:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" $@

{$(S)\}.rc{$(R)\Debug\OBJ\}.res:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.rc{$$(R)\Debug\OBJ\}.res:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

# Inference rules to build a C++ program, inferring the memory model and debug mode from the output path specified.
# (Define C++ inferences rules before C inferences rules, so that if both a .c and .cpp file are present, the .cpp is used preferably.)
#   First rules for a target with no memory model defined. Output directly into the $(R)[\Debug] directory.
{$(S)\}.cpp{$(R)\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\}.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\OBJ\T\$(*F).obj $(R)\BIN\T\$(*F).com

{$(S)\}.cpp{$(R)\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\}.exe:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" $@

{$(S)\}.cpp{$(R)\Debug\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\}.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\Debug\OBJ\T\$(*F).obj $(R)\Debug\BIN\T\$(*F).com

{$(S)\}.cpp{$(R)\Debug\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\}.exe:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

#   Rules for the tiny memory model. Output into the $(R)[\Debug][\OBJ\T] directory.
{$(S)\}.cpp{$(R)\BIN\T\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\BIN\T\}.com:
    $(HEADLINE) Building $(@F) $(T) tiny release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\OBJ\T\$(*F).obj $(R)\BIN\T\$(*F).com

{$(S)\}.cpp{$(R)\Debug\BIN\T\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\T\}.com:
    $(HEADLINE) Building $(@F) $(T) tiny debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\Debug\OBJ\T\$(*F).obj $(R)\Debug\BIN\T\$(*F).com

#   Rules for the small memory model. Output into the $(R)[\Debug][\OBJ\S] directory.
{$(S)\}.cpp{$(R)\BIN\S\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\BIN\S\}.exe:
    $(HEADLINE) Building $(@F) $(T) small release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=S" dirs $(R)\OBJ\S\$(*F).obj $(R)\BIN\S\$(*F).exe

{$(S)\}.cpp{$(R)\Debug\BIN\S\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\BIN\S\}.exe:
    $(HEADLINE) Building $(@F) $(T) small debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=S" dirs $(R)\Debug\OBJ\S\$(*F).obj $(R)\Debug\BIN\S\$(*F).exe

#   Rules for the large memory model. Output into the $(R)[\Debug][\OBJ\L] directory.
{$(S)\}.cpp{$(R)\BIN\L\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\BIN\L\}.exe:
    $(HEADLINE) Building $(@F) $(T) large release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=L" dirs $(R)\OBJ\L\$(*F).obj $(R)\BIN\L\$(*F).exe

{$(S)\}.cpp{$(R)\Debug\BIN\L\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.cpp{$$(R)\Debug\BIN\L\}.exe:
    $(HEADLINE) Building $(@F) $(T) large debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=L" dirs $(R)\Debug\OBJ\L\$(*F).obj $(R)\Debug\BIN\L\$(*F).exe

# Inference rules to build a C program, inferring the memory model and debug mode from the output path specified.
#   First rules for a target with no memory model defined. Output directly into the $(R)[\Debug] directory.
{$(S)\}.c{$(R)\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\}.com:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\OBJ\T\$(*F).obj $(R)\BIN\T\$(*F).com

{$(S)\}.c{$(R)\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\}.exe:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" $@

{$(S)\}.c{$(R)\Debug\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\}.com:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\Debug\OBJ\T\$(*F).obj $(R)\Debug\BIN\T\$(*F).com

{$(S)\}.c{$(R)\Debug\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\}.exe:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

#   Rules for the tiny memory model. Output into the $(R)[\Debug][\OBJ\T] directory.
{$(S)\}.c{$(R)\BIN\T\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\BIN\T\}.com:
    $(HEADLINE) Building $(@F) $(T) tiny version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\OBJ\T\$(*F).obj $(R)\BIN\T\$(*F).com

{$(S)\}.c{$(R)\Debug\BIN\T\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\BIN\T\}.com:
    $(HEADLINE) Building $(@F) $(T) tiny debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\Debug\OBJ\T\$(*F).obj $(R)\Debug\BIN\T\$(*F).com

#   Rules for the small memory model. Output into the $(R)[\Debug][\OBJ\S] directory.
{$(S)\}.c{$(R)\BIN\S\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\BIN\S\}.exe:
    $(HEADLINE) Building $(@F) $(T) small version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=S" dirs $(R)\OBJ\S\$(*F).obj $(R)\BIN\S\$(*F).exe

{$(S)\}.c{$(R)\Debug\BIN\S\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\BIN\S\}.exe:
    $(HEADLINE) Building $(@F) $(T) small debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=S" dirs $(R)\Debug\OBJ\S\$(*F).obj $(R)\Debug\BIN\S\$(*F).exe

#   Rules for the large memory model. Output into the $(R)[\Debug][\OBJ\L] directory.
{$(S)\}.c{$(R)\BIN\L\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\BIN\L\}.exe:
    $(HEADLINE) Building $(@F) $(T) large release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=L" dirs $(R)\OBJ\L\$(*F).obj $(R)\BIN\L\$(*F).exe

{$(S)\}.c{$(R)\Debug\BIN\L\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.c{$$(R)\Debug\BIN\L\}.exe:
    $(HEADLINE) Building $(@F) $(T) large debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=L" dirs $(R)\Debug\OBJ\L\$(*F).obj $(R)\Debug\BIN\L\$(*F).exe

# Inference rules to build an ASM program, inferring the memory model and debug mode from the output path specified.
#   First rules for a target with no memory model defined. Output directly into the $(R)[\Debug] directory.
{$(S)\}.asm{$(R)\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\}.com:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\OBJ\T\$(*F).obj $(R)\BIN\T\$(*F).com

{$(S)\}.asm{$(R)\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\}.exe:
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" $@

{$(S)\}.asm{$(R)\Debug\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\}.com:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\Debug\OBJ\T\$(*F).obj $(R)\Debug\BIN\T\$(*F).com

{$(S)\}.asm{$(R)\Debug\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\}.exe:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

#   Rules for the tiny memory model. Output into the $(R)[\Debug][\OBJ\T] directory.
{$(S)\}.asm{$(R)\BIN\T\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\BIN\T\}.com:
    $(HEADLINE) Building $(@F) $(T) tiny version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\OBJ\T\$(*F).obj $(R)\BIN\T\$(*F).com

{$(S)\}.asm{$(R)\Debug\BIN\T\}.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\BIN\T\}.com:
    $(HEADLINE) Building $(@F) $(T) tiny debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" dirs $(R)\Debug\OBJ\T\$(*F).obj $(R)\Debug\BIN\T\$(*F).com

#   Rules for the small memory model. Output into the $(R)[\Debug][\OBJ\S] directory.
{$(S)\}.asm{$(R)\BIN\S\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\BIN\S\}.exe:
    $(HEADLINE) Building $(@F) $(T) small version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=S" dirs $(R)\OBJ\S\$(*F).obj $(R)\BIN\S\$(*F).exe

{$(S)\}.asm{$(R)\Debug\BIN\S\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\BIN\S\}.exe:
    $(HEADLINE) Building $(@F) $(T) small debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=S" dirs $(R)\Debug\OBJ\S\$(*F).obj $(R)\Debug\BIN\S\$(*F).exe

#   Rules for the large memory model. Output into the $(R)[\Debug][\OBJ\L] directory.
{$(S)\}.asm{$(R)\BIN\L\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\BIN\L\}.exe:
    $(HEADLINE) Building $(@F) $(T) large release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=L" dirs $(R)\OBJ\L\$(*F).obj $(R)\BIN\L\$(*F).exe

{$(S)\}.asm{$(R)\Debug\BIN\L\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.asm{$$(R)\Debug\BIN\L\}.exe:
    $(HEADLINE) Building $(@F) $(T) large debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=L" dirs $(R)\Debug\OBJ\L\$(*F).obj $(R)\Debug\BIN\L\$(*F).exe

# Inference rules to build a library, inferring the memory model and debug mode from the output path specified.
{$(S)\}.mak{$(R)\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.mak{$$(R)\}.lib:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

{$(S)\}.mak{$(R)\Debug\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.mak{$$(R)\Debug\}.lib:
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" $@

{$(S)\}.mak{$(R)\BIN\T\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.mak{$$(R)\BIN\T\}.lib:
    $(HEADLINE) Building $(@F) $(T) tiny release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=T" dirs $@

{$(S)\}.mak{$(R)\Debug\BIN\T\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.mak{$$(R)\Debug\BIN\T\}.lib:
    $(HEADLINE) Building $(@F) $(T) tiny debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=T" dirs $@

{$(S)\}.mak{$(R)\BIN\S\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.mak{$$(R)\BIN\S\}.lib:
    $(HEADLINE) Building $(@F) $(T) small release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=S" dirs $@

{$(S)\}.mak{$(R)\Debug\BIN\S\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.mak{$$(R)\Debug\BIN\S\}.lib:
    $(HEADLINE) Building $(@F) $(T) small debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=S" dirs $@

{$(S)\}.mak{$(R)\BIN\L\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.mak{$$(R)\BIN\L\}.lib:
    $(HEADLINE) Building $(@F) $(T) large release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(*F)" "MEM=L" dirs $@

{$(S)\}.mak{$(R)\Debug\BIN\L\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) {$$(S)\}.mak{$$(R)\Debug\BIN\L\}.lib:
    $(HEADLINE) Building $(@F) $(T) large debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(*F)" "MEM=L" dirs $@

!ELSE # if DEFINED(PROGRAM)

# Inference rules generating the output path, using the predefined debug mode.
.cpp.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .cpp.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(O)\$(*F).obj

.c.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .c.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(O)\$(*F).obj

.asm.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .asm.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(O)\$(*F).obj

.rc.res:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .rc.res:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(O)\$(*F).res

.cpp.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .cpp.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(O)\$(*F).obj $(B)\$(*F).exe

.c.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .c.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(O)\$(*F).obj $(B)\$(*F).exe

.asm.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .asm.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(O)\$(*F).obj $(B)\$(*F).exe

.cpp.com:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .cpp.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" "MEM=T" dirs $(R)$(DS)\OBJ\T\$(*F).obj $(R)$(DS)\BIN\T\$(*F).com

.c.com:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .c.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" "MEM=T" dirs $(R)$(DS)\OBJ\T\$(*F).obj $(R)$(DS)\BIN\T\$(*F).com

.asm.com:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .asm.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" "MEM=T" dirs $(R)$(DS)\OBJ\T\$(*F).obj $(R)$(DS)\BIN\T\$(*F).com

{$(S)\}.cpp{$(R)\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.cpp{$$(R)\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj

{$(S)\}.cpp{$(R)\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.cpp{$$(R)\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj

{$(S)\}.cpp{$(R)\Debug\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.cpp{$$(R)\Debug\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj

{$(S)\}.cpp{$(R)\Debug\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.cpp{$$(R)\Debug\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj

{$(S)\}.c{$(R)\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.c{$$(R)\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj

{$(S)\}.c{$(R)\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.c{$$(R)\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj

{$(S)\}.c{$(R)\Debug\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.c{$$(R)\Debug\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj

{$(S)\}.c{$(R)\Debug\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.c{$$(R)\Debug\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj

{$(S)\}.asm{$(R)\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.asm{$$(R)\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj

{$(S)\}.asm{$(R)\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.asm{$$(R)\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj

{$(S)\}.asm{$(R)\Debug\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.asm{$$(R)\Debug\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj

{$(S)\}.asm{$(R)\Debug\OBJ\}.obj:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.asm{$$(R)\Debug\}.obj:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj

{$(S)\}.cpp{$(R)\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.cpp{$$(R)\}.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj $(R)\BIN\$(MEM)\$(*F).exe

{$(S)\}.cpp{$(R)\Debug\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.cpp{$$(R)\Debug\}.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj $(R)\Debug\BIN\$(MEM)\$(*F).exe

{$(S)\}.c{$(R)\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.c{$$(R)\}.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj $(R)\BIN\$(MEM)\$(*F).exe

{$(S)\}.c{$(R)\Debug\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.c{$$(R)\Debug\}.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj $(R)\Debug\BIN\$(MEM)\$(*F).exe

{$(S)\}.asm{$(R)\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.asm{$$(R)\}.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) release version
    $(SUBMAKE) "DEBUG=0" "PROGRAM=$(PROGRAM)" dirs $(R)\OBJ\$(MEM)\$(*F).obj $(R)\BIN\$(MEM)\$(*F).exe

{$(S)\}.asm{$(R)\Debug\}.exe:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.asm{$$(R)\Debug\}.exe:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\OBJ\$(MEM)\$(*F).obj $(R)\Debug\BIN\$(MEM)\$(*F).exe

# Inference rules to build a library, inferring the memory model and debug mode from the output path specified.
.mak.lib:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .mak.lib:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(B)\$(*F).lib

{$(S)\}.mak{Debug\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.mak{Debug\}.lib:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\BIN\$(MEM)\$(*F).lib

{$(S)\}.mak{$(R)\}.lib:
    @echo Applying $(T).mak inference rule {$$(S)\}.mak{$$(R)\}.lib:
    $(HEADLINE) Building $(@F) $(T) $(MMN) $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" dirs $(B)\$(*F).lib

{$(S)\}.mak{$(R)\Debug\}.lib:
    @echo Applying $(T).mak inference rule (PROGRAM defined) {$$(S)\}.mak{$$(R)\Debug\}.lib:
    $(HEADLINE) Building $(@F) $(T) $(MMN) debug version
    $(SUBMAKE) "DEBUG=1" "PROGRAM=$(PROGRAM)" dirs $(R)\Debug\BIN\$(MEM)\$(*F).lib

# Inference rule for C++ compilation
{$(S)\}.cpp{$(O)\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.cpp{$$(O)\}.obj:
    $(MSG) Compiling $(<F) ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(REMOVE_UTF8_BOM) $< $(O)\$(<F)
    $(COMPACT_PATHS) & $(CC) $(CFLAGS) /c $(TC) $(O)\$(<F) || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule for C compilation
{$(S)\}.c{$(O)\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.c{$$(O)\}.obj:
    $(MSG) Compiling $(<F) ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(REMOVE_UTF8_BOM) $< $(O)\$(<F)
    $(COMPACT_PATHS) & $(CC) $(CFLAGS) /c $(TC) $(O)\$(<F) || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule for C compilation of resident modules
{$(S)\}.r{$(O)\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.r{$$(O)\}.obj:
    $(MSG) Compiling $(<F) ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(REMOVE_UTF8_BOM) $< $(O)\$(<F)
    $(COMPACT_PATHS) & $(CC) $(CFLAGS) /NTRESID /c $(TC) $(O)\$(<F) || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule for Assembly language.
{$(S)\}.asm{$(O)\}.obj:
    @echo Applying $(T).mak inference rule {$$(S)\}.asm{$$(O)\}.obj:
    $(MSG) Assembling $(<F) ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(COMPACT_PATHS) & $(AS) $(AFLAGS) /c $< || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule to compile Windows 16-bits resources
{$(S)\}.rc{$(O)\}.res:
    @echo Applying $(T).mak inference rule {$$(S)\}.rc{$$(O)\}.res:
    $(MSG) Compiling $(<F) resources ...
    set INCLUDE=$(INCLUDE)
    set PATH=$(PATH)
    $(COMPACT_PATHS) & $(RC) /Fo$@ $(RFLAGS) /r $< || $(REPORT_FAILURE)
    $(MSG) ... done.

# Inference rule to link a program
{$(O)\}.obj{$(B)\}.com:
    @echo Applying $(T).mak inference rule {$$(O)\}.obj{$$(B)\}.com:
    $(MSG) Linking $(B)\$(@F) ...
    set LIB=$(LIB)
    set PATH=$(PATH)
    copy << $(L)\$(*B).LNK
$(STARTCOM) $**
$@
$(L)\$(*F)
$(LIBS)
$(LFLAGS) /tiny
<<NOKEEP
    @echo "	type $(L)\$(*B).LNK"
    @$(COMSPEC) /c "type $(L)\$(*B).LNK"
    $(LK) @$(L)\$(*B).LNK || $(REPORT_FAILURE)
    if exist $@ copy $@ $(BB)
    cd $(L)
    -$(MAPSYM) $(*F).map
    cd $(MAKEDIR)
    $(MSG) ... done.

# Inference rule to link a program
{$(O)\}.obj{$(B)\}.exe:
    @echo Applying $(T).mak inference rule {$$(O)\}.obj{$$(B)\}.exe:
    $(MSG) Linking $(B)\$(@F) ...
    set LIB=$(LIB)
    set PATH=$(PATH)
    copy << $(L)\$(*B).LNK
$(STARTEXE) $**
$@
$(L)\$(*F)
$(LIBS)
$(LFLAGS) /knoweas /stack:32768
<<NOKEEP
    @echo "	type $(L)\$(*B).LNK"
    @$(COMSPEC) /c "type $(L)\$(*B).LNK"
    $(LK) @$(L)\$(*B).LNK || $(REPORT_FAILURE)
    if exist $@ copy $@ $(BB)
    cd $(L)
    -$(MAPSYM) $(*F).map
    cd $(MAKEDIR)
    $(MSG) ... done.

# Inference rule to build a library
{$(O)\}.mak{$(B)\}.lib:
    @echo Applying $(T).mak inference rule {$$(O)\}.mak{$$(B)\}.lib:
    $(MSG) Creating $(B)\$(@F) ...
    if exist $@ del $@
    set PATH=$(PATH)
    $(LB) /batch @<< || $(REPORT_FAILURE)
$@
$(OBJECTS:/=\)
$(L)\$(@B).lst
;
<<NOKEEP
    if exist $@ copy $@ $(BB)
    $(MSG) ... done.

###############################################################################
#									      #
#			        Specific rules				      #
#									      #
###############################################################################

# $(PROGRAM).mak and/or Files.mak may define macros SOURCES, OBJECTS, LIBRARIES, and PROGRAM.
# These make files are intended to be OS-independant, and be used in both Windows and Unix build environments. 
# These macros in turn allow the following rules to work, and build more complex programs with more than one source.
#
# 2015-10-30 JFL Moved the inclusion of $(PROGRAM).mak or FILES.mak at the top of this make file.
#                This move allows defining the DOS memory model in individual $(PROGRAM).mak files.
# 2015-11-02 JFL We still need a second inclusion of the same make files here,
#		 as the definition of the memory model may have changed the $(B) and $(O) definitions.
# 2016-09-21 JFL Actually we must only do the full make files inclusion here, else we get warnings
#		 about goals defined twice.
!IF DEFINED(PROGRAM) && EXIST("$(PROGRAM).mak")
!  MESSAGE Getting specific rules from $(PROGRAM).mak.
!  INCLUDE $(PROGRAM).mak
!ELSE IF EXIST("Files.mak")
!  MESSAGE Getting specific rules from Files.mak.
!  INCLUDE Files.mak
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
OBJECTS=$(O)\$(PROGRAM).obj
!ENDIF

# Generate LIBRARIES based on LIB & LIBS
!IF DEFINED(LIBS) && !DEFINED(LIBRARIES)
!  INCLUDE lib2libs.mak # Generate LIBRARIES based on LIB & LIBS
LIBS= # Avoid having them defined twice in the linker input list
!ENDIF

# Generic rule to build program
!IF !DEFINED(SKIP_THIS)
$(B)\$(EXENAME): $(OBJECTS:+=) $(LIBRARIES)
    @echo Applying $(T).mak build rule $$(B)\$$(EXENAME):
    $(MSG) Linking $(B)\$(@F) ...
    set LIB=$(LIB)
    set PATH=$(PATH)
    copy << $(L)\$(*B).LNK
$(STARTEXE) $(OBJECTS:+=)
"$@"
$(L)\$(*F)
$(LIBRARIES) $(LIBS)
$(LFLAGS) /knoweas /stack:32768
<<NOKEEP
    @echo "	type $(L)\$(*B).LNK"
    @$(COMSPEC) /c "type $(L)\$(*B).LNK"
    $(LK) @$(L)\$(*B).LNK || $(REPORT_FAILURE)
    if exist $@ copy $@ $(BB)
    cd $(L)
    -$(MAPSYM) $(*F).map
    cd $(MAKEDIR)
    $(MSG) ... done.

# Generic rule to build a library
$(B)\$(PROGRAM).lib: $(OBJECTS:+=) $(LIBRARIES)
    @echo Applying $$(B)\$$(PROGRAM).lib build rule:
    $(MSG) Creating $@ ...
    if exist $@ del $@
    set PATH=$(PATH)
    $(LB) /batch @<<$(L)\$(PROGRAM).inp || $(REPORT_FAILURE)
"$@"
$(OBJECTS:/=\)
$(L)\$(PROGRAM).lst
;
<<KEEP
    if exist $@ copy $@ $(BB)
    $(MSG) ... done.

!ELSE # DEFINED(SKIP_THIS)
$(OBJECTS) $(B)\$(EXENAME) $(B)\$(PROGRAM).lib: skip_this
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

files: $(UTF8_BOM_FILE) $(REMOVE_UTF8_BOM) $(CONV_SCRIPT) $(COMPACT_PATHS)
!ELSE
dirs files: skip_this
    @rem This rem prevents inference rules from firing. Do not remove.
!ENDIF

$(UTF8_BOM_FILE): $(MAKEFILE)
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

$(REMOVE_UTF8_BOM): $(MAKEFILE)
    $(MSG) Generating script $@
    copy <<$@ NUL
	@echo off
	findstr /B /G:$(UTF8_BOM_FILE) <"%~1" >NUL
	if errorlevel 1 (
	  echo No UTF-8 BOM in "%~1". Copying the file.
	  copy /y "%~1" "%~2"
	) else ( rem :# Remove the BOM before compiling the source
	  echo UTF-8 BOM found in "%~1". Converting the file.
	  :# Must be compatible both with conv.exe and $(CONV_SCRIPT)
	  $(CONV) 8 d "%~1" "%~2" -B 
	)
<<KEEP

$(CONV_SCRIPT): $(MAKEFILE)	# Poor man's version of conv.exe, limited to what this make file needs
    $(MSG) Generating script $@
    copy <<$@ NUL
	@if (@Language == @Batch) @then /* NOOP for Batch; Begins a comment for JScript.
	@echo off & cscript //E:JScript //nologo "%~f0" %* & exit /b
	:# End of the Batch section, and beginning of the JScript section */ @end
	var args = WScript.Arguments;
	// Use text streams: https://msdn.microsoft.com/en-us/library/ms675032(v=vs.85).aspx
	var adTypeText = 2;
	var adSaveCreateOverWrite = 2;
	var ReadTextFile = function(fileName, encoding) {
	  var inStream  = WScript.CreateObject("adodb.stream");
	  inStream.type = adTypeText;
	  inStream.charset = encoding;
	  inStream.open();
	  inStream.LoadFromFile(fileName);
	  var text = inStream.ReadText()
	  inStream.Close();
	  return text;
	}
	var WriteTextFile = function(fileName, encoding, text) {
	  var outStream  = WScript.CreateObject("adodb.stream");
	  outStream.type = adTypeText;
	  outStream.charset = encoding;
	  outStream.open();
	  outStream.WriteText(text);
	  outStream.SaveToFile(fileName, adSaveCreateOverWrite);
	  outStream.Close();
	}
	text = ReadTextFile(args(2), "utf-8");
	WriteTextFile(args(3), "$(DOS_CS)", text);
	WScript.Quit(0);
<<KEEP

$(COMPACT_PATHS): $(MAKEFILE)
    $(MSG) Generating script $@
    copy <<$@ NUL
	@echo off
	:# Get the absolute pathname of the parent directory
	for /f %%f in ('pushd .. ^& if not errorlevel 1 cd ^& popd') do @set "UP=%%f"
	:# Get the absolute pathname of the grand parent directory
	for /f %%f in ('pushd ..\.. ^& if not errorlevel 1 cd ^& popd') do @set "UP2=%%f"
	for %%v in (INCLUDE PATH %*) do @( :# Foreach variable name passed as an argument
	  setlocal EnableExtensions EnableDelayedExpansion &:# Allow expanding !variables!
	  set VAR=!%%v!			&:# Get the variable value
	  set VAR=!VAR:%UP%=..!		&:# Replace the parent pathname by ..
	  set VAR=!VAR:%UP2%=..\..!	&:# Replace the grand parent pathname by ..\..
	  :# Exit the local scope, and update the variable in the parent scope
	  for /f "delims=" %%r in ('echo !VAR!') do (endlocal & set %%v=%%r)
	)
<<KEEP

# Erase all output files
clean:
    -rd /S /Q $(R)	>NUL 2>&1
    -del /Q *.bak	>NUL 2>&1
    -del /Q *~		>NUL 2>&1

# Help message describing the targets
help:
    type <<
Targets:
  clean                    Erase all files in the $(R) directory
  $(R)\{prog}.com           Build {prog}.com release version from {prog}.c/cpp
  $(R)\Debug\{prog}.com     Build {prog}.com debug version from {prog}.c/cpp
  $(R)\{prog}.exe           Build {prog}.exe release version from {prog}.c/cpp
  $(R)\Debug\{prog}.exe     Build {prog}.exe debug version from {prog}.c/cpp
  $(R)\OBJ\{prog}.obj       Compile {prog}.obj release version from {prog}.c/cpp
  $(R)\Debug\OBJ\{prog}.obj Compile {prog}.obj debug version from {prog}.c/cpp
  $(R)[\Debug][\OBJ]\T\*    Compile/Build the tiny version of *
  $(R)[\Debug][\OBJ]\S\*    Compile/Build the small version of *
  $(R)[\Debug][\OBJ]\L\*    Compile/Build the large version of *
  {prog}.com               Build $(R)[\Debug]\{prog}.com from {prog}.c/cpp
  {prog}.exe               Build $(R)[\Debug]\{prog}.exe from {prog}.c/cpp
  {prog}.obj               Compile $(R)[\Debug]\OBJ\{prog}.obj from {prog}.c/cpp
  {prog}.res               Compile $(R)[\Debug]\OBJ\{prog}.res from {prog}.rc

The debug mode is set based on the first definition found in...
 1) The nmake command line option "DEBUG=0|1"
 2) The target directory $(R)|$(R)\Debug
 3) The environment variable DEBUG=0|1
 4) Default: DEBUG=0

The memory model is set based on the first definition found in...
 1) The nmake command line option "MEM=T|S|L"
 2) The target directory $(R)[\Debug]\T|S|L
 3) The {prog}.mak definition MEM=T|S|L
 4) Default: MEM=S for .exe targets, or MEM=T for .com targets
<<NOKEEP

!ENDIF # !DEFINED(DISPATCH_OS)
