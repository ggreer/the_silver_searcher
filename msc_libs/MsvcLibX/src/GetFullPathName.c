/*****************************************************************************\
*                                                                             *
*   Filename	    GetFullPathName.c					      *
*									      *
*   Description:    UTF-8 version of WIN32's GetFullPathName()		      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2016-09-12 JFL Created this file, from the routine in truename.c.	      *
*    2017-03-20 JFL Include stdio.h, to get the UTF-8 version of printf.      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#define _UTF8_SOURCE

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ 2005 security warnings */

#include <windows.h>	/* Also includes MsvcLibX' WIN32 UTF-8 extensions */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "debugm.h"	/* MsvcLibX debugging macros */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    GetFullPathNameU					      |
|                                                                             |
|   Description:    Get the absolute pathname for a relative UTF-8 path.      |
|                                                                             |
|   Parameters:     See WIN32's GetFullPathName()			      |
|                                                                             |
|   Return value:   The length of the full pathname, or 0 if error	      |
|                                                                             |
|   Notes:	    Warning: Windows' GetFullPathName trims trailing dots and |
|		    spaces from the path.				      |
|		    This derived function reproduces the bug.		      |
|                   The caller MUST add trailing dots & spaces back if needed.|
|                                                                             |
|                   Warning: Windows' GetFullPathName returns unpredictable   |
|                   results when invoked in a multithreaded application.      |
|                   Windows' own doc recommends not using GetFullPathName in  |
|                   this case.                                                |
|                   Method 2 works around this limitation by doing its own    |
|                   path management.                                          |
|                                                                             |
|   History:								      |
|    2014-02-07 JFL Created this routine.				      |
*                                                                             *
\*---------------------------------------------------------------------------*/

DWORD WINAPI GetFullPathNameU(LPCTSTR lpName, DWORD nBufferLength, LPTSTR lpBuf, LPTSTR *lpFilePart) {
  WCHAR wszName[MAX_PATH];
  WCHAR wszBuf[MAX_PATH];
  char szName[MAX_PATH*4]; /* Worst case for UTF-8 is 4 bytes/Unicode character */
  int n;
  DWORD dwResult;
  WCHAR *wlpFilePart;

  DEBUG_ENTER(("GetFullPathNameU(\"%s\", %d, %p, %p);\n", lpName, (int)nBufferLength, lpBuf, lpFilePart));

  n = MultiByteToWideChar(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  lpName,		/* lpMultiByteStr, */
			  lstrlen(lpName)+1,	/* cbMultiByte, */
			  wszName,		/* lpWideCharStr, */
			  MAX_PATH		/* cchWideChar, */
			  );
  if (!n) RETURN_INT_COMMENT(0, ("Failed to convert the name to Unicode\n"));

  dwResult = GetFullPathNameW(wszName, MAX_PATH, wszBuf, &wlpFilePart);
  if (!dwResult) RETURN_INT_COMMENT(0, ("GetFullPathNameW() failed\n"));

  /* nRead = UnicodeToBytes(pwStr, len, buf, bufsize); */
  n = WideCharToMultiByte(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  wszBuf,		/* lpWideCharStr, */
			  (int)dwResult + 1,	/* cchWideChar, */
			  lpBuf,		/* lpMultiByteStr, */
			  (int)nBufferLength,	/* cbMultiByte, */
			  NULL,			/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) RETURN_INT_COMMENT(0, ("Failed to convert the full name from Unicode\n"));

  if (lpFilePart) { /* Convert the file part, and get the length of the converted string */
    int m;	/* Length of the converted string */
    m = WideCharToMultiByte(CP_UTF8,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			    0,				/* dwFlags, */
			    wlpFilePart,		/* lpWideCharStr, */
			    lstrlenW(wlpFilePart),	/* cchWideChar, */
			    szName,			/* lpMultiByteStr, */
			    sizeof(szName),		/* cbMultiByte, */
			    NULL,			/* lpDefaultChar, */
			    NULL			/* lpUsedDefaultChar */
			    );
    /* (n-1) is the length of the full UTF-8 pathname */
    /* So ((n-1) - m) is the offset of the file part in the full UTF-8 pathname */
    *lpFilePart = lpBuf + (n - 1) - m;
  }
  n -= 1; /* Do not count the final NUL */

  RETURN_INT_COMMENT(n, ("\"%s\" \"%s\"\n", lpBuf, lpFilePart?*lpFilePart:"(NULL)"));
}

#endif /* defined(_WIN32) */

