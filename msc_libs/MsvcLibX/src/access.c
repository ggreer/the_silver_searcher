/*****************************************************************************\
*                                                                             *
*   Filename	    access.c						      *
*									      *
*   Description:    WIN32 UTF-8 version of access			      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-03-24 JFL Created this module.				      *
*    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <stdio.h>
#include <errno.h>
#include <io.h>
#include "msvclibx.h"
#include "debugm.h"

#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        access	                                              |
|                                                                             |
|   Description     UTF-8 version of access				      |
|                                                                             |
|   Parameters      char *pszName	File name			      |
|                   int iMode		Access mode to test		      |
|                                                                             |
|   Returns         File handle						      |
|                                                                             |
|   Notes                                                                     |
|                                                                             |
|   History								      |
|    2014-03-24 JFL Created this routine.                      		      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
*                                                                             *
\*---------------------------------------------------------------------------*/

int _accessU(const char *pszName, int iMode) {
  WCHAR wszName[PATH_MAX];
  int n;

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

  return _waccess(wszName, iMode);
}

#endif /* defined(_WIN32) */

