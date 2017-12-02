/*****************************************************************************\
*                                                                             *
*   Filename:	    io.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to io.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2017-03-13 JFL Created this file.                                        *
*									      *
*         © Copyright 2017 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_IO_H
#define	_MSVCLIBX_IO_H	1

#include "msvclibx.h"

#include UCRT_INCLUDE_FILE(io.h) /* Include MSVC's own <io.h> file */

#if defined(_WIN32)
int _setmodeX(int iFile, int iMode);	/* Intercepts _setmode() calls */
#define _setmode _setmodeX		/* Intercept _setmode() calls */
#endif /* defined(_WIN32) */

#endif /* defined(_MSVCLIBX_IO_H)  */
