MsvcLibX - A set of MSVC Library eXtensions
===========================================


Introduction
------------

The MsvcLibX library is built upon 30 years of work.  
It exists because the Microsoft Visual C/C++ library is only a subset of the
standard C library, and I wanted to port to DOS and Windows programs using some
of the missing parts.  
Initially Microsoft C compilers for MS-DOS tried to be as compliant as possible
with the standard C library. Then Microsoft rewrote their library for Windows.
But when the WIN32 API became prevalent, Microsoft apparently lost interest,
and stopped porting to MS-DOS and Windows the new C include files and library
routines that got standardized over the years.  
To the point that's it's now impossible to rebuild any recent Posix/Unix/Linux
program using the MSVC library.  

MsvcLibX adds a number of standard include files and library routines that I've
needed over the years. For example the directory access routines defined in
dirent.h, or the symbolic link support in unistd.h.  
It also includes a number of useful routines and macros, either needed 
internally to implement and debug the above, or made necessary by irresolvable 
incompatibilities between Unix and Windows.  

Other major features of the MsvcLibX library are:

* A powerful make system, based on Microsoft nmake files, that allow building
  multiple versions of the same program (Ex: DOS/WIN32/WIN64) with one simple
  command. And in most cases without having to write any dedicated make file.
* Support for UTF-8 sources, allowing to build programs working in any code page.
* Support for NTFS symlinks and junctions.
* Support for DOS, and Windows 95 targets, for targeting old computers.
* Support for bound DOS+Windows programs, for truly universal executables that
  work in *all* versions of DOS and Windows.

An obvious alternative exists for building Windows programs: MinGW.
But I started all this before MinGW even existed. And, surprisingly, even MinGW
still has holes in 2014, where MsvcLibX in significantly more advanced.  
Another alternative is CygWin. This one is more complete, but also much more 
heavyweight. Using programs built with CygWin requires installing CygWin DLLs.
Copying the program .exe from one system to the next is not sufficient. In that
sense, MinGW or MsvcLibX are much better.  

Contrary to MinGW and CygWin, MsvcLibX does not attempt to be complete, and is
unlikely to ever be. But any contribution of improvements is welcome.  
Likewise, any help at contributing unique MsvcLibX features to MinGW is welcome. 

Jean-François Larvoire
2016-09-29


Building the MsvcLibX library
-----------------------------

On a recent Windows PC, with Microsoft Visual C++ compilers installed:

- Select a base work directory. I'll call it %BASEDIR% in the examples below:

        set "HOME=%HOMEDRIVE%%HOMEPATH%"
        set "BASEDIR=%HOME%\Documents\SRC"
        md "%BASEDIR%"
        cd "%BASEDIR%"
  
- Extract the MsvcLibX archive into a work subdirectory. Ex: %BASEDIR%\MsvcLibX\   
  This will put files in several subdirectories: include, src

- Open a cmd window, and run:

        cd "%BASEDIR%\MsvcLibX\src" 
        configure
        make

The configure.bat script will locate your MSVC tools, and generate a config
file. It needs to be run once initially, then again if new MSVC tool versions
are installed.  
The make.bat script should take care of everything, and rebuild normal and debug
versions of the MsvcLibX.lib library for all operating systems your tools support.
In the end, it defines system environment variable MSVCLIBX, necessary for
building programs using the MsvcLibX.lib library.  

Requirements:

- Microsoft Visual C++ compiler and linker for Windows 32 and 64 bits targets.
- Microsoft Windows SDK. (May be installed along with Visual C++)

As of 2017-03-10, I've tested the make.bat script and make files with Visual C++
2005, 2008, 2012, 2015, and 2017, and Windows SDK 5.2, 8.1, and 10.  
Support for older versions is still built-in, but I've not tested it for long.  
Support for newer versions will require minor tweaks in configure.bat.
Note that configure.bat does not depend on MSVC's vcvars.bat. It will attempt
to locate and use the latest usable version of MSVC it finds in Program Files.

Optional:

- Microsoft Visual C++ 1.52 compiler and linker for MS-DOS targets.
  If present in C:\MSVC, make.bat will also generate libraries for DOS.
  Note that generating DOS versions is still more useful than it looks:
  Experience has shown that the severe memory constraints under DOS are a very
  good revelator of memory leaks, and other bad pointer issues. Build your 
  programs for DOS too if you can, and test them in a VM with Windows XP.
  This takes a very small extra build time. And it'll be very much worth the time
  if the Windows version appears to work, but the DOS version does not.
  Visual C++ 1.52 is available on the Visual Studio 2005 CD.
- The Visual C++ 8 compiler and tools from Microsoft Visual Studio 2005.
  This is the last version that allowed building programs for Windows 95/NT4.

