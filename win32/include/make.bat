@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename:	    make.bat						      *
:#                                                                            *
:#  Description:    Front end to nmake, to build 16, 32 and 64 bits targets   *
:#                                                                            *
:#  Notes:	    Designed to have a look-and-feel like that of Unix make.  *
:#                                                                            *
:#		    Options beginning with a - are interpreted locally.	      *
:#		    Ideally they should be compatible with that of Unix make. *
:#		    Options beginning with a / are passed on to nmake. 	      *
:#		    							      *
:#		    Builds the 16-bits MS-DOS version if Visual C++ 1.52 is   *
:#		    installed in its default location in C:\MSVC.	      *
:#		    							      *
:#		    When building real targets, logs all output, including    *
:#		    that on stderr, into a TARGET.log file. This file is      *
:#		    automatically loaded in Notepad in case of error.	      *
:#		    							      *
:#		    When building pseudo-targets, like clean or list_dirs,    *
:#		    no logging is done, and all output goes to stdout/stderr. *
:#		    							      *
:#		    Invokes configure.bat if config.HOSTNAME.bat is absent.   *
:#		    See configure.bat header for a description of its action, *
:#		    and of that of the optional configure.XXX.bat files.      *
:#		    							      *
:#  History:                                                                  *
:#   2003-03-31 JFL Adapted from previous projects			      *
:#   2008-02-13 JFL Adapted to Visual Studio 2005			      *
:#   2009-04-29 JFL Adapted to Visual Studio 2008			      *
:#   2009-10-18 JFL Adapted to Windows 7 64-bits			      *
:#   2010-03-19 JFL Added support for building 64-bits Windows programs.      *
:#   2010-04-07 JFL Added option -f to specify a make file.                   *
:#   2011-02-02 JFL Use a dedicated logging routine.                          *
:#                  Added boost paths.                                        *
:#   2011-05-23 JFL Restructured symmetrically 16/32/64 bits tools discovery. *
:#   2012-10-01 JFL Fixed the location of the 64-bits rc.exe.                 *
:#   2012-10-16 JFL Use a default NMakeFile if one is available               *
:#   2014-03-05 JFL Add support for Visual Studio 10 to 13.                   *
:#   2014-03-06 JFL Changed the log file extension to .log.                   *
:#   2014-03-21 JFL Moved personal settings to an outside config.bat file.    *
:#   2014-03-24 JFL Define MSVCINC16/32/64 = short path of MSVC include files.*
:#   2014-04-04 JFL Fix and document in help support for using nmake options. *
:#   2014-04-21 JFL Split-off Microsoft tools discovery into configure.bat.   *
:#                  Split-off all project-specific settings into ad-hoc	      *
:#                  configure.XXX.bat scripts.                                *
:#   2014-04-22 JFL Fixed the clean target, when 16-tools are missing.        *
:#   2014-06-10 JFL Added the ability to get the default goal name from a     *
:#                  rule in the make file, with the target "goal_name".       *
:#   2014-06-24 JFL Added option -v to display verbose make output.           *
:#   2014-12-03 JFL Added option -q to skip displaying the result message.    *
:#   2015-01-06 JFL Log two bars around config.bat contents.                  *
:#   2015-01-14 JFL Updated PATH to be compatible with more VS versions.      *
:#                  Rename config.bat as config.%COMPUTERNAME%.bat.	      *
:#   2015-01-16 JFL Updated help.                                             *
:#   2015-10-15 JFL Do not run nmake if configure.bat fails.                  *
:#                  Fixed make help.                                          *
:#   2015-10-16 JFL Renamed %POST_MAKE_ACTIONS%, and return nmake's exit code.* 
:#   2015-10-30 JFL Added option -c to force using a different config.        *
:#   2015-11-09 JFL Added my debugging framework.                             *
:#                  Infer the makefile name from the subdir of the targets.   *
:#   2015-10-30 JFL Added option -o to select which nmake to use.             *
:#                  Added option -L to avoid logging the output. This allows  *
:#                  running make.bat recursively.                             *
:#   2016-07-12 JFL Store the log file in %OUTDIR% if defined.		      *
:#   2016-09-02 JFL Fixed the -f option. (Likely broken by 2015-01-09 changes)*
:#   2016-09-30 JFL Merged in the latest batch debugging library updates.     *
:#                  Set the PID variable with the cmd.exe process ID. Useful  *
:#                  for sub-scripts or make files to generate unique IDs.     *
:#   2016-10-03 JFL Display list_programs pseudo target output directly.      *
:#                  Fixed the :nmake routine to avoid creating a log file.    *
:#   2016-10-04 JFL Fixed logging in case an OUTDIR is defined.               *
:#                  Display build messages only if var. MESSAGES is defined.  *
:#   2016-10-05 JFL Added Unix make-compatible option -C to change directory. *
:#                  Added option -u to upgrade the make scripts.              *
:#   2016-10-06 JFL Added option -cde to clean debug environment variables.   *
:#   2016-10-08 JFL Do not show the result when invoked recursively with -L.  *
:#   2016-10-11 JFL Adapted for use in SysToolsLib global C include dir.      *
:#                  Clear a few variables that pollute the (nmake /P) logs.   *
:#   2016-10-13 JFL Added support for target cleanenv.                        *
:#   2016-10-19 JFL Gracefully fail if configuration failed.                  *
:#   2016-10-20 JFL Bug fix: Must now look for %OS%.mak in %STINCLUDE% dir.   *
:#   2016-10-31 JFL Bug fix: Avoid a "file not found" error in recursive makes.
:#                  Define variables NMAKE, BMAKE and BCONF.                  *
:#   2016-11-02 JFL Automatically inherit the log file in recursive makes.    *
:#                  Added option -l, and rewrote option -L.                   *
:#   2016-11-03 JFL Removed variable SHOW_RESULT and added variable MAKEDEPTH.*
:#   2016-11-04 JFL Fixed routine :CleanBuildEnvironment.		      *
:#   2016-11-05 JFL Fixed :Exec bug in XP/64.				      *
:#                  Indent sub-scripts output in debug mode.                  *
:#                  Avoid getting the PID if we have it already.              *
:#   2016-11-07 JFL Updated the 10/19 errorlevel fix to work for DO and EXEC. *
:#   2016-11-08 JFL Change back to using variable MAKE, required by nmake<=7.1*
:#   2016-12-13 JFL Any goal that begins with "list_" and that contains no .  *
:#		    or \ is assumed to be a pseudo-target not to be logged.   *
:#		    Added options -q and -Q to set the output capture manually.
:#   2016-12-16 JFL Updated the batch library framework.		      *
:#   2017-02-22 JFL Look for default make files in the STINCLUDE directory.   *
:#                  Added option -U to undefine variables.		      *
:#   2017-02-24 JFL Renamed the special "goal_name" as "module_name".         *
:#                  Use this module_name earlier when making the log file name.
:#                  Default to the current directory name as the last resort. *
:#   2017-03-01 JFL Added variable IGNORE_NMAKEFILE for dealing with unwanted *
:#                  NMakefile homonyms.                                       *
:#   2017-03-05 JFL Added variable LOGDIR to control where to store the log.  *
:#   2017-03-10 JFL Get nmake.exe path from WIN32_PATH or WIN64_PATH.	      *
:#   2017-10-22 JFL Changed OUTDIR default to the bin subdirectory.           *
:#                  Added %MD_OUTDIR% to allow creating a junction or symlink.*
:#   2017-10-22 JFL Support goals like bin\WIN32\myprog.exe.                  *
:#   2017-10-27 JFL Fixed OUTDIR changes.				      *
:#   2020-01-06 JFL Output a more helpful message if can't find nmake.exe.    *
:#   2020-01-29 JFL In the end, count warnings, and open the log if any found.*
:#   2020-12-16 JFL Added optional LINK_OUDIR to link OUTDIR to another one.  *
:#                                                                            *
:#      © Copyright 2016-2020 Hewlett Packard Enterprise Development LP       *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:#*****************************************************************************

setlocal EnableExtensions EnableDelayedExpansion
set "VERSION=2020-12-16"
set "SCRIPT=%~nx0"				&:# Script name
set "SPATH=%~dp0" & set "SPATH=!SPATH:~0,-1!"	&:# Script path, without the trailing \
set  "ARG0=%~f0"				&:# Script full pathname
set ^"ARGS=%*^"					&:# Argument line

:# Initialize the most commonly used library components.
call :Library.Init

:# Go process command-line arguments
goto Main

:#----------------------------------------------------------------------------#
:#                                                                            #
:#  Function	    Library.Init					      #
:#                                                                            #
:#  Description     Initialize the most commonly used library components      #
:#                                                                            #
:#----------------------------------------------------------------------------#

:Library.Init
:# Initialize this library modules definitions.
:# Each one depends on the preceding ones, so if you need one, you need all the preceding ones as well.
call :Call.Init			&:# Function calls and argument extraction
call :Macro.Init		&:# Inline macros generation
call :Debug.Init		&:# Debug routines
call :Exec.Init			&:# Conditional execution routines

:# FOREACHLINE macro. (Changes the delimiter to none to catch the whole lines.)
set FOREACHLINE=for /f "delims="

:# HOME variable. For analogy with Unix systems.
if not defined HOME set "HOME=%HOMEDRIVE%%HOMEPATH%"

goto :eof

:#----------------------------------------------------------------------------#
:#                                                                            #
:#  Module	    Call						      #
:#                                                                            #
:#  Description     Manage function calls and argument extraction             #
:#                                                                            #
:#  Functions	    PopArg          Pop the first argument from %ARGS% into   #
:#				     %ARG% and %"ARG"%			      #
:#		    PopSimpleArg    Simpler and faster version, incompatible  #
:#                                   with ! or ^ characters in ARG values.    #
:#		    Prep2ExpandVars Prepare variables to return from the      #
:#		 		    local scope (with expansion on or off)    #
:#				    to a parent scope with expansion on.      #
:#		    PrepArgVars     Prepare variables containing pathnames    #
:#				    that will be passed as arguments.	      #
:#                                                                            #
:#  Macros	    %POPARG%        Pop one argument using :PopArg            #
:#                  %POPSARG%       Pop one argument using :PopSimpleArg      #
:#                  %LCALL%         Call a routine in this library, either    #
:#                                   locally, or from an outside script.      #
:#                  %XCALL%         Call an outside script routine, from      #
:#                                   another instance of that outside script. #
:#                  %XCALL@%        Idem, but with all args stored in one var.#
:#                                                                            #
:#  Variables	    %ARG%           The unquoted argument                     #
:#                  %"ARG"%         The actual argument, possibly quoted      #
:#                  %ARGS%	    Remaining command line arguments          #
:#                                                                            #
:#                  %CR%            An ASCII Carrier Return character '\x0D'  #
:#                  %LF%            An ASCII Line Feed character '\x0A'       #
:#                  %BS%            An ASCII Back Space character '\x08'      #
:#                  %FF%            An ASCII Form Feed character '\x0C'       #
:#                                                                            #
:#  Notes 	    PopArg works around the defect of the shift command,      #
:#                  which pops the first argument from the %* list, but does  #
:#                  not remove it from %*.                                    #
:#                  Also works around another defect with tricky characters   #
:#                  like ! or ^ being lost when variable expansion is on.     #
:#                                                                            #
:#                  Important: The performance of this routine is much better #
:#                  when invoked with variable expansion disabled. This is    #
:#                  due to the complex processing done to avoid issues with   #
:#                  tricky characters like ! or ^ when expansion is enabled.  #
:#                  If you're sure that NONE of the arguments contain such    #
:#                  tricky characters, then call :PopSimpleArg.               #
:#                                                                            #
:#                  Uses an inner call to make sure the argument parsing is   #
:#                  done by the actual cmd.exe parser. This guaranties that   #
:#                  arguments are split exactly as shift would have done.     #
:#                                                                            #
:#                  But call itself has a quirk, which requires a convoluted  #
:#                  workaround to process the /? argument.                    #
:#                                                                            #
:#                  Known limitation: After using :PopArg, all consecutive    #
:#                  argument separators in %ARGS% are replaced by one space.  #
:#                  For example: "A==B" becomes "A B"                         #
:#                  This does not change the result of subsequent calls to    #
:#                  :PopArg, but this prevents from using the tail itself as  #
:#                  an argument. => Do not use :PopArg to get :Exec args!     #
:#                                                                            #
:#                  To do: Detect if the last arg has mismatched quotes, and  #
:#                  if it does, append one.                                   #
:#                  Right now such mismatched quotes will cause an error here.#
:#                  Do not work around this error to only pass back the bad   #
:#                  argument, as this will only cause more errors further down#
:#                                                                            #
:#  History                                                                   #
:#   2015-04-03 JFL Bug fix: Quoted args with an & inside failed to be poped. #
:#   2015-07-06 JFL Bug fix: Call quirk prevented inner call from popping /?. #
:#   2016-11-18 JFL Fixed popping arguments containing % characters.          #
:#   2016-11-21 JFL Fixed popping quoted arguments containing &|<> characters.#
:#   2016-11-22 JFL Fixed popping arguments containing ^ characters.          #
:#   2016-11-24 JFL Updated %POPARG% to work with trick characters ! and ^ in #
:#                  delayed expansion mode. The old and faster version is now #
:#		    called %POPSARG%.                                         #
:#		    Added routine :Prep2ExpandVars allowing to pass any       #
:#		    tricky string across call or endlocal barriers.           #
:#   2016-12-01 JFL Added a %FF% Form Feed character variable.                #
:#                                                                            #
:#----------------------------------------------------------------------------#

