/*****************************************************************************\
*                                                                             *
*   Filename	    GetLongPathName.c					      *
*									      *
*   Description:    UTF-8 version of WIN32's GetLongPathName()		      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2016-09-12 JFL Created this file, from the routine in truename.c.	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#include <windows.h>	/* Also includes MsvcLibX' WIN32 UTF-8 extensions */
#include <limits.h>
#include "debugm.h"	/* MsvcLibX debugging macros */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    GetLongPathNameU					      |
|                                                                             |
|   Description:    Get the long pathname for a short UTF-8 path.	      |
|                                                                             |
|   Parameters:     See WIN32's GetLongPathName()			      |
|                                                                             |
|   Return value:   The length of the full pathname, or 0 if error	      |
|                                                                             |
|   Notes:								      |
|                                                                             |
|   History:								      |
|    2015-12-18 JFL Created this routine.				      |
*                                                                             *
\*---------------------------------------------------------------------------*/

DWORD WINAPI GetLongPathNameU(LPCTSTR lpShortName, LPTSTR lpBuf, DWORD nBufferLength) {
  WCHAR wszShortName[MAX_PATH];
  WCHAR wszBuf[MAX_PATH];
  int n;
  DWORD dwResult;

  DEBUG_ENTER(("GetLongPathNameU(\"%s\", %p, %d);\n", lpShortName, lpBuf, nBufferLength));

  n = MultiByteToWideChar(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  lpShortName,		/* lpMultiByteStr, */
			  lstrlen(lpShortName)+1,  /* cbMultiByte, */
			  wszShortName,		/* lpWideCharStr, */
			  MAX_PATH		/* cchWideChar, */
			  );
  if (!n) RETURN_INT_COMMENT(0, ("Failed to convert the short name to Unicode\n"));

  dwResult = GetLongPathNameW(wszShortName, wszBuf, MAX_PATH);
  if (!dwResult) RETURN_INT_COMMENT(0, ("GetLongPathNameW() failed\n"));

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
  if (!n) RETURN_INT_COMMENT(0, ("Failed to convert the Long name from Unicode\n"));

  RETURN_INT_COMMENT(n-1, ("\"%s\"\n", lpBuf));
}

#endif /* defined(_WIN32) */

