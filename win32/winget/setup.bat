@echo off
:##############################################################################
:#                                                                            #
:#  Filename        setup.bat			                              #
:#                                                                            #
:#  Description     Install or upgrade ag.exe				      #
:#                                                                            #
:#  Notes 	                                                              #
:#                                                                            #
:#  Authors     JFL "Jean-François Larvoire" <jf.larvoire@free.fr>            #
:#                                                                            #
:#  History                                                                   #
:#   2020-06-03 JFL Created this script.                                      #
:#   2020-07-01 JFL Added support for versions with 4 parts: X.Y.Z.T          #
:#   2021-07-08 JFL Store Ag uninstallation information in the registry.      #
:#                  Added option -u for testing this uninstallation.          #
:#                  Allow testing the script from within the source tree.     #
:#   2021-07-09 JFL Improved the debugging framework.                         #
:#                  Use regedit.exe instead of multiple calls to reg.exe.     #
:#                                                                            #
:##############################################################################

setlocal EnableExtensions DisableDelayedExpansion &:# Make sure ! characters are preserved
set "VERSION=2021-07-09"
set "SCRIPT=%~nx0"		&:# Script name
set "SNAME=%~n0"		&:# Script name, without its extension
set "SPATH=%~dp0"		&:# Script path
set "SPATH=%SPATH:~0,-1%"	&:# Script path, without the trailing \
set "SFULL=%~f0"		&:# Script full pathname
set ^"ARG0=%0^"			&:# Script invokation name
set ^"ARGS=%*^"			&:# Argument line
setlocal EnableExtensions EnableDelayedExpansion &:# Use the ! expansion now on

call :Debug.Init
goto :Main

:#----------------------------------------------------------------------------#
:# Simple debug framework

:Debug.Init
set FOREACHLINE=for /f "delims="

:# Define a LF variable containing a Line Feed ('\x0A')
set LF=^
%# The two blank lines here are necessary. #%
%# The two blank lines here are necessary. #%

:# Define a CR variable containing a Carriage Return ('\x0D')
for /f %%a in ('copy /Z %COMSPEC% nul') do set "CR=%%a"

:# Define variables for problematic characters, that cause parsing issues.
:# Use the ASCII control character name, or the html entity name.
:# Warning: The excl and hat characters need different quoting depending on context.
set  "#percnt=%%"	&:# One percent sign
set  "#excl=^!"		&:# One exclamation mark
set  "#hat=^"		&:# One caret, aka. circumflex accent, or hat sign
set ^"#quot=""		&:# One double quote
set  "#apos='"		&:# One apostrophe
set  "#amp=&"		&:# One ampersand
set  "#vert=|"		&:# One vertical bar
set  "#gt=>"		&:# One greater than sign
set  "#lt=<"		&:# One less than sign
set  "#sp= "		&:# One space
set  "#tab=	"	&:# One tabulation
set  "#cr=!CR!"		&:# One carrier return
set  "#lf=!LF!"		&:# One line feed

set "INDENT="
set "+INDENT=call :IncIndent"
set "-INDENT=call :DecIndent"
set "ECHO=call :Echo"
set "COMMENT=%ECHO% :#"
set "ERROR=>&2 %ECHO% Error:"
set "LOG=call :Log"
set "LOGVARS=call :LogVars"
set "LOGFILE=NUL"
set "RETURN=call :Return"
set "EXEC=call :Exec"
set "PREPVAR=call :Prep2ExpandVar"

:Debug.off
set "ECHO.D=call :Log"
set "ECHOVARS.D=call :LogVars"
set "ENTER=call :Log call %%0 %%* & %+INDENT%"
exit /b

:Debug.on
set "ECHO.D=call :Echo.d"
set "ECHOVARS.D=call :EchoVars.d"
set "ENTER=call :Echo call %%0 %%* & %+INDENT%"
exit /b

