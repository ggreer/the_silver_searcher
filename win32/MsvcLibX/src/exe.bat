@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename:	    exe.bat						      *
:#                                                                            *
:#  Description:    Front end to make.bat, to simply build multiple targets   *
:#                                                                            *
:#  Arguments:	    Use option -? to display a help screen		      *
:#                                                                            *
:#  Notes:	    Builds the 16-bits MS-DOS version if Visual C++ 1.52 is   *
:#		    installed in its default location in C:\MSVC.	      *
:#									      *
:#  History:                                                                  *
:#   2003-03-31 JFL Adapted from previous projects			      *
:#   2014-03-21 JFL Builds the 16-bits MS-DOS version if Visual C++ 1.52 is   *
:#		    installed in its default location in C:\MSVC.	      *
:#   2014-03-27 JFL Changed option -f to use nmake option /A.                 *
:#		    Added option -r for completeness.                         *
:#   2015-11-13 JFL Adapted to the new multitarget make system.               *
:#			                                                      *
:#         © Copyright 2016 Hewlett Packard Enterprise Development LP         *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:#*****************************************************************************

setlocal enableextensions enabledelayedexpansion
goto main

:main
set "FORCE=0"
set "ACTION=default"
set "EXEC="
set "MAKEOPTS="

goto get_arg
:next_arg
shift
:get_arg
if .%1.==.-?. goto help
if .%1.==./?. goto help
if .%1.==.-a. set "ACTION=all" & goto next_arg
if .%1.==.-d. set "ACTION=debug" & goto next_arg
if .%1.==.-f. set "FORCE=1" & goto next_arg
if .%1.==.-r. set "ACTION=release" & goto next_arg
if .%1.==.-X. set "EXEC=echo" & goto next_arg
set MAKEOPTS=%2 %3 %4 %5 %6 %7 %8 %9
if "%FORCE%"=="1" set "MAKEOPTS=%MAKEOPTS% /A"
goto %ACTION%

:help
echo.
echo..exe program builder from a C or C++ source
echo.
echo.Usage: exe [options] program [nmake_options]
echo.
echo.Options:
echo.
echo.  -?    Display this help page
echo.  -a    Builds all release ^& debug versions (default)
echo.  -d    Builds all debug versions only
echo.  -f    Force building all program targets, irrespective of file dates
echo.  -r    Builds all release versions only
echo.  -X    Display the make command generated and exit
echo.
echo.Notes:
echo.* exe myprog ^<==^> make myprog.exe debug\myprog.exe
echo.* This builds all possible OS targets
echo.* To force rebuilding all targets, irrespective of their date,
echo.  use nmake option /A. So: exe -f myprog ^<==^> exe myprog /A
goto :eof

:release
%EXEC% make %MAKEOPTS% %1.exe
goto :eof

:debug
%EXEC% make %MAKEOPTS% debug\%1.exe
goto :eof

:default
:all
%EXEC% make %MAKEOPTS% %1.exe debug\%1.exe
goto :eof
