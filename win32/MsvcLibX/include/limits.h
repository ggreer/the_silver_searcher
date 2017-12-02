/*****************************************************************************\
*                                                                             *
*   Filename:	    limits.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to limits.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-06-30 JFL Created this file.					      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_LIMITS_H
#define	_MSVCLIBX_LIMITS_H	1

#include "msvclibx.h"

#include MSVC_INCLUDE_FILE(limits.h) /* Include MSVC's own <limits.h> file */

/************************ MS-DOS-specific definitions ************************/

#ifdef _MSDOS	/* Automatically defined when targeting an MS-DOS application */

#define PATH_MAX 255	/* Many APIs actually limit it to 128 bytes, but longer paths paths are legal. */

#endif /* defined(_MSDOS) */

/************************ Win32-specific definitions *************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#undef PATH_MAX

#define ANSI_PATH_MAX      260	/* Number of ANSI characters, including final NUL ( = Windef.h MAX_PATH) */
#define UNICODE_PATH_MAX 32768	/* Number of Unicode characters, including final NUL */
#define UTF8_PATH_MAX (4 * UNICODE_PATH_MAX) /* Worst UTF-8 case is 4 bytes / Unicode char */

#define PATH_MAX UNICODE_PATH_MAX /* MsvcLibX uses Unicode internally for file management */

#endif /* defined(_WIN32) */

/************************* OS/2-specific definitions *************************/

#ifdef _OS2	/* Automatically defined when targeting an OS/2 application? */

#endif /* defined(_OS2) */

/********************** End of OS-specific definitions ***********************/

#endif /* defined(_MSVCLIBX_LIMITS_H)  */

