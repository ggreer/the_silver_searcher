@echo off
:******************************************************************************
:*                                                                            *
:*  Filename:	    src2objs.bat					      *
:*                                                                            *
:*  Description:    Generate object files names from source files names	      *
:*                                                                            *
:*  Notes:								      *
:*									      *
:*  History:                                                                  *
:*   2010-04-07 JFL Created this batch.                                       *
:*                                                                            *
:#         © Copyright 2016 Hewlett Packard Enterprise Development LP         *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:******************************************************************************

setlocal 2>NUL
set ARG0=%0

:# Mechanism for calling subroutines. Done by {call %0 _call_ label [arguments]}.
if .%1.==._call_. shift /2 & goto %2
set CALL=call %0 _call_
set RETURN=goto end
:# Silent return. Used for routines silently called via call :label.
set _RETURN=goto _end
set PUTVARS=call :putvars

:# Set global defaults
setlocal
set OUTFILE=obj\objects.mak
set OUTPATH=obj
set NO_EXEC=0
goto get_args

:help
echo Generate object files names from source files names
echo.
echo Usage: %ARG0% [options] source ...
echo.
echo Options:
echo   -?^|-h         This help
echo   -o {pathname} Output file pathname. Default: %OUTFILE%
echo   -X            Display object files names, but do not create the output file
goto end

:get_args
if .%1.==.. goto help
if .%1.==.-?. goto help
if .%1.==./?. goto help
if %1==-o shift & goto set_out
if %1==-X shift & goto no_exec
goto go

:set_out
set OUTFILE=%1
:# Split the path. Uses a fake drive @ to prevent prepending the current path on an existing drive.
for %%F in (@:"%OUTFILE%") do (
  set "OUTPATH=%%~dpF"
)
:# Remove the head "@:\ and tail \ to the path
set "OUTPATH=%OUTPATH:~4,-1%"
shift
goto get_args

:no_exec
set NO_EXEC=1
goto get_args

:# Check prerequisites
:check
verify other 2>nul
setlocal enableextensions
if errorlevel 1 (
  echo>&2 Error: Unable to enable command extensions.
  exit /b 1
)
set VAR=before
if "%VAR%" == "before" (
  set VAR=after
  if not "!VAR!" == "after" (
    echo>&2 Error: Delayed environment variable expansion must be enabled.
    echo>&2 Please restart your cmd.exe shell with the /V option,
    echo>&2 or set HKLM\Software\Microsoft\Command Processor\DelayedExpansion=1
    exit /b 1
  )
)
%_RETURN%

:go
%CALL% check
if errorlevel 1 exit /b 1
set OBJECTS=
:next
if .%1.==.. goto done
for %%s in (%1) do (
  set EXT=%%~xs
  set OBJ=
  if .!EXT!.==..c. set OBJ=obj
  if .!EXT!.==..C. set OBJ=obj
  if .!EXT!.==..cpp. set OBJ=obj
  if .!EXT!.==..CPP. set OBJ=obj
  if .!EXT!.==..asm. set OBJ=obj
  if .!EXT!.==..ASM. set OBJ=obj
  if .!EXT!.==..rc. set OBJ=res
  if .!EXT!.==..RC. set OBJ=res
  if .!OBJ!.==.. (
    echo>&2 Error: Unsupported source type: !EXT!
    echo>&2        Please add a conversion rule in %ARG0%
  ) else (
    if .!OBJECTS!.==.. (
      set OBJECTS=%OUTPATH%\%%~ns.!OBJ!
    ) else (
      set OBJECTS=!OBJECTS! %OUTPATH%\%%~ns.!OBJ!
    )
  )
)
shift
goto next
:done
echo OBJECTS=%OBJECTS%
if %NO_EXEC%==0 echo>%OUTFILE% OBJECTS=%OBJECTS%

:end
:_end

