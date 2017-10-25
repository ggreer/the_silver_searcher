/*****************************************************************************\
*                                                                             *
*   Filename	    unlink.c						      *
*									      *
*   Description:    WIN32 MBCS port of standard C library's unlink()	      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-02-17 JFL Created routine unlink() in lstat.c.                      *
*    2014-02-28 JFL Added support for UTF-8 pathnames.                 	      *
*    2014-06-30 JFL Added support for 32K Unicode paths.           	      *
*    2017-03-18 JFL Created this module, with 3 versions unlink[AUM]().       *
*    2017-10-03 JFL Fixed support for pathnames >= 260 characters. 	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

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
|   Function:	    unlink						      |
|									      |
|   Description:    Remove a file or a symbolic link		              |
|									      |
|   Parameters:     const char *path		The file or symlink name      |
|									      |
|   Returns:	    0 = Success, -1 = Failure				      |
|									      |
|   Notes:	    							      |
|									      |
|   History:								      |
|    2014-02-17 JFL Created this routine                               	      |
|    2014-02-28 JFL Added support for UTF-8 pathnames.                 	      |
*									      *
\*---------------------------------------------------------------------------*/

int unlinkM(const char *path, UINT cp) {
  int iErr;
  BOOL bDone;
  struct stat st;
  WCHAR *pwszName;

  DEBUG_ENTER(("unlink(\"%s\");\n", path));

  iErr = lstat(path, &st); /* TODO: Change this to lstatM() or lstatW when available */
  if (iErr) RETURN_INT(iErr);

  if ((!S_ISREG(st.st_mode)) && (!S_ISLNK(st.st_mode))) {
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("Pathname exists, but is not a file or a link\n"));
  }

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(cp, path);
  if (!pwszName) RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));

#if _MSVCLIBX_STAT_DEFINED
  if (S_ISLNK(st.st_mode) && (st.st_Win32Attrs & FILE_ATTRIBUTE_DIRECTORY)) {
    /* This link is a junction or a symlinkd */
    bDone = RemoveDirectoryW(pwszName);
  } else
#endif
  bDone = DeleteFileW(pwszName);
  free(pwszName);
  if (bDone) {
    RETURN_INT_COMMENT(0, ("Success\n"));
  } else {
    errno = Win32ErrorToErrno();
    RETURN_INT_COMMENT(-1, ("Failed\n"));
  }
}

int unlinkA(const char *path) {
  return unlinkM(path, CP_ACP);
}

int unlinkU(const char *path) {
  return unlinkM(path, CP_UTF8);
}

#endif /* _WIN32 */

