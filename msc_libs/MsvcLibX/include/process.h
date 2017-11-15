/*****************************************************************************\
*                                                                             *
*   Filename:	    process.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to process.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-03-27 JFL Created this file.                                        *
*    2015-11-15 JFL Visual Studio 2015 moved this file to the Windows Kit UCRT.
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_PROCESS_H
#define	_MSVCLIBX_PROCESS_H	1

#include "msvclibx.h"

#include UCRT_INCLUDE_FILE(process.h) /* Include MSVC's own <process.h> file */

#if defined(_WIN32)
extern intptr_t _spawnvpU(int iMode, const char *pszCommand, char *const *argv);
#if defined(_UTF8_SOURCE)
#define _spawnvp _spawnvpU
#else /* _ANSI_SOURCE */
#define _spawnvp _spawnvp
#endif /* defined(_UTF8_SOURCE) */
#endif /* defined(_WIN32) */

#endif /* defined(_MSVCLIBX_PROCESS_H)  */


