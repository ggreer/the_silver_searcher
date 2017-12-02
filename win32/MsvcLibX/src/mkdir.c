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
*    2017-10-02 JFL Fixed support for pathnames >= 260 characters. 	      *
*    2017-10-04 JFL Improved the debugging output.			      *
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
|                   							      |
|   History								      |
|    2014-03-04 JFL Created this routine.                      		      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
|    2017-10-02 JFL Removed the dependency on PATH_MAX.			      |
|                   Added common routine mkdirM, called by mkdirA and mkdirU. |
*                                                                             *
\*---------------------------------------------------------------------------*/

#pragma warning(disable:4100) /* Ignore the "unreferenced formal parameter" warning */

int mkdirM(const char *pszName, mode_t iMode, UINT cp) {
  WCHAR *pwszName = NULL;
  BOOL bDone;
  int iErr = -1; /* Assume failure */

  DEBUG_ENTER(("mkdir(\"%s\", 0x%X)\n", pszName, iMode));

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(cp, pszName);
  if (!pwszName) goto cleanup_and_return;

  bDone = CreateDirectoryW(pwszName, NULL);
  if (!bDone) {
    errno = Win32ErrorToErrno();
  } else {
    iErr = 0; /* Success confirmed */
  }

cleanup_and_return:
  free(pwszName);
  RETURN_INT_COMMENT(iErr, (iErr ? "Failed. errno=%d - %s\n" : "Success\n", errno, strerror(errno)));
}

int mkdirU(const char *pszName, mode_t iMode) {
  return mkdirM(pszName, iMode, CP_UTF8);
}

int mkdirA(const char *pszName, mode_t iMode) {
  return mkdirM(pszName, iMode, CP_ACP);
}

#endif /* defined(_WIN32) */

