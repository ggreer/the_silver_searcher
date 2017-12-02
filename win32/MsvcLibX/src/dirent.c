/*****************************************************************************\
*                                                                             *
*   Filename:	    dirent.c						      *
*                                                                             *
*   Description:    DOS/WIN32 port of standard C library's dirent.h functions *
*                                                                             *
*   Notes:	    TO DO: Make Wide & MultiByte versions of scandir 4 Windows*
*                                                                             *
*		    There are also remains of an OS/2 implementation here.    *
*		    It's not maintained anymore. Left in as a historic        *
*		    reference, in the unlikely case somebody needs it.        *
*									      *
*   History:								      *
*    2012-01-09 JFL Created this file, based on dirc and other programs.      *
*    2012-01-19 JFL Added standard errno management.                          *
*    2012-05-22 JFL Fixed a bug in the DOS version, which failed if the       *
*		    directory name was longer than 12 bytes.                  *
*    2012-05-23 JFL Fixed opendir to return errors as per the C lib spec.     *
*    2013-02-12 JFL Added code to filter reparse points, and keep only        *
*		    real junctions and symlinks.                              *
*    2013-02-13 JFL Moved dirent2stat() to lstat.c, as there must actually    *
*		    be 4 WIN32 versions, for the four versions of struct stat.*
*    2013-02-26 JFL Moved the proprietary file time management routines to    *
*		    the new filetime.c module.				      *
*    2014-02-27 JFL Changed the WIN32 output name encoding to UTF-8.	      *
*    2014-03-20 JFL Restructured Windows opendir and readdir functions into   *
*		    Wide and MultiByte versions, and changed the Unicode and  *
*		    Ansi versions to macros.				      *
*    2014-03-24 JFL Renamed "statx.h" as the standard <sys/stat.h>.	      *
*    2015-12-14 JFL Bug fix: WIN32 readdirW always read the root on "D:".     *
*		    Bug fix: DOS opendir failed on root dirs, like "D:\".     *
*    2017-05-11 JFL Recognize LinuX SubSystem symlinks.			      *
*    2017-06-27 JFL Renamed IO_REPARSE_TAG_LXSS_SYMLINK as ..._TAG_LX_SYMLINK.*
*		    Added IO_REPARSE_TAG_NFS to known symbolic link types.    *
*    2017-10-02 JFL Removed dependencies on MAX_PATH or PATH_MAX.	      *
*		    Fixed support for pathnames >= 260 characters.	      *
*		    							      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of WIN32 printf & scandir */

#include "dirent.h" /* Include our associated .h, in the same dir as this .c. Do not use <>. */
#ifndef _DIRENT_FOR_DOS_WINDOWS
#error "This requires MsvcLibX own version of dirent.h for DOS/Windows"
#endif
/* Microsoft C libraries include files */
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
/* MsvcLibX library extensions */
#include <unistd.h>	/* For readlink() */
#include <sys/stat.h>	/* For Filetime2String() */
#include "debugm.h"	/* Use our house debugging framework */

/*****************************************************************************\
*                                                                             *
*                               MS_DOS Version                                *
*                                                                             *
\*****************************************************************************/

#ifdef _MSDOS

#define OFFSET_OF(pointer) ((uint16_t)(uint32_t)(void far *)pointer)
#define SEGMENT_OF(pointer) ((uint16_t)(((uint32_t)(void far *)pointer) >> 16))

void put_dta(char *p_dta) {	/* Set the MS-DOS Disk Transfer Address */
  union REGS inreg;
  union REGS outreg;
  struct SREGS sregs;

  inreg.h.ah = 0x1a;
  inreg.x.dx = OFFSET_OF(p_dta);
  sregs.ds = SEGMENT_OF(p_dta);

  intdosx(&inreg, &outreg, &sregs);
}

#define CF 0x0001            /* Carry flag bit mask */

int get_file_attributes(const char *name, unsigned *pAttr) {	/* Get File Attributes */
  union REGS inreg;
  union REGS outreg;
  struct SREGS sregs;

  inreg.x.ax = 0x4300;
  inreg.x.dx = OFFSET_OF(name);
  sregs.ds = SEGMENT_OF(name);

  intdosx(&inreg, &outreg, &sregs);

  if (CF & outreg.x.cflag) {
    errno = outreg.x.ax;
    return errno;
  }

  *pAttr = outreg.x.cx;
  return 0;
}