The MsvcLibX.lib libraries are generated in subdirectories of src, then copied 
into the %BASEDIR%\MsvcLibX\lib directory, and renamed as shown in this table:

Subdirectory	 | Description				  | Renamed as
---------------- | -------------------------------------- | ----------------
DOS\BIN\T\	 | DOS normal version, tiny memory model  | MsvcLibXdt.lib
DOS\DEBUG\BIN\T\ | DOS debug version, tiny memory model	  | MsvcLibXdtd.lib
DOS\BIN\S\	 | DOS normal version, small memory model | MsvcLibXds.lib
DOS\DEBUG\BIN\S\ | DOS debug version, small memory model  | MsvcLibXdsd.lib
DOS\BIN\L\	 | DOS normal version, large memory model | MsvcLibXdl.lib
DOS\DEBUG\BIN\L\ | DOS debug version, large memory model  | MsvcLibXdld.lib
WIN95\		 | WIN32 normal version for Windows 95	  | MsvcLibXw32.lib
WIN95\DEBUG\	 | WIN32 debug version for Windows 95	  | MsvcLibXw32d.lib
WIN32\		 | WIN32 (X86) normal version		  | MsvcLibXw32.lib
WIN32\DEBUG\	 | WIN32 (X86) debug version		  | MsvcLibXw32d.lib
WIN64\		 | WIN64 (AMD64) normal version		  | MsvcLibXw64.lib
WIN64\DEBUG\	 | WIN64 (AMD64) debug version		  | MsvcLibXw64d.lib


Building programs using the MsvcLibX library
--------------------------------------------

Create a work directory, distinct from the MsvcLibX directories. Ex:

    set "HOME=%HOMEDRIVE%%HOMEPATH%"
    set "BASEDIR=%HOME%\Documents\SRC"
    md "%BASEDIR%\MyTools"
    cd "%BASEDIR%\MyTools"
    :# Define a variable giving the location of the MsvcLibX base directory
    :# (Automatically defined if you built the library already on that same system.)
    set "MSVCLIBX=%BASEDIR%\MsvcLibX"
    :# Get batch files and make files from MsvcLibX sources
    copy "%MSVCLIBX%\src\*.bat"
    copy "%MSVCLIBX%\src\*.mak"
    :# Create the configuration file (To be done just once)
    configure
    :# Compile and link your C or C++ program.
    :# Ex, for the dirc.c sample, to create all dirc.exe versions, type:
    make dirc.exe
    :# If there is any error, the dirc.log file will pop up.
    :# If there's no error, it's possible to check for warnings by reading dirc.log:
    notepad dirc.log
    :# All generated object files, listings, executables, etc, are in
    :# target-OS-specific subdirectories, like for the MsvcLibX builds above.
    :# They're automatically linked with the corresponding (renamed) MsvcLibX*.lib.

make.bat will generate WIN32 (X86) and WIN64 (AMD64) versions by default,
and put them respectively in the WIN32\ and WIN64\ subdirectories.  
It will also generate a DOS version in DOS\ if MSVC 1.52 is installed.  
It will also generate a WIN95 version in WIN95\ if MSVC 8 (aka. 2005) is installed.  
Run `make -?` get a help screen for make.bat.

Note that the configure.bat and make.bat scripts are actually independent of the
rest of the MsvcLibX library. They can be used to easily build any console 
programs for DOS and Windows, without loading the Visual Studio GUI.

An exe.bat script is a front-end to make.bat, saving a few characters to type:  
`exe dirc` <==> `make dirc.exe`


Building Linux versions of the same programs
--------------------------------------------

Install virtual machines with Linux, and give them access to the host's file
system, for example in the /c directory.  
Then execute the following commands, adapting the paths as needed:

    # Go to the work directory
    BASEDIR=/c/Users/YOURNAME/Documents/SRC
    mkdir $BASEDIR/MyTools
    cd $BASEDIR/MyTools
    # Get a bash script to build Linux versions using similar directory outputs 
    cp $BASEDIR/MsvcLibX/src/exe .
    # Make sure the Linux C compiler finds MsvcLibX debug macros, but not other MsvcLibX include files.
    # Important: Do not point C_INCLUDE_PATH at MsvcLibX/include, as this directory
    # contains duplicates for standard include files (Ex: stdio.h), that will fail
    # to compile in Linux.
    mkdir ~/include
    cp $BASEDIR/MsvcLibX/include/debugm.h ~/include
    export C_INCLUDE_PATH=~/include
    # Make sure that variable is defined in all future sessions
    echo "export C_INCLUDE_PATH=~/include" >>~/.bashrc
    # Compile your sample dirc.c program (which uses MsvcLibX debug macros)
    ./exe dirc
    # The output will be in subdirectories such as Linux.i686/ or Linux.x86_64/.
    # (The exact name depends on `echo "$(uname -s).$(uname -m)"` output)


