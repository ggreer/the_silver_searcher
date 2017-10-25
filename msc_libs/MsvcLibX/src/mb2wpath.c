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
*    2017-10-02 JFL Added routine MultiByteToNewWidePath().		      *
*                   Lowered the threshold for using a long name prefix, to    *
*                   work around limits in functions like CreateDirectoryW().  *
*    2017-10-04 JFL Added routine MultiByteToNewWideString().		      *
*                   Added the management of relative paths.                   *
*    2017-10-25 JFL Fixed MultiByteToWidePath support for paths with / seps.  *
*                   Added routine TrimLongPathPrefix().			      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of printf routines */

#include "msvclibx.h"
#include "debugm.h"

#if defined(_WIN32)

#include <windows.h>
#include <direct.h>	/* For _getdrive() */
#include <errno.h>
#include <unistd.h>	/* For CompactPathW() */

/* Allocate a new wide string converted from the input multi-byte string */
/* In case of failure, sets errno */
LPWSTR MultiByteToNewWideString(UINT cp, DWORD dwFlags, LPCSTR pszStr) {
  int n;
  WCHAR *pwBuf;
  int lStr = lstrlen(pszStr) + 1;

  n = MultiByteToWideChar(cp, dwFlags, pszStr, lStr, NULL, 0); /* Get the necessary buffer size */
  if (!n) {
    errno = Win32ErrorToErrno();
    return NULL;
  }
  pwBuf = malloc(sizeof(WCHAR) * n);
  if (!pwBuf) return NULL;
  n = MultiByteToWideChar(cp, dwFlags, pszStr, lStr, pwBuf, n);
  if (!n) {
    errno = Win32ErrorToErrno();
    return NULL;
  }
  return pwBuf;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MultiByteToWidePath					      |
|									      |
|   Description	    Convert a pathname to a unicode string, inserting the     |
|		     proper \\?\ prefix if it's longer than 260 bytes	      |
|									      |
|   Parameters      UINT nCodePage	    The pathname code page	      |
|		    LPCSTR pszName	    The pathname to convert	      |
|		    LPWSTR pwszBuf	    Output buffer		      |
|		    int nWideBufSize	    Output buffer size in WCHARs      |
|									      |
|   Returns	    0 = Failure, and errno set.				      |
|		    >0 = # of WCHARS in the unicode string.		      |
|		    							      |
|   Notes	    Also convert relative paths to absolute paths, to prevent |
|		    WIN32 APIs failures if the absolute path passes the limit.|
|									      |
|   History								      |
|    2014-07-01 JFL Created this routine                               	      |
|    2017-10-02 JFL Changed the return value to the string size. Was size+1.  |
|    2017-10-04 JFL Added the management of relative paths.                   |
|    2017-10-25 JFL Fixed support for paths with / separators.                |
*                   							      *
\*---------------------------------------------------------------------------*/

#define CRITICAL_LENGTH 240 /* Documented as 255 or 260, but some APIs (ex: CreateDirectoryW) fail between 240 and 250 */

int MultiByteToWidePath(
  UINT nCodePage,
  LPCSTR pszName,
  LPWSTR pwszBuf,
  int nWideBufSize
) {
  int n = 0;
  int lName;
  LPWSTR pwszBuf0 = pwszBuf;
  LPWSTR pwszName = MultiByteToNewWideString(nCodePage, 0, pszName);
  LPWSTR pwsz;

  if (!pwszName) return 0;
  lName = lstrlenW(pwszName);

  /* Replace all / with \ */
  for (pwsz = pwszName; *pwsz; pwsz++) if (*pwsz == L'/') *pwsz = L'\\';
  
  /* Relative pathnames will cause failures if the absolute name passes the 260 character limit */
  if (   (pwszName[0] != L'\\')				/* If this is not an absolute path */
      && (   (pwszName[1] && (pwszName[1] != L':'))	/* And there's no drive letter */
	  || (!pwszName[2])				   /* Or it's just a drive letter */
	  || (pwszName[2] != L'\\'))) {			   /* Or there's no absolute path after the drive letter */
    /* Then it's a relative path. Convert it to an absolute path to prevent WIN32 APIs failures */
    int iDrive = 0;
    WCHAR *pwszRelPath = pwszName;
    int lRelPath = lName;
    int iDrive0 = _getdrive();
    int iLen;
    WCHAR *pwszBuf2;
    pwszBuf2 = malloc(sizeof(WCHAR) * UNICODE_PATH_MAX);
    if (!pwszBuf2) goto cleanup_and_return;
    if (pwszName[1] == L':') {
      iDrive = pszName[0] - '@';		/* A=1, B=2, ... */
      if (iDrive > 0x20) iDrive -= 0x20;	/* The drive letter was lower case */
      pwszRelPath += 2;
      lRelPath -= 2;
    }
    if (iDrive && (iDrive != iDrive0)) _chdrive(iDrive);
    iLen = (int)GetCurrentDirectoryW(UNICODE_PATH_MAX, pwszBuf2);
    if (iDrive && (iDrive != iDrive0)) _chdrive(iDrive0);
    if (pwszBuf2[iLen-1] != L'\\') pwszBuf2[iLen++] = L'\\';
    lstrcpyW(pwszBuf2+iLen, pwszRelPath);
    iLen += lRelPath;
    iLen = CompactPathW(pwszBuf2, pwszBuf2, UNICODE_PATH_MAX);
    if (iLen >= CRITICAL_LENGTH) { /* Then processing this pathname requires prepending a special prefix */
      free(pwszName);
      pwszName = pwszBuf2;
      lName = iLen;
      DEBUG_CODE({
	char *pszUtf8;
	DEBUG_WSTR2NEWUTF8(pwszName, pszUtf8);
	DEBUG_PRINTF(("// Relative name changed to \"%s\"\n", pszUtf8));
	DEBUG_FREEUTF8(pszUtf8);
      });
      /* Then prepend it with "\\?\" to get 32K Unicode paths instead of 260-byte ANSI paths */
      goto prepend_win32_prefix;
    }
  } else if (lName >= CRITICAL_LENGTH) { /* Then processing this pathname requires prepending a special prefix */
    /* See http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx */
    if (!strncmp(pszName, "\\\\?\\", 4)) {		/* The name is in the Win32 file namespace */
      /* Do nothing, the "\\?\" extended-length prefix is already there */
    } else if (!strncmp(pszName, "\\\\.\\", 4)) {	/* The name is in the Win32 device namespace */
      /* Do nothing, devices names should not be changed */
    } else if (!strncmp(pszName, "\\\\", 2)) {		/* The name is a UNC path */
      /* Then prepend it with "\\?\UNC\" to get 32K Unicode paths instead of 260-byte ANSI paths */
      if (nWideBufSize <= 8) goto fail_no_space;
      lstrcpyW(pwszBuf, L"\\\\?\\UNC\\");
      pwszBuf += 8;
      nWideBufSize -= 8;
      pwszName += 2; /* Skip the initial \\ in the UNC name */
      lName -= 2;
    } else if (pwszName[0] == L'\\') {	/* The name is an absolute path with no drive */
      /* Then prepend it with "\\?\" to get 32K Unicode paths instead of 260-byte ANSI paths */
      if (nWideBufSize <= 6) goto fail_no_space;
      lstrcpyW(pwszBuf, L"\\\\?\\");
      /* And also add the drive, as it's required in the Win32 file namespace */
      pwszBuf[4] = L'@' + (wchar_t)_getdrive(); /* _getdrive() returns 1 for drive A, 2 for B, etc */
      pwszBuf[5] = L':';
      pwszBuf += 6;
      nWideBufSize -= 6;
    } else if (pwszName[0] && (pwszName[1] == L':') && (pwszName[2] == L'\\')) { /* The name is an absolute path */
prepend_win32_prefix:
      if (nWideBufSize <= 4) goto fail_no_space;
      /* Then prepend it with "\\?\" to get 32K Unicode paths instead of 260-byte ANSI paths */
      lstrcpyW(pwszBuf, L"\\\\?\\");
      pwszBuf += 4;
      nWideBufSize -= 4;
    } else { /* Else this is a relative pathname. Extended-length is not supported for them. */
      /* But we've already handled the relative name case previously, so this should not happen */
      DEBUG_PRINTF(("// Assert failed. Please review the MultiByteToWidePath() logic.\n"));
    }
  }

  if (lName >= nWideBufSize) {
fail_no_space:
    errno = ENOSPC;
    goto cleanup_and_return;
  }
  lstrcpyW(pwszBuf, pwszName);
  DEBUG_CODE(
    if (pwszBuf != pwszBuf0) {
      char *pszUtf8;
      DEBUG_WSTR2NEWUTF8(pwszBuf0, pszUtf8);
      DEBUG_PRINTF(("// Long name changed to \"%s\"\n", pszUtf8));
      DEBUG_FREEUTF8(pszUtf8);
    }
  );
  n = lName + (int)(pwszBuf - pwszBuf0);   /* Count the added prefix length, if any */

cleanup_and_return:
  free(pwszName);
  return n;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MultiByteToNewWidePath				      |
|									      |
|   Description	    Convert a pathname to a new unicode string, inserting the |
|		     proper \\?\ prefix if it's longer than 260 bytes	      |
|									      |
|   Parameters      UINT nCodePage	    The pathname code page	      |
|		    LPCSTR pszName	    The pathname to convert	      |
|									      |
|   Returns	    NULL = Failure, and errno set.			      |
|		    Else a pointer to the allocated buffer with the string.   |
|		    							      |
|   Notes	    Also convert relative paths to absolute paths, to prevent |
|		    WIN32 APIs failures if the absolute path passes the limit.|
|									      |
|   History								      |
|    2017-10-02 JFL Created this routine                               	      |
*                   							      *
\*---------------------------------------------------------------------------*/

/* Allocate a new wide string, and set errno in case of failure */
LPWSTR MultiByteToNewWidePath(
  UINT nCodePage,
  LPCSTR pszName
) {
  WCHAR *pwszName;
  int lName = UNICODE_PATH_MAX;	/* Number of WCHARS in the unicode name (Worst case) */
  int n;
  WCHAR *pwszName2;

  pwszName = malloc(sizeof(WCHAR) * lName);
  if (!pwszName) return NULL;

  n = MultiByteToWidePath(nCodePage, pszName, pwszName, lName);
  if (!n) {
    free(pwszName);
    return NULL;
  }
  n += 1;	/* Count the final NUL */

  pwszName2 = realloc(pwszName, sizeof(WCHAR) * n); /* This is unlikely to fail, as the buffer is shrinking, but this _can_ happen */ 
  if (!pwszName2) pwszName2 = pwszName; /* In the unlikely case that it did fail, keep using the large buffer */
  return pwszName2;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    TrimLongPathPrefix					      |
|									      |
|   Description	    Remove the \\?\ and \\?\UNC\ prefixes from a pathname     |
|									      |
|   Parameters      LPSTR pszName	    The pathname to trim	      |
|									      |
|   Returns	    The length of the final string			      |
|		    							      |
|   Notes	    							      |
|		    							      |
|   History								      |
|    2017-10-25 JFL Created this routine                               	      |
*                   							      *
\*---------------------------------------------------------------------------*/

int TrimLongPathPrefix(LPSTR pszName) {
  int n = lstrlen(pszName);
  if (!strncmp(pszName, "\\\\?\\", 4)) {
    int iLen = 4;
    LPSTR lpTo = pszName;
    if (!strncmp(pszName, "\\\\?\\UNC\\", 8)) { /* Change "\\?\UNC\server\share" to "\\server\share" */
      iLen = 6;
      lpTo = pszName+2;
    }
    n -= iLen; /* Actual length of the output path (with NUL, without prefix) */
    memmove(lpTo, lpTo+iLen, n+1);
  }
  return n;
}

#endif /* defined(_WIN32) */

