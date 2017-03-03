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

:# Pthread has its own nmakefile, that we do not want to use
set "IGNORE_NMAKEFILE=1"
