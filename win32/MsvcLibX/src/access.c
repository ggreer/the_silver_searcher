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
*    2017-10-03 JFL Fixed support for pathnames >= 260 characters. 	      *
*    2018-03-22 JFL Added support for access mode X_OK.			      *
*                                                                             *
*       © Copyright 2016-2018 Hewlett Packard Enterprise Development LP       *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <unistd.h>
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
|    2017-10-03 JFL Removed the dependency on PATH_MAX and fixed size buffers.|
|                   Added routine accessM, called by accessA and accessU.     |
*                                                                             *
\*---------------------------------------------------------------------------*/

int _accessM(const char *pszName, int iMode, UINT cp) {
  WCHAR *pwszName;
  int iAccess;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(cp, pszName);
  if (!pwszName) return -1;

  /* The C library returns errno=EINVAL for iMode X_OK.
     As all files are executable in Windows, change it to an existence test. */
  if (iMode == X_OK) iMode = F_OK;

  iAccess = _waccess(pwszName, iMode);
  free(pwszName);
  return iAccess;
}

int _accessA(const char *pszName, int iMode) {
  return _accessM(pszName, iMode, CP_ACP);
}

int _accessU(const char *pszName, int iMode) {
  return _accessM(pszName, iMode, CP_UTF8);
}

#endif /* defined(_WIN32) */

