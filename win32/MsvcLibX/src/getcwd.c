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
*    2018-04-25 JFL Added routine getcwdW().                       	      *
*		    Use the locally managed cur. dir. for paths > 260 bytes.  *
*    2025-11-11 JFL Added routines dos_getcwd() and getcwdX() for DOS. 	      *
*    2025-11-19 JFL Changed dos_getcwd() return value to be same as getcwd's. *
*    2025-12-03 JFL Added routine getcwd0().                       	      *
*    2025-12-31 JFL Moved DOS FS Info structures and definitions to dos.h.    *
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
#include "debugm.h"

/*---------------------------------------------------------------------------*\
|									      *
|   Function	    getcwd0						      |
|									      |
|   Description     Front end to getcwd, dynamically allocating the string    |
|									      |
|   Parameters      None						      |
|		    							      |
|   Returns	    A pointer to the allocated CWD string, or NULL if error   |
|		    							      |
|   Notes	    							      |
|		    							      |
|   History								      |
|    2025-12-03 JFL Initial implementation.				      |
*									      *
\*---------------------------------------------------------------------------*/

/* Allocate a new string containing the current directory */
char *getcwd0(void) {
  char *pszBuf = malloc(PATH_MAX);
  char *pszRet;
  if (!pszBuf) return NULL;
  pszRet = getcwd(pszBuf, PATH_MAX);
  if (pszRet) {	/* Valid CWD in pszRet = pszBuf */
    return ShrinkBuf(pszBuf, strlen(pszBuf)+1); /* Trim the buffer */
  } else {	/* Invalid CWD */
    free(pszBuf);
    return NULL;
  }
}

#if defined(_MSDOS)

/*---------------------------------------------------------------------------*\
|									      *
|   Function	    getcwdX						      |
|									      |
|   Description     Get the current directory, overcoming the 64-char limit   |
|									      |
|   Parameters      char *buf	    Buffer for the output                     |
|		    size_t bufSize  Buffer size				      |
|		    							      |
|   Returns	    buf if success, or NULL if error, and errno set	      |
|		    							      |
|   Notes	    MSVC's _getcwd() works with paths up to 255 characters.   |
|		    This one works with paths up to 260 characters.	      |
|		    							      |
|   History								      |
|    2025-11-11 JFL Initial implementation.				      |
*									      *
\*---------------------------------------------------------------------------*/

#include <dos.h>

#define OFFSET_OF(pointer) ((uint16_t)(uint32_t)(void far *)pointer)
#define SEGMENT_OF(pointer) ((uint16_t)(((uint32_t)(void far *)pointer) >> 16))

#define CF 0x0001            /* Carry flag bit mask */

/* Call MS-DOS function 47H "Get Current Directory" */
/* Args & return value compatible with those of MSVC's _getdcwd() */
/* The DOS error code is returned in _doserrno, and the C error code in errno */
char *dos_getcwd(int iDrive, char *pszBuf, int iBufLen) { /* Limited to 64-byte paths */
  union REGS inreg;
  union REGS outreg;
#if DATA_PTR_WIDTH == 32  /* Memory models with long data pointers */
  struct SREGS sregs;
#endif
  UNUSED_ARG(iBufLen);

  inreg.h.ah = 0x47;
  inreg.h.dl = (uint8_t)iDrive; /* 0=default 1=A 2=B 3=C ... */
  inreg.x.si = OFFSET_OF(pszBuf);
#if DATA_PTR_WIDTH == 32  /* Memory models with long data pointers */
  sregs.ds = SEGMENT_OF(pszBuf);
  intdosx(&inreg, &outreg, &sregs);
#else /* DATA_PTR_WIDTH == 16 - Memory models with short data pointers */
  intdos(&inreg, &outreg);
#endif

  if (CF & outreg.x.cflag) { /* errno and _doserrno set by intdos() */
    DEBUG_PRINTF(("dos_getcwd() -> return %d; // _doserrno=%d; errno=%d = %s\n", outreg.x.ax, _doserrno, errno, strerror(errno)));
    return NULL;	/* The DOS error code is available in _doserrno */
  }

  return pszBuf;
}

