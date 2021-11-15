/*****************************************************************************\
*                                                                             *
*   Filename	    getcwd.c						      *
*									      *
*   Description:    WIN32 port of standard C library's getcwd()		      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-02-28 JFL Created this module.				      *
*    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      *
*    2017-10-04 JFL Fixed support for pathnames >= 260 characters. 	      *
*    2018-04-25 JFL Added routine getcwdW().                       	      *
*		    Use the locally managed cur. dir. for paths > 260 bytes.  *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
#include <string.h>
/* MsvcLibX library extensions */
#include <unistd.h>
#include "debugm.h"

#if defined(_MSDOS)

/* DOS own getcwd() is OK. */

#endif /* defined(_MSDOS) */


#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    getcwdW / getcwdM / getcwdA / getcwdU		      |
|									      |
|   Description:    Get the current directory, in UTF-16 / MBCS / ANSI / UTF-8|
|									      |
|   Parameters:     char *buf	    Buffer for the output                     |
|		    size_t bufSize  Buffer size				      |
|		    							      |
|   Returns:	    The converted string size. -1=error, and errno set.	      |
|		    							      |
|   Notes:	    The getcwd macro references either getcwdA or getcwdU.    |
|		    							      |
|		    Contrary to most other WIN32 APIs, SetCurrentDirectoryW() |
|		    does NOT allow extending the path length beyond 260 bytes |
|		    by prepending a \\?\ prefix.			      |
|		    https://stackoverflow.com/a/44519069/2215591	      |
|		    							      |
|		    In Windows 10 version 1607 and later, this can be fixed   |
|		    by using a manifest defining longPathAware=true, AND      |
|		    by setting the registry value LongPathsEnabled to 1 in    |
|		    HKLM\SYSTEM\CurrentControlSet\Control\FileSystem.         |
|		    Using both lifts the 260-bytes WIN32 APIs limitation.     |
|		    							      |
|		    If support for long path lengths in older versions of     |
|		    Windows is desired (XP to 8), then avoid using chdir() or |
|		    SetCurrentDirectoryW().				      |
|		    							      |
|		    As a weak workaround, for paths longer than 260 bytes,    |
|		    the chdir routine locally manages the current directory.  |
|		    No attempt is made to manage multiple drive-specific      |
|		    current directories, as the goal is Unix-compatibility,   |
|		    not Windows compatibility.				      |
|		    							      |
|   History:								      |
|    2014-02-28 JFL Created this routine                               	      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
|    2017-10-03 JFL Removed the dependency on PATH_MAX and fixed size buffers.|
|                   Added routine getcwdM, called by getcwdA and getcwdU.     |
|    2017-10-04 JFL Remove the long pathname prefix, if any.		      |
|    2018-04-25 JFL Added routine getcwdW(), and use it in getcwdM().	      |
|		    Use the locally managed cur. dir. for paths > 260 bytes.  |
|		    Bugfix: Must test for \\?\UNC\ before testing for \\?\.   |
*									      *
\*---------------------------------------------------------------------------*/

extern WCHAR *pwszLongCurrentDir; /* Locally managed CD, defined in chdir.c */

