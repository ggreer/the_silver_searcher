@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename	    src2objs.bat					      *
:#                                                                            *
:#  Description	    Generate object files names from source files names	      *
:#                                                                            *
:#  Notes	    The output is read back by a make file, and so must be    *
:#		    a valid make file itself. For example use # for comments. *
:#		    							      *
:#  History	                                                              *
:#   2010-04-07 JFL Created this batch.                                       *
:#   2017-03-01 JFL Streamlined the code.                                     *
:#   2017-03-02 JFL Added VERSION and option -V.                              *
:#   2017-03-03 JFL Output two distinct variables: OBJECTS and PLUSOBJS       *
:#   2018-01-12 JFL Updated comments.				              *
:#   2018-03-11 JFL Output a third variable: _OBJECTS = list without paths    *
:#                                                                            *
:#      © Copyright 2016-2017 Hewlett Packard Enterprise Development LP       *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:#*****************************************************************************

setlocal EnableExtensions EnableDelayedExpansion
set "VERSION=2018-03-11"
set ARG0=%0
goto :main

:help
echo Generate object files names from source files names
echo.
echo Usage: %ARG0% [options] source ...
echo.
echo Options:
echo   -?^|-h         This help
echo   -+            Prefix each object with a +
echo   -o {pathname} Object directory pathname. Default: .
echo   -V            Display this script version
goto :eof

:main
:# Set global defaults
set "OBJPATH=."
set "OBJPATH\="
set "PLUS="
set NO_EXEC=0
goto get_args

:next_arg
shift
:get_args
if .%1.==.. goto :help
if .%1.==.-?. goto :help
if .%1.==./?. goto :help
if .%1.==.-+. set "PLUS=+" & goto :next_arg
if %1==-o set "OBJPATH=%~2" & set "OBJPATH\=%~2\" & shift & goto :next_arg
if %1==-V (echo %VERSION%) & goto :eof
goto :go

:go
set "OBJECTS="
set "+OBJECTS="
set "_OBJECTS="
set "ERR=0"

set "OBJ[.c]=obj"
set "OBJ[.cpp]=obj"
set "OBJ[.asm]=obj"
set "OBJ[.r]=obj"
set "OBJ[.rc]=res"

:# Convert the list of sources to a list of object files
:next
if .%1.==.. goto :done
for %%s in (%1) do (
  set "EXT=%%~xs"
  set "OBJ=!OBJ[%%~xs]!"
  if not defined OBJ (
    >&2 echo Error: Unsupported source type: !EXT!
    >&2 echo        Please add a conversion rule in %ARG0%
    set ERR=1
  ) else (
    set "_OBJECT=%%~ns.!OBJ!
    set "OBJECT=%PLUS%%OBJPATH\%!_OBJECT!
    set "OBJECTS=!OBJECTS! !OBJECT!"
    set "+OBJECTS=!+OBJECTS! +!OBJECT!"
    set "_OBJECTS=!_OBJECTS! !_OBJECT!"
  )
)
shift
goto :next
:done
if defined OBJECTS (
  set "OBJECTS=!OBJECTS:~1!"
  set "+OBJECTS=!+OBJECTS:~1!"
  set "_OBJECTS=!_OBJECTS:~1!"
)

:# Output the result
echo OBJECTS=%OBJECTS%
echo PLUSOBJS=%+OBJECTS%
echo _OBJECTS=%_OBJECTS%
exit /b %ERR%