call :Call.Init
goto Call.end

:Call.Init
if not defined LCALL set "LCALL=call"	&:# Macro to call functions in this library
set "POPARG=%LCALL% :PopArg"
set "POPSARG=%LCALL% :PopSimpleArg"

:# Mechanism for calling subroutines in a second external instance of the top script.
set ^"XCALL=call "!ARG0!" -call^"	&:# This is the top script's (or this lib's if called directly) ARG0
set ^"XCALL@=!XCALL! :CallVar^"		&:# Indirect call, with the label and arguments in a variable

:# Define a LF variable containing a Line Feed ('\x0A')
set LF=^
%# The two blank lines here are necessary. #%
%# The two blank lines here are necessary. #%

:# Define a CR variable containing a Carriage Return ('\x0D')
for /f %%a in ('copy /Z %COMSPEC% nul') do set "CR=%%a"

:# Define a BS variable containing a BackSpace ('\x08')
:# Use prompt to store a  backspace+space+backspace into a DEL variable.
for /F "tokens=1 delims=#" %%a in ('"prompt #$H# & echo on & for %%b in (1) do rem"') do set "DEL=%%a"
:# Then extract the first backspace
set "BS=%DEL:~0,1%"

:# Define a FF variable containing a Form Feed ('\x0C')
for /f %%A in ('cls') do set "FF=%%A"

:# Define variables for problematic characters, that cause parsing issues.
:# Use the ASCII control character name, or the html entity name.
:# Warning: The excl and hat characters need different quoting depending on context.
set  "DEBUG.percnt=%%"	&:# One percent sign
set  "DEBUG.excl=^!"	&:# One exclamation mark
set  "DEBUG.hat=^"	&:# One caret, aka. circumflex accent, or hat sign
set ^"DEBUG.quot=""	&:# One double quote
set  "DEBUG.apos='"	&:# One apostrophe
set  "DEBUG.amp=&"	&:# One ampersand
set  "DEBUG.vert=|"	&:# One vertical bar
set  "DEBUG.gt=>"	&:# One greater than sign
set  "DEBUG.lt=<"	&:# One less than sign
set  "DEBUG.lpar=("	&:# One left parenthesis
set  "DEBUG.rpar=)"	&:# One right parenthesis
set  "DEBUG.lbrack=["	&:# One left bracket
set  "DEBUG.rbrack=]"	&:# One right bracket
set  "DEBUG.sp= "	&:# One space
set  "DEBUG.tab=	"	&:# One tabulation
set  "DEBUG.quest=?"	&:# One question mark
set  "DEBUG.ast=*"	&:# One asterisk
set  "DEBUG.cr=!CR!"	&:# One carrier return
set  "DEBUG.lf=!LF!"	&:# One line feed
set  "DEBUG.bs=!BS!"	&:# One backspace
set  "DEBUG.ff=!FF!"	&:# One form feed
goto :eof

:PopArg
if "!!"=="" goto :PopArg.Eon
:PopArg.Eoff
:PopSimpleArg :# Will corrupt result if expansion is on and ARG contains ^ or ! characters.
:# Gotcha: The call parser first scans its command line for an unquoted /?.
:# If it finds one anywhere on the command line, then it ignores the target label and displays call help.
:# To work around that, we initialize %ARG% and %"ARG"% with an impossible combination of values.
set "ARG=Yes"
set ""ARG"=No"
set "PopArg.ARGS="
if defined ARGS (
  setlocal EnableDelayedExpansion
  for /f "delims=" %%a in ("!ARGS:%%=%%%%!") do endlocal & set ^"PopArg.ARGS=%%a^"
)
call :PopArg.Helper %PopArg.ARGS% >NUL 2>NUL &:# Output redirections ensure the call help is not actually output.
:# Finding that impossible combination now is proof that the call was not executed.
:# In this case, try again with the /? quoted, to prevent the call parser from processing it.
:# Note that we can not systematically do this /? enquoting, else it's "/?" that would break the call.
if "%ARG%"=="Yes" if [%"ARG"%]==[No] call :PopArg.Helper %PopArg.ARGS:/?="/?"%
set "PopArg.ARGS="
goto :eof
:PopArg.Helper
set "ARG=%~1"		&:# Remove quotes from the argument
set ^""ARG"=%1^"	&:# The same with quotes, if any, should we need them
if defined ARG set "ARG=%ARG:^^=^%"
if defined "ARG" set ^""ARG"=%"ARG":^^=^%^"
:# Rebuild the tail of the argument line, as shift does not do it
:# Never quote the set ARGS command, else some complex quoted strings break
set ARGS=%2
:PopArg.GetNext
shift
if defined ARGS set ^"ARGS=%ARGS:^^=^%^"
if [%2]==[] goto :eof
:# Leave quotes in the tail of the argument line
set ARGS=%ARGS% %2
goto :PopArg.GetNext

:PopArg.Eon
setlocal DisableDelayedExpansion
call :PopArg
call :Prep2ExpandVars ARG ^""ARG"^" ARGS
setlocal EnableDelayedExpansion
for /f %%a in ("-!ARG!") do for /f %%b in ("-!"ARG"!") do for /f %%c in ("-!ARGS!") do (
  endlocal
  endlocal
  set "ARG=%%a"
  set "ARG=!ARG:~1!"
  set ^""ARG"=%%b^"
  set ^""ARG"=!"ARG":~1!^"
  set ^"ARGS=%%c^"
  set "ARGS=!ARGS:~1!"
)
goto :eof

:# Prepare variables to return from the local scope (with expansion on or off) to a parent scope with expansion on
:Prep2ExpandVars VAR [VAR ...]
if "!!"=="" goto :Prep2ExpandVars.Eon
:Prep2ExpandVars.Eoff	:# The local scope has expansion off
setlocal EnableDelayedExpansion
set "VALUE=!%~1!"
call :Prep2ExpandVars.Eon VALUE
endlocal & set "%~1=%VALUE%"
if not [%2]==[] shift & goto :Prep2ExpandVars.Eoff
goto :eof

:# Prepare variables, assuming the local scope itself has expansion on
:Prep2ExpandVars.Eon VAR [VAR ...]
if defined %1 (
  for %%e in (sp tab cr lf quot amp vert lt gt hat percnt) do ( :# Encode named character entities
    for %%c in ("!DEBUG.%%e!") do (
      set "%~1=!%~1:%%~c= DEBUG.%%e !"
    )
  )
  call set "%~1=%%%~1:^!= DEBUG.excl %%" 	& rem :# Encode exclamation points
  call set "%~1=%%%~1: =^!%%"			& rem :# Encode final expandable entities
)
if not [%2]==[] shift & goto :Prep2ExpandVars.Eon
goto :eof

:# Prepare variables containing pathnames that will be passed as "arguments"
:PrepArgVars
set "%~1=!%~1:%%=%%%%!"				&:# Escape percent signs
if not [%2]==[] shift & goto :PrepArgVars
goto :eof

:# Indirect call, with the label and arguments in a variable
:CallVar CMDVAR
call !%1:%%=%%%%!
exit /b

:Call.end

:#----------------------------------------------------------------------------#
:#                                                                            #
:#  Module          Macro						      #
:#                                                                            #
:#  Description     Tools for defining inline functions,                      #
:#                  also known as macros by analogy with Unix shells macros   #
:#                                                                            #
:#  Macros          %MACRO%         Define the prolog code of a macro         #
:#                  %/MACRO%        Define the epilog code of a macro         #
:#                                                                            #
:#  Variables       %LF1%           A Line Feed ASCII character '\x0A'        #
:#                  %LF2%           Generates a LF when expanded twice        #
:#                  %LF3%           Generates a LF when expanded 3 times      #
:#                                  Etc...                                    #
:#                  %\n%            Macro command line separator              #
:#                                                                            #
:#  Notes           The principle is to define a variable containing the      #
:#                  complete body of a function, like this:                   #
:#                  set $macro=for %%$ in (1 2) do if %%$==2 ( %\n%           #
:#                    :# Define the body of your macro here %\n%              #
:#                    :# Then return the result to the caller %\n%            #
:#                    for /f "delims=" %%r in ('echo.%!%RETVAL%!%') do ( %\n% #
:#                      endlocal %&% set "RETVAL=%%~r" %\n%                   #
:#                    ) %\n%                                                  #
:#                  ) else setlocal enableDelayedExpansion %&% set ARGS=      #
:#                                                                            #
:#                  It is then invoked just like an external command:         #
:#                  %$macro% ARG1 ARG2 ...                                    #
:#                                                                            #
:#                  The ideas that make all this possible were published on   #
:#                  the dostips.com web site, in multiple messages exchanged  #
:#                  by community experts.                                     #
:#                  By convention on the dostips.com web site, macro names    #
:#                  begin by a $ character; And the %\n% variable ends lines. #
:#                  The other variables are mine.                             #
:#                                                                            #
:#                  The use of a for loop executed twice, is critical for     #
:#                  allowing to place arguments behind the macro.             #
:#                  The first loop executes only the tail line, which defines #
:#                  the arguments; The second loop executes the main body of  #
:#                  the macro, which processes the arguments, and returns the #
:#                  result(s).                                                #
:#                  To improve the readability of macros, replace the code in #
:#                  the first line by %MACRO%, and the code in the last line  #
:#                  by %/MACRO%                                               #
:#                                                                            #
:#                  The use of the Line Feed character as command separator   #
:#                  within macros is a clever trick, that helps debugging,    #
:#                  but it is not necessary for macros to work.               #
:#                  This helps debugging, because this allows to output the   #
:#                  macro definition as a structured string spanning several  #
:#                  lines, looking exactly like a normal function with one    #
:#                  instruction per line.                                     #
:#                  But it would be equally possible to define macros using   #
:#                  the documented & character as command separator.          #
:#                                                                            #
:#                  Limitations:                                              #
:#                  - A macro cannot call another macro.                      #
:#                    (This would require escaping all control characters in  #
:#                     the sub-macro, so that they survive an additional      #
:#                     level of expansion.)                                   #
:#                                                                            #
:#  History                                                                   #
:#   2015-04-15 JFL Initial version, based on dostips.com samples, with       #
:#                  changes so that they work with DelayedExpansion on.       #
:#   2015-11-27 JFL Added a primitive macro debugging capability.             #
:#                                                                            #
:#----------------------------------------------------------------------------#

call :Macro.Init
goto :Macro.End

:Macro.Init
:# LF generator variables, that become an LF after N expansions
:# %LF1% == %LF% ; %LF2% == To expand twice ; %LF3% == To expand 3 times ; Etc
:# Starting with LF2, the right # of ^ doubles on every line,
:# and the left # of ^ is 3 times the right # of ^.
set ^"LF1=^%LF%%LF%"
set ^"LF2=^^^%LF1%%LF1%^%LF1%%LF1%"
set ^"LF3=^^^^^^%LF2%%LF2%^^%LF2%%LF2%"
set ^"LF4=^^^^^^^^^^^^%LF3%%LF3%^^^^%LF3%%LF3%"
set ^"LF5=^^^^^^^^^^^^^^^^^^^^^^^^%LF4%%LF4%^^^^^^^^%LF4%%LF4%"