WCHAR *getcwdW(WCHAR *pwBuf, size_t dwBufSize) {
  DWORD dwSize;
  WCHAR *pwDir;
  int iAlloc = FALSE;	/* TRUE if pwBuf allocated here */
  int iReAlloc = FALSE;	/* TRUE if pwBuf should be reallocated in the end */
  DEBUG_CODE(
  WCHAR *pwszWhere = L"Windows";
  )

  DEBUG_ENTER(("getcwd(0x%p, %Iu);\n", pwBuf, dwBufSize));

  if (!pwBuf) {
    iAlloc = TRUE;
    if (!dwBufSize) {
      iReAlloc = TRUE;
      dwBufSize = WIDE_PATH_MAX;
    }
    pwBuf = malloc(dwBufSize * sizeof(WCHAR));
    if (!pwBuf) return NULL;
  }
  if (pwszLongCurrentDir) {		/* If we manage the CD locally */
    dwSize = lstrlenW(pwszLongCurrentDir);	/* Then copy that CD */
    if (dwSize < dwBufSize) {
      lstrcpyW(pwBuf, pwszLongCurrentDir);
    } else {
      dwSize += 1; /* Buffer size needed */
    }
    DEBUG_CODE(
    pwszWhere = L"MsvcLibX";
    )
  } else {					/* Else ask Windows' */
    dwSize = GetCurrentDirectoryW((DWORD)dwBufSize, pwBuf);
  }
  if (dwSize > dwBufSize) { /* The buffer is too small. dwSize = the size needed */
    DEBUG_LEAVE(("return NULL; // Error: The buffer is too small. %d bytes needed.\n", dwSize));
getcwdW_failed:
    errno = Win32ErrorToErrno();
    if (iAlloc) free(pwBuf);
    return NULL;
  }
  if (!dwSize) {
    DEBUG_LEAVE(("return NULL; // Error: GetCurrentDirectoryW() Failed\n"));
    goto getcwdW_failed;
  }

  /* Remove the long pathname prefix, if any */
  pwDir = pwBuf;
  if (!strncmpW(pwBuf, L"\\\\?\\UNC\\", 8)) {
    pwDir += 6;		/* Remove the '\\?\UNC\' prefix, except for the final two characters */
    dwSize -= 6;
    *pwDir = L'\\';	/* Change the 'C' to '\', so that the output begins by '\\server\share' */
  } else if (!strncmpW(pwBuf, L"\\\\?\\", 4)) {
    pwDir += 4;
    dwSize -= 4;
  }
  if (pwDir > pwBuf) memmove(pwBuf, pwDir, (dwSize+1) * sizeof(WCHAR));

  /* Cleanup and return */
  if (iReAlloc) {
    WCHAR *pwBuf2 = realloc(pwBuf, (dwSize + 1) * sizeof(WCHAR));
    if (pwBuf2) pwBuf = pwBuf2;
  }
  DEBUG_WLEAVE((L"return \"%s\"; // [%s]\n", pwBuf, pwszWhere));
  return pwBuf;
}

char *getcwdM(char *buf, size_t bufSize, UINT cp) {
  int n;
  WCHAR *pwDir;

  if (!buf) {
    errno = EINVAL;
    return NULL;
  }

  pwDir = getcwdW(NULL, 0);
  if (!pwDir) return NULL;

  /* Copy the pathname to the output buffer */
  n = WideCharToMultiByte(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pwDir,		/* lpWideCharStr, */
			  -1,			/* cchWideChar, -1=NUL-terminated string */
			  buf,			/* lpMultiByteStr, */
			  (int)bufSize,		/* cbMultiByte, */
			  NULL,			/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  free(pwDir);
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("getcwd(0x%p, %d); // Error: WideCharToMultiByte() Failed\n", buf, bufSize));
    return NULL;
  }

  DEBUG_PRINTF(("getcwd(0x%p, %d); // \"%s\"\n", buf, bufSize, buf));
  return buf;
}

char *getcwdA(char *buf, size_t bufSize) {
  return getcwdM(buf, bufSize, CP_ACP);
}

char *getcwdU(char *buf, size_t bufSize) {
  return getcwdM(buf, bufSize, CP_UTF8);
}

char *_getdcwdM(int iDrive, char *buf, int iBuflen, UINT cp) {
  char *pBuf;
  int iDrive0 = _getdrive();
  if (iDrive && (iDrive != iDrive0)) _chdrive(iDrive);
  pBuf = getcwdM(buf, iBuflen, cp);
  if (iDrive && (iDrive != iDrive0)) _chdrive(iDrive0);
  DEBUG_CODE(
    if (pBuf) {
      DEBUG_PRINTF(("_getdcwd(%d, 0x%p, %d); // \"%s\"\n", iDrive, buf, iBuflen, pBuf));
    } else {
      DEBUG_PRINTF(("_getdcwd(%d, 0x%p, %d); // Failed\n", iDrive, buf, iBuflen));
    }
  )
  return pBuf;
}

char *_getdcwdA(int iDrive, char *buf, int iBuflen) {
  return _getdcwdM(iDrive, buf, iBuflen, CP_ACP);
}

char *_getdcwdU(int iDrive, char *buf, int iBuflen) {
  return _getdcwdM(iDrive, buf, iBuflen, CP_UTF8);
}

#endif /* _WIN32 */

