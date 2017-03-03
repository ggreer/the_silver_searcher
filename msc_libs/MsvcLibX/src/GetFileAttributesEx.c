/*****************************************************************************\
*                                                                             *
*   Filename	    GetFileAttributesEx.c				      *
*									      *
*   Description:    UTF-8 version of WIN32's GetFileAttributesEx()	      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2016-09-12 JFL Created this file.					      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#include <windows.h>	/* Also includes MsvcLibX' WIN32 UTF-8 extensions */
#include <limits.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    GetFileAttributesExU				      |
|                                                                             |
|   Description:    Get file information for a UTF-8 pathname.		      |
|                                                                             |
|   Parameters:     See WIN32's GetFileAttributesEx()			      |
|                                                                             |
|   Return value:   1 = done, or 0 if error				      |
|                                                                             |
|   Notes:	    Sets the file length, dates, attributes, etc.	      |
|                                                                             |
|   History:								      |
|    2016-09-12 JFL Created this routine.				      |
*                                                                             *
\*---------------------------------------------------------------------------*/

BOOL WINAPI GetFileAttributesExU(
  LPCTSTR                lpFileName,
  GET_FILEEX_INFO_LEVELS fInfoLevelId,
  LPVOID                 lpFileInformation
) {
  WIN32_FILE_ATTRIBUTE_DATA *lpFileData = lpFileInformation;
  WCHAR *pwszName;
  BOOL bDone;
  int n;

  /* Allocate a buffer large enough for any Unicode pathname */
  pwszName = (void *)LocalAlloc(LMEM_FIXED, UNICODE_PATH_MAX * sizeof(WCHAR));
  if (!pwszName) return 0;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  lpFileName,		/* lpMultiByteStr, */
			  pwszName,		/* lpWideCharStr, */
			  UNICODE_PATH_MAX	/* cchWideChar, */
			  );
  if (!n) {
    LocalFree((HLOCAL)pwszName);
    return 0;
  }

  /* Get the file information, using the Unicode version of the function */
  bDone = GetFileAttributesExW(pwszName, fInfoLevelId, lpFileData);

  /* Cleanup and return */
  LocalFree((HLOCAL)pwszName);
  return bDone;
}

#endif /* defined(_WIN32) */