:# Variables for use in inline macro functions
set ^"\n=%LF3%^^^"	&:# Insert a LF and continue macro on next line
set "^!=^^^^^^^!"	&:# Define a %!%DelayedExpansion%!% variable
set "'^!=^^^!"		&:# Idem, but inside a quoted string
set ">=^^^>"		&:# Insert a redirection character
set "<=^^^<"		&:# Insert a redirection character
set "&=^^^&"		&:# Insert a command separator in a macro
:# Idem, to be expanded twice, for use in macros within macros
set "^!2=^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^!"
set "'^!2=^^^^^^^!"
set "&2=^^^^^^^^^^^^^^^&"

set "MACRO=for %%$ in (1 2) do if %%$==2"				&:# Prolog code of a macro
set "/MACRO=else setlocal enableDelayedExpansion %&% set MACRO.ARGS="	&:# Epilog code of a macro
set "ENDMACRO=endlocal"	&:# Ends the macro local scope started in /MACRO. Necessary before macro exit.

set "ON_MACRO_EXIT=for /f "delims=" %%r in ('echo"	&:# Begin the return variables definitions 
set "/ON_MACRO_EXIT=') do %ENDMACRO% %&% %%r"		&:# End the return variables definitions

:# Primitive macro debugging definitions
:# Macros, usable anywhere, including within other macros, for conditionally displaying debug information
:# Use option -xd to set a > 0 macro debugging level.
:# Usage: %IF_XDLEVEL% N command
:# Runs command if the current macro debugging level is at least N.
:# Ex: %IF_XDLEVEL% 2 set VARIABLE
:# Recommended: Use set, instead of echo, to display variable values. This is sometimes
:# annoying because this displays other unwanted variables. But this is the only way
:# to be sure to display _all_ tricky characters correctly in any expansion mode. 
:# Note: These debugging macros slow down a lot their enclosing macro.
:#       They should be removed from the released code.
set "XDLEVEL=0" &:# 0=No macro debug; 1=medium debug; 2=full debug; 3=Even more debug
set "IF_XDLEVEL=for /f %%' in ('call echo.%%XDLEVEL%%') do if %%' GEQ"

goto :eof
:Macro.end

:#----------------------------------------------------------------------------#
:#                                                                            #
:#  Module	    Debug						      #
:#                                                                            #
:#  Description     A collection of debug routines                            #
:#                                                                            #
:#  Functions       Debug.Init	    Initialize debugging. Call once at first. #
:#                  Debug.Off	    Disable the debugging mode		      #
:#                  Debug.On	    Enable the debugging mode		      #
:#                  Debug.SetLog    Set the log file         		      #
:#                  Debug.Entry	    Log entry into a routine		      #
:#                  Debug.Return    Log exit from a routine		      #
:#                  Verbose.Off	    Disable the verbose mode                  #
:#                  Verbose.On	    Enable the verbose mode		      #
:#                  Echo	    Echo and log strings, indented            #
:#                  EchoVars	    Display the values of a set of variables  #
:#                  EchoArgs	    Display the values of all arguments       #
:#                                                                            #
:#  Macros          %FUNCTION%	    Define and trace the entry in a function. #
:#                  %UPVAR%         Declare a var. to pass back to the caller.#
:#                  %RETURN%        Return from a function and trace it       #
:#                                                                            #
:#                  Always match uses of %FUNCTION% and %RETURN%. That is     #
:#                  never use %RETURN% if there was no %FUNCTION% before it.  #
:#                                                                            #
:#                  :# Example of a factorial routine using this framework    #
:#                  :Fact                                                     #
:#                  %FUNCTION% enableextensions enabledelayedexpansion        #
:#		    %UPVAR% RETVAL					      #
:#                  set N=%1                                                  #
:#                  if .%N%.==.0. (                                           #
:#                    set RETVAL=1                                            #
:#                  ) else (                                                  #
:#                    set /A M=N-1                                            #
:#                    call :Fact !M!                                          #
:#                    set /A RETVAL=N*RETVAL                                  #
:#                  )                                                         #
:#                  %RETURN%					              #
:#                                                                            #
:#                  %ECHO%	    Echo and log a string, indented           #
:#                  %LOG%	    Log a string, indented                    #
:#                  %ECHO.V%	    Idem, but display it in verbose mode only #
:#                  %ECHO.D%	    Idem, but display it in debug mode only   #
:#                                                                            #
:#                  %ECHOVARS%	    Indent, echo and log variables values     #
:#                  %ECHOVARS.V%    Idem, but display them in verb. mode only #
:#                  %ECHOVARS.D%    Idem, but display them in debug mode only #
:#                                                                            #
:#                  %IF_DEBUG%      Execute a command in debug mode only      #
:#                  %IF_VERBOSE%    Execute a command in verbose mode only    #
:#                                                                            #
:#                  %FUNCTION0%	    Weak functions with no local variables.   #
:#                  %RETURN0%       Return from a %FUNCTION0% and trace it    #
:#                  %RETURN#%       Idem, with comments after the return      #
:#                                                                            #
:#  Variables       %>DEBUGOUT%     Debug output redirect. Either "" or ">&2".#
:#                  %LOGFILE%       Log file name. Inherited. Default=""==NUL #
:#                                  Always set using call :Debug.SetLog       #
:#                  %DEBUG%         Debug mode. 0=Off; 1=On. Use functions    #
:#                                  Debug.Off and Debug.On to change it.      #
:#                                  Inherited. Default=0.                     #
:#                  %VERBOSE%       Verbose mode. 0=Off; 1=On. Use functions  #
:#                                  Verbose.Off and Verbose.On to change it.  #
:#                                  Inherited. Default=0.                     #
:#                  %INDENT%        Spaces to put ahead of all debug output.  #
:#                                  Inherited. Default=. (empty string)       #
:#                                                                            #
:#  Notes           All output from these routines is sent to the log file.   #
:#                  In debug mode, the debug output is also sent to stderr.   #
:#                                                                            #
:#                  Traced functions are indented, based on the call depth.   #
:#                  Use %ECHO% to get the same indentation of normal output.  #
:#                                                                            #
:#                  The output format matches the batch language syntax       #
:#                  exactly. This allows copying the debug output directly    #
:#                  into another command window, to check troublesome code.   #
:#                                                                            #
:#  History                                                                   #
:#   2011-11-15 JFL Split Debug.Init from Debug.Off, to improve clarity.      #
:#   2011-12-12 JFL Output debug information to stderr, so that stdout can be #
:#                  used for returning information from the subroutine.       #
:#   2011-12-13 JFL Standardize use of RETVAR/RETVAL, and display it on return.
:#   2012-07-09 JFL Restructured functions to a more "object-like" style.     #
:#                  Added the three flavors of the Echo and EchoVars routines.#
:#   2012-07-19 JFL Added optimizations to improve performance in non-debug   #
:#                  and non-verbose mode. Added routine Debug.SetLog.         #
:#   2012-11-13 JFL Added macro LOG. Fixed setlocal bug in :EchoVars.         #
:#   2013-08-27 JFL Changed %RETURN% to do exit /b. This allows returning     #
:#                  an errorlevel by doing: %RETURN% %ERRORLEVEL%             #
:#   2013-11-12 JFL Added macros %IF_DEBUG% and %IF_VERBOSE%.                 #
:#   2013-12-04 JFL Added variable %>DEBUGOUT% to allow sending debug output  #
:#                  either to stdout or to stderr.                            #
:#   2015-10-29 JFL Added macro %RETURN#% to return with a comment.           #
:#   2015-11-19 JFL %FUNCTION% now automatically generates its name & %* args.#
:#                  (Simplifies usage, but comes at a cost of about a 5% slow #
:#                   down when running in debug mode.)                        #
:#                  Added an %UPVAR% macro allowing to define the list of     #
:#                  variables that need to make it back to the caller.        #
:#                  %RETURN% (Actually the Debug.return routine) now handles  #
:#                  this variable back propagation using the (goto) trick.    #
:#                  This works well, but the performance is poor.             #
:#   2015-11-25 JFL Rewrote the %FUNCTION% and %RETURN% macros to manage      #
:#                  most common cases without calling a subroutine. This      #
:#                  resolves the performance issues of the previous version.  #
:#   2015-11-27 JFL Redesigned the problematic character return mechanism     #
:#                  using a table of predefined generic entities. Includes    #
:#                  support for returning strings with CR & LF.		      #
:#   2015-11-29 JFL Streamlined the macros and added lots of comments.        #
:#                  The FUNCTION macro now runs with expansion enabled, then  #
:#                  does a second setlocal in the end as requested.           #
:#                  The RETURN macro now displays strings in debug mode with  #
:#                  delayed expansion enabled. This fixes issues with CR & LF.#
:#                  Added a backspace entity.                                 #
:#   2015-12-01 JFL Bug fix: %FUNCTION% with no arg did change the exp. mode. #
:#   2016-09-01 JFL Bug fix: %RETURN% incorrectly returned empty variables.   #
:#   2016-11-02 JFL Bug fix: Avoid log file redirection failures in recursive #
:#                  scripts.                                                  #
:#   2016-11-13 JFL Bug fix: Correctly return special characters & | < > ? *  #
:#   2016-11-24 JFL Fixed tracing %FUNCTION% arguments with ^ and % chars.    #
:#                                                                            #
:#----------------------------------------------------------------------------#

call :Debug.Init
goto :Debug.End

