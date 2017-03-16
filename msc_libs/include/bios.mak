###############################################################################
#									      #
#   File name:	    bios.mak						      #
#									      #
#   Description:    A NMake makefile to build BIOS and MINICOM programs.      #
#									      #
#   Notes:	    Use with make.bat, which defines the necessary variables. #
#		    Usage: make -f bios.mak [definitions] [targets]	      #
#									      #
#		    MINICOM programs are DOS .com programs that _only_ use    #
#		    BIOS function calls, defined in the BiosLib library.      #
#		    This allows testing in DOS programs intended to	      #
#		    eventually reside in BIOS option ROMs.		      #
#		    This also allows creating minimized DOS programs (Hence   #
#		    the name MINICOM), much smaller than the ones linked with #
#		    MSVC's standard C library. This can be very useful on     #
#		    systems with extremely limited storage space, like JFL's  #
#		    Universal Boot Disks (UBD).				      #
#		    							      #
#		    The LoDosLib library allows building hybrid programs,     #
#		    using mostly BIOS calls, and a few selected DOS calls.    #
#		    It's useful for building DOS drivers and TSRs, that need  #
#		    to be as small as possible, and can call only a few       #
#                   limited DOS functions.                                    #
#		    							      #
#		    The PMode library allows building BIOS & DOS programs     #
#		    that switch between the x86 real, v86, and protected mode.#
#		    							      #
#		    The SysLib library defines a set of utility routines      #
#		    usable in all environments.				      #
#		    							      #
#		    Targets:						      #
#		    clean	    Erase all files in the BIOS subdirectory. #
#		    {prog}.com	    Build BIOS[\Debug]\{prog}.com.	      #
#		    {prog}.obj	    Build BIOS[\Debug]\OBJ\{prog}.obj.	      #
#		    BIOS\{prog}.com       Build the BIOS release version.     #
#		    BIOS\Debug\{prog}.com Build the BIOS debug version.       #
#		    BIOS\OBJ\{prog}.obj       Compile the BIOS release version.
#		    BIOS\Debug\OBJ\{prog}.obj Compile the BIOS debug version. #
#									      #
#		    Command-line definitions:				      #
#		    DEBUG=0	 Build the release ver. (<=> program in BIOS) #
#		    DEBUG=1	 Build the debug ver. (<=> pgm in BIOS\DEBUG) #
#		    MEM=T	 Build the tiny ver.  (<=> objects in OBJ\T)  #
#		    MEM=S	 Build the small ver. (<=> objects in OBJ\S)  #
#		    MEM=L	 Build the large ver. (<=> objects in OBJ\L)  #
#		    OUTDIR=path  Output to path\BIOS\. Default: To .\BIOS\    #
#		    PROGRAM=name Set the output file base name		      #
#									      #
#		    The MEM variable is left to minimize differences with     #
#		    the DOS.MAK make file. But the default value T should     #
#		    work in all cases here.				      #
#		    							      #
#		    Likewise, rules for building .exe targets are left in.    #
#		    They should never be needed either.			      #
#		    							      #
#		    If a specific target [path\]{prog}.com is specified,      #
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
#		    Another design goal is to use that same bios.mak	      #
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
#    2015-10-23 JFL Adapted from DOS.mak.                                     #
#    2015-11-03 JFL Added rules to build a library from a .mak file.          #
#    2015-11-13 JFL Use new generic environment definition variables.         #
#    2015-12-07 JFL Added support for a base output directory other than .\   #
#    2016-04-01 JFL Do not change the PROGRAM value, once it has been set.    #
#		    Added an inference rule for compiling resident C modules. #
#    2016-04-11 JFL Renamed NODOSLIB as BIOSLIB.                              #
#    2016-04-14 JFL Forward HAS_<lib> flags to the C compiler.		      #
#    2016-04-22 JFL Renamed the MULTIOS library as SYSLIB.		      #
#    2016-09-02 JFL Added scripts for removing the UTF-8 BOM from C sources.  #
#    2016-09-21 JFL Fixed an issue that caused double definition warnings.    #
#    2016-09-28 JFL Display FAILED messages when compilation or link fails.   #
#		    Avoid having the word "Error" in the log unnecessarily.   #
#    2016-10-04 JFL Use the shell PID to generate unique temp file names.     #
#		    Display messages only if variable MESSAGES is defined.    #
#    2016-10-11 JFL Adapted for use in SysToolsLib global C include dir.      #
#    2017-03-02 JFL Fixed src2objs.bat and use it indirectly via src2objs.mak.#
#		    							      #
#         © Copyright 2016 Hewlett Packard Enterprise Development LP          #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

