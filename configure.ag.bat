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
:#                                                                            *
:#*****************************************************************************

:# To avoid overwriting the existing WIN32 subdirectories in . and src and win32\zlib...
if not defined OUTDIR ( :# Unless already redirected elsewhere
  set "LOGDIR=."	&rem :# But leave the log file in the build directory itself.
)

:# Avoid trying to build ag.exe for DOS and WIN95, which are not supported yet.
if "%OS%"=="Windows_NT" set "OS=" &:# This is the Windows' homonym default, same as undefined for us here.
if not defined OS set "OS=WIN32 WIN64" &:# Only WIN32 and WIN64 are supported so far.

:# We need the 7-Zip LZMA SDK, from https://www.7-zip.org/sdk.html
%USE_SDK% LZMA
