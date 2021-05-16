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
*    2018-04-25 JFL Manage the current directory locally for paths > 260 ch.  *
*    2018-04-28 JFL Split chdirW off of chdirM.				      *
*    2018-04-29 JFL Make sure chdirW always enters the deepest possible dir.  *
*                   Improved the error handling.                              *
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
|		    this routine locally manages the current directory.	      |
|		    No attempt is made to manage multiple drive-specific      |
|		    current directories, as the goal is Unix-compatibility,   |
|		    not Windows compatibility.				      |
|		    							      |
|   History:								      |
|    2014-02-28 JFL Created this routine                               	      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
|                   Added common routine chdirM, called by chdirA and chdirU. |
|    2018-04-25 JFL Manage the current directory locally for paths > 260 ch.  |
*									      *
\*---------------------------------------------------------------------------*/

WCHAR *pwszLongCurrentDir = NULL;

int chdirW(const WCHAR *pwszDir) {
  WCHAR *pwszCD = NULL;
  WCHAR *pwszAbsDir = NULL;
  WCHAR *pwszPrefixedAbsDir = NULL;
  BOOL bDone;
  int iErr = 0;
  DEBUG_CODE(
  WCHAR *pwszWhere = L"Windows";
  )

  DEBUG_WENTER((L"chdir(\"%s\");\n", pwszDir));

  /* Make sure every path is an absolute path with a drive letter or UNC prefix */
  pwszCD = getcwdW(NULL, 0);
  if (!pwszCD) goto chdirW_failed;
  pwszAbsDir = ConcatPathW(pwszCD, pwszDir, NULL, 0);
  if (!pwszAbsDir) goto chdirW_failed;

  /* Make sure long paths have the \\? prefix */
  pwszPrefixedAbsDir = CorrectNewWidePath(pwszAbsDir);
  if (!pwszPrefixedAbsDir) goto chdirW_failed;

/* Due to SetCurrentDirectoryW bug, prefixing the CD does not help.
   As this confuses some applications, use the non-prefixed CD for now. */
#define pwszSETDIR pwszAbsDir /* pwszAbsDir or pwszPrefixedAbsDir */

  bDone = SetCurrentDirectoryW(pwszSETDIR);
  if (!bDone) {
    int lAbsDir = lstrlenW(pwszSETDIR);
    WCHAR *pwszParent = NULL;
    if (   (GetLastError() == ERROR_FILENAME_EXCED_RANGE) /* The filename is too long, */
        && (lAbsDir < WIDE_PATH_MAX)) {			  /* But it does not look too long */
      /* Validate that this pwszAbsDir directory exists */
      iErr = _waccess(pwszPrefixedAbsDir, 0);
      if (iErr) goto exit_chdirW;
      /* Then cache it locally */
      if (pwszLongCurrentDir) free(pwszLongCurrentDir);
      pwszLongCurrentDir = pwszAbsDir;
      /* Kludge: Change Windows CD to the deepest directory that is accessible */
      pwszParent = malloc((lAbsDir+1) * sizeof(WCHAR));
      if (pwszParent) {
      	lstrcpyW(pwszParent, pwszSETDIR);
	do {
	  while ((lAbsDir > 0) && (pwszParent[--lAbsDir] != L'\\')) ;
	  pwszParent[lAbsDir] = L'\0';
	  bDone = SetCurrentDirectoryW(pwszParent);
	} while ((lAbsDir > 0) && !bDone);
	free(pwszParent);
      }
      DEBUG_CODE(
      pwszWhere = L"MsvcLibX";
      )
      goto exit_chdirW; /* Pretend success */
    }
    errno = Win32ErrorToErrno();
    goto chdirW_failed;
  }
  if (pwszLongCurrentDir) {
    free(pwszLongCurrentDir);
    pwszLongCurrentDir = NULL;
  }
  goto exit_chdirW; /* Success */
chdirW_failed:
  iErr = -1;
exit_chdirW:
  DEBUG_WPRINTF((L"return %d; // [%s] \"%s\"\n", iErr, pwszWhere, pwszAbsDir));
  free(pwszPrefixedAbsDir);
  if (pwszLongCurrentDir != pwszAbsDir) free(pwszAbsDir);
  free(pwszCD);
  DEBUG_QUIET_LEAVE();
  return iErr;
}

int chdirM(const char *pszDir, UINT cp) {
  WCHAR *pwszDir;
  int iErr;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszDir = MultiByteToNewWideString(cp, pszDir);
  if (!pwszDir) return -1;
  
  iErr = chdirW(pwszDir);

  free(pwszDir);
  return iErr;
}

int chdirA(const char *pszDir) {
  return chdirM(pszDir, CP_ACP);
}

int chdirU(const char *pszDir) {
  return chdirM(pszDir, CP_UTF8);
}

#endif /* defined(_WIN32) */

