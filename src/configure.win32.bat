@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename        configure.msc.bat                                         *
:#                                                                            *
:#  Description     Define ag MSVC-specific configuration settings            *
:#                                                                            *
:#  Notes                                                                     *
:#                                                                            *
:#  History                                                                   *
:#   2017-02-14 JFL Created this file.                                        *
:#                                                                            *
:#*****************************************************************************

:# First search in Ag's wincompat libraries
set "MY_SDKS=..\win32 %MY_SDKS%"

:# Store output files one level up
:# set "OUTDIR=..\bin"
:# We use a junction instead of a symlinkd, because Windows allows creating junctions by default, but not symlinkds.
set "JUNCTION=mklink /j" &:# Default command installed on all Vista and later systems
where junction >NUL 2>NUL && set "JUNCTION=junction" &:# Better command that also works on network drives
set "MD_OUTDIR=%JUNCTION% %OUTDIR% ..\%OUTDIR% >NUL"

:# Even if the right tools are available, avoid trying to build ag.exe for DOS and WIN95, which are not supported yet.
if "%OS%"=="Windows_NT" set "OS=" &:# This is the Windows' homonym default, same as undefined for us here.
if not defined OS set "OS=WIN32 WIN64" &:# Only WIN32 and WIN64 are supported so far.

:# Configure.bat does not know about these libraries
set "SDK.PCRE.NAME=Perl Compatible Regular Expressions"
set "SDK.PCRE.FILE=pcre.h.in"     &:# A unique file at the base of that library tree
set "SDK.PCRE.INCPATH=."          &:# Include files are in the base directory
set "SDK.PCRE.LIBPATH=$(BR)"      &:# $(BR) = SysToolsLib OS-specific output directory

set "SDK.PTHREAD.NAME=Posix Threads for Windows"
set "SDK.PTHREAD.FILE=pthread.h"  &:# A unique file at the base of that library tree
set "SDK.PTHREAD.INCPATH=."       &:# Include files are in the base directory
set "SDK.PTHREAD.LIBPATH=VC2_LIB\$(BR)" &:# $(BR) = SysToolsLib OS-specific output directory

set "SDK.ZLIB.NAME=General Purpose Compression Library"
set "SDK.ZLIB.FILE=zlib.h"        &:# A unique file at the base of that library tree
set "SDK.ZLIB.INCPATH=."          &:# Include files are in the base directory
set "SDK.ZLIB.LIBPATH=$(BR)"      &:# $(BR) = SysToolsLib OS-specific output directory

:# Declare the SDKs and libraries we need
%BEGIN_SDK_DEFS%
%USE_SDK% MSVCLIBX
%USE_SDK% PCRE
%USE_SDK% PTHREAD
%USE_SDK% ZLIB
%END_SDK_DEFS%
