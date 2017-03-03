/*****************************************************************************\
*                                                                             *
*   Filename	    fopen.c						      *
*									      *
*   Description:    WIN32 UTF-8 version of fopen			      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-03-04 JFL Created this module.				      *
*    2014-07-01 JFL Added support for pathnames >= 260 characters. 	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_SOURCE /* Generate the UTF-8 version of printf routines */

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <stdio.h>
#include <errno.h>
#include "msvclibx.h"
#include "debugm.h"

#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        fopen	                                              |
|                                                                             |
|   Description     UTF-8 version of fopen				      |
|                                                                             |
|   Parameters      char *pszName	File name			      |
|                   char *pszMode	Opening mode			      |
|                                                                             |
|   Returns         File handle						      |
|                                                                             |
|   Notes                                                                     |
|                                                                             |
|   History								      |
|    2014-03-04 JFL Created this routine.                      		      |
|    2014-07-01 JFL Added support for pathnames >= 260 characters. 	      |
*                                                                             *
\*---------------------------------------------------------------------------*/

FILE *fopen(const char *pszName, const char *pszMode) {
  WCHAR wszName[UNICODE_PATH_MAX];
  WCHAR wszMode[20];
  int n;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  pszName,		/* lpMultiByteStr, */
			  wszName,		/* lpWideCharStr, */
			  COUNTOF(wszName)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    return NULL;
  }

  /* Convert the mode to a unicode string. This is not a pathname, so just do a plain conversion */
  n = MultiByteToWideChar(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pszMode,		/* lpMultiByteStr, */
			  lstrlen(pszMode)+1,	/* cbMultiByte, */
			  wszMode,		/* lpWideCharStr, */
			  COUNTOF(wszMode)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    return NULL;
  }

/*  return _wfopen(wszName, wszMode); */
{
  FILE *hFile;
  DEBUG_CODE({
    char szUtf8[UTF8_PATH_MAX];
    DEBUG_WSTR2UTF8(wszName, szUtf8, sizeof(szUtf8));
    DEBUG_PRINTF(("_wfopen(\"%s\", \"%s\");\n", szUtf8, pszMode));
  });
  hFile = _wfopen(wszName, wszMode);
  DEBUG_PRINTF(("  return 0x%p;\n", hFile));
  return hFile;
}
}

#endif /* defined(_WIN32) */

