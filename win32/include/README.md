SysToolsLib include files and make system
=========================================

This directory contains the include files and make system necessary for rebuilding all SysToolsLib C tools:

File name       | Descrition
--------------- | -----------------------------------------------------
debugm.h        | Debug macros, for adding debugging features to the debug version of a C program.
*.bat           | Windows scripts for rebuilding SysToolsLib components.
*.mak           | Windows nmake files for rebuilding SysToolsLib components.

These files are usable independently of the SysToolsLib C tools, and of the MsvcLibX library. They're useful to help
adapt Unix tools and libraries, so that they can be build both with Unix and Microsoft development tools.

### Home pages

[This include files and make system](https://github.com/JFLarvoire/SysToolsLib/tree/master/C/include)  
[The SysToolsLib library](https://github.com/JFLarvoire/SysToolsLib)  

### License

[The Apache 2 license](https://www.apache.org/licenses/LICENSE-2.0)  
(Compatible with most other project licenses; Does not "contaminate" them!)


The debug macros
----------------
`debugm.h` contains a set of OS-independent macros for managing distinct debug and release versions of a C or C++ program.
The debug version is generated if the _DEBUG constant is defined. Else the release version is generated.
These macros produce no extra code in the release version and thus have no overhead in that release version.
Even in the debug version, the debug output is disabled by default. It must be enabled by using DEBUG_ON().

Usage:

- One source file must instanciate global variables used by the debug system, by using the DEBUG_GLOBALS macro.
- The source file parsing the arguments must look for one argument (Ex: --debug) and enable the debug mode. Ex:

        DEBUG_CODE(
          if (!strcmp(arg, "--debug")) DEBUG_ON();
        )

- Insert DEBUG_ENTER() calls at the beginning of all routines that should be traced, and replace all their
  return instructions with RETURN_XXX() macros.
- Pepper the sources with DEBUG_PRINTF() calls displaying critical intermediate values.

The debug output will show the function call stack by indenting traced subroutines proportionally to their call depth.
To make the debug output more readable, it is recommended to format it so that it looks like valid C code.

The macros actually support multiple debug levels.  
Level 1 is the normal debug mode.  
Level 2 is the eXtra debug mode. Use it for displaying more detailed debug information, that is not needed in normal
debugging sessions.  
More levels could be used if desired.

Most common macros:

Macro				| Description
------------------------------- | ----------------------------------------------------
DEBUG_CODE(...)			| Code to be present only in the debug version.
DEBUG_CODE_IF_ON(...)		| Code to be present only in the debug version, and that will run only if debug is enabled.
DEBUG_PRINTF((format, ...))	| Print something if debug is enabled. (Notice the double parenthesis!)

For a complete list of available macros, see the `debugm.h` header.


The Windows make system
-----------------------

The Microsoft Visual C++ compiler comes with a make tool called nmake.exe.  
This nmake tool is very similar to Unix make, for dependency and inference rules definitions.  
But it is completely incompatible with Unix make for advanced features, such as preprocessor directives, inline scripts, etc.

This make system was designed with the following goals:

- Have a build look and feel similar on Windows and Unix.
- Be compatible with a wide range of Visual C++ versions. (As far back as Visual Studio 2003)
- Allow building multiple versions of a target in a simple single command, and store them in distinct subdirectories.
- Support multiple target operating systems.
- Support a "release" and a "debug" version for each target OS.
- Support building the same sources remotely from multiple systems and VMs having different tools, storing results
   in different places.

File name       | Description
--------------- | -----------------------------------------------------
configure.bat   | Locates available development tools, and creates a configuration script.
make.bat        | Front end to nmake.exe
--------------- | -----------------------------------------------------
All.mak         | Rules for building several versions of a program at once
arm.mak         | Rules for building the Windows 32-bits ARM version of a program (Experimental)
bios.mak        | Rules for building the PC BIOS 16-bits version of a program
dos.mak         | Rules for building the MS-DOS 16-bits version of a program
win32.mak       | Rules for building the Windows 32-bits x86 version of a program
win64.mak       | Rules for building the Windows 64-bits amd64 version of a program
win95.mak       | Rules for building a win32 version of a program compatible with Windows 95

- All scripts have a -? option to display a help screen.
- Most make files have a "help" pseudo-target to display a help screen. Ex:

        make.bat -f win32.mak help

The `configure.bat` script must to be run once in each directory.  
This is done automatically by `make.bat` the first time it runs.  
Then you only need to run `configure.bat` again if you install new development tools.


## Output directories

By default, all output goes in target-OS-specific subdirectories:

OS		| Base output directory
--------------- | --------------------------------
PC BIOS 	| bin\BIOS\
MS-DOS		| bin\DOS\
Windows 95	| bin\WIN95\
Windows XP+ x86	| bin\WIN32\
Windows x86_64	| bin\WIN64\
Linux i686	| bin\Linux.i686\
Linux x86_64	| bin\Linux.x86_64\

Within each target directory, the output files are located in the base and various subdirectories:

Directory	| Contents
--------------- | -----------------------------------------------------------------
\\*		| The "release" executables for the target OS
\obj\\*		| The object files produced by the compilers and assemblers
\list\\*	| The listings and map files produced by the compilers and linkers
\debug\\*	| The "debug" executables for the target OS
\debug\obj\\*	| The object files produced by the compilers and assemblers
\debug\list\\*	| The listings and map files produced by the compilers and linkers

For virtual machines that build sources in their host's file system, or for network system that build them remotely,
it's possible to override the default `bin` output base.  
This is useful for testing builds with older compilers for example, and the goal is to avoid overwriting the "official"
builds on the main host system.  
For that, in each VM or remote system, create a `%windir%\configure.ID.bat` file (with ID a unique string for each system),
that defines variables `OUTDIR` and/or `MY_SDKS`:

    set "OUTDIR=XPVM"        &:# Optional: Base output path, overriding the default OUTDIR=bin directory.
    set "MY_SDKS=H:\JFL\SDK" &:# Optional: Path to the shared SDKs, as seen from this system.


## Using this make system for a new project

Everything is designed to minimize the amount of things to do, while having the ability to build things easily for
multiple operating systems in a single make command.

For that, developers should create in their source directory one or more of these three special files:

File name       | Description
--------------- | -----------------------------------------------------
Files.mak       | OS-independent declarations of variables (all optional), with lists of files and directories:
                | DIRS = list of subdirectories, with their own subproject to build first.
                | PROGRAMS = list of programs to build. (Without the .exe extension for Windows)
                | SOURCES = Sources to compile and link together, when building a single program.
                | OBJECTS = List of object files link together. Rarely needed, as it's usually computed automatically from SOURCES.
                | LIBRARIES = Libraries to link with the program. Rarely needed, as this list is usually built automatically.
                | Files.mak is required in most projects, and is sufficient in most simple cases.
makefile        | GNU make file, with gmake-specific rules for building the project in Unix.
NMakefile       | MS nmake file, with nmake-specific rules for building the project in Windows, for DOS & Windows.

Note that `configure.bat` will also use the DIRS definitions in `Files.mak`, to automatically run recursively in the
subproject directories. So if all `Files.mak` files are configured correctly, `configure.bat` needs only to be run once
in the project top directory.


### Examples

1) Trivial case with one C source and one corresponding executable for Windows

- Copy the include directory inside your project directory:

        include\
        myprogram.c

  No need to create any specific make file.

- Run `include\configure.bat`  (Note that this creates a local make.bat script, invoking include\make.bat)  
  Run `make.bat myprogram.exe`

2) Several sources, that need to be compiled and linked together into a single executable

- Same as case 1, plus create a Files.mak file containing:

        PROGRAMS = myprogram
        SOURCES = source1.c source2.c source3.c

- The project directory contains:

        include\
        Files.mak
        source1.c
        source2.c
        source3.c

- Run `include\configure.bat`  
  Run `make.bat` to rebuild myprogram.exe.

3) Several sources, each generating a corresponding executable, with one requiring special make instructions