/* Workaround for a VMWare player shared folders bug:
   DOS function 4e (search next) sometimes returns several times the same entry. */
static fileinfo previousFI;
#ifdef _DEBUG
static int report_workaround(char *s) {
  DEBUG_PRINTF((s));
  return 1;
}
#define REPORT_WORKAROUND(args) report_workaround args
#else
#define REPORT_WORKAROUND(args) 1
#endif

int srch1st(char *pszFile, uint16_t wAttr, fileinfo *pInfo) { /* Search first matching file */
  union REGS inreg;
  union REGS outreg;
  struct SREGS sregs;

  DEBUG_ENTER(("srch1st(\"%s\", 0x%04X, 0x%p);\n", pszFile, wAttr, pInfo));

  /* Make sure the DTA is assigned before calling DOS functions 4E and 4F */
  put_dta((char *)pInfo);

  inreg.h.ah = 0x4e;
  inreg.x.cx = wAttr;
  inreg.x.dx = OFFSET_OF(pszFile);
  sregs.ds = SEGMENT_OF(pszFile);

  intdosx(&inreg, &outreg, &sregs);

  if (CF & outreg.x.cflag) {
    DEBUG_LEAVE(("return %d; // DOS error code\n", outreg.x.ax));
    return (int)(outreg.x.ax);
  }

  previousFI = *pInfo; /* Save it for the workaround for the VMWare player bug */

  DEBUG_LEAVE(("return 0;  // Success\n"));
  return 0;
}

int srchnext(fileinfo *pInfo) { /* Search next matching file */
  union REGS inreg;
  union REGS outreg;

  DEBUG_ENTER(("srchnext(0x%p);\n", pInfo));

  /* Make sure the DTA is assigned before calling DOS functions 4E and 4F */
  put_dta((char *)pInfo);

  inreg.h.ah = 0x4f;

  do {
    intdos(&inreg, &outreg);

    if (CF & outreg.x.cflag) {
      DEBUG_LEAVE(("return %d; // DOS error code\n", outreg.x.ax));
      return(outreg.x.ax);
    }
  } while ((!strncmp(previousFI.fiFileName, pInfo->fiFileName, sizeof(previousFI)))
           && REPORT_WORKAROUND(("// Skipped one duplicate entry\n")));

  previousFI = *pInfo; /* Save it for the workaround for the VMWare player bug */

  DEBUG_LEAVE(("return 0;  // Success\n"));
  return 0;
}

DIR *opendir(const char *name) { /* Open a directory */
  DIR *pDir = NULL;
  size_t lName;
  unsigned attr;
  char *pszWildCards = "\\*.*";
  char *pszCopy;
  DEBUG_ENTER(("opendir(\"%s\");\n", name));
  lName = strlen(name);
  if (lName == 0) {
opendir_noent:
    errno = ENOENT;
opendir_failed:
    if (!_sys_errlist[ENOTDIR][0]) _sys_errlist[ENOTDIR] = "Not a directory"; /* Workaround for the missing entry in MSVC list */
    if (pDir) free(pDir);
    DEBUG_LEAVE(("return NULL; // errno=%d - %s\n", errno, strerror(errno)));
    return NULL;
  }
  pDir = (DIR *)malloc(sizeof(DIR) + lName + 5); /* + 5 for wildcards suffix */
  if (!pDir) goto opendir_failed;
  /* Work on a copy of the directory name */
  pszCopy = (char *)(pDir + 1);
  strcpy(pszCopy, name);
  /* First change: Except for the root, Remove the trailing \s, which confuses get_file_attributes() */
  while ((lName > 1) && (name[lName-1] == '\\') && (name[lName-2] != ':')) pszCopy[--lName] = '\0';
  if (get_file_attributes(pszCopy, &attr)) goto opendir_noent;
  if (!(attr & _A_SUBDIR)) {
    errno = ENOTDIR;
    goto opendir_failed;
  }
  if (name[lName-1] == '\\') pszWildCards += 1; /* Avoid duplicating the \ */
  strcpy(pszCopy+lName, pszWildCards);
  pDir->first = 1;
  DEBUG_LEAVE(("return 0x%p;\n", pDir));
  return pDir;
}

int closedir(DIR *pDir) { /* Close the directory. Return 0 if successful, -1 if not. */
  DEBUG_PRINTF(("closedir(0x%p);\n", pDir));
  if (pDir) free(pDir);
  return 0;
}

