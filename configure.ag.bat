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

:# To avoid overwriting the existing WIN32 subdirectories in msc_libs\zlib and ag\src,
:# output all build files in the bin subdirectory.
if not defined OUTDIR set "OUTDIR=bin"
