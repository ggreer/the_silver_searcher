/*****************************************************************************\
*                                                                             *
*   Filename	    err2errno.c						      *
*									      *
*   Description:    Convert a WIN32 error to a Unix errno		      *
*                                                                             *
*   Notes:	                                                              *
*		    							      *
*   History:								      *
*    2014-02-17 JFL Created this module.				      *
*    2016-10-05 JFL Fixed compatibility with Visual Studio 2003 and older.    *
*                   Removed a few useless special cases, and added EZERO case.*
*                   Make sure the global errno is _not_ changed by this funct.*
*    2020-08-28 JFL Remove the CR characters in the error message.            *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
/* MsvcLibX library extensions */
#include "debugm.h"


#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    Win32ErrorToErrno					      |
|									      |
|   Description:    Convert a Win32 system error code to a Posix errno        |
|									      |
|   Parameters:     DWORD dwError		The Win32 error code          |
|									      |
|   Returns:	    The corresponding Posix errno			      |
|									      |
|   Notes:	    There's no 1-to-1 correspondance between Windows and      |
|		    Posix errors. This routine attempts to convert codes for  |
|		    the most likely errors for MsvcLibX.		      |
|		    Please add those you encounter, that end up in the	      |
|		    default category, and incorrectly return EIO by default.  |
|		    							      |
|		    Does not change errno. (Contrary to _dosmaperr, etc)      |
|		    You must set errno thereafter if desired.		      |
|		    							      |
|   History:								      |
|    2014-02-05 JFL Created this routine                                      |
|    2014-03-05 JFL Added the default call to _get_errno_from_oserr().        |
|    2015-12-07 JFL Use the new error conversion routine name in the UCRT.    |
|    2017-10-31 JFL Added case ERROR_FILENAME_EXCED_RANGE.		      |
*									      *
\*---------------------------------------------------------------------------*/

#if defined(_UCRT) /* Visual Studio 14 and later */
#define _get_errno_from_oserr __acrt_errno_from_os_error /* The name changed in the UCRT */
#endif

#if (_MSC_VER < 1400) /* Anything older than Visual Studio 8 (= VS 2003 and older) */
#pragma message("Defining our own _get_errno_from_oserr()")
extern void __cdecl _dosmaperr(unsigned long);
int _get_errno_from_oserr(unsigned long dwErr) {
  _dosmaperr(dwErr); /* Sets errno from WIN32 error */
  return errno;
}
#else
#pragma message("Using the default " MSVCLIBX_STRINGIZE(_get_errno_from_oserr) "()")
/* Equivalent function in MSVC library. Does not know about symlink errors. */
extern int __cdecl _get_errno_from_oserr(unsigned long oserrno);
#endif

#if 0
/* Initially I thought there was a duplicate CR in the formatted string */
/* Remove duplicate characters from a string */
static int RemoveDuplicateChars(char *pszText, char cToDeDup) {
  int i, j;
  char c = *pszText, c0 = ~cToDeDup;
  for (i=j=0; ; c0=c, c=pszText[++i]) {
    if (i > j) pszText[j] = c;
    if (!c) return (i-j); /* Return the number of characters removed */
    if ((c != cToDeDup) || (c0 != cToDeDup)) j++;
  }
}
#endif
/* Remove a specific character from a string */
static int RemoveChars(char *pszText, char cToRemove) {
  int i, j;
  char c = *pszText;
  for (i=j=0; ; c=pszText[++i]) {
    if (i > j) pszText[j] = c;
    if (!c) return (i-j); /* Return the number of characters removed */
    if (c != cToRemove) j++;
  }
}

int Win32ErrorToErrno() {
  DWORD dwError = GetLastError();

  DEBUG_CODE({
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		  NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPTSTR)&lpMsgBuf, 0, NULL);
    /* Messages end with CR LF. But the printf(LF) generates an extra CR, which confuses some software. */
    if (lpMsgBuf) RemoveChars(lpMsgBuf, '\x0D'); /* So remove the CR characters */
    DEBUG_PRINTF(("// Win32 error %d (0x%X): %s", dwError, dwError, lpMsgBuf ? lpMsgBuf : "Unknown\n"));
    LocalFree( lpMsgBuf );
  });

  switch (dwError) {
    case ERROR_SUCCESS:
      return 0;
    case ERROR_PRIVILEGE_NOT_HELD: /* Not running with the SE_CREATE_SYMBOLIC_LINK_NAME privilege */
    case ERROR_REPARSE_ATTRIBUTE_CONFLICT:
      return EACCES;
    case ERROR_INSUFFICIENT_BUFFER:
      return E2BIG;
    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:
      return EEXIST;
    case ERROR_WRITE_PROTECT:
      return EROFS;
    case ERROR_HANDLE_DISK_FULL:
      return ENOSPC;
    case ERROR_NOT_A_REPARSE_POINT:
    case ERROR_REPARSE_TAG_MISMATCH:
    case ERROR_INVALID_FLAGS:
    case ERROR_INVALID_PARAMETER:
      return EINVAL;
    case ERROR_INVALID_REPARSE_DATA:
    case ERROR_REPARSE_TAG_INVALID:
      return EBADF; /* Not supposed to happen in Posix OSs, but may happen when experimenting with junction() IOCTLs. */
    case ERROR_NO_UNICODE_TRANSLATION:
      return EILSEQ;
    case ERROR_FILENAME_EXCED_RANGE:
      return ENAMETOOLONG;
    default: {
      int errno0, errno1;
      errno0 = errno; /* Preserve the initial errno */
      errno1 = _get_errno_from_oserr(dwError); /* Let MSVC library decide. May change errno. */
      errno = errno0; /* Restore the initial errno */
      return errno1;
    }
  }
}

#endif /* _WIN32 */