:# Change the debug output indentation
:IncIndent
set "INDENT=%INDENT%  "
exit /b
:DecIndent
if defined INDENT set "INDENT=%INDENT:~2%"
exit /b

:# Trace the return from a subroutine, and do the actual return
:Return
:# gotcha: setlocal sometimes clears %ERRORLEVEL%, so the reading must be on same line
setlocal & set "ERR=%~1" & if not defined ERR set "ERR=%ERRORLEVEL%"
%ECHO.D% exit /b %ERR%
endlocal & 2>NUL (goto) & %-INDENT% & exit /b %ERR% &:# Pop one call stack, then return to the upper level

:# Display and log the definition of a multiple variables
:EchoVars.d
for %%v in (%*) do echo %INDENT%set "%%v=!%%v!"
:LogVars
for %%v in (%*) do %LOG% set "%%v=!%%v!"
exit /b

:# Display and log a message
:Echo
setlocal EnableDelayedExpansion
set ^"MSG=%*^"
echo.!MSG!
goto :Log.Msg

:# Display and log a debug message
:Echo.d
setlocal EnableDelayedExpansion
set ^"MSG=%*^"
echo.%INDENT%!MSG!
goto :Log.Msg

:# Log a message, with a timestamp, into the log file
:Log
setlocal EnableDelayedExpansion
set ^"MSG=%*^"
:Log.Msg
call :Now
set "NOW=%YEAR%-%MONTH%-%DAY% %HOUR%:%MINUTE%:%SECOND%.%MS%"
>>"%LOGFILE%" echo %NOW% %INDENT%!MSG!
endlocal & exit /b

:# Display and log a batch command, and run it.
:Exec
%ECHO% %*
%*
exit /b

:# Prepare one variable, in a local scope with !expansion! either on or off, for %expansion% in another scope with !expansion! on
:Prep2ExpandVar     INVAR [OUTVAR]
:# TO DO: Test and manage the case when the local expansion is off
:# Prepare one variable, in a local scope with !expansion! on, for %expansion% in another scope with !expansion! on
:Prep2ExpandVar.Eon INVAR [OUTVAR]
if not [%2]==[] (
  set "%~2="
  if defined %1 set "%~2=!%~1!"
  shift
) &:# Fallthrough into :Prep2ExpandVars.Eon
:# Prepare variables, in a local scope with !expansion! on, for %expansion% in another scope with !expansion! on
:Prep2ExpandVars.Eon VAR [VAR ...]
if defined %1 (
  for %%e in (sp tab cr lf quot amp vert lt gt hat percnt) do ( :# Encode named character entities
    for %%c in ("!#%%e!") do (
      set "%~1=!%~1:%%~c= #%%e !"
    )
  )
  call set "%~1=%%%~1:^!= #excl %%" 	& rem :# Encode exclamation points
  call set "%~1=%%%~1: =^!%%"		& rem :# Encode final expandable entities
)
if not [%2]==[] shift & goto :Prep2ExpandVars.Eon
exit /b

:#----------------------------------------------------------------------------#

:# Get the current date/time
:# Implementation based on wmic. Not available in early XP versions?
:Now
setlocal EnableExtensions
for /f %%i in ('WMIC OS GET LocalDateTime /value') do for /f "tokens=2 delims==" %%j in ("%%i") do set "dt=%%j"
endlocal & set "YEAR=%dt:~,4%" & set "MONTH=%dt:~4,2%" & set "DAY=%dt:~6,2%" & set "HOUR=%dt:~8,2%" & set "MINUTE=%dt:~10,2%" & set "SECOND=%dt:~12,2%" & set "MS=%dt:~15,3%"
exit /b

:#----------------------------------------------------------------------------#

