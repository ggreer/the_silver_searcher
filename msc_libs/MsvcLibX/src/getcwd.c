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
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

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
|    2017-10-03 JFL Removed the dependency on PATH_MAX and fixed size buffers.|
|                   Added routine getcwdM, called by getcwdA and getcwdU.     |
|    2017-10-04 JFL Remove the long pathname prefix, if any.		      |
*									      *
\*---------------------------------------------------------------------------*/

#define strncmpW(s1, s2, l) (CompareStringW(LOCALE_INVARIANT, 0, s1, l, s2, l)-2)

char *getcwdM(char *buf, size_t bufSize, UINT cp) {
  int n;
  WCHAR *pwBuf;
  DWORD dwBufSize = UNICODE_PATH_MAX; /* This should be sufficient in all cases. Even if not, the buf will be extended below */
  DWORD dwSize;
  WCHAR *pwDir;

realloc_wBuf:
  pwBuf = malloc(sizeof(WCHAR) * dwBufSize);
  if (!pwBuf) return NULL;
  dwSize = GetCurrentDirectoryW(dwBufSize, pwBuf);
  if (dwSize > dwBufSize) { /* The buffer is too small. dwSize = the size needed */
    free(pwBuf);		/* No need to copy the old buffer */
    dwBufSize = dwSize;		/* Use the specified size */
    goto realloc_wBuf;
  }
  if (!dwSize) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("getcwd(0x%p, %d); // Error: GetCurrentDirectoryW() Failed\n", buf, bufSize));
    free(pwBuf);
    return NULL;
  }

  /* Remove the long pathname prefix, if any */
  pwDir = pwBuf;
  if (!strncmpW(pwBuf, L"\\\\?\\", 4)) {
    pwDir += 4;
    dwSize -= 4;
  } else if (!strncmpW(pwBuf, L"\\\\?\\UNC\\", 8)) {
    pwDir += 6;		/* Remove the '\\?\UNC\' prefix, except for the final two characters */
    dwSize -= 6;
    *pwDir = L'\\';	/* Change the 'C' to '\', so that the output begins by '\\server\share' */
  }

  /* Copy the pathname to the output buffer */
  n = WideCharToMultiByte(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pwDir,		/* lpWideCharStr, */
			  dwSize+1,		/* cchWideChar, */
			  buf,			/* lpMultiByteStr, */
			  (int)bufSize,		/* cbMultiByte, */
			  NULL,			/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  free(pwBuf);
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