:Debug.Init
:# Preliminary checks to catch common problems
if exist echo >&2 echo WARNING: The file "echo" in the current directory will cause problems. Please delete it and retry.
:# Inherited variables from the caller: DEBUG, VERBOSE, INDENT, >DEBUGOUT
:# Initialize other debug variables
set "ECHO=%LCALL% :Echo"
set "ECHOVARS=%LCALL% :EchoVars"
:# The FUNCTION, UPVAR, and RETURN macros should work with delayed expansion on or off
set MACRO.GETEXP=(if "%'!2%%'!2%"=="" (set MACRO.EXP=EnableDelayedExpansion) else set MACRO.EXP=DisableDelayedExpansion)
set UPVAR=call set DEBUG.RETVARS=%%DEBUG.RETVARS%%
set RETURN=call set "DEBUG.ERRORLEVEL=%%ERRORLEVEL%%" %&% %MACRO% ( %\n%
  set DEBUG.EXITCODE=%!%MACRO.ARGS%!%%\n%
  if defined DEBUG.EXITCODE set DEBUG.EXITCODE=%!%DEBUG.EXITCODE: =%!%%\n%
  if not defined DEBUG.EXITCODE set DEBUG.EXITCODE=%!%DEBUG.ERRORLEVEL%!%%\n%
  for %%l in ("%'!%LF%'!%") do ( %# Make it easy to insert line-feeds in any mode #% %\n%
    set "DEBUG.SETARGS=""" %# The initial "" makes sure that for loops below never get an empty arg list #% %\n%
    for %%v in (%!%DEBUG.RETVARS%!%) do ( %\n%
      set "DEBUG.VALUE=%'!%%%v%'!%" %# We must remove problematic characters in that value #% %\n%
      if defined DEBUG.VALUE ( %# Else the following lines will generate phantom characters #% %\n%
	set "DEBUG.VALUE=%'!%DEBUG.VALUE:%%=%%DEBUG.percnt%%%'!%"	%# Encode percent #% %\n%
	for %%e in (sp tab cr lf quot amp vert lt gt) do for %%c in ("%'!%DEBUG.%%e%'!%") do ( %# Encode named character entities #% %\n%
	  set "DEBUG.VALUE=%'!%DEBUG.VALUE:%%~c=%%DEBUG.%%e%%%'!%" %\n%
	) %\n%
	set "DEBUG.VALUE=%'!%DEBUG.VALUE:^^=%%DEBUG.hat%%%'!%"	%# Encode carets #% %\n%
	call set "DEBUG.VALUE=%%DEBUG.VALUE:%!%=^^^^%%" 		%# Encode exclamation points #% %\n%
	set "DEBUG.VALUE=%'!%DEBUG.VALUE:^^^^=%%DEBUG.excl%%%'!%"	%# Encode exclamation points #% %\n%
      ) %\n%
      set DEBUG.SETARGS=%!%DEBUG.SETARGS%!% "%%v=%'!%DEBUG.VALUE%'!%"%\n%
    ) %\n%
    if %!%DEBUG%!%==1 ( %# Build the debug message and display it #% %\n%
      set "DEBUG.MSG=return %'!%DEBUG.EXITCODE%'!%" %\n%
      for /f "delims=" %%v in ("%'!%DEBUG.SETARGS: =%%~l%'!%") do if not %%v=="" ( %# for /f avoids issues with ? and * #% %\n%
	set "DEBUG.MSG=%'!%DEBUG.MSG%'!% %%DEBUG.amp%% set %%v" %!% %\n%
      ) %\n%
      call set "DEBUG.MSG=%'!%DEBUG.MSG:%%=%%DEBUG.excl%%%'!%" %# Change all percent to ! #%  %\n%
      if defined ^^%>%DEBUGOUT ( %# If we use a debugging stream distinct from stdout #% %\n%
	%!%LCALL%!% :Echo.Eval2DebugOut DEBUG.MSG %# Use a helper routine, as delayed redirection does not work #% %\n%
      ) else ( %# Output directly here, which is faster #% %\n%
	for /f "delims=" %%c in ("%'!%INDENT%'!%%'!%DEBUG.MSG%'!%") do echo %%c%# Use a for loop to do a double !variable! expansion #%%\n%
      ) %\n%
      if defined LOGFILE ( %# If we have to send a copy to a log file #% %\n%
	%!%LCALL%!% :Echo.Eval2LogFile DEBUG.MSG %# Use a helper routine, as delayed redirection does not work #% %\n%
      ) %\n%
    ) %\n%
    for %%r in (%!%DEBUG.EXITCODE%!%) do ( %# Carry the return values through the endlocal barriers #% %\n%
      for /f "delims=" %%a in ("%'!%DEBUG.SETARGS%'!%") do ( %\n%
	endlocal %&% endlocal %&% endlocal %# Exit the RETURN and FUNCTION local scopes #% %\n%
	set "DEBUG.SETARGS=%%a" %\n%
	if "%'!%%'!%"=="" ( %# Delayed expansion is ON #% %\n%
	  call set "DEBUG.SETARGS=%'!%DEBUG.SETARGS:%%=%%DEBUG.excl%%%'!%" %# Change all percent to ! #%  %\n%
	  for /f "delims=" %%v in ("%'!%DEBUG.SETARGS: =%%~l%'!%") do if not %%v=="" ( %# for /f avoids issues with ? and * #% %\n%
	    set %%v %# Set each upvar variable in the caller's scope #% %\n%
	  ) %\n%
	) else ( %# Delayed expansion is OFF #% %\n%
	  setlocal EnableDelayedExpansion %\n%
	  for /f "delims=" %%v in ("%'!%DEBUG.SETARGS: =%%~l%'!%") do if %%v=="" ( %# for /f avoids issues with ? and * #% %\n%
	    endlocal %\n%
	  ) else ( %\n%
	    call set %%v %# Set each upvar variable in the caller's scope #% %\n%
	  ) %\n%
	) %\n%
	set "DEBUG.SETARGS=" %\n%
	exit /b %%r %# Return to the caller #% %\n%
      ) %\n%
    ) %\n%
  ) %\n%
) %/MACRO%
:Debug.Init.2
set "LOG=%LCALL% :Echo.Log"
set ">>LOGFILE=>>%LOGFILE%"
if not defined LOGFILE set "LOG=rem" & set ">>LOGFILE=rem"
if .%LOGFILE%.==.NUL. set "LOG=rem" & set ">>LOGFILE=rem"
if .%NOREDIR%.==.1. set "LOG=rem" & set ">>LOGFILE=rem" &:# A parent script is already redirecting output. Trying to do it again here would fail. 
set "ECHO.V=%LCALL% :Echo.Verbose"
set "ECHO.D=%LCALL% :Echo.Debug"
set "ECHOVARS.V=%LCALL% :EchoVars.Verbose"
set "ECHOVARS.D=%LCALL% :EchoVars.Debug"
:# Variables inherited from the caller...
:# Preserve INDENT if it contains just spaces, else clear it.
for /f %%s in ('echo.%INDENT%') do set "INDENT="
:# Preserve the log file name, else by default use NUL.
:# if not defined LOGFILE set "LOGFILE=NUL"
:# VERBOSE mode can only be 0 or 1. Default is 0.
if not .%VERBOSE%.==.1. set "VERBOSE=0"
call :Verbose.%VERBOSE%
:# DEBUG mode can only be 0 or 1. Default is 0.
if not .%DEBUG%.==.1. set "DEBUG=0"
goto :Debug.%DEBUG%

:Debug.SetLog
set "LOGFILE=%~1"
goto :Debug.Init.2

:Debug.Off
:Debug.0
set "DEBUG=0"
set "DEBUG.ENTRY=rem"
set "IF_DEBUG=if .%DEBUG%.==.1."
set "FUNCTION0=rem"
set FUNCTION=%MACRO.GETEXP% %&% %MACRO% ( %\n%
  call set "FUNCTION.NAME=%%0" %\n%
  call set ARGS=%%*%# Do not quote this, to keep string/non string aternance #%%\n%
  if defined ARGS set ARGS=%!%ARGS:^^^^^^^^^^^^^^^^=^^^^^^^^%!%%# ^carets are doubled in quoted strings, halved outside. => Quadruple them if using unquoted ones #%%\n%
  set "DEBUG.RETVARS=" %\n%
  if not defined MACRO.ARGS set "MACRO.ARGS=%'!%MACRO.EXP%'!%" %\n%
  setlocal %!%MACRO.ARGS%!% %\n%
) %/MACRO%
set "RETURN0=exit /b"
set "RETURN#=exit /b & rem"
set "EXEC.ARGS= %EXEC.ARGS%"
set "EXEC.ARGS=%EXEC.ARGS: -d=%"
set "EXEC.ARGS=%EXEC.ARGS:~1%"
:# Optimization to speed things up in non-debug mode
if not defined LOGFILE set "ECHO.D=rem"
if .%LOGFILE%.==.NUL. set "ECHO.D=rem"
if not defined LOGFILE set "ECHOVARS.D=rem"
if .%LOGFILE%.==.NUL. set "ECHOVARS.D=rem"
goto :eof

:Debug.On
:Debug.1
set "DEBUG=1"
set "DEBUG.ENTRY=:Debug.Entry"
set "IF_DEBUG=if .%DEBUG%.==.1."
set "FUNCTION0=call %LCALL% :Debug.Entry0 %%0 %%*"
set FUNCTION=%MACRO.GETEXP% %&% %MACRO% ( %\n%
  call set "FUNCTION.NAME=%%0" %\n%
  call set ARGS=%%*%# Do not quote this, to keep string/non string aternance #%%\n%
  if defined ARGS set ARGS=%!%ARGS:^^^^^^^^^^^^^^^^=^^^^^^^^%!%%# ^carets are doubled in quoted strings, halved outside. => Quadruple them if using unquoted ones #%%\n%
  if %!%DEBUG%!%==1 ( %# Build the debug message and display it #% %\n%
    set DEBUG.MSG=call %!%FUNCTION.NAME%!% %!%ARGS%!%%\n%
    if defined ^^%>%DEBUGOUT ( %# If we use a debugging stream distinct from stdout #% %\n%
      %!%LCALL%!% :Echo.2DebugOut DEBUG.MSG %# Use a helper routine, as delayed redirection does not work #% %\n%
    ) else ( %# Output directly here, which is faster #% %\n%
      echo%!%INDENT%!% %!%DEBUG.MSG%!%%\n%
    ) %\n%
    if defined LOGFILE ( %# If we have to send a copy to a log file #% %\n%
      %!%LCALL%!% :Echo.2LogFile DEBUG.MSG %# Use a helper routine, as delayed redirection does not work #% %\n%
    ) %\n%
    call set "INDENT=%'!%INDENT%'!%  " %\n%
  ) %\n%
  set "DEBUG.RETVARS=" %\n%
  if not defined MACRO.ARGS set "MACRO.ARGS=%'!%MACRO.EXP%'!%" %\n%
  setlocal %!%MACRO.ARGS%!% %\n%
) %/MACRO%
set "RETURN0=call %LCALL% :Debug.Return0 %%ERRORLEVEL%% & exit /b"
:# Macro for displaying comments on the return log line
set RETURN#=call set "RETURN.ERR=%%ERRORLEVEL%%" %&% %MACRO% ( %\n%
  %LCALL% :Debug.Return# %# Redirections can't work in macro. Do it in a function. #% %\n%
  for %%r in (%!%RETURN.ERR%!%) do %ENDMACRO% %&% set "RETURN.ERR=" %&% call set "INDENT=%%INDENT:~2%%" %&% exit /b %%r %\n%
) %/MACRO%
set "EXEC.ARGS= %EXEC.ARGS%"
set "EXEC.ARGS=%EXEC.ARGS: -d=% -d"
set "EXEC.ARGS=%EXEC.ARGS:~1%"
:# Reverse the above optimization
set "ECHO.D=%LCALL% :Echo.Debug"
set "ECHOVARS.D=%LCALL% :EchoVars.Debug"
goto :eof

:Debug.Entry0
setlocal DisableDelayedExpansion
%>DEBUGOUT% echo %INDENT%call %*
if defined LOGFILE %>>LOGFILE% echo %INDENT%call %*
endlocal
set "INDENT=%INDENT%  "
goto :eof

:Debug.Entry
setlocal DisableDelayedExpansion
%>DEBUGOUT% echo %INDENT%call %FUNCTION.NAME% %ARGS%
if defined LOGFILE %>>LOGFILE% echo %INDENT%call %FUNCTION.NAME% %ARGS%
endlocal
set "INDENT=%INDENT%  "
goto :eof

:Debug.Return0 %1=Exit code
%>DEBUGOUT% echo %INDENT%return %1
if defined LOGFILE %>>LOGFILE% echo %INDENT%return %1
set "INDENT=%INDENT:~0,-2%"
exit /b %1

:Debug.Return# :# %RETURN.ERR% %MACRO.ARGS%
setlocal DisableDelayedExpansion
%>DEBUGOUT% echo %INDENT%return %RETURN.ERR% ^&:#%MACRO.ARGS%
if defined LOGFILE %>>LOGFILE% echo %INDENT%return %RETURN.ERR% ^&:#%MACRO.ARGS%
endlocal
goto :eof &:# %RETURN.ERR% will be processed in the %DEBUG#% macro.

:# Routine to set the VERBOSE mode, in response to the -v argument.
:Verbose.Off
:Verbose.0
set "VERBOSE=0"
set "IF_VERBOSE=if .%VERBOSE%.==.1."
set "EXEC.ARGS= %EXEC.ARGS%"
set "EXEC.ARGS=%EXEC.ARGS: -v=%"
set "EXEC.ARGS=%EXEC.ARGS:~1%"
:# Optimization to speed things up in non-verbose mode
if not defined LOGFILE set "ECHO.V=rem"
if .%LOGFILE%.==.NUL. set "ECHO.V=rem"
if not defined LOGFILE set "ECHOVARS.V=rem"
if .%LOGFILE%.==.NUL. set "ECHOVARS.V=rem"
goto :eof

:Verbose.On
:Verbose.1
set "VERBOSE=1"
set "IF_VERBOSE=if .%VERBOSE%.==.1."
set "EXEC.ARGS= %EXEC.ARGS%"
set "EXEC.ARGS=%EXEC.ARGS: -v=% -v"
set "EXEC.ARGS=%EXEC.ARGS:~1%"
:# Reverse the above optimization
set "ECHO.V=%LCALL% :Echo.Verbose"
set "ECHOVARS.V=%LCALL% :EchoVars.Verbose"
goto :eof

