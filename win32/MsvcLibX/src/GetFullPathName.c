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
*    2017-10-02 JFL Fixed support for pathnames >= 260 characters.	      *
*    2017-10-25 JFL Fixed again support for pathnames >= 260 characters.      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#define _UTF8_LIB_SOURCE

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
|                   							      |
|   History:								      |
|    2014-02-07 JFL Created this routine.				      |
|    2017-10-02 JFL Removed the pathname length limitation. Also use far less |
|                   stack memory.					      |
*                                                                             *
\*---------------------------------------------------------------------------*/

DWORD WINAPI GetFullPathNameU(LPCTSTR pszName, DWORD nBufferLength, LPTSTR lpBuf, LPTSTR *lpFilePart) {
  WCHAR *pwszName = NULL;
  WCHAR *pwBuf = NULL;
  int n;
  int lResult;
  WCHAR *pwszFilePart;
  int lName = lstrlen(pszName);
  int lNameNul = lName + 1;
  int lwBuf = lNameNul + MAX_PATH; /* In most cases, the current directory will be shorter than MAX_PATH. If not, the buffer will be extended below. */

  DEBUG_ENTER(("GetFullPathNameU(\"%s\", %d, %p, %p);\n", pszName, (int)nBufferLength, lpBuf, lpFilePart));

  pwszName = MultiByteToNewWidePath(CP_UTF8, pszName);
  if (!pwszName) {
out_of_mem:
    RETURN_INT_COMMENT(0, ("Not enough memory\n"));
  }

realloc_wBuf:
  pwBuf = GlobalAlloc(GMEM_FIXED, sizeof(WCHAR) * lwBuf);
  if (!pwBuf) {
    free(pwszName);
    goto out_of_mem;
  }
  lResult = (int)GetFullPathNameW(pwszName, lwBuf, pwBuf, &pwszFilePart);
  if (lResult > lwBuf) { /* In the possible but unlikely case that the result does not fit in pwBuf, */
    GlobalFree(pwBuf);	 /* then extend the buffer and retry. */
    lwBuf = lResult;
    goto realloc_wBuf;
  }
  free(pwszName);	 /* We won't need this buffer anymore */
  if (!lResult) {
    GlobalFree(pwBuf);
    RETURN_INT_COMMENT(0, ("GetFullPathNameW() failed\n"));
  }

  /* nRead = UnicodeToBytes(pwStr, len, buf, bufsize); */
  n = WideCharToMultiByte(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pwBuf,		/* lpWideCharStr, */
			  lResult + 1,		/* cchWideChar, */
			  lpBuf,		/* lpMultiByteStr, */
			  (int)nBufferLength,	/* cbMultiByte, */
			  NULL,			/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) {
    GlobalFree(pwBuf);
    RETURN_INT_COMMENT(0, ("Failed to convert the full name from Unicode\n"));
  }
  n -= 1; /* Do not count the final NUL */

  /* Remove the long pathname \\?\ prefix, if it was not there before */
  if ((!strncmp(lpBuf, "\\\\?\\", 4)) && strncmp(pszName, "\\\\?\\", 4)) {
    n = TrimLongPathPrefix(lpBuf);
  }

  if (lpFilePart) { /* Convert the file part, and get the length of the converted string */
    int m;	/* Length of the converted string */
    char *pName;
    lName = lstrlenW(pwszFilePart);		/* This accesses pwBuf. Do not free it above! */
    pName = GlobalAlloc(GMEM_FIXED, lName*4);	/* Worst case for UTF-8 is 4 bytes/Unicode character */
    if (!pName) {
      GlobalFree(pwBuf);
      goto out_of_mem;
    }
    m = WideCharToMultiByte(CP_UTF8,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			    0,				/* dwFlags, */
			    pwszFilePart,		/* lpWideCharStr, */
			    lName,			/* cchWideChar, */
			    pName,			/* lpMultiByteStr, */
			    lName*4,			/* cbMultiByte, */
			    NULL,			/* lpDefaultChar, */
			    NULL			/* lpUsedDefaultChar */
			    );
    /* n is the length of the full UTF-8 pathname */
    /* So (n - m) is the offset of the file part in the full UTF-8 pathname */
    *lpFilePart = lpBuf + n - m;
    GlobalFree(pName);
  }

  GlobalFree(pwBuf);
  RETURN_INT_COMMENT(n, ("\"%s\" \"%s\"\n", lpBuf, lpFilePart?*lpFilePart:"(NULL)"));
}

#endif /* defined(_WIN32) */

