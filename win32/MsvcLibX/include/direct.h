/*****************************************************************************\
*                                                                             *
*   Filename:	    direct.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to direct.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-03-24 JFL Created this file, with content moved from unistd.h.      *
*    2015-11-15 JFL Visual Studio 2015 moved this file to the Windows Kit UCRT.
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_DIRECT_H
#define	_MSVCLIBX_DIRECT_H	1

#include "msvclibx.h"

#include UCRT_INCLUDE_FILE(direct.h) /* Include MSVC's own <direct.h> file */

#undef mkdir /* This MSVC macro is incompatible with mkdir() function in unistd.h */

#endif /* defined(_MSVCLIBX_DIRECT_H)  */