:# Echo and log a string, indented at the same level as the debug output.
:Echo
echo.%INDENT%%*
:Echo.Log
if defined LOGFILE %>>LOGFILE% echo.%INDENT%%*
goto :eof

:Echo.Verbose
:Echo.V
%IF_VERBOSE% goto :Echo
goto :Echo.Log

:Echo.Debug
:Echo.D
%IF_DEBUG% %>DEBUGOUT% echo.%INDENT%%*
goto :Echo.Log

:Echo.Eval2DebugOut %1=Name of string, with !variables! that need to be evaluated first
setlocal EnableDelayedExpansion &:# Make sure that !variables! get expanded
set "STRING=!%1!" &:# !variables! not yet expanded; They will be on next line
%>DEBUGOUT% echo.%INDENT%%STRING%
goto :eof

:Echo.2DebugOut	%1=Name of string to output to the DEBUGOUT stream
setlocal EnableDelayedExpansion &:# Make sure that !variables! get expanded
%>DEBUGOUT% echo.%INDENT%!%1!
goto :eof

:Echo.Eval2LogFile %1=Name of string, with variables that need to be evaluated first
setlocal EnableDelayedExpansion &:# Make sure that !variables! get expanded
set "STRING=!%1!" &:# !variables! not yet expanded; They will be on next line
%>>LOGFILE% echo.%INDENT%%STRING%
goto :eof

:Echo.2LogFile %1=Name of string to output to the LOGFILE
setlocal EnableDelayedExpansion &:# Make sure that !variables! get expanded
%>>LOGFILE% echo.%INDENT%!%1!
goto :eof

:# Echo and log variable values, indented at the same level as the debug output.
:EchoVars
setlocal EnableExtensions EnableDelayedExpansion
:EchoVars.loop
if "%~1"=="" endlocal & goto :eof
%>DEBUGOUT% echo %INDENT%set "%~1=!%~1!"
if defined LOGFILE %>>LOGFILE% echo %INDENT%set "%~1=!%~1!"
shift
goto EchoVars.loop

:EchoVars.Verbose
%IF_VERBOSE% (
  call :EchoVars %*
) else ( :# Make sure the variables are logged
  call :EchoVars %* >NUL 2>NUL
)
goto :eof

:EchoVars.Debug
%IF_DEBUG% (
  call :EchoVars %*
) else ( :# Make sure the variables are logged
  call :EchoVars %* >NUL 2>NUL
)
goto :eof

:# Echo a list of arguments.
:EchoArgs
setlocal EnableExtensions DisableDelayedExpansion
set N=0
:EchoArgs.loop
if .%1.==.. endlocal & goto :eof
set /a N=N+1
%>DEBUGOUT% echo %INDENT%set "ARG%N%=%1"
shift
goto EchoArgs.loop

:Debug.End

:#----------------------------------------------------------------------------#
:#                                                                            #
:#  Module	    Exec                                                      #
:#                                                                            #
:#  Description     Run a command, logging its output to the log file.        #
:#                                                                            #
:#                  In VERBOSE mode, display the command line first.          #
:#                  In DEBUG mode, display the command line and the exit code.#
:#                  In NOEXEC mode, display the command line, but don't run it.
:#                                                                            #
:#  Functions       Exec.Init	Initialize Exec routines. Call once at 1st    #
:#                  Exec.Off	Disable execution of commands		      #
:#                  Exec.On	Enable execution of commands		      #
:#                  Do          Always execute a command, logging its output  #
:#                  Exec	Conditionally execute a command, logging it.  #
:#                  Exec.SetErrorLevel	Change the current ERRORLEVEL	      #
:#                                                                            #
:#  Exec Arguments  -l          Log the output to the log file.               #
:#                  -L          Do not send the output to the log file. (Dflt)#
:#                  -t          Tee all output to the log file if there's a   #
:#                              usable tee.exe.                               #
:#                              Known limitation: The exit code is always 0.  #
:#                  -e          Always echo the command.		      #
:#                  -v          Trace the command in verbose mode. (Default)  #
:#                  -V          Do not trace the command in verbose mode.     #
:#                  %*          The command and its arguments                 #
:#                              Quote redirection operators. Ex:              #
:#                              %EXEC% find /I "error" "<"logfile.txt ">"NUL  #
:#                              Note: Quote redirections, NOT file numbers.   #
:#                              Ex: 2">&"1 will work; "2>&1" will NOT work.   #
:#                                                                            #
:#  Macros          %DO%        Always execute a command, logging its output  #
:#                  %EXEC%      Conditionally execute a command, logging it.  #
:#                  %ECHO.X%    Echo a string indented in -X mode, and log it.#
:#                  %ECHO.XD%   Idem in -X or -D modes.                       #
:#                  %ECHO.XVD%  Idem in -X or -V or -D modes.                 #
:#                              Useful to display commands in cases where     #
:#                              %EXEC% can't be used, like in for ('cmd') ... #
:#                  %IF_EXEC%   Execute a command if _not_ in NOEXEC mode     #
:#                  %IF_NOEXEC% Execute a command in NOEXEC mode only         #
:#                  %_DO%       Echo and run a command. No opts. No logging.  #
:#                  %_DO.D%     Idem, echoing it in debug mode only.          #
:#                  %_DO.XVD%   Idem, echoing it in -X or -V or -D modes only.#
:#                  %XEXEC%     Call :Exec from an external scriptlet, such   #
:#                               one in a (for /f in ('commands')) block.     #
:#                  %XEXEC@%    Idem, but with all args stored in one var.    #
:#                                                                            #
:#  Variables       %NOEXEC%	Exec mode. 0=Execute commands; 1=Don't. Use   #
:#                              functions Exec.Off and Exec.On to change it.  #
:#                              Inherited from the caller. Default=On.	      #
:#                  %NOREDIR%   0=Log command output to the log file; 1=Don't #
:#                              Inherited. Default=0.                         #
:#                              Useful in cases where the output must be      #
:#                              shown to the user, and no tee.exe is available.
:#                  %EXEC.ARGS%	Arguments to recursively pass to subcommands  #
:#                              with the same execution options conventions.  #
:#                                                                            #
:#  Notes           %EXEC% can't be used from inside ('command') blocks.      #
:#                  This is because these blocks are executed separately in   #
:#                  a child shell. Use %XEXEC% or %XEXEC@% instead.	      #
:#		    These macros rely on the %XCALL% mechanism for calling    #
:#		    subroutines in a second instance of a script. They depend #
:#		    on the following line being present after the ARGS	      #
:#		    variable definition at the top of your script:	      #
:#		    if '%1'=='-call' !ARGS:~1!& exit /b			      #
:#                                                                            #
:#  History                                                                   #
:#   2010-05-19 JFL Created this routine.                                     #
:#   2012-05-04 JFL Support logging ">" redirections.                         #
:#   2012-07-09 JFL Restructured functions to a more "object-like" style.     #
:#   2012-07-11 JFL Support logging both "<" and ">" redirections.            #
:#   2012-09-18 JFL Added macro %ECHO.X% for cases where %EXEC% can't be used.#
:#   2012-11-13 JFL Support for "|" pipes too.                                #
:#   2013-11-12 JFL Added macro %IF_NOEXEC%.                                  #
:#   2013-12-04 JFL Added option -t to tee the output if possible.            #
:#                  Split %ECHO.X% and %ECHO.XVD%.                            #
:#   2014-05-13 JFL Call tee.exe explicitely, to avoid problems if there's    #
:#                  also a tee.bat script in the path.                        #
:#   2015-03-12 JFL If there are output redirections, then cancel any attempt #
:#		    at redirecting output to the log file.		      #
:#   2016-10-19 JFL Bug fix: Make sure the :Exec initialization preserves the #
:#                  errorlevel that was there on entrance.                    #
:#   2016-11-02 JFL Bug fix: Avoid log file redirection failures in recursive #
:#                  scripts.                                                  #
:#   2016-11-05 JFL Fixed :Exec bug in XP/64.				      #
:#                  Indent sub-scripts output in debug mode.                  #
:#   2016-11-06 JFL Updated the 10/19 errorlevel fix to work for DO and EXEC. #
:#   2016-11-17 JFL Fixed tracing the exit code when caller has exp. disabled.#
:#		    Added option -V to disable tracing exec in verbose mode.  #
:#		    Added macro %ECHO.XD%.                                    #
:#		    Faster and more exact method for separating the %EXEC%    #
:#		    optional arguments from the command line to run. (The old #
:#		    method lost non-white batch argument separators = , ; in  #
:#		    some cases.)                                              #
:#   2016-11-24 JFL Fixed executing commands containing a ^ character.        #
:#		    Added routine :_Do.                                       #
:#   2016-12-13 JFL Rewrote _DO as a pure macro.                              #
:#   2016-12-15 JFL Changed the default to NOT redirecting the output to log. #
:#		                                                              #
:#----------------------------------------------------------------------------#

call :Exec.Init
goto :Exec.End

:# Global variables initialization, to be called first in the main routine
:Exec.Init
set "DO=%LCALL% :Do"
set "EXEC=%LCALL% :Exec"
set "ECHO.X=%LCALL% :Echo.X"
set "ECHO.XD=%LCALL% :Echo.XD"
set "ECHO.XVD=%LCALL% :Echo.XVD"
if not .%NOEXEC%.==.1. set "NOEXEC=0"
:# Quick and simple DO macros, supporting a single command, no redirections, no tricky chars!
set _DO=%MACRO%     ( %LCALL% :Echo     %!%MACRO.ARGS%!% %&% %ON_MACRO_EXIT%%!%MACRO.ARGS%!%%/ON_MACRO_EXIT% ) %/MACRO%
set _DO.D=%MACRO%   ( %LCALL% :Echo.D   %!%MACRO.ARGS%!% %&% %ON_MACRO_EXIT%%!%MACRO.ARGS%!%%/ON_MACRO_EXIT% ) %/MACRO%
set _DO.XD=%MACRO%  ( %LCALL% :Echo.XD  %!%MACRO.ARGS%!% %&% %ON_MACRO_EXIT%%!%MACRO.ARGS%!%%/ON_MACRO_EXIT% ) %/MACRO%
set _DO.XVD=%MACRO% ( %LCALL% :Echo.XVD %!%MACRO.ARGS%!% %&% %ON_MACRO_EXIT%%!%MACRO.ARGS%!%%/ON_MACRO_EXIT% ) %/MACRO%
:# Execute commands from another instance of the main script
set "XEXEC=%XCALL% :Exec"
set "XEXEC@=%XCALL% :Exec.ExecVar"
:# Check if there's a tee.exe program available
:# set "Exec.HaveTee=0"
:# tee.exe --help >NUL 2>NUL
:# if not errorlevel 1 set "Exec.HaveTee=1"
for %%t in (tee.exe) do set "Exec.tee=%%~$PATH:t"
:# Initialize ERRORLEVEL with known values
set "TRUE.EXE=(call,)"	&:# Macro to silently set ERRORLEVEL to 0
set "FALSE.EXE=(call)"	&:# Macro to silently set ERRORLEVEL to 1
goto :NoExec.%NOEXEC%

:Exec.On
:NoExec.0
set "NOEXEC=0"
set "IF_NOEXEC=if .%NOEXEC%.==.1."
set "IF_EXEC=if .%NOEXEC%.==.0."
set "EXEC.ARGS= %EXEC.ARGS%"
set "EXEC.ARGS=%EXEC.ARGS: -X=%"
set "EXEC.ARGS=%EXEC.ARGS:~1%"
goto :eof

:# Routine to set the NOEXEC mode, in response to the -X argument.
:Exec.Off
:NoExec.1
set "NOEXEC=1"
set "IF_NOEXEC=if .%NOEXEC%.==.1."
set "IF_EXEC=if .%NOEXEC%.==.0."
set "EXEC.ARGS= %EXEC.ARGS%"
set "EXEC.ARGS=%EXEC.ARGS: -X=% -X"
set "EXEC.ARGS=%EXEC.ARGS:~1%"
goto :eof

:Echo.XVD
%IF_VERBOSE% goto :Echo
:Echo.XD
%IF_DEBUG% goto :Echo
:Echo.X
%IF_NOEXEC% goto :Echo
goto :Echo.Log

