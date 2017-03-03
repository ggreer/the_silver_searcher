/*****************************************************************************\
*                                                                             *
*   Filename	    mkdir.c						      *
*									      *
*   Description:    WIN32 UTF-8 version of mkdir			      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-03-04 JFL Created this module.				      *
*    2014-03-24 JFL Renamed "statx.h" as the standard <sys/stat.h>.	      *
*    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

/* Microsoft C libraries include files */
#include <stdio.h>
#include <direct.h>	/* For _mkdir() */
#include <errno.h>
/* MsvcLibX library extensions */
#include <sys/stat.h>
#include "msvclibx.h"

#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        mkdir	                                              |
|                                                                             |
|   Description     UTF-8 version of mkdir				      |
|                                                                             |
|   Parameters      char *pszName	File name			      |
|                   mode_t iMode	File permission bits		      |
|                                                                             |
|   Returns         0 = Success; -1 = Error and errno set.		      |
|                                                                             |
|   Notes                                                                     |
|                                                                             |
|   History								      |
|    2014-03-04 JFL Created this routine.                      		      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
*                                                                             *
\*---------------------------------------------------------------------------*/

#pragma warning(disable:4100) /* Ignore the "unreferenced formal parameter" warning */

int mkdirU(const char *pszName, mode_t iMode) {
  WCHAR wszName[PATH_MAX];
  int n;
  BOOL bDone;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  pszName,		/* lpMultiByteStr, */
			  wszName,		/* lpWideCharStr, */
			  COUNTOF(wszName)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    return -1;
  }

  bDone = CreateDirectoryW(wszName, NULL);
  if (!bDone) {
    errno = Win32ErrorToErrno();
    return -1;
  }
  return 0;
}

int mkdirA(const char *pszName, mode_t iMode) {
  WCHAR wszName[PATH_MAX];
  int n;
  BOOL bDone;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_ACP,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  pszName,		/* lpMultiByteStr, */
			  wszName,		/* lpWideCharStr, */
			  COUNTOF(wszName)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    return -1;
  }

  bDone = CreateDirectoryW(wszName, NULL);
  if (!bDone) {
    errno = Win32ErrorToErrno();
    return -1;
  }
  return 0;
}

#endif /* defined(_WIN32) */

