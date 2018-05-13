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
*    2018-04-24 JFL Define PATH_MAX and NAME_MAX for all OSs.		      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_LIMITS_H
#define	_MSVCLIBX_LIMITS_H	1

#include "msvclibx.h"

#include MSVC_INCLUDE_FILE(limits.h) /* Include MSVC's own <limits.h> file */

/*
  PATH_MAX	Maximum # of bytes needed for a pathname, including NUL
  NAME_MAX	Maximum # of bytes needed for a file name, NOT including NUL
  
  FILESIZEBITS	# of bits needed for file sizes
*/

/************************ MS-DOS-specific definitions ************************/

#ifdef _MSDOS	/* Automatically defined when targeting an MS-DOS application */

#define PATH_MAX 255	/* Many APIs actually limit it to 128 bytes, but longer paths are legal. */
#define NAME_MAX 12	/* MsvcLibX currently only supports 8.3 file names. */

#define FILESIZEBITS 32

#endif /* defined(_MSDOS) */

/************************ Win32-specific definitions *************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#undef PATH_MAX
#undef NAME_MAX

#define ANSI_PATH_MAX    260	/* Number of ANSI characters, including final NUL ( = Windef.h MAX_PATH) */
#define WIDE_PATH_MAX  32768	/* Number of Unicode characters, including final NUL */
#define UTF8_PATH_MAX (4 * WIDE_PATH_MAX) /* Worst UTF-8 case is 4 bytes / Unicode char */

#define ANSI_NAME_MAX    255	/* Number of ANSI characters, NOT including final NUL */
#define WIDE_NAME_MAX    255	/* Number of Unicode characters, NOT including final NUL */
#define UTF8_NAME_MAX (4 * WIDE_NAME_MAX) /* Worst UTF-8 case is 4 bytes / Unicode char */

#if defined(_UTF8_SOURCE)
#define PATH_MAX UTF8_PATH_MAX
#define NAME_MAX UTF8_PATH_MAX
#else /* ANSI source */
#define PATH_MAX WIDE_PATH_MAX /* MsvcLibX uses Unicode internally for file management */
#define NAME_MAX WIDE_PATH_MAX
#endif

#define FILESIZEBITS 64

#endif /* defined(_WIN32) */

/************************* OS/2-specific definitions *************************/

#ifdef _OS2	/* Automatically defined when targeting an OS/2 application? */

#define PATH_MAX CCHMAXPATH        /* FILENAME_MAX incorrect in stdio.h */
#define NAME_MAX CCHMAXPATHCOMP

#define FILESIZEBITS 32

#endif /* defined(_OS2) */

/********************** End of OS-specific definitions ***********************/

#endif /* defined(_MSVCLIBX_LIMITS_H)  */

