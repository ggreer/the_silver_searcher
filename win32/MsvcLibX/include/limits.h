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
*    2021-11-07 JFL Added TS 18661-1:2014 integer types widths macros.        *
*    2022-12-11 JFL Define SIZE_MAX & SSIZE_MAX if needed.                    *
*    2023-11-11 JFL Changed DOS PATH_MAX from 255 to 1024 bytes for testing.  *
*		    Added CODE_PTR_WIDTH and DATA_PTR_WIDTH macros.	      *
*    2025-11-25 JFL Changed DOS PATH_MAX from 1024 to 260 bytes. It's enough. *
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

#define PATH_MAX 260	/* Many APIs actually limit it to 64 or 128 bytes, but longer paths are legal, and do occur */
			/* In DOS 7, Get Volume Information returns PATH_MAX <= 260 for all FS types I tested */
#define NAME_MAX 12	/* MsvcLibX currently only supports 8.3 file names. */

#define FILESIZEBITS 32

#define CHAR_WIDTH	8
#define SCHAR_WIDTH	8
#define UCHAR_WIDTH	8
#define SHRT_WIDTH	16
#define USHRT_WIDTH	16
#define INT_WIDTH	16
#define UINT_WIDTH	16
#define LONG_WIDTH	32
#define ULONG_WIDTH	32

#if defined(_M_I86TM) || defined(_M_I86SM) || defined(_M_I86CM)
#  define CODE_PTR_WIDTH 16	/* Memory models with short code pointers */
#else /* defined(_M_I86MM) || defined(_M_I86LM) || defined(_M_I86HM) */
#  define CODE_PTR_WIDTH 32	/* Memory models with long code pointers */
#endif

#if defined(_M_I86TM) || defined(_M_I86SM) || defined(_M_I86MM)
#  define DATA_PTR_WIDTH 16	/* Memory models with short data pointers */
#else /* defined(_M_I86CM) || defined(_M_I86LM) || defined(_M_I86HM) */
#  define DATA_PTR_WIDTH 32	/* Memory models with long data pointers */
#endif

#ifndef SIZE_MAX /* This is the case in MSVC 1.x for DOS */
#  ifdef _M_I86HM	/* Huge memory model */
#    define SIZE_MAX  ULONG_MAX
#    define SSIZE_MAX  LONG_MAX
#  else		/* All other memory models */
#    define SIZE_MAX   UINT_MAX
#    define SSIZE_MAX   INT_MAX
#  endif
#endif

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

#define CHAR_WIDTH	8
#define SCHAR_WIDTH	8
#define UCHAR_WIDTH	8
#define SHRT_WIDTH	16
#define USHRT_WIDTH	16
#define INT_WIDTH	32
#define UINT_WIDTH	32
#define LONG_WIDTH	32
#define ULONG_WIDTH	32
#define LLONG_WIDTH	64
#define ULLONG_WIDTH	64

#ifdef _WIN64	/* 64-bit versions of Windows */
#  define CODE_PTR_WIDTH   64
#  define DATA_PTR_WIDTH   64
#else		/* 32-bit versions of Windows */
#  define CODE_PTR_WIDTH   32
#  define DATA_PTR_WIDTH   32
#endif

#ifndef SSIZE_MAX
#  ifdef _WIN64	/* 64-bit versions of Windows */
#    define SSIZE_MAX _I64_MAX
#  else		/* 32-bit versions of Windows */
#    define SSIZE_MAX  INT_MAX
#  endif
#endif

#endif /* defined(_WIN32) */

/************************* OS/2-specific definitions *************************/

#ifdef _OS2	/* Automatically defined when targeting an OS/2 application? */

#define PATH_MAX CCHMAXPATH        /* FILENAME_MAX incorrect in stdio.h */
#define NAME_MAX CCHMAXPATHCOMP

#define FILESIZEBITS 32

#endif /* defined(_OS2) */

/********************** End of OS-specific definitions ***********************/

#ifndef SIZE_MAX
#  error "Unexpected case with SIZE_MAX undefined"
#endif
#ifndef SSIZE_MAX
#  error "Unexpected case with SSIZE_MAX undefined"
#endif

#endif /* defined(_MSVCLIBX_LIMITS_H)  */

