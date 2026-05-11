/*****************************************************************************\
*                                                                             *
*   Filename	    errno.h						      *
*                                                                             *
*   Description     MsvcLibX extensions to errno.h.			      *
*                                                                             *
*   Notes	    							      *
*                                                                             *
*   History								      *
*    2021-12-12 JFL Created this file.					      *
*    2022-12-12 JFL Added a definition for EOVERFLOW.			      *
*    2026-01-18 JFL Added a definition for ENOTSUP.			      *
*    2026-02-19 JFL Added a prototype for MlxGetWin32ErrorMessage().	      *
*									      *
*         © Copyright 2021 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_errno_H
#define	_MSVCLIBX_errno_H	1

#include "msvclibx.h"

#ifndef _INC_ERRNO	/* #include guard macro defined by MSVC errno.h */

#ifdef errno	/* Also defined in stddef.h, in which case the UCRT_INCLUDE_FILE() macro fails */
#undef errno		  /* It'll be redefined identically in errno.h anyway */
#undef _CRT_ERRNO_DEFINED /* _CRT_ERRNO_DEFINED would prevent that redefinition */
#endif		/* And now we're sure that the UCRT_INCLUDE_FILE() macro will work properly */

#include UCRT_INCLUDE_FILE(errno.h) /* Include MSVC's own <errno.h> file */

#endif /* defined(_INC_ERRNO) */

#ifdef __cplusplus
extern "C" {
#endif

/************************ MS-DOS-specific definitions ************************/

#ifdef _MSDOS	/* Automatically defined when targeting an MS-DOS application */

/*
// MS Visual C++ 1.52 for DOS is standard up to errno 34, then diverges up to errno 36.
// Many errnos within the list are actually unused, and for them _sys_errlist[] = "".
// List of apparently unused errnos, pointing to "": 1 3-6 10-11 14-16 19-21 23 25-27 29-32 35
// _sys_nerr = 37
// For all the above, and all codes >= 37, strerror() returns the string 37 "Unknown error".
*/

/*
// The _sys_errlist[] table can be updated in MSVC 16-bits libraries.
// But it's not possible to add any message for codes >= 37.
// Also it's probably unsafe to add messages longer than 29 characters, as strerror()
// copies them to a common static buffer before returning the pointer to that buffer.
// That buffer size is unknown, but the longest message is "Resource deadlock would occur", 29 chars.
*/

/* Update the _sys_errlist[] table whenever adding a new error code below. */

#ifndef ENOSYS		/* System function not implemented */
#define ENOSYS 1
#endif

#ifndef ENOTSUP		/* Operation not supported */
#define ENOTSUP 3
#endif

#ifndef ENAMETOOLONG /* Not defined in MSVC 1.5 errno.h, but defined in winsock.h */
#define ENAMETOOLONG 4 /* It's 38 in WIN32, but 38 > _sys_nerr here */
#endif

#endif /* defined(_MSDOS) */

/************************ Win32-specific definitions *************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#include "sys/types.h" /* For DWORD definition */

/* Library-specific routine used internally by many standard routines */
extern char *MlxGetWin32ErrorMessage(DWORD dwError); /* Must be freed with LocalFree() */
extern int Win32ErrorToErrno(); /* Converts the last WIN32 error to a Posix error code */

/*
// MS Visual C++ 9 for Windows is standard up to errno 34, then diverges up to errno 42.
// Also MSVC9 does not define errno:
//  15      // The posix standard ENOTBLK "Block device required"
//  26      // The posix standard ETXTBSY "Text file busy"
//  35      // Positioned between standard ERANGE and EDEADLK
//  37      // Positioned between standard EDEADLK and ENAMETOOLONG
// _sys_nerr = 43
// For all the above, and all codes >= 43, strerror() returns the string 43 "Unknown error".
*/

/*
// The _sys_errlist[] table _cannot_ be updated in MS WIN32 C libraries. Instead,
// update MsvcLibX's strerror.c with new error messages whenever adding a new error code below.
*/

#ifndef ELOOP
/*
// Unix defines ELOOP as errno 40.
// MS Visual C++ 10 and later define ELOOP as 114.
*/
#define ELOOP  35	/* ELOOP error message added in strerror.c */
#endif

#ifndef ENOTSUP		/* Operation not supported */
#define ENOTSUP 37	/* ENOTSUP error message added in strerror.c */
#endif

#endif /* defined(_WIN32) */

/********************** End of OS-specific definitions ***********************/

#ifndef EOVERFLOW /* Value too large to be stored in data type */
#define EOVERFLOW ERANGE /* ERANGE is "Result too large". TODO: Find a unique value & adapt strerror() */
#endif

#ifdef __cplusplus
}
#endif

#endif /* defined(_MSVCLIBX_errno_H)  */

