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
:#   2018-03-01 JFL Removed the dependency on remplace.exe.                   *
:#                  Added the DEFAULTLIBS from the OBJECTS files.             *
:#   2018-03-11 JFL Use variable _OBJECTS if defined, else use OBJECTS.	      *
:#		    							      *
:#        © Copyright 2018 Hewlett Packard Enterprise Development LP          *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:#*****************************************************************************

setlocal EnableExtensions EnableDelayedExpansion
set "VERSION=2018-03-11"
set ARG0=%0
goto :main

:# Clear the %1[] array
:DeleteArray %1=ArrayName
for /f "tokens=1 delims==" %%v in ('set %1[ 2^>NUL') do set "%%v="
exit /b

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

:# Define a LF variable containing a Line Feed ('\x0A')
set LF=^
%# The two blank lines here are necessary. #%
%# The two blank lines here are necessary. #%
:# Define variables that will expand to a Line Feed after N expansions
set ^"LF1=^%LF%%LF%"
set ^"LF2=^^^%LF1%%LF1%^%LF1%%LF1%"

:# Show the context variables
:# The debug output will be included in the output make file, and must begin with a # to be commented-out
echo # Input variables:
echo # LIBS=%LIBS%
echo # LIB = ( &:# Split the content on multiple lines, with one pathname per line, to make it easier to read.
set LIB_LINES=!LIB:;=%LF2%!
for /f "delims=" %%l in ("!LIB_LINES!") do echo #   "%%l"
echo # )
echo # OBJECTS=%OBJECTS%
echo # O=%O%
echo.
echo # Computed variables:

:# Find additional libraries in the objects we'll link
set "OLIBS="
call :DeleteArray OLIBS &:# Clear the OLIBS[] array
if defined _OBJECTS (
  set "OBJVAR=_OBJECTS"
) else (
  set "OBJVAR=OBJECTS"
)
if defined %OBJVAR% (
  for %%o in (!%OBJVAR%!) do (
    if exist "%%~o" (
      set "OBJ=%%~o"
    ) else if exist "%O%\%%~o" (
      set "OBJ=%O%\%%~o"
    ) else (
      set "OBJ=NUL"
    )
    :# Look for the /DEFAULTLIB strings list in the object file
    set "LIST="
    for /f "delims=" %%l in ('find "DEFAULTLIB" ^< "!OBJ!"') do set "LIST=%%l"
    :# Split the list into one entry per line
    set LINES=!LIST: =%LF2%!
    for /f "delims=" %%l in ("!LINES!") do (
      set "LINE=%%l"
      :# Remove the quotes around library names
      set "LINE=!LINE:"=!"
      :# Valid lines are those that are like /DEFAULTLIB:library.lib. Skip default names like "LIBCMT".
      if not "!LINE:DEFAULTLIB=!"=="!LINE!" if not "!LINE:.=!"=="!LINE!" (
	for /f "tokens=2 delims=:" %%l in ("!LINE!") do if not defined OLIBS[%%l] (
	  set "OLIBS[%%l]=1"
	  set "OLIBS=!OLIBS! %%l"
	)
      )
    )
  )
)
call :DeleteArray OLIBS &:# Clear the OLIBS[] array
if defined OLIBS set "OLIBS=!OLIBS:~1!" &:# Remove the head space
echo # OLIBS=%OLIBS%

:# Merge the LIBS and OLIBS arrays, removing duplicates
call :DeleteArray LIBS2 &:# Clear the LIBS2[] array
set "LIBS2="
for %%l in (%LIBS% %OLIBS%) do if not defined LIBS2[%%l] (
  set "LIBS2[%%l]=1"
  set "LIBS2=!LIBS2! %%l"
)
call :DeleteArray LIBS2 &:# Clear the LIBS2[] array
if defined LIBS2 set "LIBS2=!LIBS2:~1!" &:# Remove the head space
echo # LIBS+OLIBS=%LIBS2%

:# Find the libraries we depend on
set "LIBRARIES="
echo # LIBRARIES = (
if defined LIB if defined LIBS (
  for %%l in (%LIBS2%) do (
    set "L=%%~f$LIB:l"
    if defined L (
      echo #   "!L!" 
      if not defined LIBRARIES (
	set "LIBRARIES="!L!""
      ) else (
	set "LIBRARIES=!LIBRARIES! "!L!""
      )
    ) else (
      echo #   # Failed to find %%l in %%LIB%%
    )
  )
)
echo # )
echo.

:# Output the result
echo LIBRARIES=%LIBRARIES%
exit /b %ERR%
