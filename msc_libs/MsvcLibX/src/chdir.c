/*****************************************************************************\
*                                                                             *
*   Filename	    chdir.c						      *
*									      *
*   Description:    WIN32 port of standard C library's chdir()		      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-02-28 JFL Created this module.				      *
*    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      *
*    2017-10-03 JFL Fixed support for pathnames >= 260 characters. 	      *
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
#include <iconv.h>
#include "debugm.h"

#if defined(_MSDOS)

/*---------------------------------------------------------------------------*\
|									      *
|   Function:	    chdir						      |
|									      |
|   Description:    Change directory, overcoming the 64-character DOS limit   |
|									      |
|   Parameters:     char *pszDir	Target directory pathname	      |
|									      |
|   Returns:	    0=Done; Else OS error code.		 		      |
|									      |
|   Notes:	    Unfortunately this works only in a DOS box within Win9X.  |
|									      |
|   History:								      |
|     2000-12-04 JFL Initial implementation.				      |
|    2017-10-03 JFL Removed the dependency on PATH_MAX and fixed size buffers.|
*									      *
\*---------------------------------------------------------------------------*/

int chdir(const char *pszDir)
    {
    char szBuf[64];
    char *pszBuf = szBuf;
    char *pc;
    int iDirLen;
    int iStrLen;
    int iErr = 0;
    
    iDirLen = strlen(pszDir);
    /* Copy the drive letter if specified, and leave it ahead of the buffer. */
    if ((iDirLen>2) && (pszDir[1]==':'))
	{
	szBuf[0] = pszDir[0];
	szBuf[1] = ':';
	pszDir += 2;
	pszBuf += 2;
	}
    /* Repeat relative moves down the directory tree */
    while (iDirLen > 60)
	{
	pc = strchr(pszDir+45, '\\');		/* There has to be one in segment [45:60]. */
	iStrLen = pc-pszDir;			/* Segment length */
	strncpy(pszBuf, pszDir, iStrLen);       /* Copy until the \ found */
	pszBuf[iStrLen] = '\0';
	iErr = chdir(szBuf);
	if (iErr) return iErr;
	pszDir += iStrLen+1;
	iDirLen -= iStrLen+1;
	} ;

    if (iDirLen) 
        {
        strcpy(pszBuf, pszDir);
        iErr = chdir(szBuf);
        }
        
    return iErr;
    }

#endif /* defined(_MSDOS) */


#if defined(_WIN32)

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    chdir						      |
|									      |
|   Description:    Set the current directory, encoded in UTF-8               |
|									      |
|   Parameters:     const char *pszDir	   Target directory pathname	      |
|									      |
|   Returns:	    0=Done; -1=Failed.			 		      |
|									      |
|   Notes:	    Contrary to most other WIN32 APIs, SetCurrentDirectoryW() |
|		    does NOT allow extending the path length beyond 260 bytes |
|		    by prepending a \\?\ prefix.			      |
|		    https://stackoverflow.com/a/44519069/2215591	      |
|		    							      |
|		    To fix this, use a manifest defining longPathAware=true.  |
|		    Setting longPathAware=true will lift the 260-bytes WIN32  |
|		    APIs limitation in Windows 10 version 1607 and later,     |
|		    if and only if the registry value LongPathsEnabled in     |
|		    HKLM\SYSTEM\CurrentControlSet\Control\FileSystem is set   |
|		    to 1.						      |
|		    							      |
|		    If support for long path lengths in older versions of     |
|		    Windows is desired (XP to 8), then avoid using chdir() or |
|		    SetCurrentDirectoryW().				      |
|		    							      |
|   History:								      |
|    2014-02-28 JFL Created this routine                               	      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
|                   Added common routine chdirM, called by chdirA and chdirU. |
*									      *
\*---------------------------------------------------------------------------*/

int chdirM(const char *pszDir, UINT cp) {
  WCHAR *pwszDir;
  BOOL bDone;
  int iErr = 0;

  DEBUG_ENTER(("chdir(\"%s\");\n", pszDir));

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszDir = MultiByteToNewWidePath(cp, pszDir);
  if (!pwszDir) return -1;

  bDone = SetCurrentDirectoryW(pwszDir);
  if (!bDone) {
    errno = Win32ErrorToErrno();
    iErr = -1;
  }
  free(pwszDir);
  DEBUG_QUIET_LEAVE();
  return iErr;
}

int chdirA(const char *pszDir) {
  return chdirM(pszDir, CP_ACP);
}

int chdirU(const char *pszDir) {
  return chdirM(pszDir, CP_UTF8);
}

#endif /* defined(_WIN32) */