:Exec.SetErrorLevel %1
exit /b %1

:# Execute a command, logging its output.
:# Use for informative commands that should always be run, even in NOEXEC mode. 
:Do
set "Exec.ErrorLevel=%ERRORLEVEL%" &:# Save the initial errorlevel
setlocal EnableExtensions DisableDelayedExpansion &:# Clears the errorlevel
%IF_NOEXEC% call :Exec.On
goto :Exec.Start

:# Execute critical operations that should not be run in NOEXEC mode.
:# Version supporting input and output redirections, and pipes.
:# Redirection operators MUST be surrounded by quotes. Ex: "<" or ">" or ">>"
:Exec
set "Exec.ErrorLevel=%ERRORLEVEL%" &:# Save the initial errorlevel
setlocal EnableExtensions DisableDelayedExpansion &:# Clears the errorlevel
:Exec.Start
set "Exec.NOREDIR=%NOREDIR%"
set "Exec.Redir="				&:# The selected redirection. Default: none
set "Exec.2Redir=>>%LOGFILE%,2>&1"		&:# What to change it to, to enable redirection
if .%NOREDIR%.==.1. set "Exec.2Redir="		&:# Several cases forbid redirection
if not defined LOGFILE set "Exec.2Redir="
if /i .%LOGFILE%.==.NUL. set "Exec.2Redir="
set "Exec.IF_VERBOSE=%IF_VERBOSE%"		&:# Echo the command in verbose mode
:# Record the command-line to execute.
:# Never comment (set Exec.cmd) lines themselves, to avoid appending extra spaces.
:# Use %*, but not %1 ... %9, because %N miss non-white argument separators like = , ;
set ^"Exec.Cmd=%*^" &:# Doubles ^carets within "quoted" strings, and halves those outside
set ^"Exec.Cmd=%Exec.Cmd:^^=^%^" &:# Fix the # of ^carets within "quoted" strings
:# Process optional arguments
goto :Exec.GetArgs
:Exec.NextArg
:# Remove the %EXEC% argument and following spaces from the head of the command line
setlocal EnableDelayedExpansion &:# The next line works because no :exec own argument may contain an '=' or a '!'
for /f "tokens=1* delims= " %%a in ("-!Exec.Cmd:*%1=!") do endlocal & set Exec.Cmd=%%b
shift
:Exec.GetArgs
if "%~1"=="-l" set "Exec.Redir=%Exec.2Redir%" & goto :Exec.NextArg :# Do send the output to the log file
if "%~1"=="-L" set "Exec.Redir=" & goto :Exec.NextArg :# Do not send the output to the log file
if "%~1"=="-t" if defined Exec.2Redir ( :# Tee the output to the log file
  :# Warning: This prevents from getting the command exit code!
  if defined Exec.tee set "Exec.Redir= 2>&1 | %Exec.tee% -a %LOGFILE%"
  goto :Exec.NextArg
)
if "%~1"=="-e" set "Exec.IF_VERBOSE=if 1==1" & goto :Exec.NextArg :# Always echo the command
if "%~1"=="-v" set "Exec.IF_VERBOSE=%IF_VERBOSE%" & goto :Exec.NextArg :# Echo the command in verbose mode
if "%~1"=="-V" set "Exec.IF_VERBOSE=if 0==1" & goto :Exec.NextArg :# Do not echo the command in verbose mode
:# Anything else is part of the command. Prepare to display it and run it.
:# First stage: Split multi-char ops ">>" "2>" "2>>". Make sure to keep ">" signs quoted every time.
:# Do NOT use surrounding quotes for these set commands, else quoted arguments will break.
set Exec.Cmd=%Exec.Cmd:">>"=">"">"%
set Exec.Cmd=%Exec.Cmd:">>&"=">"">""&"%
set Exec.Cmd=%Exec.Cmd:">&"=">""&"%
:# If there are output redirections, then cancel any attempt at redirecting output to the log file.
set "Exec.Cmd1=%Exec.Cmd:"=%" &:# Remove quotes in the command string, to allow quoting the whole string.
if not "%Exec.Cmd1:>=%"=="%Exec.Cmd1%" set "Exec.Redir="
if defined Exec.Redir set "Exec.NOREDIR=1" &:# make sure child scripts do not try to redirect output again 
:# Second stage: Convert quoted redirection operators (Ex: ">") to a usable (Ex: >) and a displayable (Ex: ^>) value.
:# Must be done once for each of the four < > | & operators.
:# Since each operation removes half of ^ escape characters, then insert
:# enough ^ to still protect the previous characters during the subsequent operations.
set Exec.toEcho=%Exec.Cmd:"|"=^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^|%
set Exec.toEcho=%Exec.toEcho:"&"=^^^^^^^^^^^^^^^&%
set Exec.toEcho=%Exec.toEcho:">"=^^^^^^^>%
set Exec.toEcho=%Exec.toEcho:"<"=^^^<%
:# Finally create the usable command, by removing the last level of ^ escapes.
set Exec.Cmd=%Exec.toEcho%
set "Exec.Echo=rem"
%IF_NOEXEC% set "Exec.Echo=echo"
%IF_DEBUG% set "Exec.Echo=echo"
%Exec.IF_VERBOSE% set "Exec.Echo=echo"
%>DEBUGOUT% %Exec.Echo%.%INDENT%%Exec.toEcho%
if defined LOGFILE %>>LOGFILE% echo.%INDENT%%Exec.toEcho%
:# Constraints at this stage:
:# The command exit code must make it through, back to the caller.
:# The local variables must disappear before return.
:# But the new variables created by the command must make it through.
:# This should work whether :Exec is called with delayed expansion on or off.
endlocal & if not .%NOEXEC%.==.1. (
  set "NOREDIR=%Exec.NOREDIR%"
  %IF_DEBUG% set "INDENT=%INDENT%  "
  call :Exec.SetErrorLevel %Exec.ErrorLevel% &:# Restore the errorlevel we had on :Exec entrance
  %Exec.Cmd%%Exec.Redir%
  call set "Exec.ErrorLevel=%%ERRORLEVEL%%"  &:# Save the new errorlevel set by the command executed
  set "NOREDIR=%NOREDIR%" &:# Sets ERRORLEVEL=1 in Windows XP/64
  %IF_DEBUG% set "INDENT=%INDENT%"
  call :Exec.TraceExit
)
exit /b

:Exec.TraceExit
for %%e in (%Exec.ErrorLevel%) do (
  set "Exec.ErrorLevel="
  %IF_DEBUG% %>DEBUGOUT% echo.%INDENT%  exit %%e
  if defined LOGFILE %>>LOGFILE% echo.%INDENT%  exit %%e
  exit /b %%e
)

:Exec.ExecVar CMDVAR
call :Exec !%1:%%=%%%%!
exit /b

:Exec.End

:#----------------------------------------------------------------------------#
:#                        End of the debugging library                        #
:#----------------------------------------------------------------------------#

:is_dir pathname        -- Check if a pathname refers to an existing directory
for /f "tokens=1,2 delims=d" %%a in ("-%~a1") do if not "%%~b"=="" exit /b 0
exit /b 1

:#----------------------------------------------------------------------------#
:#                                                                            #
:#  Function        GetPid                                                    #
:#                                                                            #
:#  Description     Get the PID of the current console                        #
:#                                                                            #
:#  Arguments       %1 = Variable name. Default: PID                          #
:#                                                                            #
:#  Notes 	    Uses a lock file to make sure that two scripts running    #
:#                  within 0.01s of each other do not get the same uid string.#
:#                  The instance that fails retries until it succeeds.        #
:#                  No side effect on the window title.                       #
:#                                                                            #
:#  History                                                                   #
:#   2014-12-23 DB  D.Benham published on dostips.com:                        #
:#                  http://www.dostips.com/forum/viewtopic.php?p=38870#p38870 #
:#   2016-09-11 JFL Adapted to my %FUNCTION%/%UPVAR%/%RETURN% mechanism.      #
:#                                                                            #
:#----------------------------------------------------------------------------#

:GetPID [VARNAME]
%FUNCTION% EnableExtensions DisableDelayedExpansion
set "PIDVAR=%~1" & if not defined PIDVAR set "PIDVAR=PID"
%UPVAR% %PIDVAR%
:GetPID.retry
set "lock=%temp%\%~nx0.%time::=.%.lock"
set "uid=%lock:\=:b%"
set "uid=%uid:,=:c%"
set "uid=%uid:'=:q%"
set "uid=%uid:_=:u%"
setlocal enableDelayedExpansion
set "uid=!uid:%%=:p!"
endlocal & set "uid=%uid%"
2>nul ( 9>"%lock%" (
  for /f "skip=1" %%A in (
    'wmic process where "name='cmd.exe' and CommandLine like '%%<%uid%>%%'" get ParentProcessID'
  ) do for %%B in (%%A) do set "%PIDVAR%=%%B"
  (call )
))||goto :GetPID.retry
del "%lock%" 2>nul
%RETURN%

:#----------------------------------------------------------------------------#
:#                                                                            #
:#  Function        Main                                                      #
:#                                                                            #
:#  Description     Process command line arguments, and main routine body     #
:#                                                                            #
:#  Arguments       %*	    Command line arguments                            #
:#                                                                            #
:#  Notes 	                                                              #
:#                                                                            #
:#  History                                                                   #
:#                                                                            #
:#----------------------------------------------------------------------------#

:GetDefaultMakeFile
%FUNCTION%
%UPVAR% MAKEFILE
set "NMAKEFILE=NMakeFile"
if "%IGNORE_NMAKEFILE%"=="1" set "NMAKEFILE=:" &:# An impossible name
if exist %NMAKEFILE% (
  set "MAKEFILE=NMakeFile"	&rem :# Default make file name
) else if exist All.mak (
  set "MAKEFILE=All.mak"	&rem :# Most powerful one, that can build for all MS OS versions
) else if exist DosWin.mak (
  set "MAKEFILE=DosWin.mak"	&rem :# Previous version of the same, more complex and less powerfull
) else if exist "%SPATH%\All.mak" (
  set "MAKEFILE="%SPATH%\All.mak""  &rem :# Most powerful one, that can build for all MS OS versions
) else (
  set "MAKEFILE=NMakeFile"	&rem :# Revert to the default, even though we know it's not there
)
%RETURN%

:#-----------------------------------------------------------------------------

:GetConfig
if not exist %CONFIG.BAT% if exist configure.bat (
  echo Generating %CONFIG.BAT%
  %DO% call configure.bat
  if errorlevel 1 (
    >&2 echo The configure.bat script failed.
    exit /b
  )
)
:# Load tools settings
if exist %CONFIG.BAT% %DO% call %CONFIG.BAT%
goto :eof

:#-----------------------------------------------------------------------------

:# Get help about this script
:help
if not defined MAKEFILE call :GetDefaultMakeFile
:# Avoid calling :GetConfig here, as we don't want help to have any side effect
:# when users just want to know what this script does.
echo Build targets using nmake.exe
echo.
echo Usage: %SCRIPT% [options] [nmake_options] [macrodefs] [targets] ...
echo.
echo Options:
echo   -?^|-h         This help
echo   -c CONFIG     Use conf. from config.CONFIG.bat. Default: config.%COMPUTERNAME%.bat
echo   -C DIRECTORY  Change the current directory before doing anything
echo   -cde          Clean Debug Environment variables, if the script was interrupted
echo   -d            Run this script in debug mode
echo   -f MAKEFILE   Make file to use. Default: %MAKEFILE%
echo   -H [PROGRAM]  Display nmake.exe help (or that of another MSVC32 program)
echo   -l LOGFILE    Log output into a file. Default: Don't
echo   -L            Disable logging. Default: Use the parent script log file, if any
echo   -O OS         Use nmake for OS. Default: Use that for %MAKEORIGIN%
echo   -q            Run nmake without output capture. (Default for pseudo-targets)
echo   -Q            Capture nmake output into TARGET.log. (Default for real targets)
echo   -u DIRECTORY  Upgrade configure.bat and make.bat scripts in this dir.
echo   -v            Enable verbose output
echo   -V            Display %SCRIPT% version
echo   -X            Display the nmake command to run, but don't run it
echo.
echo Notes:
echo   * Type "make [-f MAKEFILE] help" to get many a MAKEFILE Usage.
echo   * The make output will be logged in a file called LAST_TARGET.log.
echo   * If the first target is in SUBDIR, the default MAKEFILE changes to 
echo     SUBDIR.mak. In this case, all other targets must be in that same SUBDIR.
echo.
exit /b 0

