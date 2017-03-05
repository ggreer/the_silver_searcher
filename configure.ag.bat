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

:# To avoid overwriting the existing WIN32 subdirectories in msc_libs\zlib and ag\src...
if not defined OUTDIR ( :# Unless already redirected elsewhere
  set "OUTDIR=bin"	&rem :# Output all built files in the bin subdirectory. (Instead of the default WIN32 and WIN64)
  set "LOGDIR=."	&rem :# But leave the log file in the build directory itself.
)
