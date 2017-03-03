SysToolsLib global include files
================================

This folder contains the include files and make system necessary for rebuilding all SysToolsLib C tools:

File name	| Descrition
--------------- | -----------------------------------------------------
debugm.h	| Debug macros, for adding debugging features to the debug version of a C program.
*.bat		| Windows scripts for rebuilding SysToolsLib components.
*.mak		| Windows nmake files for rebuilding SysToolsLib components.


The Windows make system
-----------------------

The Microsoft Visual C++ compiler comes with a make tool called nmake.exe.
This nmake tool is very similar to Unix make, for dependecy and inference rules definitions.
But it is completely incompatible with Unix make for advanced features, such as conditional tests, etc.

This make system was designed with the following goals:

- Have a build look and feel similar on Windows and Unix.
- Be compatible with a wide range of Visual C++ versions. (As far back as Visual Studio 2003)
- Allow building multiple versions of a target in a simple single command.

File name	| Descrition
--------------- | -----------------------------------------------------
configure.bat	| Locates available development tools, and creates a configuration script.
make.bat	| Front end to nmake.exe
--------------- | -----------------------------------------------------
All.mak		| Rules for building several versions of a program at once
arm.mak		| Rules for building the Windows 32-bits ARM version of a program (Experimental)
bios.mak	| Rules for building the BIOS 16-bits version of a program
dos.mak		| Rules for building the MS-DOS 16-bits version of a program
ia64.mak	| Rules for building the Windows 64-bits ia64 version of a program
win32.mak	| Rules for building the Windows 32-bits x86 version of a program
win64.mak	| Rules for building the Windows 64-bits amd64 version of a program
win95.mak	| Rules for building a win32 version of a program compatible with Windows 95

- All scripts have a -? option to display a help screen.
- Most make files have a "help" pseudo-target to display a help screen. Ex:

        make.bat -f win32.mak help

Configure.bat must to be run once in each directory.  
This is done automatically by make.bat the first time it runs.  
Then you only need to run configure.bat again if you install new development tools.