:# Quote file pathnames that require it.
:condquote	 %1=Input variable. %2=Opt. output variable.
setlocal EnableExtensions Disabledelayedexpansion
set "RETVAR=%~2"
if not defined RETVAR set "RETVAR=%~1" &:# By default, change the input variable itself
call set "P=%%%~1%%"
:# If the value is empty, don't go any further.
if not defined P set "P=""" & goto :condquote_ret
:# Remove double quotes inside P. (Fails if P is empty)
set "P=%P:"=%"
:# If the value is empty, don't go any further.
if not defined P set "P=""" & goto :condquote_ret
:# Look for any special character that needs quoting
:# Added "@" that needs quoting ahead of commands.
:# Added "|&<>" that are not valid in file names, but that do need quoting if used in an argument string.
echo."%P%"|findstr /C:" " /C:"&" /C:"(" /C:")" /C:"[" /C:"]" /C:"{" /C:"}" /C:"^^" /C:"=" /C:";" /C:"!" /C:"'" /C:"+" /C:"," /C:"`" /C:"~" /C:"@" /C:"|" /C:"&" /C:"<" /C:">" >NUL
if not errorlevel 1 set P="%P%"
:condquote_ret
:# Contrary to the general rule, do NOT enclose the set commands below in "quotes",
:# because this interferes with the quoting already added above. This would
:# fail if the quoted string contained an & character.
:# But because of this, do not leave any space around & separators.
endlocal&set %RETVAR%=%P%&exit /b 0

:#----------------------------------------------------------------------------#

:# Returns the directory part of the pathname, so that %DIRVAR%\NAME refers to that file
:basename PATHNAME DIRVAR
setlocal EnableExtensions EnableDelayedExpansion
set "DIR=%~1"
:dirname.next
if "!DIR:~-1!"=="\" set "DIR=!DIR:~0,-1!" & goto :dirname.done
if "!DIR:~-1!"==":" set "DIR=!DIR!." & goto :dirname.done
set "DIR=!DIR:~0,-1!"
if defined DIR goto :dirname.next
set "DIR=."			&:# If empty, return the current directory
:dirname.done
%PREPVAR% DIR
endlocal & set "%~2=%DIR%" & exit /b

:#----------------------------------------------------------------------------#

:IsAdmin
>NUL 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
exit /b

:#----------------------------------------------------------------------------#

:# Get the current console code page.
:GetCP %1=variable name
for /f "tokens=2 delims=:" %%n in ('chcp') do for %%p in (%%n) do set "%1=%%p"
exit /b

:#----------------------------------------------------------------------------#

:# Get a registry value content. Args: KEY NAME [VALUEVAR [TYPEVAR]]
:GetRegistryValue
%ENTER%
setlocal enableextensions enabledelayedexpansion
set "KEY=%~1"
set "NAME=%~2"
set "VALUEVAR=%~3"
set "TYPEVAR=%~4"
if not defined VALUEVAR set "VALUEVAR=VALUE"
set "%VALUEVAR%="
:# Returning the type is optional. Do not define a default for TYPEVAR.
%ECHOVARS.D% KEY NAME VALUEVAR TYPEVAR
if defined NAME (
  set CMD=reg query "%KEY%" /v "%NAME%"
) else (
  set CMD=reg query "%KEY%" /ve
)
%ECHO.D% %CMD%
set "RETCODE=1"
:# For each line in CMD output...
%FOREACHLINE% %%i in ('%CMD% 2^>NUL') do (
  set "RETCODE=0"
  set "LINE=%%i"
  %ECHOVARS.D% LINE
  :# Values are indented by 4 spaces.
  set "HEAD=!LINE:~0,4!"
  set "LINE=!LINE:~4!"
  :# But extra lines of multi-lined values are indented by >20 spaces.
  set "HEAD2=!LINE:~0,4!"
  if "!HEAD!"=="    " if not "!HEAD2!"=="    " (
    :# Some versions of reg.exe use 4 spaces as field separator; others use a TAB. 
    :# Change the 4-spaces around the REG_XX type word to a TAB.
    set "TOKENS=!LINE:    =	!"
    %ECHOVARS.D% TOKENS
    :# Extract the value name as the first item before the first TAB.
    :# Names can contain spaces, but assume they don't contain TABs.
    for /f "tokens=1,2* delims=	" %%j in ("!TOKENS!") do (
      set "NAME=%%j"
      set "TYPE=%%k"
      set "VALUE=%%l"
      %ECHOVARS.D% NAME TYPE VALUE
    )
  )
)
%PREPVAR% VALUE
endlocal & (if defined TYPEVAR set "%TYPEVAR%=%TYPE%") & set "%VALUEVAR%=%VALUE%" & %RETURN% %RETCODE%

