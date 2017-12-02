@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename	    src2objs.bat					      *
:#                                                                            *
:#  Description	    Generate object files names from source files names	      *
:#                                                                            *
:#  Notes								      *
:#									      *
:#  History	                                                              *
:#   2010-04-07 JFL Created this batch.                                       *
:#   2017-03-01 JFL Streamlined the code.                                     *
:#   2017-03-02 JFL Added VERSION and option -V.                              *
:#   2017-03-03 JFL Output two distinct variables: OBJECTS and PLUSOBJS       *
:#                                                                            *
:#      © Copyright 2016-2017 Hewlett Packard Enterprise Development LP       *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:#*****************************************************************************

setlocal EnableExtensions EnableDelayedExpansion
set "VERSION=2017-03-03"
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
set "ERR=0"

set "OBJ[.c]=obj"
set "OBJ[.cpp]=obj"
set "OBJ[.asm]=obj"
set "OBJ[.r]=obj"
set "OBJ[.rc]=res"

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
    set "OBJECT=%PLUS%%OBJPATH\%%%~ns.!OBJ!
    if not defined OBJECTS (
      set "OBJECTS=!OBJECT!"
      set "+OBJECTS=+!OBJECT!"
    ) else (
      set "OBJECTS=!OBJECTS! !OBJECT!"
      set "+OBJECTS=!+OBJECTS! +!OBJECT!"
    )
  )
)
shift
goto :next
:done
echo OBJECTS=%OBJECTS%
echo PLUSOBJS=%+OBJECTS%
exit /b %ERR%
