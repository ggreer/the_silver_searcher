/*****************************************************************************\
*                                                                             *
*   Filename	    mb2wpath.c						      *
*									      *
*   Description:    WIN32 utility routine MultiByteToWidePath()		      *
*                                                                             *
*   Notes:	    Used to overcome the 260-byte limitation of many Windows  *
*		    file management APIs.				      *
*		    							      *
*   History:								      *
*    2014-07-01 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of printf routines */

#include "msvclibx.h"
#include "debugm.h"

#if defined(_WIN32)

#include <windows.h>
#include <direct.h>  /* For _getdrive() */

int MultiByteToWidePath(
  UINT nCodePage,
  LPCSTR pszName,
  LPWSTR pwszName,
  int nWideBufSize
) {
  int n;
  int iNameLength = lstrlen(pszName);
  DEBUG_CODE(
    LPWSTR pwszName0 = pwszName;
  );

  if (iNameLength >= MAX_PATH) { /* Then processing this pathname requires prepending a special prefix */
    /* See http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx */
    if (!strncmp(pszName, "\\\\?\\", 4)) {		/* The name is in the Win32 file namespace */
      /* Do nothing, the "\\?\" extended-length prefix is already there */
    } else if (!strncmp(pszName, "\\\\.\\", 4)) {	/* The name is in the Win32 device namespace */
      /* Do nothing, devices names should not be changed */
    } else if (!strncmp(pszName, "\\\\", 2)) {		/* The name is a UNC path */
      /* Then prepend it with "\\?\UNC\" to get 32K Unicode paths instead of 260-byte ANSI paths */
      lstrcpyW(pwszName, L"\\\\?\\UNC\\");
      pwszName += 8;
      nWideBufSize -= 8;
      pszName += 2; /* Skip the initial \\ in the UNC name */
      iNameLength -= 2;
    } else if (pszName[0] == '\\') {	/* The name is an absolute path with no drive */
      /* Then prepend it with "\\?\" to get 32K Unicode paths instead of 260-byte ANSI paths */
      lstrcpyW(pwszName, L"\\\\?\\");
      /* And also add the drive, as it's required in the Win32 file namespace */
      pwszName[4] = L'@' + (wchar_t)_getdrive(); /* _getdrive() returns 1 for drive A, 2 for B, etc */
      pwszName[5] = L':';
      pwszName += 6;
      nWideBufSize -= 6;
    } else if (pszName[0] && (pszName[1] == ':') && (pszName[2] == '\\')) {	/* The name is an absolute path */
      /* Then prepend it with "\\?\" to get 32K Unicode paths instead of 260-byte ANSI paths */
      lstrcpyW(pwszName, L"\\\\?\\");
      pwszName += 4;
      nWideBufSize -= 4;
    } /* Else this is a relative pathname. Extended-length is not supported for them. */
  }

  n = MultiByteToWideChar(nCodePage,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pszName,		/* lpMultiByteStr, */
			  iNameLength+1,	/* cbMultiByte, */ /* +1 to include the final NUL */
			  pwszName,		/* lpWideCharStr, */
			  nWideBufSize		/* cchWideChar, */
			  );
  DEBUG_CODE(
    if (pwszName != pwszName0) {
      char szUtf8[UTF8_PATH_MAX];
      DEBUG_WSTR2UTF8(pwszName0, szUtf8, sizeof(szUtf8));
      DEBUG_PRINTF(("MultiByteToWidePath(); // Long name changed to \"%s\"\n", szUtf8));
    }
  );
  return n;
}

#endif /* defined(_WIN32) */

