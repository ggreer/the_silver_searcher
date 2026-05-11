@echo off
:#*****************************************************************************
:#                                                                            *
:#  Filename:	    configure.include.bat				      *
:#                                                                            *
:#  Description:    Special make actions for recording the include dir. path  *
:#                                                                            *
:#  Notes:	                                                              *
:#                                                                            *
:#  History:                                                                  *
:#   2016-10-10 JFL jf.larvoire@hpe.com created this script.		      *
:#   2016-12-16 JFL Only use setx if requested by user, with PERSISTENT_VARS. *
:#   2026-02-22 JFL Define both STINCLUDE and NMINCLUDE.		      *
:#   2026-03-11 JFL Simplified, and adapted to the new NMINCLUDE location.    *
:#                                                                            *
:#         © Copyright 2016 Hewlett Packard Enterprise Development LP         *
:# Licensed under the Apache 2.0 license  www.apache.org/licenses/LICENSE-2.0 *
:#*****************************************************************************

:# Get the full pathname of the include subdirectories
if defined NMINCLUDE if not exist "%NMINCLUDE%\debugm.h" set "NMINCLUDE=" &:# Allow overriding with another instance, but ignore invalid overrides
if not defined NMINCLUDE set "NMINCLUDE=%~dp0\NMaker\include" &:# Default: Use the local instance
if defined STINCLUDE if not exist "%STINCLUDE%\dict.h" set "STINCLUDE=" &:# Allow overriding with another instance, but ignore invalid overrides
if not defined STINCLUDE set "STINCLUDE=%~dp0\include" &:# Default: Use the local instance

:# Set the system environment variable, so that other CMD windows opened later on inherit it
if defined PERSISTENT_VARS setx STINCLUDE "%STINCLUDE%" >NUL & setx NMINCLUDE "%NMINCLUDE%" >NUL

