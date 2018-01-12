@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename	    lib2libs.bat					      *
:#                                                                            *
:#  Description	    Generate LIBRARIES from LIB and LIBS		      *
:#                                                                            *
:#  Notes	    The output is read back by a make file, and so must be    *
:#		    a valid make file itself. For example use # for comments. *
:#		    							      *
:#  History	                                                              *
:#   2018-01-12 JFL Created this script                                       *
:#                                                                            *
:#        © Copyright 2018 Hewlett Packard Enterprise Development LP          *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:#*****************************************************************************

setlocal EnableExtensions EnableDelayedExpansion
set "VERSION=2018-01-12"
set ARG0=%0
goto :main

:help
echo Generate LIBRARIES from LIB and LIBS
echo.
echo Usage: set "LIB=..." ^& set "LIBS=..." ^& %ARG0% [OPTIONS]
echo.
echo Options:
echo   -?^|-h         This help
echo   -V            Display this script version
goto :eof

:main
:# Set global defaults
goto get_args

:next_arg
shift
:get_args
if .%1.==.. goto :go
if .%1.==.-?. goto :help
if .%1.==./?. goto :help
if %1==-V (echo %VERSION%) & goto :eof
goto :next_arg

:go
set "ERR=0"

:# Find the libraries we depend on
:# The debug output will be included in the output make file, and must begin with a # to be commented-out
echo # LIBS=%LIBS%
echo # LIB=... &:# Split the content on multiple lines, with one pathname per line, to make it easier to read.
for /f "delims=" %%l in ('echo.!LIB!^| C:\JFL\Tools\Win32\remplace ";" \r\n') do echo #    "%%l"
set "LIBRARIES="
if defined LIB if defined LIBS (
  for %%l in (%LIBS%) do (
    set "L=%%~f$LIB:l"
    if defined L (
      echo # Adding "!L!" to the LIBRARIES
      if not defined LIBRARIES (
	set "LIBRARIES="!L!""
      ) else (
	set "LIBRARIES=!LIBRARIES! "!L!""
      )
    ) else (
      echo # Failed to find %%l in %%LIB%%
    )
  )
)

:# Output the result
echo LIBRARIES=%LIBRARIES%
exit /b %ERR%
