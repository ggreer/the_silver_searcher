@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename:	    configure.pthread.bat				      *
:#                                                                            *
:#  Description:    Define pthread-specific configuration settings            *
:#                                                                            *
:#  Notes:	                                                              *
:#                                                                            *
:#  History:                                                                  *
:#   2017-02-14 JFL Created this file.					      *
:#                                                                            *
:#*****************************************************************************

:# Declare the SDKs and libraries we need
%BEGIN_SDK_DEFS%
:# %USE_SDK% MSVCLIBX
%END_SDK_DEFS%

:# Pthread has its own uwin nmakefile, that we do not want to use
set "IGNORE_NMAKEFILE=1"

:# Avoid trying to build pthreads for DOS, even if a DOS compiler is available.
if "%OS%"=="Windows_NT" set "OS=" &:# This is the Windows' homonym default, same as undefined for us here.
if not defined OS set "OS=WIN32 WIN64" &:# Only WIN32 and WIN64 are supported so far.
