@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename:	    configure.ag.bat					      *
:#                                                                            *
:#  Description:    Define ag-specific global configuration settings	      *
:#                                                                            *
:#  Notes:	                                                              *
:#                                                                            *
:#  History:                                                                  *
:#   2017-03-04 JFL Created this file.					      *
:#   2026-03-11 JFL Define the full pathname of the STINCLUDE subdirectory.   *
:#                                                                            *
:#*****************************************************************************

:# To avoid overwriting the existing WIN32 subdirectories in . and src and win32\zlib...
if not defined OUTDIR ( :# Unless already redirected elsewhere
  set "LOGDIR=."	&rem :# But leave the log file in the build directory itself.
)

:# Avoid trying to build ag.exe for DOS and WIN95, which are not supported yet.
if "%OS%"=="Windows_NT" set "OS=" &:# This is the Windows' homonym default, same as undefined for us here.
if not defined OS set "OS=WIN32 WIN64" &:# Only WIN32 and WIN64 are supported so far.

:# Get the full pathname of the include subdirectories
if defined NMINCLUDE if not exist "%NMINCLUDE%\debugm.h" set "NMINCLUDE=" &:# Allow overriding with another instance, but ignore invalid overrides
if not defined NMINCLUDE set "NMINCLUDE=%~dp0\win32\NMaker\include" &:# Default: Use the local instance
if defined STINCLUDE if not exist "%STINCLUDE%\dict.h" set "STINCLUDE=" &:# Allow overriding with another instance, but ignore invalid overrides
if not defined STINCLUDE set "STINCLUDE=%~dp0\win32\include" &:# Default: Use the local instance

:# We need the 7-Zip LZMA SDK, from https://www.7-zip.org/sdk.html
%USE_SDK% LZMA
