@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename:	    configure.msc.bat					      *
:#                                                                            *
:#  Description:    Define ag MSVC-specific configuration settings	      *
:#                                                                            *
:#  Notes:	                                                              *
:#                                                                            *
:#  History:                                                                  *
:#   2017-02-14 JFL Created this file.					      *
:#                                                                            *
:#*****************************************************************************

:# First search in Ag's wincompat libraries
set "MY_SDKS=..\msc_libs %MY_SDKS%"

:# Configure.bat does not know about these libraries
set "SDK.PCRE.NAME=Perl Compatible Regular Expressions"
set "SDK.PCRE.FILE=pcre.h.in"	  &:# A unique file at the base of that library tree
set "SDK.PCRE.INCPATH=."	  &:# Include files are in the base directory
set "SDK.PCRE.LIBPATH=$(BR)"	  &:# $(BR) = SysToolsLib OS-specific output directory

set "SDK.PTHREAD.NAME=Posix Threads for Windows"
set "SDK.PTHREAD.FILE=pthread.h"  &:# A unique file at the base of that library tree
set "SDK.PTHREAD.INCPATH=."	  &:# Include files are in the base directory
set "SDK.PTHREAD.LIBPATH=$(BR)"   &:# $(BR) = SysToolsLib OS-specific output directory

set "SDK.ZLIB.NAME=General Purpose Compression Library"
set "SDK.ZLIB.FILE=zlib.h"	  &:# A unique file at the base of that library tree
set "SDK.ZLIB.INCPATH=."	  &:# Include files are in the base directory
set "SDK.ZLIB.LIBPATH=$(BR)"	  &:# $(BR) = SysToolsLib OS-specific output directory

:# Declare the SDKs and libraries we need
%BEGIN_SDK_DEFS%
%USE_SDK% MSVCLIBX
%USE_SDK% PCRE
%USE_SDK% PTHREAD
%USE_SDK% ZLIB
%END_SDK_DEFS%
