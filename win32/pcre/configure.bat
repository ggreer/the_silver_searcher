@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename:	    configure.bat					      *
:#                                                                            *
:#  Description:    Detect system-specific settings and create config.*.bat   *
:#                                                                            *
:#  Notes:	    Proxy script for %STINCLUDE%\configure.bat.		      *
:#                                                                            *
:#                  Make any change needed in %STINCLUDE%\configure.bat.      *
:#                                                                            *
:#  History:                                                                  *
:#   2016-10-10 JFL jf.larvoire@hpe.com created this file.		      *
:#   2016-12-15 JFL Search for the real make.bat in [.|..|../..]\include.     *
:#   2017-03-12 JFL Avoid defining STINCLUDE as a side effect.                *
:#   2017-11-13 JFL Bug fix: Avoid breaking the cmd windows title.            *
:#                  Added a search in the PATH, to be forward-compatible.     *
:#                  Added the :CheckDir subroutine to search consistently.    *
:#                                                                            *
:#        © Copyright 2016 Hewlett Packard Enterprise Development LP          *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:#*****************************************************************************

setlocal EnableExtensions EnableDelayedExpansion

goto main

:# Check if a directory contains the SysToolsLib make system
:CheckDir %1=DIRNAME :# Sets STINCLUDE and ERRORLEVEL=0 if success
:# echo searching "%~1"
set "STINCLUDE="
if exist "%~1\make.bat" if exist "%~1\configure.bat" if exist "%~1\win32.mak" (
  for /f "delims=" %%d in ('pushd %1 ^& cd ^& popd') do set "STINCLUDE=%%d"
)
if defined STINCLUDE exit /b 0
exit /b 1

:main
set "CHECKDIR=call :CheckDir"

:# Get the full pathname of the STINCLUDE library directory
if defined STINCLUDE %CHECKDIR% "%STINCLUDE%" &:# If pre-defined, make sure the value is valid

:# As a first choice, use the make.bat provided in this project
if not defined STINCLUDE %CHECKDIR% include :# If we have one here, use it
:# Else try finding it in the PATH
set P=!PATH!
:NextP
if not defined STINCLUDE (
  for /f "tokens=1* delims=;" %%a in ("!P!") do (
    set "DIR=%%a
    set "P=%%b"
    if defined DIR %CHECKDIR% "!DIR!"
    if defined P goto :NextP
  )
)
:# Else try in the near context
for %%p in (.. ..\..) do if not defined STINCLUDE %CHECKDIR% %%p\include :# Default: Search it the parent directory, and 2nd level.
:# We might also store the information in the registry
if not defined STINCLUDE ( :# Try getting the copy in the master environment
  for /f "tokens=3" %%v in ('reg query "HKCU\Environment" /v STINCLUDE 2^>NUL') do set "STINCLUDE=%%v"
)
:# If we still can't find it, admit defeat
if not exist %STINCLUDE%\make.bat (
  >&2 echo %0 Error: Cannot find SysToolsLib's global C include directory. Please define variable STINCLUDE.
  exit /b 1
)

if [%1]==[-d] echo "%STINCLUDE%\%~n0.bat" %*
:# Must call the target batch, not run it directly, else the cmd window title is not removed in the end!
endlocal & call "%STINCLUDE%\%~n0.bat" %*