_dirent *readdir(DIR *pDir) { /* Read a directory entry. Return pDir, or NULL for EOF or error. */
  int iErr;
  _dirent *pDirent = &pDir->sDirent;
  fileinfo *pFI = (fileinfo *)(pDirent->d_reserved); /* Address of the fileinfo structure embedded there */
#ifdef _DEBUG
  char szTime[40];
#endif

  DEBUG_ENTER(("readdir(0x%p);\n", pDir));
  if (pDir->first) { /* First search */
    iErr = srch1st((char *)(pDir+1), 0x3F, pFI);
    pDir->first = 0;
  } else {
    iErr = srchnext(pFI);
  }
  if (!iErr) {
    pDirent->d_type = DT_REG;			/* A normal file by default */
    if (pDirent->d_attribs & _A_SUBDIR) pDirent->d_type = DT_DIR;  /* Subdirectory */
    if (pDirent->d_attribs & _A_VOLID) pDirent->d_type = DT_VOLID; /* Volume ID file */
    DEBUG_LEAVE(("return 0x%p; // %s %02X %10ld %s\n",
		  pDirent,
		  Filetime2String(pDirent->d_date, pDirent->d_time, szTime, sizeof(szTime)),
		  pDirent->d_attribs,
		  pDirent->d_filesize,
		  pDirent->d_name));
    return &pDir->sDirent;
  }
  switch (iErr) { /* Correct a few errors that do not map well to C library errors */
    case ESRCH: iErr = ENOTDIR; break;
    case EXDEV: iErr = 0; break;	/* End of files is NOT an error */
  }
  if (iErr) {
    errno = iErr; /* MS-DOS' errno.h maps C-library errnos to DOS' errors */
    DEBUG_LEAVE(("return NULL; // errno=%d - %s\n", errno, strerror(errno)));
  } else {
    DEBUG_LEAVE(("return NULL; // End of directory\n"));
  }
  return NULL;
}

#endif /* defined(_MSDOS) */

/*****************************************************************************\
*                                                                             *
*                               WIN32 Version                                 *
*                                                                             *
\*****************************************************************************/

#ifdef _WIN32

#include <windows.h>
#include "reparsept.h"	/* For the undocumented IO_REPARSE_TAG_LX_SYMLINK */

/* Requires including windows.h and especially the kernel section */

DIR *opendirW(const WCHAR *wszName) { /* Open a directory - Wide char version */
  DIR *pDir;
  int err;
  DWORD dw;
  DEBUG_CODE(
  char *pszUtf8;
  )
  int lName;

  DEBUG_WSTR2NEWUTF8(wszName, pszUtf8);
  DEBUG_ENTER(("opendir(\"%s\");\n", pszUtf8));
  DEBUG_FREEUTF8(pszUtf8);

  dw = GetFileAttributesW(wszName);
  err = 0;
  if (dw == INVALID_FILE_ATTRIBUTES) {
    err = errno = Win32ErrorToErrno();
  } else {
    if (!(dw & _A_SUBDIR)) {
      err = errno = ENOTDIR;
    }
  }
  if (err) {
return_err:
    RETURN_CONST_COMMENT(NULL, ("errno=%d - %s\n", errno, strerror(errno)));
  }
  pDir = malloc(sizeof(DIR));
  if (!pDir) {
return_ENOMEM:
    errno = ENOMEM;
    goto return_err;
  }
  lName = lstrlenW(wszName);
  pDir->pwszDirName = malloc(sizeof(WCHAR) * (lName + 1));
  if (!pDir->pwszDirName) {
    free(pDir);
    goto return_ENOMEM;
  }
  pDir->hFindFile = INVALID_HANDLE_VALUE;
  lstrcpyW(pDir->pwszDirName, wszName);
  DEBUG_LEAVE(("return 0x%p; // Success\n", pDir));
  return pDir;
}

DIR *opendirM(const char *pszName, UINT cp) { /* Open a directory - MultiByte char version */
  WCHAR *pwszName;
  DIR *result;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(cp, pszName);
  if (!pwszName) return NULL;

  result = opendirW(pwszName);
  free(pwszName);
  return result;
}

