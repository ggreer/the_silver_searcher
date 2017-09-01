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
|   Returns:	    0=Done; Else OS error code.		 		      |
|									      |
|   History:								      |
|    2014-02-28 JFL Created this routine                               	      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
*									      *
\*---------------------------------------------------------------------------*/

int chdirA(const char *pszDir) {
  WCHAR wszDir[PATH_MAX];
  BOOL bDone;
  int n;
  DEBUG_PRINTF(("chdir(\"%s\");\n", pszDir));
  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_ACP,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  pszDir,		/* lpMultiByteStr, */
			  wszDir,		/* lpWideCharStr, */
			  COUNTOF(wszDir)	/* cchWideChar, */
			  );
  bDone = SetCurrentDirectoryW(wszDir);
  if (!bDone) {
    errno = Win32ErrorToErrno();
  }
  return bDone ? 0 : -1;
}

int chdirU(const char *pszDir) {
  WCHAR wszDir[PATH_MAX];
  BOOL bDone;
  int n;
  DEBUG_PRINTF(("chdir(\"%s\");\n", pszDir));
  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  pszDir,		/* lpMultiByteStr, */
			  wszDir,		/* lpWideCharStr, */
			  COUNTOF(wszDir)	/* cchWideChar, */
			  );
  bDone = SetCurrentDirectoryW(wszDir);
  if (!bDone) {
    errno = Win32ErrorToErrno();
  }
  return bDone ? 0 : -1;
}

#endif /* defined(_WIN32) */