:#----------------------------------------------------------------------------#

:# Append a file pathname to a list of pathnames if it's not already there
:FileSet.append %1=LIST_VAR_NAME %2=VALUE
setlocal EnableDelayedExpansion
set "LIST=!%1!"
set "FOUND=0"
for %%e in (!LIST!) do if /i "%%~e"=="%~2" set "FOUND=1"
if "%FOUND%"=="0" set "LIST=!LIST! %2"
%PREPVAR% LIST
endlocal & set "%1=%LIST%" & exit /b

:#----------------------------------------------------------------------------#

:# Manage version strings

:GetAgVersion %1=[path\]ag.exe
for %%c in (AG MSVCLIBX PCRE PTHREADS ZLIB OS) do set "%%c_VER="
if exist %1 (
  for /f "tokens=1-5 delims=;" %%1 in ('%1 --version 2^>NUL') do (
    for %%t in ("%%~1" "%%~2" "%%~3" "%%~4" "%%~5") do (
      set "TOKEN=%%~t"
      if defined TOKEN (
	if not defined AG_VER (
	  set "AG_VER=!TOKEN:ag version =!"
	  if "!AG_VER!"=="!TOKEN!" ( :# It's not the ag version, clear this var
	    set "AG_VER="
	  ) else (		     :# It is the ag version, trim this var
	    for /f "tokens=1" %%v in ("!AG_VER!") do set "AG_VER=%%~v"
	  )
	)
	rem Scan component version strings
	for %%s in ("MSVCLIBX MsvcLibX" "PCRE PCRE" "PTHREADS pthreads4w" "ZLIB zlib") do (
	  for /f "tokens=1,2" %%p in (%%s) do (
	    if not defined %%p_VER (
	      set "%%p_VER=!TOKEN:%%q =!"
	      if "!%%p_VER!"=="!TOKEN!" ( :# It's not that component, clear this var
		set "%%p_VER="
	      ) else (			  :# It is that component, trim this var
		for /f "tokens=1" %%v in ("!%%p_VER!") do set "%%p_VER=%%~v"
	      )
	    )
	  )
	)
	if not defined OS_VER (
	  if not "!TOKEN: Win32 =!"=="!TOKEN!" set "OS_VER=WIN32"
	  if not "!TOKEN: Win64 =!"=="!TOKEN!" set "OS_VER=WIN64"
	)
      )
    )
  )
)
exit /b

:SplitVer %1=X.Y.Z.T %2=XVAR %3=YVAR %4=ZVAR %5=TVAR
for /f "tokens=1-4 delims=-." %%a in ("%~1") do (
  set "%2=%%a"
  set "%3=%%b"
  set "%4=%%c"
  set "%5=%%d"
)
exit /b

:CompareInteger %1=Integer1 %2=Integer2
if "%1%2"=="" exit /b 0				&:# They're both empty, thus equal
if "%2"=="" exit /b 1				&:# Integer1 > nothing
if "%1"=="" exit /b -1				&:# nothing < Integer2
setlocal
set /a "DIFF=%1-%2"
if "%DIFF%"=="0" endlocal & exit /b 0		&:# They're equal
if "%DIFF:-=%"=="%DIFF%" endlocal & exit /b 1	&:# Integer1 > Integer2
endlocal & exit /b -1				&:# Integer1 < Integer2

