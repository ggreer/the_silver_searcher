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
*    2017-03-12 JFL Restructured the UTF16 writing mechanism.		      *
*    2017-03-18 JFL Bug fix: Only change Xlation when writing to a valid file.*
*    2017-04-27 JFL Fixed 2017-03-18 bug: Failures returned a wrong errno.    *
*    2017-05-12 JFL Correctly indent the debug output.                        *
*    2017-10-03 JFL Fixed support for pathnames >= 260 characters. 	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of printf routines */

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <stdio.h>
#include <errno.h>
#include "msvclibx.h"
#include "debugm.h"

#ifdef _WIN32

#include <windows.h>
#include <io.h>		/* For _setmode() */
#include <fcntl.h>	/* For file I/O modes */

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
|    2017-10-03 JFL Removed the dependency on PATH_MAX and fixed size buffers.|
*                                                                             *
\*---------------------------------------------------------------------------*/

FILE *fopenM(const char *pszName, const char *pszMode, UINT cp) {
  WCHAR *pwszName;
  WCHAR wszMode[20];
  int n;
  FILE *hFile;
  int iFile;
  int iMode;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(cp, pszName);
  if (!pwszName) return NULL;

  /* Convert the mode to a unicode string. This is not a pathname, so just do a plain conversion */
  n = MultiByteToWideChar(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pszMode,		/* lpMultiByteStr, */
			  lstrlen(pszMode)+1,	/* cbMultiByte, */
			  wszMode,		/* lpWideCharStr, */
			  COUNTOF(wszMode)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    free(pwszName);
    return NULL;
  }

  /* return _wfopen(pwszName, wszMode); */
  DEBUG_CODE({
    char *pszUtf8;
    DEBUG_WSTR2NEWUTF8(pwszName, pszUtf8);
    DEBUG_ENTER(("_wfopen(\"%s\", \"%s\");\n", pszUtf8, pszMode));
    DEBUG_FREEUTF8(pszUtf8);
  });
  hFile = _wfopen(pwszName, wszMode);

  /* Find out the initial translation mode, and change it if appropriate */
  if (hFile && (strchr(pszMode, 'w') || strchr(pszMode, 'a'))) {
    iFile = fileno(hFile);
    iMode = _setmodeX(iFile, _O_TEXT);	/* Get the initial mode. Any mode can switch to _O_TEXT. */
    if ((iMode & _O_TEXT) && _isatty(iFile)) { /* If writing text to the console */
      iMode = _O_WTEXT;				/* Then write Unicode instead */
    }
    _setmodeX(iFile, iMode);		/* Restore the initial mode */
  }

  free(pwszName);
  DEBUG_LEAVE(("return 0x%p;\n", hFile));
  return hFile;
}

FILE *fopenU(const char *pszName, const char *pszMode) {
  return fopenM(pszName, pszMode, CP_UTF8);
}

FILE *fopenA(const char *pszName, const char *pszMode) {
  return fopenM(pszName, pszMode, CP_ACP);
}

#endif /* defined(_WIN32) */