.SUFFIXES: # Clear the predefined suffixes list.
.SUFFIXES: .com .exe .sys .obj .asm .c .r .cpp .mak

###############################################################################
#									      #
#			        Definitions				      #
#									      #
###############################################################################

T=BIOS				# Target OS
!IF DEFINED(MESSAGES)
!MESSAGE Started $(T).mak in $(MAKEDIR) # Display this file name
!ENDIF

# Command-line definitions that need carrying through to sub-make instances
# Note: Cannot redefine MAKEFLAGS, so defining an alternate variable instead.
MAKEDEFS=
!IF DEFINED(MEM)	# Memory model for 16-bits compilation. T|S|C|D|L|H.
MAKEDEFS=$(MAKEDEFS) "MEM=$(MEM)"
!ENDIF

MAKEFILE=bios.mak		# This make file name
!IF (!EXIST("$(MAKEFILE)")) && EXIST("$(STINCLUDE)\$(MAKEFILE)")
MAKEFILE=$(STINCLUDE)\$(MAKEFILE)
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
DD=$(DD) /D_BIOS /DMINICOMS	# Tell sources what environment they're built for

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
MEM=T				# Memory model for C compilation
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
!ELSEIF  "$(MEM)"=="S"
MMN=small
!ELSEIF  "$(MEM)"=="C"
MMN=code
!ELSEIF  "$(MEM)"=="D"
MMN=data
!ELSEIF  "$(MEM)"=="L"
MMN=large
!ELSEIF  "$(MEM)"=="H"
MMN=huge
!ELSE
!ERROR "Invalid memory model: $(MEM)"
!ENDIF

# Define directories
S=.				# Where to find source files
R=$(T)				# Root output path.
!IF DEFINED(OUTDIR)
R=$(OUTDIR)\$(T)
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
AFLAGS=/Cx $(DD) /I$(O) /Fl$(L)\ /Fo$(O)\ /San /Zdim
CFLAGS=/A$(MEM) $(DD) /Fc$(L)\ /Fd$(B)\ /Fo$(O)\ /G2rs /Oaes /W4 /Zpil
!IF DEFINED(DOS_VCINC)
CFLAGS=$(CFLAGS) "/DMSVCINCLUDE=$(DOS_VCINC:\=/)" # Path of MSVC compiler include files, without quotes, and with forward slashes
!ENDIF
!IF DEFINED(DOS_CRTINC)
CFLAGS=$(CFLAGS) "/DUCRTINCLUDE=$(DOS_CRTINC:\=/)" # Path of MSVC CRT library include files, without quotes, and with forward slashes
!ENDIF
LFLAGS=/map /li /batch /nod /noe /onerror:noexe
!IF "$(DEBUG)"=="1"
# Note: The MSVC 1.52 linker does not support the /debug option
LFLAGS=$(LFLAGS) /co
!ENDIF
RFLAGS=$(DD)

INCPATH=$(BIOSLIB)
LIBPATH=$(BIOSLIB)
LIBS=bios.lib
!IF DEFINED(LODOSLIB)
INCPATH=$(INCPATH);$(LODOSLIB)
LIBPATH=$(LIBPATH);$(LODOSLIB)
LIBS=$(LIBS) + lodos.lib
!ENDIF
!IF DEFINED(PMODE)
INCPATH=$(INCPATH);$(PMODE)
LIBPATH=$(LIBPATH);$(PMODE)
LIBS=$(LIBS) + pmode.lib
!ENDIF
!IF DEFINED(SYSLIB)
INCPATH=$(INCPATH);$(SYSLIB)
LIBPATH=$(LIBPATH);$(SYSLIB)\$(B)
LIBS=$(LIBS) + syslib.lib
!ENDIF
!IF DEFINED(GNUEFI)
INCPATH=$(INCPATH);$(GNUEFI)\INC
!ENDIF

PATH=$(DOS_PATH)
CC=$(DOS_CC)
AS=$(DOS_AS)
INCLUDE=$(S);$(STINCLUDE);$(INCPATH);$(USER_INCLUDE)
LK=$(DOS_LK)
LIBS=$(LIBS) $(USER_LIBS)
LIB=$(LIBPATH)
LB=$(DOS_LB)

STARTCOM=$(BIOSLIB)\OBJ\startcom.obj
STARTEXE=$(LODOSLIB)\OBJ\startexe.obj
!ENDIF # !DEFINED(T_VARS)

