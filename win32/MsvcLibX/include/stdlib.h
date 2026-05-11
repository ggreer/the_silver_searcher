/*****************************************************************************\
*                                                                             *
*   Filename:	    stdlib.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to stdlib.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2016-09-13 JFL Created this file.					      *
*    2020-03-29 JFL Added mkstmp() definitions.				      *
*    2025-08-07 JFL Added getenv() definitions.				      *
*    2025-08-10 JFL Added setenv() definitions.				      *
*    2025-08-15 JFL Redefine unsetenv() to a more standard definition.	      *
*    2026-01-23 JFL Added realpath() definitions.			      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_stdlib_H
#define	_MSVCLIBX_stdlib_H	1

#include "msvclibx.h"

#include UCRT_INCLUDE_FILE(stdlib.h) /* Include MSVC's own <stdlib.h> file */

#ifdef __cplusplus
extern "C" {
#endif

/************************ MS-DOS-specific definitions ************************/

#ifdef _MSDOS	/* Automatically defined when targeting an MS-DOS application */

extern char *mkdtemp(char *pszTemplate); /* Create a temporary directory */
extern int mkstemp(char *pszTemplate);	 /* Create a temporary file */

/* MSVC defines the System V's _putenv() routine, but not BSD's setenv() */
int setenv(const char *pszName, const char *pszValue, int iOverwrite);

char *realpath(const char *path, char *outbuf);

#endif /* defined(_MSDOS) */

/************************ Win32-specific definitions *************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#include <windows.h>

extern char *_fullpathU(char *absPath, const char *relPath, size_t maxLength);

#if defined(_UTF8_SOURCE)
#define _fullpath _fullpathU		/* For processing UTF-8 pathnames */
/* Else use MSVC lib's _fullpath, which supports ANSI names by default */
#endif

/* Create a temporary directory */
extern char *mkdtempM(char *pszTemplate, UINT cp); /* Multi-encoding version */
extern char *mkdtempA(char *pszTemplate);	   /* ANSI version */
extern char *mkdtempU(char *pszTemplate);	   /* UTF-8 version */

/* Create a temporary file */
extern int mkstempM(char *pszTemplate, UINT cp);   /* Multi-encoding version */
extern int mkstempA(char *pszTemplate);		   /* ANSI version */
extern int mkstempU(char *pszTemplate);		   /* UTF-8 version */

#if defined(_UTF8_SOURCE)
#define mkdtemp mkdtempU		/* For processing UTF-8 pathnames */
#define mkstemp mkstempU		/* For processing UTF-8 pathnames */
#else
#define mkdtemp mkdtempA		/* For processing ANSI pathnames */
#define mkstemp mkstempA		/* For processing ANSI pathnames */
#endif

/* Code-page-specific versions of getenv() */
char *getenvM(const char *name, UINT cp);
#define getenvA(name) getenvM(name, CP_ACP)
#define getenvU(name) getenvM(name, CP_UTF8)
#undef getenv
#if defined(_UTF8_SOURCE)
#define getenv getenvU			/* For getting UTF-8 values */
#else
#define getenv getenvA			/* For getting ANSI values */
#endif

/* Code-page-specific versions of setenv() */
/* MSVC defines the System V's _putenv() routine, but not BSD's setenv() */
int setenvM(const char *pszName, const char *pszValue, int iOverwrite, UINT cp);
#define setenvA(name, value, rw) setenvM(name, value, rw, CP_ACP)
#define setenvU(name, value, rw) setenvM(name, value, rw, CP_UTF8)
#undef setenv
#if defined(_UTF8_SOURCE)
#define setenv setenvU			/* For getting UTF-8 values */
#else
#define setenv setenvA			/* For getting ANSI values */
#endif

/* Resolve all links in a pathname */
WCHAR *realpathW(const WCHAR *wpath, WCHAR *wbuf);		/* Posix routine realpath - Wide char version */
char *realpathM(const char *path, char *buf, UINT cp);		/* Posix routine realpath - Multibyte char version */
#define realpathA(path, buf) realpathM(path, buf, CP_ACP)	/* Posix routine realpath - ANSI version */
#define realpathU(path, buf) realpathM(path, buf, CP_UTF8)	/* Posix routine realpath - UTF-8 version */
#if defined(_UTF8_SOURCE)
#define realpath realpathU		/* For resolving UTF-8 pathnames */
#else
#define realpath realpathA		/* For resolving ANSI pathnames */
#endif

#endif /* defined(_WIN32) */

/********************** End of OS-specific definitions ***********************/

/* MSVC defines the System V's _putenv() routine, but not BSD's setenv() */
#define putenv _putenv
#define unsetenv(name) setenv(name, NULL, 1);

#ifdef __cplusplus
}
#endif

#endif /* defined(_MSVCLIBX_stdlib_H)  */

