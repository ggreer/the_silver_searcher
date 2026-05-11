/*****************************************************************************\
*                                                                             *
*   Filename	    chdir.c						      *
*									      *
*   Description     WIN32 port of standard C library's chdir()		      *
*                                                                             *
*   Notes	    							      *
*		    							      *
*   History								      *
*    2014-02-28 JFL Created this module.				      *
*    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      *
*    2017-10-03 JFL Fixed support for pathnames >= 260 characters. 	      *
*    2018-04-25 JFL Manage the current directory locally for paths > 260 ch.  *
*    2018-04-28 JFL Split chdirW off of chdirM.				      *
*    2018-04-29 JFL Make sure chdirW always enters the deepest possible dir.  *
*                   Improved the error handling.                              *
*    2025-11-06 JFL Renamed the DOS version of chdir() as chdirX(), to avoid  *
*		    conflicts with the homonym in oldNames.lib.		      *
*    2025-11-11 JFL Added dos_chdir(), and use it in chdirX().		      *
*    2025-11-12 JFL Simplified chdirX(), and added debugging directives.      *
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
|   Function	    chdir						      |
|									      |
|   Description     Change directory, overcoming the 64-character DOS limit   |
|									      |
|   Parameters      char *pszDir	Target directory pathname	      |
|									      |
|   Returns	    0=Done; Else OS error code.		 		      |
|									      |
|   Notes	    Unfortunately this works only in a DOS box within Win9X.  |
|		    							      |
|		    MSVC's _chdir() works with paths up to 255 characters.    |
|		    							      |
|   History								      |
|    2000-12-04 JFL Initial implementation as chdirx() in dirc.c.	      |
|   <2016	JFL Renamed as chdir().					      |
|    2017-10-03 JFL Removed the dependency on PATH_MAX and fixed size buffers.|
|    2025-11-06 JFL Renamed as chdirX() to avoid homonymy with the chdir() in |
|		    oldNames.lib. Now define chdir() as chdirX() in unistd.h. |
|		    Added debug output.					      |
|    2025-11-11 JFL Added dos_chdir(), and use it in chdirX().		      |
*									      *
\*---------------------------------------------------------------------------*/

#include <dos.h>

#define OFFSET_OF(pointer) ((uint16_t)(uint32_t)(void far *)pointer)
#define SEGMENT_OF(pointer) ((uint16_t)(((uint32_t)(void far *)pointer) >> 16))

#define CF 0x0001            /* Carry flag bit mask */

int dos_chdir(const char *pszDir) { /* Limited to 64-byte paths */
  union REGS inreg;
  union REGS outreg;
#if DATA_PTR_WIDTH == 32  /* Memory models with long data pointers */
  struct SREGS sregs;
#endif

  inreg.h.ah = 0x3B;
  inreg.x.dx = OFFSET_OF(pszDir);
#if DATA_PTR_WIDTH == 32  /* Memory models with long data pointers */
  sregs.ds = SEGMENT_OF(pszDir);
  intdosx(&inreg, &outreg, &sregs);
#else /* DATA_PTR_WIDTH == 16 - Memory models with short data pointers */
  intdos(&inreg, &outreg);
#endif

  if (CF & outreg.x.cflag) return outreg.x.ax; /* errno set by intdos() */

  return 0;
}

/* Make it easy to change the low-level chdir routine used in chdirX below */
#define CHDIR dos_chdir /* Either dos_chdir or _chdir */

int chdirX(const char *pszDir) {
  char szBuf[64];
  char *pszBuf = szBuf;
  char *pc;
  int iDirLen;
  int iStrLen;
  int iErr = 0;

  DEBUG_ENTER(("chdir(\"%s\");\n", pszDir));

  iDirLen = strlen(pszDir);
  /* Copy the drive letter if specified, and leave it ahead of the buffer. */
  if ((iDirLen>2) && (pszDir[1]==':')) {
    szBuf[0] = pszDir[0];
    szBuf[1] = ':';
    pszDir += 2;
    pszBuf += 2;
  }
  /* Repeat relative moves down the directory tree */
  while (iDirLen > 0) {
    if (iDirLen > 60) {
      pc = strchr(pszDir+45, '\\');	/* There has to be one in segment [45:60]. */
      iStrLen = pc-pszDir;		/* Segment length */
      strncpy(pszBuf, pszDir, iStrLen); /* Copy until the \ found */
      pszBuf[iStrLen] = '\0';
    } else {
      iStrLen = iDirLen;
      strcpy(pszBuf, pszDir);		/* Copy the whole remaining string */
    }
    DEBUG_PRINTF((VALUEIZE(CHDIR) "(\"%s\");\n", szBuf));
    iErr = CHDIR(szBuf);
    if (iErr) RETURN_INT_COMMENT(-1, ("%s\n", strerror(errno)));
    pszDir += iStrLen+1;
    iDirLen -= iStrLen+1;
  }
  RETURN_INT_COMMENT(0, ("Success\n"));
}

#endif /* defined(_MSDOS) */


#if defined(_WIN32)

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    chdir						      |
|									      |
|   Description     Set the current directory, encoded in UTF-8               |
|									      |
|   Parameters      const char *pszDir	   Target directory pathname	      |
|									      |
|   Returns 	    0=Done; -1=Failed.			 		      |
|									      |
|   Notes	    Contrary to most other WIN32 APIs, SetCurrentDirectoryW() |
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
|   History								      |
|    2014-02-28 JFL Created this routine                               	      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
|                   Added common routine chdirM, called by chdirA and chdirU. |
|    2018-04-25 JFL Manage the current directory locally for paths > 260 ch.  |
|    2025-11-17 JFL No need to getcwd in the beginning for absolute targets.  |
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
  if ((pwszDir[0] == L'\\') && (pwszDir[1] != L'\\')) {
    pwszCD = malloc(3 * sizeof(WCHAR));
    if (!pwszCD) goto chdirW_failed;
    lstrcpyW(pwszCD, L"@:");
#pragma warning(disable:4244) /* Avoid WIN95 warning "conversion from 'int' to 'WCHAR', possible loss of data" */
   pwszCD[0] += (char)_getdrive();
#pragma warning(default:4244)
  } else if (!((pwszDir[0] == L'\\') || (pwszDir[0] && (pwszDir[1] == L':') && (pwszDir[2] == L'\\')))) {
    pwszCD = getcwdW(NULL, 0);
    if (!pwszCD) goto chdirW_failed;
  }
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
  /* SetCurrentDirectoryW() succeeded. No need to cache the CD locally */
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