- Same as case 1, plus create a `Files.mak` file containing:

        PROGRAMS = program1 program2 program3

- The project directory contains:

        include\
        Files.mak
        program1.c
        program2.c
        program3.c
        program3.mak

- Run `include\configure.bat`  
  Run `make.bat` to rebuild all three programs.  
  The make files will automatically search for *.c, *.cpp, *.asm, etc, and build program1.exe, program2.exe, program3.exe.  
  If one (for example program3) requires special make instructions, create a program3.mak file, and put them in there.  
  Run `make.bat program2.exe` to rebuild just the second one from program2.c.  
  Run `make.bat program3.exe` to rebuild just the third one from program3.c and instructions from program3.mak.  

4) Build for Windows and Linux

- In addition to all the above, write a `makefile` for Linux, that includes Files.mak, and uses $(PROGRAMS) as the default target:

        include Files.mak
        
        all: $(PROGRAMS)

- If additional pseudo targets and goals are desired for Windows, add an `NMakefile` file, that first includes All.mak:

        !INCLUDE <All.mak>
        
        my_goal:
            echo Doing it now

   Note: Do not specify the path of the All.mak file: nmake will find it automatically in the include directory.

5) A project with several sub-projects in subdirectories

- Put the include directory in the top directory.
- Add in that top directory a Files.mak file defining the DIRS variable:

        DIRS = subproject1 subproject2 subproject3