int closedir(DIR *pDir) { /* Close the directory. Return 0 if successful, -1 if not. */
  DEBUG_PRINTF(("closedir(0x%p);\n", pDir));
  if (pDir) {
    free(pDir->pwszDirName);
    if (pDir->hFindFile != INVALID_HANDLE_VALUE) FindClose(pDir->hFindFile);
    pDir->hFindFile = INVALID_HANDLE_VALUE;
    free(pDir);
  }
  return 0;
}

/* Read a directory entry. Return pDir, or NULL for EOF or error. Wide char version. */
_dirent *readdirW(DIR *pDir) {
  int iErr = 0;
  _dirent *pDirent = &pDir->sDirent;
  int bIsJunction = FALSE;
  int bIsMountPoint = FALSE;
  DWORD dwTag = 0; /* Reparse points tag */
  DWORD dwAttr;
  int n;
  DEBUG_CODE(
  char szTime[40];
  char *pszUtf8;
  )

  DEBUG_ENTER(("readdir(0x%p);\n", pDir));

  if (pDir->hFindFile == INVALID_HANDLE_VALUE) {
    WCHAR *pwszPattern;
    n = lstrlenW(pDir->pwszDirName);
    pwszPattern = malloc(sizeof(WCHAR) * (n + 5)); /* Room for wszDirName + "\*.*" */
    if (pwszPattern) {
      lstrcpyW(pwszPattern, pDir->pwszDirName);
      if (n && (pwszPattern[n-1] != L'\\') && (pwszPattern[n-1] != L':')) pwszPattern[n++] = L'\\';
      lstrcpyW(pwszPattern+n, L"*.*");
      pDir->hFindFile = FindFirstFileW(pwszPattern, &pDir->wfd);
      if (pDir->hFindFile == INVALID_HANDLE_VALUE) {
	iErr = Win32ErrorToErrno();
	if (!iErr) iErr = ENOENT;
      }
      free(pwszPattern);
    } else {
      iErr = errno; /* ENOMEM */
    }
  } else {
    iErr = !FindNextFileW(pDir->hFindFile, &pDir->wfd);
    if (iErr) {
      iErr = Win32ErrorToErrno();
      if (!iErr) iErr = ENOENT;
    }
  }
  if (iErr) {
    switch (iErr) { /* Correct a few errors that do not map well to C library errors */
      case ESRCH: iErr = ENOTDIR; break;
      case EXDEV: iErr = 0; break;	/* End of files is NOT an error */
    }
    if (iErr) {
      errno = iErr; /* Windows' errno.h maps C-library errnos to Windows' errors */
      DEBUG_LEAVE(("return NULL; // errno=%d - %s\n", errno, strerror(errno)));
    } else {
      DEBUG_LEAVE(("return NULL; // End of directory\n"));
    }
    return NULL;
  }

  /* Set the standard fields */
  lstrcpyW((WCHAR *)(pDirent->d_name), pDir->wfd.cFileName);
  dwAttr = pDir->wfd.dwFileAttributes;
check_attr_again:
  if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
    /* JUNCTIONs and SYMLINKDs both have the FILE_ATTRIBUTE_DIRECTORY flag also set.
    // Test the FILE_ATTRIBUTE_REPARSE_POINT flag first, to make sure they're seen as symbolic links.
    //
    // All symlinks are reparse points, but not all reparse points are symlinks. */
    dwTag = pDir->wfd.dwReserved0;	/* No need to call GetReparseTag(), we got it already. */
    switch (dwTag) {
      case IO_REPARSE_TAG_MOUNT_POINT:	/* NTFS junction or mount point */
	{ /* We must read the link to distinguish junctions from mount points. */
	WCHAR *pwszPath = NULL;
	WCHAR *pwszBuf = NULL;
	ssize_t n = lstrlenW(pDir->pwszDirName);
	ssize_t lwPath = n + 1 + lstrlenW(pDir->wfd.cFileName) + 1;
	ssize_t lwBuf = PATH_MAX; /* This will be sufficient in most cases, If not, the buf will be extended below */
	pwszPath = malloc(sizeof(WCHAR) * lwPath);
	if (!pwszPath) {
return_ENOMEM:
	  DEBUG_LEAVE(("return NULL; // Out of memory\n"));
	  return NULL;
	}
	bIsMountPoint = TRUE;
	lstrcpyW(pwszPath, pDir->pwszDirName);
	if (n && (pwszPath[n-1] != L'\\')) pwszPath[n++] = L'\\';
	lstrcpyW(pwszPath+n, pDir->wfd.cFileName);
realloc_wBuf:
	pwszBuf = malloc(sizeof(WCHAR) * lwBuf);
	if (!pwszBuf) {
	  free(pwszPath);
	  goto return_ENOMEM;
	}
	n = readlinkW(pwszPath, pwszBuf, lwBuf);
	/* Junction targets are absolute pathnames, starting with a drive letter. Ex: C: */
	/* readlink() fails if the reparse point does not target a valid pathname */
	if (n < 0) {
	  if (errno == ENAMETOOLONG) { /* The output buffer was too small. Retry with a bigger one */
	    free(pwszBuf); /* No need to copy the old content */
	    lwBuf *= 2;
	    goto realloc_wBuf;
	  }
	  free(pwszPath);
	  free(pwszBuf);
	  goto this_is_not_a_symlink; /* This is not a junction. */
	}
	bIsJunction = TRUE; /* Else this is a junction. Fall through to the symlink case. */
	free(pwszPath);
	free(pwszBuf);
	} 	      
      case IO_REPARSE_TAG_SYMLINK:		/* NTFS symbolic link */
      case IO_REPARSE_TAG_NFS:			/* NFS symbolic link */
      case IO_REPARSE_TAG_LX_SYMLINK:		/* LinuX subsystem symlink */
	pDirent->d_type = DT_LNK;		/* Symbolic link */
	break;
      default:	/* Anything else is definitely not like a Unix symlink */
this_is_not_a_symlink:
	dwAttr &= ~FILE_ATTRIBUTE_REPARSE_POINT;
	goto check_attr_again;
    }
  } else if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
    pDirent->d_type = DT_DIR;		/* Subdirectory */
  else if (dwAttr & FILE_ATTRIBUTE_DEVICE)
    pDirent->d_type = DT_CHR;		/* Device (we don't know if character or block) */
  else
    pDirent->d_type = DT_REG;		/* A normal file by default */

  /* Set the OS-specific extensions */
  lstrcpyW((WCHAR *)(pDirent->d_shortname), pDir->wfd.cAlternateFileName);
  pDirent->d_attribs = dwAttr;
  pDirent->d_ReparseTag = dwTag;
  pDirent->d_CreationTime = pDir->wfd.ftCreationTime;
  pDirent->d_LastAccessTime = pDir->wfd.ftLastAccessTime;
  pDirent->d_LastWriteTime = pDir->wfd.ftLastWriteTime;
  (*(ULARGE_INTEGER *)&(pDirent->d_filesize)).LowPart = pDir->wfd.nFileSizeLow;
  (*(ULARGE_INTEGER *)&(pDirent->d_filesize)).HighPart = pDir->wfd.nFileSizeHigh;

  DEBUG_WSTR2NEWUTF8((WCHAR *)(pDirent->d_name), pszUtf8);
  DEBUG_LEAVE(("return 0x%p; // %s 0x%05X %10lld %s\n",
		pDirent,
		Filetime2String(&pDirent->d_LastWriteTime, szTime, sizeof(szTime)),
		(int)(pDirent->d_attribs),
		pDirent->d_filesize,
		pszUtf8));
  DEBUG_FREEUTF8(pszUtf8);
  return &(pDir->sDirent);
}