:#-----------------------------------------------------------------------------

:# Get help about a Microsoft development tool
:mstool_help
call :GetConfig :# Get Development tools location
if errorlevel 1 exit /b
:# Update the PATH for running Visual Studio tools, from definitions set in %CONFIG.BAT%
set PATH=!%MAKEORIGIN%_PATH!
set PROG=nmake.exe
%POPARG%
if defined ARG set PROG=%ARG%
%ECHOVARS.D% MAKEORIGIN PATH EXEC
:# %EXEC% %PROG% /? &:# Can't use %EXEC%, nor anything that does a call, as /? breaks the call.
set /A XVD=DEBUG+VERBOSE+NOEXEC
if not %XVD%==0 echo %PROG% /?
%IF_EXEC% %PROG% /?
exit /b

:#-----------------------------------------------------------------------------

:# Run nmake without logging anything, for simple goals like help, clean, etc.
:nmake [nmake.exe options]
%FUNCTION%
if not defined MAKEFILE (
  call :GetDefaultMakeFile
) else (
  %ECHOVARS.D% MAKEFILE
)
call :GetConfig >NUL 2>NUL :# Get Development tools location. Also defines OUTDIR.
if errorlevel 1 %RETURN%
:# Update the PATH for running Visual Studio tools, from definitions set in %CONFIG.BAT%
set PATH=!%MAKEORIGIN%_PATH!
if not defined MAKE for %%m in (nmake.exe) do set "MAKE="%%~$PATH:m"" &:# Includes enclosing quotes
%ECHOVARS.D% CD MESSAGES OUTDIR MAKE MAKEORIGIN PATH INCLUDE %MAKEORIGIN%_CC
if [%MAKE%]==[""] set "MAKE="
if not defined MAKE (
  set "MSG=Can't find nmake.exe"
  if defined MSVC32LONG set ^"MSG=!MSG! in "%MSVC32LONG%"^"
  >&2 echo Error: !MSG!. Try running configure.bat.
  exit /b 1
)
set "NMAKEFLAGS=/NOLOGO /c /s"
%IF_VERBOSE% set "NMAKEFLAGS=/NOLOGO"
:# Clear a few multi-line variables that pollute the (nmake /P) logs
for %%v in (LF1 LF2 LF3 LF4 LF5 MACRO /MACRO ON_MACRO_EXIT) do set "%%v="
%EXEC% -L %MAKE% %NMAKEFLAGS% /F %MAKEFILE% %*
%RETURN%

:#-----------------------------------------------------------------------------

:# Delete everything that can be built by this make system
:distclean
call :nmake distclean
if exist config.* del config.* &:# Delete all instances of %CONFIG.BAT% for all systems
exit /b

:#-----------------------------------------------------------------------------

:# Update the make system scripts in another directory
:update_scripts
set "UPDATE=xcopy /c /d /y"
%IF_NOEXEC% set "UPDATE=%UPDATE% /l"
for %%s in (configure.bat make.bat) do (
  %IF_VERBOSE% echo %UPDATE% %%s %1
  %FOREACHLINE% %%n in ('%UPDATE% %%s %1 2^>NUL ^| findstr ":"') do (
    >con echo Upgrading %~1\%%s
  )
)
exit /b

:#-----------------------------------------------------------------------------

:# Cleanup the environment polluted by this script, in case it is interrupted
:CleanDebugEnvironment
%IF_DEBUG% @echo on
endlocal &:# Return to the shell environment
:# Delete families of variables, which we're pretty sure we all created
for %%f in (
  "DEBUG." "ECHO." "ECHOVARS" "EXEC." "IF_" "LF" "RETURN"
  VSCOMMON VSIDE VSTOOLS VSTUDIO
) do (
  for /f "delims==" %%v in ('set %%f 2^>NUL') do set "%%v="
)
:# Delete individual variables with plain names
for %%v in (
  "ARG" ARG ARG0 BS CONFIG.BAT CR DEBUG DEL DO ECHO EXEC FOREACHLINE FUNCTION FUNCTION0
  LOG MACRO MACRO.GETEXP NOEXEC NOREDIR ON_MACRO_EXIT POPARG
  SCRIPT SPATH UPVAR VERBOSE VERSION XDLEVEL BMAKE BCONF
  CONFIG.BAT GOAL ISDEF LASTGOAL LOGFILE2 MAKEARGS MAKEFILE MAKEGOALS MAKEORIGIN
  NEEDMAKEFILE NMAKE NMAKEFLAGS MAKEGOALS MAKEDEPTH POST_MAKE_ACTIONS SUBDIR UPDATE
) do set "%%v="
:# Delete individual variables with names with characters that need quoting
for %%v in (
  "/MACRO" "/ON_MACRO_EXIT" "&" "&2" ">" ">>LOGFILE" "\n"
) do set "%%~v="
:# Delete variables with tricky characters that need escaping
set "^!="
set "'^!="
set "^!2="
set "'^!2="
:# Restore variables that are changed
set "OS=Windows_NT" &:# May have been changed to target OS values
exit /b

:#-----------------------------------------------------------------------------

:# Cleanup the environment variables normally generated by builds here
:CleanBuildEnvironment
if not defined PID call :GetPID
set "VARLISTFILE=%TMP%\cleanenv-%PID%.lst"
if %MAKEDEPTH%==0 ( :# Do this only in the top make file, in case of recursive makes
  if exist "%VARLISTFILE%" del "%VARLISTFILE%"
)
set "HAD_CONFIG="
if exist "%CONFIG.BAT%" set "HAD_CONFIG=1"
call :nmake cleanenv
if not defined HAD_CONFIG del "%CONFIG.BAT%"
endlocal & ( :# Return to the shell environment. %Variables% already expanded in the (block) below.
  if %MAKEDEPTH%==0 ( :# Do this only in the top make file, in case of recursive makes
    for /f %%v in ('type "%VARLISTFILE%"') do ( :# Note: Do not use %FOREACHLINE%, as the file may contain extra spaces in each line. 
      %ECHO.D% :# Trying "%%v"
      if defined %%v ( :# Remove variable from the parent shell environment
      	:# Do not use %DO%, as we've deleted most debug variables at this stage
	echo set "%%v="
	set "%%v="
      )
    )
  )
)
exit /b

:#-----------------------------------------------------------------------------

:# Execute this make.bat command in another directory
:MakeInDir DIR
:# Get all arguments in the initial command line. Insert a space ahead. 
set ARGS= %*
:# Remove the ' -C DIR' option that brought us here
set ARGS=!ARGS: -C %"ARG"%=!
if "!ARGS:~0,1!"==" " set ARGS=!ARGS:~1!
%DO% pushd %"ARG"%
%ECHO.V% :# cd %CD%
%DO% call %BMAKE% !ARGS!
set "ERROR=%ERRORLEVEL%"
%DO% popd
%ECHO.V% :# cd %CD%
exit /b %ERROR%

:#-----------------------------------------------------------------------------

:# Locate the make file in well known dirs and in the INCLUDE path
:LocateMakefile
if exist "%~1" set "MAKEFILE=%~1" & exit /b 0
if exist "%STINCLUDE%\%~1" set "MAKEFILE=%STINCLUDE%\%~1" & exit /b 0
for %%p in ("%INCLUDE:;=" "%") do if exist "%%p\%~1" set "MAKEFILE=%%p\%~1" & exit /b 0
set "MAKEFILE=%~1" & exit /b 1

:#-----------------------------------------------------------------------------

:main
set "BMAKE="%~f0""	&:# The full pathname to this make.bat script, with quotes
set "BCONF=%BMAKE:make.bat=configure.bat%" &:# The full pathname of configure.bat
set "CONFIG.BAT=config.%COMPUTERNAME%.bat" &:# The output file for this make.bat script
set "POST_MAKE_ACTIONS=" &:# A series of commands to run after the final endlocal after make
if not defined MAKEDEPTH ( :# This is the initial make.bat instance. Show the final result.
  set "MAKEDEPTH=0"
) else ( :# This is a sub-instance. Do not show the intermediate result.
  set /a "MAKEDEPTH+=1"
)
:# Make command line parsing analysis results
set "MAKEFILE="
set "NMAKEFLAGS="	&:# Do not name this MAKEFLAGS, as this confuses nmake
set "MAKEDEFS="
set "MAKEGOALS="
set "MAKEORIGIN=WIN32"
set "LIGHTMAKE="	&:# 1=Simply run nmake and exit. 0=Capture and process its output. Default: Use heuristic to choose
set "DOLOG=1"		&:# By default, do create a log file
set ">DEBUGOUT=>&2"	&:# Send debug output to stderr, so that it does not interfere with subroutines output capture

if not defined STINCLUDE ( :# Try getting the copy in the master environment
  for /f "tokens=3" %%v in ('reg query "HKCU\Environment" /v STINCLUDE 2^>NUL') do set "STINCLUDE=%%v"
)
set "INCLUDE=%STINCLUDE%" &:# Ensure common make files are found by nmake in the %STINCLUDE% directory

:next_arg
if not defined ARGS set "ARG=" & goto go
%POPARG%
if "!ARG!"=="-?" goto help
if "!ARG!"=="/?" goto help
if "!ARG!"=="-c" %POPARG% & set "CONFIG.BAT=config.!ARG!.bat" & goto next_arg
if "!ARG!"=="-C" %POPARG% & call :MakeInDir %* & exit /b
if "!ARG!"=="-cde" goto :CleanDebugEnvironment
if "!ARG!"=="-d" call :Debug.On & call :Verbose.On & goto next_arg
if "!ARG!"=="-f" %POPARG% & call :LocateMakefile !ARG! & goto next_arg
if "!ARG!"=="/f" %POPARG% & call :LocateMakefile !ARG! & goto next_arg
if "!ARG!"=="-h" goto help
if "!ARG!"=="-H" goto mstool_help
if "!ARG!"=="-l" %POPARG% & call :Debug.SetLog !"ARG"! & goto next_arg
if "!ARG!"=="-L" call :Debug.SetLog & set "DOLOG=0" & goto next_arg
if "!ARG!"=="-O" %POPARG% & set "MAKEORIGIN=!ARG!" & goto next_arg
if "!ARG!"=="-q" set "LIGHTMAKE=1" & goto next_arg
if "!ARG!"=="-Q" set "LIGHTMAKE=0" & goto next_arg
if "!ARG!"=="-u" %POPARG% & call :update_scripts "!ARG!" & goto :eof
if "!ARG!"=="-U" %POPARG% & set "%ARG%=" & goto next_arg
if "!ARG!"=="-v" call :Verbose.On & goto next_arg
if "!ARG!"=="-V" (echo %VERSION%) & goto :eof
if "!ARG!"=="-X" call :Exec.Off & goto next_arg
:# Special targets that need special handling
if "!ARG!"=="cleanenv" goto :CleanBuildEnvironment &:# This routine exits directly
if not "LIGHTMAKE"=="0" (
  for %%t in (help clean mostlyclean distclean module_name) do (
    if "!ARG!"=="%%t" (
      call :GetConfig
      call :nmake !ARG! & exit /b
    )
  )
  :# Any goal that begins with "list_" and that contains no . or \ 
  if not "!ARG:list_=!"=="!ARG!" if "!ARG:.=!"=="!ARG!" if "!ARG:\=!"=="!ARG!" call :nmake !ARG! & exit /b
)
:# Anything else is an nmake argument. Fall through
goto go

:go
if "%LIGHTMAKE%"=="1" call :nmake !ARG! !ARGS! & exit /b

call :GetConfig :# Get Development tools location. Also defines OUTDIR.
if errorlevel 1 exit /b

:# Create the output directory if needed
if "%OUTDIR%"=="" (
  set "OUTDIR=bin"
  set "OUTDIR\=bin\"
) else if "%OUTDIR%"=="." (
  set "OUTDIR\="
) else (
  set "OUTDIR\=%OUTDIR%\"
)