/* Call MS-DOS function 71A0H "Get Volume Information" */
/* Assumes the volume is FAT on old DOS versions <= 6 */
/* Returns the DOS error code, or 0 for success */
/* The DOS error code is returned in _doserrno, and the C error code in errno */
int dos_get_volume_info(char *pszRoot, dos_fs_info *pDosFsInfo) {
  union REGS inreg;
  union REGS outreg;
#if DATA_PTR_WIDTH == 32  /* Memory models with long data pointers */
  struct SREGS sregs;
#endif

  inreg.x.ax = 0x71A0;
  inreg.x.cx = sizeof(pDosFsInfo->szFsType);
  inreg.x.dx = OFFSET_OF(pszRoot);
  inreg.x.di = OFFSET_OF(pDosFsInfo->szFsType);
  inreg.x.cflag |= CF; /* For catching the error in DOS < 7 */
#if DATA_PTR_WIDTH == 32  /* Memory models with long data pointers */
  sregs.ds = SEGMENT_OF(pszRoot);
  sregs.es = SEGMENT_OF(pDosFsInfo->szFsType);
  intdosx(&inreg, &outreg, &sregs);
#else /* DATA_PTR_WIDTH == 16 - Memory models with short data pointers */
  intdos(&inreg, &outreg);
#endif

  if (CF & outreg.x.cflag) { /* Function failed */
    if (outreg.x.ax == 0x7100) { /* DOS < 7, function not supported */
      /* Pretend it did succeed, using the values for FAT16 in DOS without LFN support */
      outreg.x.bx = 0;		/* Flags */
      outreg.x.cx = 12;		/* 8.3 File names */
      outreg.x.dx = 260;	/* Max paths */
      strcpy(pDosFsInfo->szFsType, "FAT");
    } else { /* errno set by intdos() */
      DEBUG_PRINTF(("dos_get_volume_info() -> return %d; // _doserrno=%d; errno=%d = %s\n", outreg.x.ax, _doserrno, errno, strerror(errno)));
      return outreg.x.ax; /* Return the DOS error code, not the same as errno */
    }
  }

  /* Correct a known bug in Win95 SP1 for CD-ROMs */
  if (!outreg.x.dx) outreg.x.dx = 260;

  /* Store results in the caller's structure */
  pDosFsInfo->iFlags = outreg.x.bx;
  pDosFsInfo->iNameLength = outreg.x.cx;
  pDosFsInfo->iPathLength = outreg.x.dx;

  DEBUG_PRINTF(("dos_get_volume_info() -> FS=%s; Flags=0x%04X; NAME_MAX=%d; PATH_MAX=%d;\n",
    pDosFsInfo->szFsType, pDosFsInfo->iFlags, pDosFsInfo->iNameLength, pDosFsInfo->iPathLength));

  return 0;
}

/* Make it easy to change the low-level getcwd routine used in getcwdX below */
#define GETDCWD dos_getcwd /* Either dos_getcwd or _getdcwd */

/* Reimplement getcwd() for DOS, in a way compatible with Posix */
char *getcwdX(char *pszBuf, int iBufLen) {
  char *pc = pszBuf;
  int iErr = 0;
  int iDrive = (char)bdos(0x19, 0, 0);	/* Get the current drive. 0=A 1=B ... */
  dos_fs_info dosFsInfo;
  char *pszLocalBuf = NULL;

  DEBUG_ENTER(("getcwd(%p, %d);\n", pszBuf, iBufLen));

  if (iBufLen < 4) {
    iErr = errno = ENOMEM;
    goto cleanup_and_return;
  }

  *(pc++) = (char)('A' + iDrive);
  *(pc++) = ':';
  *(pc++) = '\\';
  *pc = '\0';

  iErr = dos_get_volume_info(pszBuf, &dosFsInfo);
  if (iErr) goto cleanup_and_return;

  /* If the output buffer provided is potentially too small, 
     allocate one large enough for the worst case */
  if (iBufLen < dosFsInfo.iPathLength) {
    pszLocalBuf = malloc(dosFsInfo.iPathLength);
    if (!pszLocalBuf) goto cleanup_and_return;
    strcpy(pszLocalBuf, pszBuf);
    pc = pszLocalBuf + 3;
  }

  iBufLen -= 3;
  iDrive += 1;		/* 1=A 2=B 3=C ... */

  DEBUG_PRINTF((VALUEIZE(GETDCWD) "(%d, %p, %d);\n", iDrive, pc, iBufLen));
  iErr = (GETDCWD(iDrive, pc, iBufLen) == NULL);

  if (pszLocalBuf) {
    if ((int)strlen(pc-3) >= iBufLen) {
      iErr = errno = ERANGE;
      goto cleanup_and_return;
    }
    strcpy(pszBuf, pszLocalBuf);
  }

cleanup_and_return:
  if (pszLocalBuf) free(pszLocalBuf);
  if (iErr) RETURN_STRING_COMMENT(NULL, ("%s\n", strerror(errno)));
  RETURN_STRING(pszBuf);
}

#endif /* defined(_MSDOS) */