/* Read a directory entry. Return pDir, or NULL for EOF or error. MultiByte char version. */
_dirent *readdirM(DIR *pDir, UINT cp) {
  _dirent *pDirent;
  int n;
  char *pszDefaultChar;

  pDirent = readdirW(pDir);
  if (!pDirent) return NULL;

  /* Convert the name field back to MultiByte encoding */
  pszDefaultChar = (cp == CP_UTF8) ? NULL : "?";
  n = WideCharToMultiByte(cp,					/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,					/* dwFlags, */
			  pDir->wfd.cFileName,			/* lpWideCharStr, */
			  lstrlenW(pDir->wfd.cFileName)+1,	/* cchWideChar, */
			  pDirent->d_name,			/* lpMultiByteStr, */
			  sizeof(pDirent->d_name),		/* cbMultiByte, */
			  pszDefaultChar,			/* lpDefaultChar, */
			  NULL					/* lpUsedDefaultChar */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("readdirM(0x%p); // Error converting name back from Unicode. errno=%d - %s\n", pDir, errno, strerror(errno)));
    return NULL;
  }

  /* Convert the short name field back to MultiByte encoding */
  n = WideCharToMultiByte(cp,						/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,						/* dwFlags, */
			  pDir->wfd.cAlternateFileName,			/* lpWideCharStr, */
			  lstrlenW(pDir->wfd.cAlternateFileName)+1,	/* cchWideChar, */
			  pDirent->d_shortname,				/* lpMultiByteStr, */
			  sizeof(pDirent->d_shortname),			/* cbMultiByte, */
			  pszDefaultChar,				/* lpDefaultChar, */
			  NULL						/* lpUsedDefaultChar */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("readdirM(0x%p); // Error converting short name back from Unicode. errno=%d - %s\n", pDir, errno, strerror(errno)));
    return NULL;
  }

  return pDirent;
}