:CompareVersions %1=Version1 %2=Version2
%ENTER%
setlocal
call :SplitVer %1 V1MAJOR V1MINOR V1PATCH V1BUILD
call :SplitVer %2 V2MAJOR V2MINOR V2PATCH V2BUILD
call :CompareInteger %V1MAJOR% %V2MAJOR%
if not %ERRORLEVEL%==0 endlocal & %RETURN% %ERRORLEVEL%
call :CompareInteger %V1MINOR% %V2MINOR%
if not %ERRORLEVEL%==0 endlocal & %RETURN% %ERRORLEVEL%
call :CompareInteger %V1PATCH% %V2PATCH%
if not %ERRORLEVEL%==0 endlocal & %RETURN% %ERRORLEVEL%
call :CompareInteger %V1BUILD% %V2BUILD%
endlocal & %RETURN% %ERRORLEVEL%

:#----------------------------------------------------------------------------#

:# Upgrade an existing instance of ag.exe
:UpgradeAg %1=target_path
%ENTER%
:# Do not use setlocal, as we want to update many global variables
call :GetAgVersion %1
set "VERSIONS=%AG_VER%"
for %%p in (MSVCLIBX PCRE PTHREADS ZLIB) do if defined %%p_VER set "VERSIONS=!VERSIONS!, %%p !%%p_VER!"
%COMMENT%   It is !OS_VER! Ag version !VERSIONS!
if defined MSVCLIBX_VER (
  if defined OS_VER (
    call :basename %1 FOUND_!OS_VER!
    (call,) &rem :# Set ERRORLEVEL 0
    rem :# Sometimes a library is upgraded, while the main Ag code does not change.
    for %%c in (AG MSVCLIBX PCRE PTHREADS ZLIB) do if !ERRORLEVEL!==0 if defined %%c_VER (
      call :CompareVersions "!%%c_VER!" "!NEW_%%c_VER!"
    )
    if !ERRORLEVEL!==0 (
      %COMMENT%   The same version is already installed
    ) else if !ERRORLEVEL!==1 (
      %COMMENT%   A newer version is already installed
    ) else (
      %COMMENT%   Upgrading it with our new version
      %EXEC% copy /y "!OS_VER!\ag.exe" %1
    )
  ) else ( rem :# If this happens, this is a bug
    %LOG% Warning: Could not recognize its OS type. Leaving it unchanged.
  )
) else (
  %COMMENT% This is another unrelated port for Windows. Leaving it unchanged.
)
%RETURN%

:#----------------------------------------------------------------------------#

:# Install a new intance of ag.exe
:InstallAg %1=source_dir %2=target_dir_var
%ENTER%
setlocal EnableDelayedExpansion
set "SOURCE=%~1\ag.exe"
set "DESTDIR=!%~2!"
%COMMENT% Installing !SOURCE! into the !DESTDIR! directory
call :CondQuote DESTDIR
if not exist !DESTDIR! %EXEC% md !DESTDIR!
%EXEC% copy /y !SOURCE! !DESTDIR!
endlocal & set "FOUND_%~1=%DESTDIR%" & %RETURN%

:#----------------------------------------------------------------------------#

:# Uninstall a previous instance of Ag, using the uninstall command stored in the registry
:UninstallAg
%ENTER%
setlocal EnableDelayedExpansion

call :IsAdmin
if errorlevel 1 (
  %ERROR% This setup script must be run as Administrator
  endlocal & %RETURN% 1
)

set "UNINSTALL_CMD="
call :GetRegistryValue "%UNINSTALL_KEY%" UninstallString UNINSTALL_CMD
if not defined UNINSTALL_CMD (
  %ERROR% No previous instance of The Silver Searcher was found
  endlocal & %RETURN% 1
)
%PREPVAR% UNINSTALL_CMD UNINSTALL_CMD_MSG &:# Make sure %ECHO% displays & characters correctly
%ECHO% !UNINSTALL_CMD_MSG!
%UNINSTALL_CMD%
if errorlevel 1 (
  %ERROR% The Silver Searcher uninstallation failed
  endlocal & %RETURN% 1
)
%COMMENT% The Silver Searcher was uninstalled successfully
endlocal & %RETURN% 0