#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    getcwdW / getcwdM / getcwdA / getcwdU		      |
|									      |
|   Description:    Get the current directory, in UTF-16 / MBCS / ANSI / UTF-8|
|									      |
|   Parameters:     char *buf	    Buffer for the output                     |
|		    size_t bufSize  Buffer size				      |
|		    							      |
|   Returns:	    The converted string size. NULL=error, and errno set.     |
|		    							      |
|   Notes:	    The getcwd macro references either getcwdA or getcwdU.    |
|		    							      |
|		    Contrary to most other WIN32 APIs, SetCurrentDirectoryW() |
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
|		    the chdir routine locally manages the current directory.  |
|		    No attempt is made to manage multiple drive-specific      |
|		    current directories, as the goal is Unix-compatibility,   |
|		    not Windows compatibility.				      |
|		    							      |
|   History:								      |
|    2014-02-28 JFL Created this routine                               	      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
|    2017-10-03 JFL Removed the dependency on PATH_MAX and fixed size buffers.|
|                   Added routine getcwdM, called by getcwdA and getcwdU.     |
|    2017-10-04 JFL Remove the long pathname prefix, if any.		      |
|    2018-04-25 JFL Added routine getcwdW(), and use it in getcwdM().	      |
|		    Use the locally managed cur. dir. for paths > 260 bytes.  |
|		    Bugfix: Must test for \\?\UNC\ before testing for \\?\.   |
*									      *
\*---------------------------------------------------------------------------*/

extern WCHAR *pwszLongCurrentDir; /* Locally managed CD, defined in chdir.c */

WCHAR *getcwdW(WCHAR *pwBuf, size_t dwBufSize) {
  DWORD dwSize;
  WCHAR *pwDir;
  int iAlloc = FALSE;	/* TRUE if pwBuf allocated here */
  int iReAlloc = FALSE;	/* TRUE if pwBuf should be reallocated in the end */
  DEBUG_CODE(
  WCHAR *pwszWhere = L"Windows";
  )

  DEBUG_ENTER(("getcwd(0x%p, %Iu);\n", pwBuf, dwBufSize));

  if (!pwBuf) {
    iAlloc = TRUE;
    if (!dwBufSize) {
      iReAlloc = TRUE;
      dwBufSize = WIDE_PATH_MAX;
    }
    pwBuf = malloc(dwBufSize * sizeof(WCHAR));
    if (!pwBuf) return NULL;
  }
  if (pwszLongCurrentDir) {		/* If we manage the CD locally */
    dwSize = lstrlenW(pwszLongCurrentDir);	/* Then copy that CD */
    if (dwSize < dwBufSize) {
      lstrcpyW(pwBuf, pwszLongCurrentDir);
    } else {
      dwSize += 1; /* Buffer size needed */
    }
    DEBUG_CODE(
    pwszWhere = L"MsvcLibX";
    )
  } else {					/* Else ask Windows' */
    dwSize = GetCurrentDirectoryW((DWORD)dwBufSize, pwBuf);
  }
  if (dwSize > dwBufSize) { /* The buffer is too small. dwSize = the size needed */
    DEBUG_LEAVE(("return NULL; // Error: The buffer is too small. %d bytes needed.\n", dwSize));
getcwdW_failed:
    errno = Win32ErrorToErrno();
    if (iAlloc) free(pwBuf);
    return NULL;
  }
  if (!dwSize) {
    DEBUG_LEAVE(("return NULL; // Error: GetCurrentDirectoryW() Failed\n"));
    goto getcwdW_failed;
  }

  /* Remove the long pathname prefix, if any */
  pwDir = pwBuf;
  if (!strncmpW(pwBuf, L"\\\\?\\UNC\\", 8)) {
    pwDir += 6;		/* Remove the '\\?\UNC\' prefix, except for the final two characters */
    dwSize -= 6;
    *pwDir = L'\\';	/* Change the 'C' to '\', so that the output begins by '\\server\share' */
  } else if (!strncmpW(pwBuf, L"\\\\?\\", 4)) {
    pwDir += 4;
    dwSize -= 4;
  }
  if (pwDir > pwBuf) memmove(pwBuf, pwDir, (dwSize+1) * sizeof(WCHAR));

  /* Cleanup and return */
  if (iReAlloc) pwBuf = ShrinkBuf(pwBuf, (dwSize + 1) * sizeof(WCHAR));
  DEBUG_WLEAVE((L"return \"%s\"; // [%s]\n", pwBuf, pwszWhere));
  return pwBuf;
}

char *getcwdM(char *buf, size_t bufSize, UINT cp) {
  int n;
  WCHAR *pwDir;

  if (!buf) {
    errno = EINVAL;
    return NULL;
  }

  pwDir = getcwdW(NULL, 0);
  if (!pwDir) return NULL;

  /* Copy the pathname to the output buffer */
  n = WideCharToMultiByte(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pwDir,		/* lpWideCharStr, */
			  -1,			/* cchWideChar, -1=NUL-terminated string */
			  buf,			/* lpMultiByteStr, */
			  (int)bufSize,		/* cbMultiByte, */
			  NULL,			/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  free(pwDir);
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