Adding new files to the library
-------------------------------

All new files should have headers and comments similar to the existing ones.

To add a new source file (let's call it newfile.c) into the library:

- Put newfile.c in the %MSVCLIBX%\src directory.
- Add a +newfile.obj entry to the OBJECTS list in %MSVCLIBX%\src\Files.mak
- Also add a rule with newfile.c include files dependencies in src\Files.mak.
- Rebuild the library, and make sure there are no errors nor warnings.

To add a new include file into the library:

- All new include file names should be standard C library include files names!
- Put it in the include or include\sys subdirectory.

To add an include file overriding an homonym one in the MSVC library:

- The trick is to make it include MSVC's one, then define its own extensions.
  As MSVC compilers do not support the #include_next directive, I've implemented
  a mechanism for including MSVC include files using their full pathname.
  As an example, see include/direct.h or include/sys/stat.h.

  
The make file system
--------------------

Microsoft Visual Studio tools contain a build tool called nmake, very similar
to, but slightly incompatible with, Unix make.  
MsvcLibX's make.bat is designed to give a similar feel to Unix make.  
It uses internally a number of make files:

Make file   | Description
----------- | ----------------------------------------------------------------
NMakefile   | Default make file to use, if none is specified.
DOS.mak     | Generic rules for building MS-DOS programs into the DOS subdir.
WIN32.mak   | Generic rules for building 32-bits Windows programs into WIN32.
WIN64.mak   | Generic rules for building 64-bits Windows programs into WIN64.
WIN95.mak   | Generic rules for building Windows 95 programs into WIN95.
All.mak     | Generic rules for building one or more of the above.
Files.mak   | Application-specific source file dependancy rules. Intended to be includable from both Windows' nmakefile and Linux' makefile.

DOS.mak, WIN32.mak, WIN64.mak, and All.mak are pretty stable, and should not
be changed. The only likely case where a change is needed would be to add extra
libraries to link with _all_ programs.  
To add a library for one particular program, it'd be better to add a specific
rule in its own make file, as described below.  
In all cases, see the detailed notes in each make file header.

Specific make files:

To build a target called program.exe, the 5 make files with generic rules
(DOS.mak, WIN32.mak, WIN64.mak, WIN95.mak, and All.mak) first look for a 
specific make file called program.mak. They include it if present. This allows 
defining application-specific rules.  
These rules should use macros defined in the 4 make files to specify the
build environment and parameters. Here's a list of the most useful ones:

Macro     | Description
--------- | ----------------------------------------------------------------
T         | Target OS. One of DOS, WIN32, WIN64, WIN95.
S         | Path of the source files
O         | Path where to put the object files
B         | Path where to put the binary executable files
L         | Path where to put the listings
CFLAGS    | Flags for Microsoft C compiler
INCLUDE   | List of C include file paths, serarated by ';'
LFLAGS    | Flags for Microsoft linker
PROGRAM   | The program base name, infered from the target name.
SOURCES   | The list of sources for the program. Default: PROGRAM.c or .cpp
OBJECTS   | The list of objects to link. Default: PROGRAM.obj

Scripts:

Script                | Description
--------------------- | --------------------------------------------------------
make.bat	      | The main build tool. Invokes nmake.
config.%HOSTNAME%.bat | Defines paths to all tools used by make. Do not edit.
configure.bat 	      | Analyses your system, and generates a config.%HOSTNAME%.bat file.
configure.*.bat       | Define user or task-specific extensions to configure.bat.
exe.bat		      | Front end to make.bat, generating multiple goals.
exe		      | Linux shell script, invoking cc with multiple goals.
src2objs.bat	      | Internal script used by make files to convert SOURCES to OBJECTS

configure.bat with search for configure.*.bat scripts in %windir%, then in %HOME%,
then in the current directory.  
Put configuration scripts with your global preferences in %windir% for example.  
Within each directory, the files are called in the alphabetic order, allowing
to manage predictable dependancies.

Example 1: The 2clip program has no MS-DOS version. To prevent the make system
from attempting to build a DOS version (Only necessary if you DO have MSVC 1.52 
installed), create a 2clip.mak file with this content:

    !IF "$(T)"=="DOS"
    complain:
            @echo>con There's no DOS version of this program.
    
    dirs $(O)\2clip.obj $(B)\2clip.exe: complain
            @rem Do nothing
    !ENDIF

Example 2: Porting to Windows a resize.c program manipulating jpeg images,
and using the libjpeg library. Create a resize.mak file with lines like these:

    INCLUDE=$(INCLUDE);C:\JFL\SRC\Libs\libjpeg;C:\JFL\SRC\Libs\libjpeg\jpeg-8d
    LFLAGS=$(LFLAGS) C:\JFL\SRC\Libs\libjpeg\$(B)\libjpeg.lib

Example 3: Some Windows programs need to include additional resources, defined
in a .rc file. Ex: The update program uses a manifest to control its rights.
Create an update.mak file with directives like this:

    !IF "$(T)"=="WIN32" || "$(T)"=="WIN64"
    SOURCES=update.c update.rc
    LFLAGS=$(LFLAGS) /MANIFEST
    !ENDIF


The debug system
----------------

The MsvcLibX library makes it easy to build two versions of each program:

- A release version, small lean and fast.
- A debug version, with additional code to help developers debug the program.

It follows Microsoft's convention of defining the macro _DEBUG when compiling
a C/C++ source for a debug build.  
But it goes much further, by providing in debugm.h a set of macros to assist
debugging.

The general principle is that by default, the debug version operates exactly
like the release version. (Except possibly for performance)  
Then, if the "debug mode" is enabled, it outputs debug messages on stdout.
A major property of the MsvcLibX debugging output is that is is intended by
function call depth. This makes it considerably easier to read the debug output.

The MsvcLibX library itself is built in debug and release versions.
The make.bat system will link the release version of your program with the
release version of the MsvcLibX library, and likewise for the debug versions.  
To use it, include debugm.h in your main module, and add to your main() routine
a command-line option (-d or --debug maybe?) that calls DEBUG_ON().  
There's also an "extra debug mode", displaying even more details than the
debug mode. It is enabled by calling DEBUG_ON() twice. ==> Invoke with -d -d.

Debug macros:

Macro                       | Description
--------------------------- | ------------------------------------------------
DEBUG_ON()		    | Enable the debug mode.
DEBUG_CODE(...)		    | The code within parentheses is only compiled in the debug version
DEBUG_PRINTF((format, ...)) | Generates a printf instruction in the debug version only, that prints only if debug mode is enabled, with the output indented by call depth.
DEBUG_ENTER((format, ...))  | Like DEBUG_PRINTF, but for use at the beginning of a function. Increases the indent level.
DEBUG_LEAVE((format, ...))  | Like DEBUG_PRINTF, but for use before returning from a function. Decreases the indent level.
				
Note that every use of DEBUG_ENTER must be matched by one DEBUG_LEAVE. So if a
function has several return instructions, every return must be preceded by a
DEBUG_LEAVE.

DEBUG_LEAVE alternatives:  
To further simplify the source, a pair DEBUG_LEAVE()/return can be replaced by
one of the following macros:

Macro                         | Simplified description
----------------------------- | ------------------------------------------------
RETURN_INT(i)		      | DEBUG_LEAVE(("return %d\n", i)); return i;
RETURN_INT_COMMENT(i, (args)) | Idem, plus prints a comment behind the return
RETURN_BOOL(b)		      | DEBUG_LEAVE(("return %s\n", b?"TRUE":"FALSE")); return b;
RETURN_BOOL_COMMENT(b, (...)) | Idem, plus prints a comment behind the return
RETURN_CHAR(c)		      | DEBUG_LEAVE(("return %c\n", c)); return c;
RETURN_STRING(s)	      | DEBUG_LEAVE(("return %s\n", s)); return s;

For all the above, the release version just does return retValue;

Example for a recursive function factorial:

    int fact(int n) {
      DEBUG_ENTER((__FUNCTION__ "(%d);\n", n));
      if (n) n *= fact(n-1); else n = 1;
      RETURN_INT(n);
    }

The debug version, in debug mode, invoked with argument 4, prints:

    fact(4);
      fact(3);
        fact(2);
          fact(1);
            fact(0);
              return 1;
            return 1;
          return 2;
        return 6;
      return 24;


Support for UTF-8 sources
-------------------------

The MsvcLibX library supports writing C programs using 8-bit characters,
with strings encoded as UTF-8.
This makes the sources much more simple and readable that using full-fledged
Unicode, with 16-bits wchar_t or WCHAR and L"strings" or _T("strings").

To enable that UTF-8 support:

1. Set the C or C++ source encoding to UTF-8 with BOM. (BOM = Byte-Order Mark)
Having a BOM is important, as without it some Windows editors will incorrectly
detect the encoding, and then sometimes corrupt the source.
2. Define one of the following constants in the .c source, _before_ including
any .h include files:

        #define _BSD_SOURCE  1	/* Defined by many standard BSD-Unix programs */
        #define _GNU_SOURCE  1	/* Defined by many standard GNU-Unix/Linux programs */
        #define _UTF8_SOURCE 1	/* MsvcLibX-specific */

Note that most modern Linux compilers do expect C sources encoded as UTF-8,
and will silently ignore the UTF-8 BOM if present.

Internally, MsvcLibX extends Microsoft's convention of having two ANSI and Wide
versions of each routine, respectively with an 'A' and a 'W' suffix. Ex:  
FindFirstFile() being an alias to either FindFirstFileA() or FindFirstFileW().  
MsvcLibX uses two additional suffixes: 'U' for the UTF-8 version, and 'M' for
the common MultiByte subroutine used by both the 'A' and 'U' versions. Ex:

Function   | Description
---------- | ---------------------------------------------------------------
readlinkW  | Posix routine readlink - Wide char version
readlinkM  | MultiByte char sub-routine, used by the next two routines.
readlinkA  | Posix routine readlink - ANSI version
readlinkU  | Posix routine readlink - UTF-8 version
readlink   | Posix routine readlink - Alias to either readlinkA or readlinkU

Note that the M version has one additional argument: The code page to use for
converting strings to and from Unicode. In that sense, it's not Posix-compliant.

Gotcha: As of 2014-03-25, most file I/O and enumeration routines have been
restructured this way, but a few have not yet been:  
scandir() and lstat() only support UTF-8 file names, not ANSI names.


Support for Unicode output to the console
---------------------------------------------

Independently of the source ANSI or UTF-8 encoding, all text output to the console
through stdout and stderr is converted to Unicode.
This allows seeing the requested characters whatever the current code page is.

If stdout or stderr are redirected to a pipe or a file, then the text output is converted
to the current code page.  
This behaviour is identical to that of cmd.exe itself. For example:

        dir World_languages\

displays Unicode file names, even with characters not in the current code page.

        dir World_languages\ | more

converts Unicode file names into the current code page, changing missing characters to '?'.

The stdout and stderr encoding can be overridden by setting the global variable codePage,
defined in iconv.h.

Note: The console code page can be read and changed with the CHCP command.  
The most common code pages on US and west-European systems are:

CP    | Description
----- | ----------------------------------------------------------------------------
437   | MS-DOS OEM code page, still used by cmd.exe in US and west-European systems.
1252  | Windows "ANSI" code page, used by most GUI programs, like notepad.exe.
65001 | UTF-8 code page. Allows display any Unicode character.

Important: Changing the code page will only display characters correctly if the console
is using a TrueType font containing these new characters.  
The default "Raster" font supports characters in code page 437 only.  
Unfortunately, none of the available console fonts supports all Unicode characters.  
Many fonts only contains characters for a limited set of European languages, called the
[Windows Glyph List 4](https://en.wikipedia.org/wiki/Windows_Glyph_List_4).  
The default font is usually selected to work well with your language.
If you want to display character from widely different languages, you'll have to try
every font, until you find one that contains all to characters you want.


Support for NTFS symlinks and junctions
---------------------------------------

Posix defines only a single kind of symbolic links, usable for any kind of
targets, whether they're files, directories, or further symbolic links.
The standard C library defines functions for managing symbolic links:

Function   | Description
---------- | ----------------------------------------------------------------
readlink() | Read a link target
symlink()  | Create a link, or change its target
lstat()    | Read the metadata (timestamp, access rights) for the link itself
lchmod()   | Change the link access rights
lchown()   | Change the link owner
realpath() | Generate an absolute path, with all links resolved

Windows defines three types of links:

Type      | Description
--------- | ---------------------------------------------------------------
SYMLINK	  | Symbolic link to a file
SYMLINKD  | Symbolic link to a directory
JUNCTION  | Mount point, often used as a symbolic link to another directory

All three types can be created in Vista and later by the mklink command.
JUNCTIONS can also be created in 2000 and later by Microsoft's SysInternal's
junction command.  
One important difference is that JUNCTIONs can be created with normal user
rights, whereas SYMLINKs and SYMLINKDs require administrator rights in an 
elevated command window.  
A second important difference is that on networks, SYMLINKs and SYMLINKDs are
interpreted on the client side, whereas JUNCTIONs are interpreted on the server
side (despite having their target readable from the client side).

The MsvcLibcX library tries to hide that complexity, and implements the standard
functions as if there were only one kind of link.  
It also provides non-standard functions symlinkd() and junction() to
specifically create SYMLINKDs and JUNCTIONs respectively.

Notes about readlink():  
For SYMLINKs and SYMLINKDs, the case is straightforward.  
For JUNCTIONs, there are two problems:

- The target is stored as an absolute path, even when the JUNCTION was created
  with a relative path. readlink() tries to convert the absolute path to a
  relative path, so that file copying utilities can clone trees with internal
  JUNCTIONs.
- When the JUNCTION is on a network drive, the target refers to the absolute
  path on the server. This may not be accessible from the client through the
  existing mounted shares. Even when this is accessible, it's not always easy
  to map the server path to a valid client path.  
  readlink() uses heuristics which always work for drives shared at the drive
  level. Ex: a C: drive shared as C$, a D: drive shared as D$, etc.  
  The heuristic also very likely works for drives shared at the root or first
  directory level. Ex: C:\ shared as CROOT, or C:\Public shared as PUBLIC.
  As of 2014-03-25, it'll fail in any other case. Ex:  
  C:\Users\YOURNAME shared as YOURHOME, 
  or a C:\Public share mounted through its subdirectory C:\Public\Temp

Notes about symlink():  
symlink() will attempt to create a SYMLINK or a SYMLINKD, depending on the type
of the target. This works well when the target exists. But when it does not
(which is legal), it will create a SYMLINK by default, or a SYMLINKD if the
target ends with a '/' or a '\'. Posix allows, but does not require, providing
a trailing '/', so the link type may possibly be incorrect.  
junction() will have the same issues as readlink() above.

Notes about junction():  
The problem for junction() is to convert a client-side target pathname provided
by the program running on the client, to a server-side pathname, necessary for
junctions to work.  
For full-drive shares (i.e. any share named like D$), this is easy.  
For non-full-drive shares (i.e. anything else), it will assume this is a
first level shared directory on the C: drive. Ex: A link on share PUBLIC
will be targeted at C:\PUBLIC.  
Problem: The junction will not work if the share actually resides anywhere else
on the server. But, surprisingly, there's an easy workaround:  
Simply create manually on the server itself, a junction called C:\PUBLIC,
pointing at the actual directory shared as PUBLIC.  
This way the junctions created from the client side will work correctly, both
on the client and on the server side, after being resolved on the server side
through that second junction.


Support for Windows 95/98
-------------------------

The configure.bat script searches for old versions of the Visual C++ compiler,
which can build WIN32 programs that can run in Windows 95/98/ME/NT4.  
The most recent of these is Visual C++ 8, from Visual Studio 2005.  
The make.bat script can then use rules in win95.mak to builds WIN32 programs
that will run in any version of Windows from Windown 95 to Windows 10.

Note however that Windows 95/98/ME only have a very limited support for Unicode
built in. The rest of this section mentions 95, but applies to 98 & ME as well.
The MsvcLibX library uses a number of Unicode functions not available in default
installations of Windows 95. This includes all file management functions.  
Thus most of our WIN95 executables will not work by default in Windows 95.  
To allow them to work, it is necessary to download from Microsoft a "Microsoft
Layer for Unicode on Windows 95/98/ME Systems" (MSLU for short), and install it
on the Windows 95 system. See the following links for details:  
https://en.wikipedia.org/wiki/Microsoft_Layer_for_Unicode  
https://msdn.microsoft.com/en-us/goglobal/bb688166.aspx  
MSLU installation procedure:  

- Download the MSLU redistributable setup (unicows.exe) from:
  http://go.microsoft.com/fwlink/?LinkId=14851
- Extract unicows.dll from the unicows.exe archive.
- Copy that unicows.dll to the Windows 95 system, into %windir%\System.

Testing WIN95 executables in a Windows 95 VM  
VMWare Player does not have Windows 95 drivers for the HGFS file system.
This prevents accessing the host's files directly as network files, as is
usually done for Windows XP and later versions of Windows.  
It is not possible to use network shares either, as Windows 95 only supports
the SMB 1 protocol, which is actively blocked by Windows Vista and later hosts.
It is possible, but inconvenient, to transit through a web server, and download
the files in the Windows 95 VM using Internet Explorer 4.  
The best solution probably is to transit through a floppy or CD image, and
mount that image in the VM Player. Many tools, including our own HpMkIso.exe
can create CD images. A very convenient shareware called WinImage allows to
create floppy images.  
Another solution is to install a Web server on the host PC, and use Internet
Explorer 4 in Windows 95 to download the executable files into the VM.


Support for DOS, and bound DOS+Windows programs
-----------------------------------------------

- If Visual C++ 1.52 is installed, configure.bat will setup the make system
for building the MS-DOS version of your programs. (Output in the DOS directory)  
- If Visual C++ 8 is installed, configure.bat will setup the make system
for building 32-bits versions of your programs compatible with Windows 95
and all later versions, including 64-bits ones. (Output in the WIN95 directory)  

Both are available as part of the Visual Studio 2005 CD, still available for
download for MSDN subscribers from the MSDN web site.

Win32 programs have an "MS-DOS stub", that runs when the Windows program is
executed under MS-DOS.  
The default stub used by the Win32 linker if a tiny DOS program that displays
an error message such as: "This program can only run in Windows"

When it builds the WIN95 or WIN32 version of a program, and when it has built
the DOS version already, the MsvcLibX make system uses that DOS version as the
stub for the Windows version.  
This allows building executables that work in *all* versions of DOS and Windows!

- When run in MS-DOS, it's the DOS stub of the exe that runs.
- When run in Windows (even 64-bits versions), it's the Win32 part that runs.

Note that the make system will build such bound executables for any WIN32
build made with more recent compilers. But these recent compilers generate
executables that cannot run in old versions of Windows. For example, the
Visual C++ 14 compiler can only target Windows Vista and later systems.
Having an executable that can run in DOS and Windows 7, but not in Windows XP,
is not very useful. Make sure to install the Visual C++ 8 compiler in parallel
with Visual C++ 1.52, and the latest compiler (Visual C++ 14 at the time of 
this writing), to generate truly universal WIN95 builds, that run in 
DOS/95/98/ME/2000/XP/Vista/7/8/10.


History
-------

**1986**

I (Jean-François Larvoire) started writing command-line tools for MS-DOS.
Some were enumerating files, using inline assembly language to make MS-DOS
interrupt 21h system calls: update, dirsize, redo, backnum, which, dirc...  
To make it simple I factored these out in subroutines srch1st and srchnext,
that I manually copied from one program to the next.  
Things got a bit tricky to support recursion, which was not straightforward
in MS-DOS.


**1992**

We got an OS/2 server, and I started porting the above programs to OS/2.
MS-DOS was still important, so I used conditional compilation to support
both operating systems with the same source.  
I wrote a second version of srch1st and srchnext for OS/2, and had to
include a new routine srchdone due to OS/2 API constraints.  
dirc was the first program I ported, then I slowly duplicated the code into
the other programs. Synchronizing bug fixes became more difficult.  
A nice trick was the OS/2 supported dual-mode EXEs, with both the DOS and
OS/2 version bound in the same .exe file.


**1995**

We ditched the OS/2 server, and got a new one running Windows NT.  
Again, I created a third conditionally compiled version of srch1st/srchnext/
srchdone for WIN32, for use in dirc.  
Manually back porting the updates and bug fixes to all programs became
really painful, and took several years.  
Like OS/2, Windows supported dual-mode EXEs, and I updated my make files
to include both the DOS and Windows version in the same file.


**2005**

I started working on Linux projects. Despite the bonanza of command-line
tools available, I couldn't find one equivalent to dirc.  
So, yet another time, I created a fourth conditionally compiled version of
srch1st/srchnext/srchdone for Linux, and obtained a Linux version of dirc.
I also ported it and a couple of other programs to Tru64, which was the
first 64-bits OS I worked with. This exposed a few unsuspected bugs.  
The thing became so complex than porting the changes and updates to the
other programs was a nightmare. I did it for a few of them, but never
got the time to port them all.


**2010**

Building WIN64 versions was relatively easier due to the Tru64 precedent,
but added yet another set of conditional compilations.   
The porting nightmare worsened.


**2012**

I tried porting some unrelated Linux programs to Windows, and hit a wall:
Many include files and standard C library routines were missing in MSVC.
I then tried using MinGW, but it too was missing many required features!
I considered contributing updates to MinGW, but that project was too
complex and ill-documented at that time, and I eventually gave up.  
Instead, I started writing my own "libc-ext" library of include files and
routines to supplement MSVC.  
Ex: stdint.h, inttypes.h, fnmatch.h/fnmatch.c, ...  
Also it became obvious that multiplying conditional compilations and 
duplicating directory access code everywhere was wrong. Instead I had to
write standard dirent.h/dirent.c routines for DOS/OS2/Windows, and
rewrite all my programs around these standard routines.  
I did it for DOS and Windows versions, and left stubs of the OS/2 versions,
for the unlikely case where somebody wants to revive them.

**2013**

The restructuration was over for dirc, dirsize, which, backnum, update.
The library got its final name, "MsvcLibX", to better match it purpose:  
An extension of the Microsoft Visual C library, not of the standard libc.  
As the library grew, debugging it became more difficult, and I decided to
use my debugm.h within the library: This file contains a set of debugging
macros I had developed for other programs.  
I started thinking about sorely needed improvements:

- There was no support for Windows symlinks and junctions.
- The Windows versions of the programs output ANSI file names into a
  cmd.exe window using the OEM character set. This caused all files with
  French names to be shown as garbage.
- Worse still, I had a few files and directories with non-ANSI names
  (In Russian and Chinese), that caused the programs to fail!

The Linux version of the program did not have these issues, as all recent
versions of Linux use the UTF-8 encoding, and this works transparently
even for old programs using 8-bits characters like mine.


**2014**

I got a bit more time, and started working on a redesign of MsvcLibX's
dirent.c/dirent.h, and added lstat.c, symlink.c, readlink.c, etc, to
support symlinks, junctions, and UTF-8 file names.  
This proved to be much more work than I initially thought, but the result
was worth the effort: dirc, update got easily adapted, with remarkably
few changes to their source. Adding recursion to update was easy.  
I'm now able to use update to backup all files on my system, including
symbolic links, and junctions used as poor man's links. And the programs
now display names correctly in any code page with the right font.  
A seventh program, truename, joined the collection using dirent.h routines.
Later in the year, I made significant other changes:

- Split make.bat into a generic make.bat/configure.bat pair of scripts.
  configure.bat needs to be run once, plus everytime the environment
  changes. (Such as if a new MSVC version is installed.)
  Project-specific definitions are moved to new files, such as our
  configure.MsvcLibX.bat.
- Added a general mechanism for defining extensions to existing MSVC
  include files. This allowed defining our own homonym include files,
  containing just the missing definitions. Which in turn allowed to move
  many such definitions, that were initially stored in unistd.h for lack
  of an alternative, to their standard location.
- Updated all file functions to support WIN32 pathnames > 260 characters.
  (The basic WIN32 functions that I used until then had that limitation;
  I'm now using extended functions supporting path lengths up to 64K.)

All tools like dirc, dirsize, which, backnum, update, truename, redo
benefit from that last change.


**2015**

A major improvement was the addition of support for new operating systems
and processor targets. This required redesigning several things:  
The OS-specific definition macros have been renamed to more generic names.
The DosWin.mak file has been replaced by a more generic All.mak,
supporting the old dos.mak, win32.mak, and win64.mak, plus the new...

- bios.mak     Allows building 16-bits programs that can run in BIOS option
               ROMs. These programs are based on the BiosLib library,
               documented separately.
- win95.mak    Allows building win32 programs that run in Windows 95.
- ia64.mak     Allows building 64-bits programs that run in IA64 versions
	       of Windows. Not tested.
- arm.mak      Allows building win32 programs for Windows RT.
	       Tests failed so far, due to a missing "ARM Desktop SDK".

All.mak also skips building versions for which no tools are available;
This prevents problems in the normal case when people only have the latest
version of MSVC. In this case, it just builds the WIN32 and WIN64 versions.
The support for Windows 95 required fixing numerous issues with old
Microsoft tools and Windows SDKs.  
Conversely, the support for Visual Studio 15 also required fixing lots of
issues, as the MSVC library has been completely redesigned, and split into
two parts:

- A compiler-specific part, part of the compiler tree as before.
- A generic part, called Universal C runtime (UCRT) that's now part of
  the windows SDK.

And of course there were also issues with MsvcLibX recent improvements,
like Unicode support and symlink support, which had never been tested,
and of course did not work, in Windows 95 and XP.


**2016**

Many significant changes and improvements this year:

- Changed the UTF-8 C source support to require a UTF-8 BOM.  
  This is to prevent problems with Windows tools that tend to corrupt files
  without a UTF-8 BOM, but with other UTF-8 characters.  
  This required changing all C/C++ compilation rules to first remove the
  UTF-8 BOM, as MS compilers do not react correctly when they find one.
- Added a windows.h extension.
  Just like Standard C library APIs, windows 8-bit APIs can be overriden to
  refer to custom routines that support UTF-8 strings.  
  The MsvcLibX library was beginning to use a significant number of these
  custom routines internally. They're now accessible directly to outside
  UTF-8 programs targeting only Windows.
- Finished implementing support for the OUTDIR variable. (Started in 2015.)
  All make files now optionally create output files in an %OUTDIR% directory.
  This is intended for testing builds in VMs, with shared sources on the
  host, but the output locally in the VM, avoiding to overwrite the main
  version on the host. IF %OUTDIR% is not defined, the default output 
  still goes below the source directory as before.
- Added the ability to build projects recursively.  
  Applied that to MsvcLibX, which can now be built from its main directory.  
  And MsvcLibX itself can be built as a component of the SysToolsLib library.


**2017**

- Added support for Visual Studio 2017.  
  This one has a very different directory structure from its predecessors.
  This required a redesign of the detection routine in configure.bat.
- One major change: All text written to stdout and stderr and that goes
  to the console is written in UTF-16. This ensures that all characters are
  displayed correctly, whatever the current code page.  
  When stdout and stderr are redirected to a pipe or a file, the text is
  converted to the console code page as before. This is the same behavior
  as that of cmd.exe itself.  
  That should be transparent to all apps, with just a rebuild necessary.
- One simplification: I've fixed the long-broken src2objs.bat. It's thus
  possible to define lists of C sources instead of lists of objects.
  Sources are better, because they're OS-independant, whereas object
  names differ in Windows and Linux.
- Another simplification: It's possible to make recursive builds by just
  defining a DIRS variable in a Files.mak file.