:#----------------------------------------------------------------------------#

:help
echo Install or upgrade ag.exe for Windows
echo.
echo Usage: %SCRIPT% OPTIONS
echo.
echo Options:
echo  -?    Display this help
echo  -u    Uninstalls a previous instance of Ag, instead of installing a new one
echo  -V    Display the script version
echo  -X    Display the installation commands to execute, but don't run them
echo.
echo Notes:
echo  * Upgrades previous instances found in the PATH
echo  * Else installs in paths defined in variables bindir_%%PROCESSOR_ARCHITECTURE%%
echo  * Else installs in the path defined in variable bindir
echo  * Else installs in %windir%
exit /b

:#----------------------------------------------------------------------------#

:Main
set "LOGFILE=%TEMP%\ag_setup.log"

set "SRCDIR[x86]=WIN32"
set "SRCDIR[AMD64]=WIN64"

set "UNINSTALL_KEY=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\The Silver Searcher"
set "INSTALLED_FILES="  &:# List of files installed or updated by this script

%LOG% --------------------------------------------------------------------
%LOG% %~f0 %*
:# set >>"%LOGFILE%"

:# Process command-line arguments
goto :get_arg
:next_arg
shift
:get_arg
if [%1]==[] goto :start
if [%1]==[/?] goto :help
if [%1]==[-?] goto :help
if [%1]==[-d] call :Debug.on & goto :next_arg
if [%1]==[-u] call :UninstallAg & %RETURN%
if [%1]==[-V] (echo %VERSION%) & exit /b 0
if [%1]==[-X] set "EXEC=echo" & goto :next_arg
%ERROR% Unexpected argument: %1
%RETURN% 1

:start
call :IsAdmin
if errorlevel 1 (
  %ERROR% This setup script must be run as Administrator
  %RETURN% 1
)

cd "%SPATH%"
:# In the winget package, ag.exe is there in the win32 subdirectory.
:# But when testing setup.bat in the source tree, ag.exe is in "..\..\bin\win32".
if not exist "win32\ag.exe" if exist "..\..\bin\win32\ag.exe" cd ..\..\bin
if not exist "win32\ag.exe" (
  %ERROR% Can't find ag.exe masters
  %RETURN% 1
)

:# Get this new instance version
call :GetAgVersion "win32\ag.exe"
set "NEW_AG_VER=%AG_VER%"
set "VERSIONS=%AG_VER%"
for %%p in (MSVCLIBX PCRE PTHREADS ZLIB) do (
  set "NEW_%%p_VER=!%%p_VER!"
  set "VERSIONS=!VERSIONS!, %%p !%%p_VER!"
)
%COMMENT% Installing ag.exe version %VERSIONS%

:# Scan the PATH for previous installations to upgrade
set "FOUND_WIN32="	&:# Directory where a WIN32 instance was found
set "FOUND_WIN64="	&:# Directory where a WIN64 instance was found
for /f "delims=" %%p in ('"echo.%%PATH:;=&echo.%%"') do (
  for %%a in ("%%~p\ag.exe") do if exist %%a (
    %COMMENT% Found %%~a
    call :UpgradeAg %%a
    call :FileSet.append INSTALLED_FILES %%a
  )
)

:# Find the system's processor architeture
:# Gotcha: When run in 7zip's self-extractor, this is run in a 32-bits version of cmd.exe, even if the OS is a 64-bits OS
set "PROC=%PROCESSOR_ARCHITEW6432%" &:# This will be empty when run in a normal 64-bits cmd.exe
if not defined PROC set "PROC=%PROCESSOR_ARCHITECTURE%"

