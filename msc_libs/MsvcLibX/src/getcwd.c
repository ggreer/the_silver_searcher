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
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_SOURCE /* Generate the UTF-8 version of routines */

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
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
|   Function:	    getcwd						      |
|									      |
|   Description:    Get the current directory, encoded in UTF-8               |
|									      |
|   Parameters:     char *buf	    Buffer for the output                     |
|		    size_t bufSize  Buffer size				      |
|		    							      |
|   Returns:	    The converted string size. -1=error, and errno set.	      |
|		    							      |
|   Notes:	    We can't use the getcwd name, as MSVC has an incompatible |
|		    prototype for it. So a getcwd macro calls this routine.   |
|		    							      |
|   History:								      |
|    2014-02-28 JFL Created this routine                               	      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
*									      *
\*---------------------------------------------------------------------------*/

char *getcwdA(char *buf, size_t bufSize) {
  int n;
  WCHAR wbuf[PATH_MAX];
  DWORD dwSize;

  dwSize = GetCurrentDirectoryW(COUNTOF(wbuf), wbuf);
  if (!dwSize) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("getcwd(0x%p, %d); // Error: GetCurrentDirectoryW() Failed\n", buf, bufSize));
    return NULL;
  }

  n = WideCharToMultiByte(CP_ACP,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  wbuf,			/* lpWideCharStr, */
			  dwSize+1,		/* cchWideChar, */
			  buf,			/* lpMultiByteStr, */
			  (int)bufSize,		/* cbMultiByte, */
			  NULL,			/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("getcwd(0x%p, %d); // Error: WideCharToMultiByte() Failed\n", buf, bufSize));
    return NULL;
  }

  DEBUG_PRINTF(("getcwd(0x%p, %d); // \"%s\"\n", buf, bufSize, buf));
  return buf;
}

char *getcwdU(char *buf, size_t bufSize) {
  int n;
  WCHAR wbuf[PATH_MAX];
  DWORD dwSize;

  dwSize = GetCurrentDirectoryW(COUNTOF(wbuf), wbuf);
  if (!dwSize) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("getcwd(0x%p, %d); // Error: GetCurrentDirectoryW() Failed\n", buf, bufSize));
    return NULL;
  }

  n = WideCharToMultiByte(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  wbuf,			/* lpWideCharStr, */
			  dwSize+1,		/* cchWideChar, */
			  buf,			/* lpMultiByteStr, */
			  (int)bufSize,		/* cbMultiByte, */
			  NULL,			/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("getcwd(0x%p, %d); // Error: WideCharToMultiByte() Failed\n", buf, bufSize));
    return NULL;
  }

  DEBUG_PRINTF(("getcwd(0x%p, %d); // \"%s\"\n", buf, bufSize, buf));
  return buf;
}

char *_getdcwdA(int iDrive, char *buf, int iBuflen) {
  char *pBuf;
  int iDrive0 = _getdrive();
  if (iDrive && (iDrive != iDrive0)) _chdrive(iDrive);
  pBuf = getcwdA(buf, iBuflen);
  if (iDrive && (iDrive != iDrive0)) _chdrive(iDrive0);
  DEBUG_CODE(
    if (pBuf) {
      DEBUG_PRINTF(("_getdcwd(%d, 0x%p, %d); // \"%s\"\n", iDrive, buf, iBuflen, pBuf));
    } else {
      DEBUG_PRINTF(("_getdcwd(%d, 0x%p, %d); // Failed\n", iDrive, buf, iBuflen, pBuf));
    }
  )
  return pBuf;
}

char *_getdcwdU(int iDrive, char *buf, int iBuflen) {
  char *pBuf;
  int iDrive0 = _getdrive();
  if (iDrive && (iDrive != iDrive0)) _chdrive(iDrive);
  pBuf = getcwdU(buf, iBuflen);
  if (iDrive && (iDrive != iDrive0)) _chdrive(iDrive0);
  DEBUG_CODE(
    if (pBuf) {
      DEBUG_PRINTF(("_getdcwd(%d, 0x%p, %d); // \"%s\"\n", iDrive, buf, iBuflen, pBuf));
    } else {
      DEBUG_PRINTF(("_getdcwd(%d, 0x%p, %d); // Failed\n", iDrive, buf, iBuflen, pBuf));
    }
  )
  return pBuf;
}

#endif /* _WIN32 */

