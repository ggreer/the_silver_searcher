/*****************************************************************************\
*                                                                             *
*   Filename:	    stversion.h						      *
*                                                                             *
*   Description:    Get SysToolslib components versions			      *
*                                                                             *
*   Notes:	    Include this file in the main module of a program,        *
*                   to define the routine char *version(int idetailed).       *
*                                                                             *
*   History:								      *
*    2019-04-16 JFL jf.larvoire@hp.com created this file.                     *
*		    Moved here the OS identification strings from debugm.h.   *
*		    Added the definitions for the minimum OS version, and     *
*		    the processor name.					      *
*    2019-11-19 JFL Moved most version strings definitions to new versions.h. *
*		    							      *
*        (C) Copyright 2016 Hewlett Packard Enterprise Development LP         *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_STVERSION_H
#define	_STVERSION_H	1


#define SYSTOOLSLIB_VERSION         2020,5,20,0
#define SYSTOOLSLIB_VERSION_STR     "2020-05-20"
#define SYSTOOLSLIB_NAME            "System Tools Library"


/******************** OS identification string definition ********************/

#include "versions.h"

/**************** End of OS identification string definition *****************/

/* Name and short version. Used in SysToolsLib help screens */
#define PROGRAM_NAME_AND_VERSION					\
	"\n" PROGRAM_NAME EXE_SUFFIX					\
	/* " version " PROGRAM_VERSION */				\
        /* DEBUG_VERSION        If used SysToolsLib's debugm.h */	\
        AND_DEBUG_VERSION						\

#endif /* !defined(_STVERSION_H) */