:# Update specific versions in !bindir_%PROCESSOR_ARCHITECTURE%!, if defined
set "PROCS=%PROC%"
if not "%PROCS%"=="x86" set "PROCS=x86 %PROCS%"
%LOG% :# Trying PROCS = %PROCS%
for %%p in (%PROCS%) do ( :# Ex: %%p=x86 or %%p=amd64
  for /f %%d in ("!SRCDIR[%%p]!") do if exist %%d\ag.exe ( :# Ex: %%d=WIN32 or %%d=WIN64
    %ECHOVARS.D% bindir_%%p FOUND_%%d
    if defined bindir_%%p if /i "!bindir_%%p!" neq "!FOUND_%%d!" (
      for %%a in ("!bindir_%%p!\ag.exe") do (
      	if exist %%a (
	  %COMMENT% Found %%~a
	  call :UpgradeAg %%a
	) else (
	  %COMMENT% No ag.exe found in [bindir_%%p] = !bindir_%%p!
	  call :InstallAg %%d bindir_%%p
	  %ECHOVARS.D% FOUND_%%d
	)
	call :FileSet.append INSTALLED_FILES %%a
      )
    ) else (
      %LOG% Variable bindir_%%p is defined, but we already updated it as part of the PATH
    )
  )
)

:# If we did not find one for the current architecture, install one in the default dir
set "SRCDIR=!SRCDIR[%PROC%]!"
if not defined SRCDIR set "SRCDIR=WIN32"
if not defined bindir set "bindir=%windir%"
if not defined FOUND_%SRCDIR% (
  %COMMENT% No ag.exe found for the %SRCDIR% architecture
  call :InstallAg %SRCDIR% bindir
  call :FileSet.append INSTALLED_FILES "%bindir%\ag.exe"
)

:# Make sure there's something to uninstall
if not defined INSTALLED_FILES (
  %ERROR% Nothing was installed
  %RETURN% 1
)
set "INSTALLED_FILES=!INSTALLED_FILES:~1!" &:# Remove the initial space
%ECHOVARS.D% INSTALLED_FILES

:# Write the uninstall information into the registry
:# This is used by winget.exe to locate previous installations.
%COMMENT% Writing uninstall information into the registry
call :SplitVer %NEW_AG_VER% V_MAJOR V_MINOR V_PATCH V_BUILD
set QUOTED_KEY="!UNINSTALL_KEY!"
set "UNINSTALL_STRING=%windir%\System32\cmd.exe /c del !INSTALLED_FILES! & reg delete !QUOTED_KEY! /f"
set "UNINSTALL_STRING=!UNINSTALL_STRING:\=\\!"
set "UNINSTALL_STRING=!UNINSTALL_STRING:"=\"!"
%PREPVAR% UNINSTALL_STRING &:# Make sure all tricky characters like & get output correctly
( :# The first echo begins with a UTF-8 BOM
echo ﻿Windows Registry Editor Version 5.00
echo.
echo [!UNINSTALL_KEY!]
echo "DisplayName"="The Silver Searcher"
echo "DisplayVersion"="%NEW_AG_VER%"
echo "Publisher"="Jean-François Larvoire"
echo "UninstallString"="%UNINSTALL_STRING%"
echo "URLInfoAbout"="https://github.com/JFLarvoire/the_silver_searcher"
echo "VersionMajor"="%V_MAJOR%"
echo "VersionMinor"="%V_MINOR%"
) >"%TEMP%\ag_setup.reg"
%LOG% type "%TEMP%\ag_setup.reg"
>>"%LOGFILE%" type "%TEMP%\ag_setup.reg"
%EXEC% regedit.exe /s "%TEMP%\ag_setup.reg"
if errorlevel 1 (
  %ERROR% Failed to import the uninstall information into the registry
  %RETURN% 1
)

%COMMENT% The Silver Searcher was installed successfully
%RETURN% 0