#endif

/*****************************************************************************\
*                                                                             *
*			      OS/2 1.x Version				      *
*                                                                             *
\*****************************************************************************/

#ifdef _OS2

/* Requires including os2.h at the beginning of this file, and before that
   defining the INCL_DOSFILEMGR constant to enable the necessary section */

int SetDirent(DIR *pDir) {	    /* Convert the FILEFINDBUF to a dirent structure */
  char *pc;
  _dirent *pDirent = &pDir->sDirent;
  FILEFINDBUF *pbuf = &pDir->buf;

  pbuf->achName[pbuf->cchName] = '\0';
  pc = strrchr(pbuf->achName, '\\'); /* Directory separator character */
  if (!pc)
    pc = &pbuf->achName[0];
  else
    pc += 1;    /* Skip the \ */

  pDirent->d_name = malloc(strlen(pc)+1);
  if (!pDirent->d_name) return ENOMEM;
  strcpy(pDirent->d_name, pc);
  pDirent->attribs = pbuf->attrFile;
  pDirent->d_type = Atype2Dtype(pDirent->attribs);
  pDirent->time = *(uint16_t *)&pbuf->ftimeLastWrite;
  pDirent->date = *(uint16_t *)&pbuf->fdateLastWrite;
  pDirent->filesize = pbuf->cbFile;

  return 0;
}

int srch1st(char *pszFile, uint8_t bAttrReq, uint8_t bAttrCmp, DIR *pDir) { /* Search first matching file */
  int n;
  int err;

  pDir->hDir = -1;

  n = 1;  /* Get one file */
  err = DosFindFirst2(pszFile, &pDir->hDir, bAttrReq & 0x7F, &pDir->buf,
		      sizeof(FILEFINDBUF), &n, FIL_STANDARD, 0L);
  if (err || !n) return 1;

  return SetDirent(pDir);
}

int srchnext(DIR *pDir) {   /* Search next matching file */
  int n;
  int err;

  n = 1;  /* Get one file */
  err = DosFindNext(pDir->hDir, &pDir->buf, sizeof(FILEFINDBUF), &n);
  if (err || !n) return 1;

  return SetDirent(&buf);
}

int srchdone(DIR *pDir) {
  int err;

  err = DosFindClose(pDir->hDir);
  pDir->hDir = -1;

  return err;
}

DIR *opendir(const char *name) { /* Open a directory */
  DIR *pDir;
  DEBUG_ENTER(("opendir(\"%s\");\n", name));
  err = _access(name, 0);
  /* To do: Get the file attributes, and check that it's a directory */
  if (err) {
    DEBUG_LEAVE(("return NULL; // errno=%d - %s\n", errno, strerror(errno)));
    return NULL;
  }
  pDir = malloc(sizeof(DIR));
  if (pDir) {
    pDir->hDir = -1;
    pDir->bAttrReq = _A_HIDDEN | _A_SYSTEM | _A_SUBDIR;
    pDir->bAttrCmp = 0;
    strcpy(pDir->sDirent.d_name, name);
  }
  DEBUG_LEAVE(("return 0x%p;\n", pDir));
  return pDir;
}

int closedir(DIR *pDir) { /* Close the directory. Return 0 if successful, -1 if not. */
  DEBUG_PRINTF(("closedir(0x%p);\n", pDir));
  if (pDir) {
    srchdone(pDir);
    free(pDir);
  }
  return 0;
}