# Forward library detections by configure.bat to the C compiler
CFLAGS=$(CFLAGS) $(HAS_SDK_FLAGS)

# Files and scripts used for compilation
UTF8_BOM_FILE=$(O)\UTF8_BOM	# A file containing the UTF-8 Byte-Order Mark
REMOVE_UTF8_BOM=$(O)\RemBOM.bat	# Tool for conditionally removing the UTF-8 BOM
CONV_SCRIPT=$(O)\MiniConv.bat	# Script emulating what conv.exe would do for us
COMPACT_PATHS=$(O)\CompactP.bat # Compact paths, to avoid passing the 256-char length limit that causes out-of-memory errors for the DOS C compiler
!IF !DEFINED(CONV)
CONV=$(COMSPEC) /c $(CONV_SCRIPT)
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
SUBMAKE=$(MAKE) /$(MAKEFLAGS) /F "$(MAKEFILE)" $(MAKEDEFS) # Recursive call to this make file

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


.cpp.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .cpp.exe:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.c.exe:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .c.exe:
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" $@

.cpp.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .cpp.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(*F)" "MEM=T" dirs $(R)$(DS)\OBJ\T\$(*F).obj $(R)$(DS)\BIN\T\$(*F).com

.c.com:
    @echo Applying $(T).mak inference rule (PROGRAM undefined) .c.com:
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

.cpp.com:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .cpp.com:
    $(HEADLINE) Building $(@F) $(T) tiny $(DM) version
    $(SUBMAKE) "DEBUG=$(DEBUG)" "PROGRAM=$(PROGRAM)" "MEM=T" dirs $(R)$(DS)\OBJ\T\$(*F).obj $(R)$(DS)\BIN\T\$(*F).com

.c.com:
    @echo Applying $(T).mak inference rule (PROGRAM defined) .c.com:
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

# $(PROGRAM).mak and/or Files.mak may define macros SOURCES, OBJECTS, and PROGRAM.
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
#!  MESSAGE Getting specific rules from $(PROGRAM).mak.
!  INCLUDE $(PROGRAM).mak
!ELSE IF EXIST("Files.mak")
#!  MESSAGE Getting specific rules from Files.mak.
!  INCLUDE Files.mak
!ELSE
!  MESSAGE There are no specific rules.
EXENAME=_-_-_-_.com	# An unlikely name, to prevent the $(EXENAME) dependency rule below from firing.
OBJECTS=
!ENDIF

!IF !DEFINED(EXENAME)
EXENAME=$(PROGRAM).com
!ENDIF

# If needed, convert the SOURCES list to an OBJECTS list
!IF DEFINED(SOURCES) && !DEFINED(OBJECTS)
!  INCLUDE src2objs.mak # Convert the SOURCES list to an OBJECTS list
!ENDIF

# Dependencies for the specified program
$(B)\$(EXENAME): $(OBJECTS:+=)

# Generic rule to build a library
$(B)\$(PROGRAM).lib: $(OBJECTS:+=)
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

!ENDIF # if DEFINED(PROGRAM)

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

dirs: $(B) $(O) $(L) files

files: $(UTF8_BOM_FILE) $(REMOVE_UTF8_BOM) $(CONV_SCRIPT) $(COMPACT_PATHS)

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
	WriteTextFile(args(3), "cp437", text); // All PCs ROM BIOSs use the IBM CP 437
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
    copy << con
Targets:
 clean                    Erase all files in the $(R) directory
 $(R)\{prog}.com           Build {prog}.com release version from {prog}.c/cpp
 $(R)\Debug\{prog}.com     Build {prog}.com debug version from {prog}.c/cpp
 $(R)\OBJ\{prog}.obj       Compile {prog}.obj release version from {prog}.c/cpp
 $(R)\Debug\OBJ\{prog}.obj Compile {prog}.obj debug version from {prog}.c/cpp

 {prog}.com               Build $(R)[\Debug]\{prog}.com from {prog}.c/cpp

 {prog}.obj               Compile $(R)[\Debug]\OBJ\{prog}.obj from {prog}.c/cpp


The debug mode is set based on the first definition found in...
 1) The nmake command line option "DEBUG=0|1"
 2) The target directory $(R)|$(R)\Debug
 3) The environment variable DEBUG=0|1
 4) Default: DEBUG=0
<<NOKEEP

!ENDIF # !DEFINED(DISPATCH_OS)
