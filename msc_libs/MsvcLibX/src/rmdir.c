/*****************************************************************************\
*                                                                             *
*   Filename	    rmdir.c						      *
*									      *
*   Description:    WIN32 MBCS port of standard C library's rmdir()	      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-03-05 JFL Created routine rmdir() in lstat.c.                       *
*    2014-06-30 JFL Added support for 32K Unicode paths.           	      *
*    2017-03-18 JFL Created this module, with 3 versions rmdir[AUM]().        *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_SOURCE /* Generate the UTF-8 version of routines */

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
#include <string.h>
/* MsvcLibX library extensions */
#include "msvclibx.h"
#include <sys/stat.h>
#include "debugm.h"

#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    rmdir						      |
|									      |
|   Description:    Remove a directory				              |
|									      |
|   Parameters:     const char *path		The directory name	      |
|									      |
|   Returns:	    0 = Success, -1 = Failure				      |
|									      |
|   Notes:	    							      |
|									      |
|   History:								      |
|    2014-03-05 JFL Created this routine with support for UTF-8 pathnames.    |
*									      *
\*---------------------------------------------------------------------------*/

int rmdirM(const char *path, UINT cp) {
  int iErr;
  BOOL bDone;
  struct stat st;
  WCHAR wszName[UNICODE_PATH_MAX];
  int n;

  DEBUG_ENTER(("rmdir(\"%s\");\n", path));

  iErr = lstat(path, &st);
  if (iErr) RETURN_INT(iErr);

  if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    RETURN_INT_COMMENT(-1, ("Pathname exists, but is not a directory\n"));
  }

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(cp,   		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszName,		/* lpWideCharStr, */
			  UNICODE_PATH_MAX	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));
  }

  bDone = RemoveDirectoryW(wszName);

  if (bDone) {
    RETURN_INT_COMMENT(0, ("Success\n"));
  } else {
    errno = Win32ErrorToErrno();
    RETURN_INT_COMMENT(-1, ("Failed\n"));
  }
}

int rmdirA(const char *path) {
  return rmdirM(path, CP_ACP);
}

int rmdirU(const char *path) {
  return rmdirM(path, CP_UTF8);
}
#endif /* _WIN32 */

