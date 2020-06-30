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
:#                                                                            #
:##############################################################################

setlocal EnableExtensions DisableDelayedExpansion &:# Make sure ! characters are preserved
set "VERSION=2020-06-04"
set "SCRIPT=%~nx0"		&:# Script name
set "SNAME=%~n0"		&:# Script name, without its extension
set "SPATH=%~dp0"		&:# Script path
set "SPATH=%SPATH:~0,-1%"	&:# Script path, without the trailing \
set "SFULL=%~f0"		&:# Script full pathname
set ^"ARG0=%0^"			&:# Script invokation name
set ^"ARGS=%*^"			&:# Argument line
setlocal EnableExtensions EnableDelayedExpansion &:# Use the ! expansion now on

goto :Main

:#----------------------------------------------------------------------------#
:# Simple debug framework

:# Do nothing
:nop
exit /b

:# Trace the return from a subroutine, and do the actual return
:Return
setlocal
set "ERR=%~1"
if not defined ERR set "ERR=%ERRORLEVEL%"
echo   exit /b %ERR%
2>NUL (goto) & exit /b %ERR% &:# Pop one call stack, then return to the upper level

:# Display and log the definition of a multiple variables
:EchoVars.d
for %%v in (%*) do echo set %%v=!%%v!
:LogVars
for %%v in (%*) do %LOG% set %%v=!%%v!
exit /b

:# Display and log a message
:Echo
echo %*
%LOG% %*
exit /b

:# Display and log a batch comment
:Comment
%ECHO% :# %*
exit /b

:# Display a message, with a timestamp, into the log file
:Log
setlocal DisableDelayedExpansion
call :Now
set "NOW=%YEAR%-%MONTH%-%DAY% %HOUR%:%MINUTE%:%SECOND%.%MS%"
>>"%LOGFILE%" echo %NOW% %*
endlocal
exit /b

:# Display and log a batch command, and run it.
:Exec
%ECHO% %*
%*
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
endlocal & set "%~2=%DIR%" & exit /b

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

:SplitVer %1=X.Y.Z %2=XVAR %3=YVAR %4=ZVAR
for /f "tokens=1-3 delims=-." %%x in ("%~1") do (
  set "%2=%%x"
  set "%3=%%y"
  set "%4=%%z"
)
exit /b

:CompareInteger %1=Integer1 %2=Integer2
setlocal
if "%1%2"=="" exit /b 0				&:# They're both empty, thus equal
if "%2"=="" exit /b 1				&:# Integer1 > nothing
if "%1"=="" exit /b -1				&:# nothing < Integer2
set /a "DIFF=%1-%2"
if "%DIFF%"=="0" endlocal & exit /b 0		&:# They're equal
if "%DIFF:-=%"=="%DIFF%" endlocal & exit /b 1	&:# Integer1 > Integer2
endlocal & exit /b -1				&:# Integer1 < Integer2

:CompareVersions %1=Version1 %2=Version2
%ECHO.D% call %0 %*
call :SplitVer %1 V1MAJOR V1MINOR V1PATCH
call :SplitVer %2 V2MAJOR V2MINOR V2PATCH
call :CompareInteger %V1MAJOR% %V2MAJOR%
if not %ERRORLEVEL%==0 %RETURN% %ERRORLEVEL%
call :CompareInteger %V1MINOR% %V2MINOR%
if not %ERRORLEVEL%==0 %RETURN% %ERRORLEVEL%
call :CompareInteger %V1PATCH% %V2PATCH%
%RETURN% %ERRORLEVEL%

:#----------------------------------------------------------------------------#

:# Upgrade an existing instance of ag.exe
:UpgradeAg %1=target_path
%ECHO.D% call %0 %*
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
exit /b

:# Install a new intance of ag.exe
:InstallAg %1=source_dir %2=target_dir_var
%ECHO.D% call %0 %*
setlocal EnableDelayedExpansion
set "SOURCE=%~1\ag.exe"
set "DESTDIR=!%~2!"
%COMMENT% Installing !SOURCE! into the !DESTDIR! directory
call :CondQuote DESTDIR
if not exist !DESTDIR! %EXEC% md !DESTDIR!
%EXEC% copy /y !SOURCE! !DESTDIR!
endlocal & set "FOUND_%~1=%DESTDIR%"
exit /b

:#----------------------------------------------------------------------------#

:help
echo Install or upgrade ag.exe for Windows
echo.
echo Usage: %SCRIPT% OPTIONS
echo.
echo Options:
echo  -?    Display this help
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
set "SRCDIR[x86]=WIN32"
set "SRCDIR[AMD64]=WIN64"

set "COMMENT=call :Comment"
set "ECHO=call :Echo"
set "EXEC=call :Exec"
set "RETURN=exit /b"
set "ECHO.D=call :nop"
set "ECHOVARS.D=call :LogVars"

set "LOG=call :Log"
set "LOGFILE=%TEMP%\ag_setup.log"

set "QUIET=0"

%LOG% --------------------------------------------------------
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
if [%1]==[-d] set "ECHO.D=echo" & set "ECHOVARS.D=call :EchoVars.d" & set "RETURN=call :Return" & goto :next_arg
if [%1]==[-V] (echo %VERSION%) & exit /b 0
if [%1]==[-X] set "EXEC=echo" & goto :next_arg
if [%1]==[-y] set "QUIET=1" & goto :next_arg
>&2 echo Error: Unexpected argument: %1
exit /b 1

:start
cd "%SPATH%"

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
      for %%a in ("!bindir_%%p!\ag.exe") do if exist %%a (
      	%COMMENT% Found %%~a
	call :UpgradeAg %%a
      ) else (
      	%COMMENT% No ag.exe found in [bindir_%%p] = !bindir_%%p!
	call :InstallAg %%d bindir_%%p
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
)

exit /b 0