- Put a `Files.mak`, and others *mak* as needed in each subproject's subdirectory.  
  (No need to duplicate the include directory in each subdirectory!)
- Run `include\configure.bat` once in the top directory. 
  Run `make.bat` in the top directory to rebuild all subprojects recursively.  

6) A project with several programs, each having several sources, and all these sources in the same directory.

- In `Files.mak`, define the PROGRAMS variable as above.

        PROGRAMS = program1 program2 program3

- In `Files.mak`, for each of the above programs, define the $(PROGRAM)_SOURCES as in this example:

        program1_SOURCES = program1a.c program1b.c program1c.c
        program2_SOURCES = program2a.c program2b.c
        program3_SOURCES = program3a.c program3b.c program3c.c

- In `makefile` for Linux, define the rules for each program:
  (Note: The automatic handling of $(PROGRAM)_SOURCES is not yet implemented.)

        SP := .
        BP := bin/$(shell uname -s).$(shell uname -p)
        
        [...]
        
        $(BP)/program1: $(SP)/program1a.c $(SP)/program1b.c $(SP)/program1c.c

        $(BP)/program2: $(SP)/program2a.c $(SP)/program2b.c

        $(BP)/program3: $(SP)/program3a.c $(SP)/program3b.c $(SP)/program3c.c


## Managing program properties

In Windows Explorer, right-clicking on a program and selecting "Properties" displays a dialog box with multiple tabs.
The "Details" tab displays a list of properties of the program.exe: File description, Type, File version, Product name,
Product Version, Copyright, Legal trademarks, Original filename, etc.

The SysToolsLib configuration and make system helps generating this information without having to create a dedicated
program.rc resource file. For that, your C source must contain the following definitions:

C constants definitions                                    | Description
---------------------------------------------------------- | ----------------------------------------------------------
`#define PROGRAM_DESCRIPTION "Demo of program properties"` | A short one-line string describing the program
`#define PROGRAM_NAME        "demoprop"`                   | The base name of the program
`#define PROGRAM_VERSION     "1.0.0"`                      | Program version MAJOR.MINOR.PATH[.BUILD] (Optional)
`#define PROGRAM_DATE        "2019-06-16"`                 | Program date in ISO format. (Used to generate PROGRAM_VERSION if it's missing)

make.bat and win32.mak will extract this information from demoprop.c, and generate a demoprop.rc with it.  
If you *do* have your own demoprop.rc, then #include "SysToolsLib.rc" to do the same.

Note that, for consistency, it's strongly recommended to reuse these same strings in the program's built-in help:
When the user invokes it with options -? for help, or -V for the program version, then display these very strings.

Configure.bat queries Windows for your full name and email address, to generate the copyright strings.  
As these values may not always be the right ones, you can override them by adding in the sources directory
a configure.YOURNAME.bat script defining the values you want:

Batch variables definitions                | Description
------------------------------------------ | -----------------------------
`set "MY_FULLNAME=Jean-François Larvoire"` | The build author's full name
`set "MY_EMAIL=jf.larvoire@free.fr"`       | The build author's email

Important: If, as in this example, the MY_FULLNAME string contains non-ASCII characters, then configure.YOURNAME.bat
must temporarily change the console code page to match its own encoding. Else the string would not be loaded correctly
by configure.bat.    
Configure.bat defines variable CON.CP with the current code page. Configure.USER.bat can use it to set, then restore the
initial code page as needed. For example, assuming configure.USER.bat is encoded using code page 1252, it should contain:

    if not %CON.CP%==1252 chcp 1252 >nul     &:# Make sure the next lines are executed using code page 1252
    set "MY_FULLNAME=Jean-François Larvoire" &:# The build author's full name
    set "MY_EMAIL=jf.larvoire@free.fr"       &:# The build author's email
    if not %CON.CP%==1252 chcp %CON.CP% >nul &:# Restore the initial code page