_dirent *readdir(DIR *pDir) { /* Read a directory entry. Return pDir, or NULL for EOF or error. */
  int iErr;
  DEBUG_ENTER(("readdir(0x%p);\n", pDir));
  if (pDir->hDir == -1) {
    iErr = srch1st(pDir->sDirent.d_name, pDir->bAttrReq, pDir->bAttrCmp, pDir);
  } else {
    iErr = srchnext(pDir);
  }
  if (iErr) {
    DEBUG_LEAVE(("return NULL; // OS/2 found nothing\n",
    return NULL;
  }

  DEBUG_LEAVE(("return 0x%p; // OS/2 found: %04X %04X %02X %10lld %s\n",
		&pDir->sDirent
		(int)(pDirent->time),
		(int)(pDirent->date),
		(int)(pDirent->attribs),
		pDirent->filesize,
		pDirent->d_name));


  return &pDir->sDirent;
}

#endif

/*****************************************************************************\
*                                                                             *
*            End of OS-specific opendir/readdir/closedir versions             *
*                                                                             *
\*****************************************************************************/

/*****************************************************************************\
*                                                                             *
*   Function:	    scandir		 				      *
*									      *
*   Description:    Select entries in a directory			      *
*									      *
*   Arguments:	    const char *name	Directory name            	      *
*		    _dirent ***namelist where to store the result array       *
*		    int (*cbSelect)()   Selection callback function           *
*		    int (*cbCompare)()  Comparison function for sorting it    *
*									      *
*   Return value:   # of entries in the array, or -1 if error.		      *
*									      *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2012-01-11 JFL Initial implementation				      *
*                                                                             *
\*****************************************************************************/

#pragma warning(disable:4706) /* Ignore the "assignment within conditional expression" warning */
int scandir(const char *pszName,
	    _dirent ***resultList,
	    int (*cbSelect) (const _dirent *),
	    int (__cdecl *cbCompare) (const _dirent **,
				      const _dirent **)) {
  int n = 0;
  DIR *pDir;
  _dirent *pDirent;
  _dirent *pDirent2;
  _dirent **pList = NULL;
  _dirent **pList2;

  DEBUG_ENTER(("scandir(\"%s\", 0x%p, 0x%p, 0x%p);\n", pszName, resultList, cbSelect, cbCompare));

  pDir = opendir(pszName);
  if (!pDir) {
    DEBUG_LEAVE(("return -1; // errno=%d\n", errno));
    return -1;
  }

  while (pDirent = readdir(pDir)) {
    if (cbSelect && !cbSelect(pDirent)) continue; /* We don't want this one. Continue search. */
    /* OK, we've selected this one. So append a copy of this _dirent to the list. */
    n += 1;
    pList2 = (_dirent **)realloc(pList, n * sizeof(_dirent *));
    pDirent2 = malloc(sizeof(_dirent));
    if (!pList2 || !pDirent2) {
      if (pDirent2) free(pDirent2);
      for (n-=1; n>0; ) free(pList[--n]);
      /* errno = ENOMEM; /* Out of memory. Should already be set by malloc failure */
      DEBUG_LEAVE(("return -1; // errno=%d\n", errno));
      return -1;
    }
    *pDirent2 = *pDirent;
    pList = pList2;
    pList[n-1] = pDirent2;
  }

  closedir(pDir);

/* 2016-09-23 JFL I don't understand why this warning still fires, so leaving it enabled for now */
#ifdef M_I86TM /* This warning appears only when compiling for the DOS tiny memory model ?!? */
/* #pragma warning(disable:4220) /* Ignore the "varargs matches remaining parameters" warning */
#endif
  if (cbCompare) qsort(pList, n, sizeof(_dirent *), cbCompare);
#ifdef M_I86TM
#pragma warning(default:4220) /* Ignore the "varargs matches remaining parameters" warning */
#endif
  *resultList = pList;
  DEBUG_LEAVE(("return %d;\n", n));
  return n;
}
#pragma warning(default:4706)

int __cdecl alphasort(const _dirent **ppDE1, const _dirent **ppDE2) {
  int ret;
  /* Sort names a-la Windows, that is case insensitive */
  ret = _strnicmp((*ppDE1)->d_name, (*ppDE2)->d_name, NAME_MAX);
  if (ret) return ret;
  /* For the remote chance that we're accessing a Unix share */
  ret = _strnicmp((*ppDE1)->d_name, (*ppDE2)->d_name, NAME_MAX);
  if (ret) return ret;
  return 0;
}