:# Allow optionally creating a junction instead of a subdirectory
if defined LINK_OUTDIR ( :# Check if creating junctions works, and if so, prepare to create one
  mklink /j TEST_JUNCTION_CREATION "%LINK_OUTDIR%\%OUTDIR%" >NUL 2>NUL
  if not errorlevel 1 (
    set "MD_OUTDIR0=!MD_OUTDIR!"
    set MD_OUTDIR=mklink /j "%OUTDIR%" "%LINK_OUTDIR%\%OUTDIR%"
    rd TEST_JUNCTION_CREATION
  )
)
if not defined MD_OUTDIR set MD_OUTDIR=md "%OUTDIR%"
%ECHOVARS.D% CD OUTDIR LINK_OUTDIR MD_OUTDIR
if not "%OUTDIR%"=="" call :is_dir "%OUTDIR%" || %MD_OUTDIR% || (
  >&2 echo Error: %MD_OUTDIR%: Cannot create the output directory.
  exit /b 1
)
if defined LINK_OUTDIR ( :# Restore the initial MD_OUTDIR saved above
  set "MD_OUTDIR=!MD_OUTDIR0!"
  set "MD_OUTDIR0="
  set "LINK_OUTDIR=" &rem Make sure it's not inherited
)

:# Select a log file
if "%DOLOG%"=="1" if not defined LOGFILE ( :# Create one, in OUTDIR if defined
  set "LOGFILE=make.log"
  set "LOGFILE=%OUTDIR\%!LOGFILE!"
  if exist "!LOGFILE!" del "!LOGFILE!"
  call :Debug.SetLog "!LOGFILE!"
) &:# else keep using the parent instance log file

:# Start logging by recording the make command.
%LOG% make %*
%LOG%

:# Log settings from %CONFIG.BAT%
%LOG% :# -------------------------- %CONFIG.BAT% --------------------------
%>>LOGFILE% type %CONFIG.BAT%
%LOG% :# ----------------------- End of %CONFIG.BAT% ----------------------
%LOG%

:# Get the shell PID. Useful for scripts or make files that want to generate unique IDs.
if not defined PID call :GetPID

:# Get the remaining args, and parse nmake command line arguments
set "LASTGOAL="
set "MAKEARGS=/x -"
set "NEEDMAKEFILE="
if defined MAKEFILE set "NEEDMAKEFILE=1"
goto :get_ra
:next_ra
%POPARG%
:get_ra
if not defined ARG goto done_ra
if "!ARG:~0,1!"=="/" ( :# This is a switch
  %ECHO.D% :# nmake switch !"ARG"!
  set "NMAKEFLAGS=!NMAKEFLAGS! !ARG!"
) else ( :# Not a switch, so either a variable definition or a goal
  set "ISDEF="
  for /l %%i in (0,1,20) do if not defined ISDEF if "!ARG:~%%i,1!!ARG:~%%i,1!"=="==" set "ISDEF=1"
  if defined ISDEF ( :# It's a variable definition
    %ECHO.D% :# nmake variable !"ARG"!
    set MAKEDEFS=!MAKEDEFS!!SPACE!!"ARG"!
    set MAKEARGS=!MAKEARGS! !"ARG"!
  ) else ( :# It's a goal = a build target
    %ECHO.D% :# nmake goal !"ARG"!
    set "LASTGOAL=!ARG!" &:# Record the last goal, without quotes
    set MAKEGOALS=!MAKEGOALS! !"ARG"!
    if not defined MAKEFILE if exist "%STINCLUDE%\All.mak" (
      set "SUBDIR="
      %ECHO.D% :# Looking for SUBDIR in goal !"ARG"!
      :# If the goal includes a path, then extract the last component of that path.
      if not "!ARG:\=!"=="!ARG!" for %%a in ("!ARG!") do for %%b in ("%%~dpa") do (
      	set "B=%%~b" &:# But this path ends with a trailing \, which we remove below
        for %%c in ("!B:~0,-1!") do set "SUBDIR=%%~nxc"
      )
      if defined SUBDIR if exist "%STINCLUDE%\!SUBDIR!.mak" (
      	set "MAKEFILE=%STINCLUDE%\!SUBDIR!.mak"
	%ECHO.D% :# Setting MAKEFILE !MAKEFILE! after goal !"ARG"!
      )
      if not defined MAKEFILE (
        call :GetDefaultMakeFile
	%ECHO.D% :# Using default MAKEFILE !MAKEFILE!
      )
      set MAKEARGS=!MAKEARGS! /f !MAKEFILE!
      :# Note: I hoped to be able to use multiple make files for multiple goals in distinct subdirs.
      :# This appears to work well in some cases, but not at all in others.
      :# So until this is resolved, I'll prevent the generation of more than 1 make file.
      :# set "MAKEFILE=" &:# Clear the make file, to allow generating others.
      rem
    ) else (
      set "NEEDMAKEFILE=1"
    )
    set MAKEARGS=!MAKEARGS! !"ARG"!
  )
)
if "!NMAKEFLAGS:~0,1!"==" " set "NMAKEFLAGS=!NMAKEFLAGS:~1!"
if "!MAKEARGS:~0,1!"==" " set "MAKEARGS=!MAKEARGS:~1!"
goto next_ra
:done_ra
if not defined MAKEGOALS set "NEEDMAKEFILE=1" &:# We do need a make file to build a default target 
%ECHOVARS.D% CD MAKEFILE NMAKEFLAGS MAKEDEFS MAKEGOALS LASTGOAL NEEDMAKEFILE INCLUDE STINCLUDE PID MAKEORIGIN %MAKEORIGIN%_CC OUTDIR

:# Set a makefile if needed, based on the target subdirectory
:# :# Select a make file if none was specified
:# if not defined MAKEFILE if exist "All.mak" if defined MAKEGOALS (
:#   for %%g in (%MAKEGOALS%) do (
:#     set "GOAL=%%~g"
:#     %ECHO.D% :# Looking for SUBDIR in goal !GOAL!
:#     :# Look for the first \ index
:#     set "SUBDIR="
:#     for /l %%i in (0,1,10) do if not defined SUBDIR if "!GOAL:~%%i,1!"=="\" set "SUBDIR=!GOAL:~0,%%i!"
:#     if defined SUBDIR if not defined MAKEFILE if exist "!SUBDIR!.mak" (
:#       set "MAKEFILE=!SUBDIR!.mak"
:#       %ECHO.D% :# Setting MAKEFILE=!MAKEFILE! after goal !GOAL!
:#     )
:#   )
:# )

:# Use default makes files if still no one was specified
if not defined MAKEFILE call :GetDefaultMakeFile
if defined NEEDMAKEFILE set MAKEARGS=/f !MAKEFILE! !MAKEARGS!

:# Now run nmake
set RESULT=Success
set "MOPATH=!%MAKEORIGIN%_PATH!"
if not defined MAKE for %%m in (nmake.exe) do set "MAKE="%%~$MOPATH:m"" &:# Includes enclosing quotes
set "MOPATH="
:# set CMD=%MAKE% /f %MAKEFILE% /x - %NMAKEFLAGS% %MAKEDEFS% %MAKEGOALS%
set CMD=%MAKE% %NMAKEFLAGS% MESSAGES=1 %MAKEARGS%
setlocal &:# Clear a few multi-line variables that pollute the (nmake /P) logs
for %%v in (LF1 LF2 LF3 LF4 LF5 MACRO /MACRO ON_MACRO_EXIT) do set "%%v="
set "-L=-L"
if defined LOGFILE set "-L=-l"
%EXEC% %-L% %CMD%
endlocal & set ERROR=%ERRORLEVEL%
if not "%ERROR%"=="0" set RESULT=Failure
%IF_NOEXEC% if defined LOGFILE del %LOGFILE% & goto :eof

%LOG%
%LOG% %RESULT%

:# Log the post-make actions we're about to do
:# I'm afraid the %ECHO% methods won't work with multi-line macros, so decomposing the log and echo tasks.
%LOG%
%>>LOGFILE% 2>&1 (if defined POST_MAKE_ACTIONS (set POST_MAKE_ACTIONS) else (echo :# No POST_MAKE_ACTIONS))
if defined POST_MAKE_ACTIONS (%IF_VERBOSE% set POST_MAKE_ACTIONS) else (%IF_DEBUG% echo :# No POST_MAKE_ACTIONS)
%ECHO.D% make.bat: return %ERROR%

:# Rename %LOGFILE% after the goal. The goal is the last argument, without the extension.
if .%LOGFILE%.==.NUL. set "LOGFILE="
if %MAKEDEPTH%==0 if defined LOGFILE ( :# If this is the top-level instance of make.bat, show the final result
  set GOAL=
  for %%F in ("%LASTGOAL%") do set "GOAL=%%~nF"
  :# If there's no goal, maybe the makefile itself has a rule to generate the default log file name
  if not defined GOAL (
    :# Gotcha: nmake always displays on stdout: "Started parsing rules in NMakeFile." So redirect stderr.
    :# Gotcha: The exit code does not survive the 'sub-shell' return. So test it inside, and change the sub-shell output.
    :# Gotcha: Testing variables in the subshell requires three pairs of ^. So use if errorlevel 1.
    for %%m in (%MAKE%) do set "MAKE=%%~sm" &rem :# Get the short name, else the following invokation fails
    %ECHO.D% !MAKE! /nologo /s /c /f %MAKEFILE% /x - module_name
    for /f "delims=" %%g in (
      '!MAKE! /nologo /s /c /f %MAKEFILE% /x - module_name 2^>NUL ^& if errorlevel 1 echo :'
    ) do set "GOAL=%%g"
    :# ":" is an impossible goal name, flagging the absence of target "module_name" in the makefile.
    if "!GOAL!"==":" set "GOAL="
  )
  :# If there's still no goal, use the make file name. (Provided it's not a generic makefile.)
  if not defined GOAL for %%F in (%MAKEFILE%) do if /i "%%~nxF" neq "NMakefile" if /i "%%~nxF" neq "All.mak" set GOAL=%%~nF
  :# If there's still no goal, and the %CD% is something like ...\PROGRAM\SRC, use the word %PROGRAM%
  if not defined GOAL (
    for %%f in ("!CD!") do set "CD0=%%~nxf"
    pushd ..
    for %%f in ("!CD!") do set "CD1=%%~nxf"
    popd
    if /i "!CD0!" equ "src" set "GOAL=!CD1!"
  )
  %ECHOVARS.D% GOAL
  :# If there's still no goal, use the current directory name.
  if not defined GOAL set "GOAL=!CD0!"
  %ECHOVARS.D% GOAL LOGDIR
  :# Rename %LOGFILE% after the %GOAL%, and display the build log.
  set LOGFILE2=!GOAL!.log
  if not defined LOGDIR if defined OUTDIR set "LOGDIR=%OUTDIR%"
  if defined LOGDIR set "LOGFILE2=!LOGDIR!\!LOGFILE2!"
  if not "!LOGFILE2!"=="!LOGFILE!" (
    if exist "!LOGFILE2!" del "!LOGFILE2!"
    move "!LOGFILE!" "!LOGFILE2!" >nul
    call :Debug.SetLog "!LOGFILE2!"
  )
  %ECHOVARS.D% LOGFILE
)

if %MAKEDEPTH%==0 ( :# If this is the top-level instance of make.bat, show the final result
  set "SHOW_LOG=%ERROR%"
  if %ERROR%==0 if defined LOGFILE ( :# Count warnings in the log file
    set "WARNINGS=0"
    for /f "delims=" %%l in ('findstr /i warning "%LOGFILE%"') do set /a "WARNINGS+=1"
    if not !WARNINGS!==0 (
      set "RESULT=!RESULT!, but with !WARNINGS! warnings"
      set "SHOW_LOG=1"
    )
  )
  >con echo.
  >con echo !RESULT!
  if not !SHOW_LOG!==0 start notepad "%LOGFILE%"
)

set "&="
if defined POST_MAKE_ACTIONS set "&=&"
endlocal %&% %POST_MAKE_ACTIONS% & exit /b %ERROR%
