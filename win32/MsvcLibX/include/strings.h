/*****************************************************************************\
*                                                                             *
*   Filename:	    strings.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to strings.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2017-02-28 JFL Created this file.					      *
*    2019-02-17 JFL Actually there's no MS equivalent to include.	      *
*									      *
*         © Copyright 2017 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_strings_H
#define	_MSVCLIBX_strings_H	1

#include "msvclibx.h"

/* 2019-02-17 JFL Actually, the MS libraries do not contain an equivalent file */
/* #include UCRT_INCLUDE_FILE(strings.h) // Include MSVC's own <strings.h> file */

#ifdef __cplusplus
extern "C" {
#endif

/* Standard routines that have a Microsoft-specific equivalent */
#include <string.h>
#define strcasecmp _stricmp

/* These are standard routines, but Microsoft thinks not */

/************************ MS-DOS-specific definitions ************************/

#ifdef _MSDOS	/* Automatically defined when targeting an MS-DOS application */

#endif /* defined(_MSDOS) */

/************************ Win32-specific definitions *************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#endif /* defined(_WIN32) */

/********************** End of OS-specific definitions ***********************/

#ifdef __cplusplus
}
#endif

#endif /* defined(_MSVCLIBX_strings_H)  */

