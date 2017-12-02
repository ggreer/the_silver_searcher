/*****************************************************************************\
*                                                                             *
*   Filename	    open.c						      *
*									      *
*   Description:    WIN32 UTF-8 version of open				      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2017-02-16 JFL Created this module.				      *
*    2017-03-12 JFL Restructured the UTF16 writing mechanism.		      *
*    2017-03-18 JFL Bug fix: Only change Xlation when writing to a valid file.*
*    2017-10-03 JFL Fixed support for pathnames >= 260 characters. 	      *
*                                                                             *
*         © Copyright 2017 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of printf routines */

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <errno.h>
#include "msvclibx.h"
#include "debugm.h"

#ifdef _WIN32

#include <windows.h>
#include <io.h>         /* For _setmode() */
#include <fcntl.h>      /* For I/O modes */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        open	                                              |
|                                                                             |
|   Description     UTF-8 or ANSI file name open(), with long path support    |
|                                                                             |
|   Parameters      char *pszName	File name			      |
|                   int iFlags		Opening mode			      |
|                   int iPerm		Permission mode for file creation     |
|                                                                             |
|   Returns         File number						      |
|                                                                             |
|   Notes           Prefixes long names with "\\?\" to enable long path suppt.|
|                                                                             |
|   History								      |
|    2017-02-16 JFL Created this routine.                      		      |
*                                                                             *
\*---------------------------------------------------------------------------*/

#define WRITE_MODE_MASK (_O_WRONLY | _O_RDWR | _O_APPEND)

int openM(UINT cp, const char *pszName, int iFlags, int iPerm) {
  WCHAR *pwszName;
  int iFile;
  int iMode;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(cp, pszName);
  if (!pwszName) return -1;

  /* return _wopen(wszName, iFlags, iPerm); */
  DEBUG_PRINTF(("_wopen(\"%s\", 0x%X, 0x%X);\n", pszName, iFlags, iPerm));
  iFile = _wopen(pwszName, iFlags, iPerm);
  free(pwszName);

  /* Find out the initial translation mode, and change it if appropriate */
  if (iFile >= 0) {
    int iWrite = 0;
    switch (iFlags & WRITE_MODE_MASK) {
    case _O_RDWR:
    case _O_WRONLY:
    case _O_APPEND:
      iWrite = 1;
    }
    if (iWrite) {
      iMode = _setmodeX(iFile, _O_TEXT);	/* Get the initial mode. Any mode can switch to _O_TEXT. */
      if ((iMode & _O_TEXT) && _isatty(iFile)) { /* If writing text to the console */
        iMode = _O_WTEXT;				/* Then write Unicode instead */
      }
      _setmodeX(iFile, iMode);		/* Restore the initial mode */
    }
  }

  DEBUG_PRINTF(("  return %d;\n", iFile));
  return iFile;
}

#pragma warning(disable:4212) /* Ignore the "nonstandard extension used : function declaration used ellipsis" warning */

int openA(const char *pszName, int iFlags, int iPerm) {
  return openM(CP_ACP, pszName, iFlags, iPerm);
}

int openU(const char *pszName, int iFlags, int iPerm) {
  return openM(CP_UTF8, pszName, iFlags, iPerm);
}

#pragma warning(default:4212) /* Restore the "nonstandard extension used : function declaration used ellipsis" warning */

#endif /* defined(_WIN32) */
