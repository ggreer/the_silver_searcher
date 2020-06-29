/*****************************************************************************\
*                                                                             *
*   Filename:	    fcntl.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to fcntl.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2017-02-16 JFL Created this file.                                        *
*    2020-03-29 JFL Added prototype for openM().                              *
*									      *
*         © Copyright 2017 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_FCNTL_H
#define	_MSVCLIBX_FCNTL_H	1

#include "msvclibx.h"

#include UCRT_INCLUDE_FILE(fcntl.h) /* Include MSVC's own <fcntl.h> file */

/* Microsoft defines _open() in io.h */
#include <io.h>

#if defined(_MSDOS)
#define open _open
#endif

#if defined(_WIN32)
extern int openM(UINT cp, const char *pszName, int iFlags, int iPerm); /* MsvcLibX Multi-encoding version of open */
extern int openA(const char *, int, ...); /* MsvcLibX ANSI version of open */
extern int openU(const char *, int, ...); /* MsvcLibX UTF-8 version of open */
#if defined(_UTF8_SOURCE)
#define open openU
#else /* _ANSI_SOURCE */
#define open openA
#endif /* defined(_UTF8_SOURCE) */
#endif /* defined(_WIN32) */

#endif /* defined(_MSVCLIBX_FCNTL_H)  */
