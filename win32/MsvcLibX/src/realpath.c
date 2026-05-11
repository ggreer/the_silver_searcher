/*****************************************************************************\
*                                                                             *
*   Filename	    realpath.c						      *
*									      *
*   Description     Resolve links and remove . and .. parts in pathnames      *
*                                                                             *
*   Notes	    TO DO: Microsoft provides a convenient routine, for which *
*                   we could provide a MultiByte version: (in MSVC's stdlib.h)*
*    char *_fullpath(char *absPath, const char *relPath, size_t maxLength);   *
*                                                                             *
*   History								      *
*    2014-02-10 JFL Created this module with routine ResolveLinks() for Win32.*
*    2014-02-19 JFL Added OS-independant CompactPath() subroutine. 	      *
*    2014-02-20 JFL Implemented realpath() for both DOS and Windows.	      *
*    2014-03-06 JFL Check for buffer overflows, and return ENAMETOOLONG.      *
*    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      *
*    2016-08-25 JFL Added routine ResolveLinksA().			      *
*    2016-09-12 JFL Moved GetFileAttributesU() to its own module.	      *
*                   Bug fix: Add the drive letter if it's not specified.      *
*                   Bug fix: Detect and report output buffer overflows.       *
*                   Convert short WIN32 paths to long paths.                  *
*    2016-09-13 JFL Resize output buffers, to avoid wasting lots of memory.   *
*    2017-10-03 JFL Added an ugly version of ResolveLinksM. To be fixed.      *
*    2017-10-04 JFL Added WIN32 routine CompactPathW().			      *
*    2017-10-30 JFL Added support for UNC paths in CompactPath[W]().	      *
*    2018-04-24 JFL Use less memory in ResolveLinksU().			      *
*    2018-04-26 JFL Added routine ConcatPathW().       			      *
*    2021-11-29 JFL Renamed ResolveLinks*() as MlxResolveLinks*().	      *
*    2025-12-22 JFL Tiny simplification in MlxResolveLinksA().                *
*    2026-01-03 JFL Moved CompactPath() & CompactPathW() to CompactPath.c.    *
*    2026-01-22 JFL Added routines MlxResolveSubstDrives(), MlxGetFileName(). *
*                   Converted MlxResolveLinks() to a wide W routine, with     *
*                   M U A versions deriving from it.			      *
*    2026-01-23 JFL Converted realpath() to a wide W routine, with M U A      *
*                   versions deriving from it.				      *
*    2026-02-12 JFL Manage the case of APPEXECLINKs.			      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <direct.h>	/* For _getdcwd() */
#include <ctype.h>	/* For toupper() */
#include <limits.h>	/* Defines PATH_MAX and NAME_MAX */
#include <sys/stat.h>
#include <iconv.h>
#include "debugm.h"

#define TRUE 1
#define FALSE 0


#ifdef _MSDOS

/* Standard C library routine realpath() */
/* Normally defined in stdlib.h. Output buf must contain PATH_MAX bytes */
char *realpath(const char *path, char *outbuf) {
  char *pOutbuf = outbuf;
  int iErr;
  const char *pc;

  if (!pOutbuf) pOutbuf = malloc(PATH_MAX);
  if (!pOutbuf) {
    errno = ENOMEM;
    return NULL;
  }

  /* Convert relative paths to absolute paths */
  pc = path;
  if (pc[0] && (pc[1] == ':')) pc += 2; /* Skip the drive letter, if any */
  if ((*pc != '/') && (*pc != '\\')) { /* This is a relative path */
    int iDrive = 0;
    if (pc != path) { /* A drive was specified */
      iDrive = _toupper(path[0]) - '@'; /* A=1, B=2, ... */
    }
    _getdcwd(iDrive, pOutbuf, PATH_MAX);
    if ((strlen(pOutbuf) + strlen(pc) + 2) > PATH_MAX) {
realpath_failed:
      errno = ENAMETOOLONG;
      if (!outbuf) free(pOutbuf);
      return NULL;
    }
    strcat(pOutbuf, "\\");
    strcat(pOutbuf, pc);
    path = pOutbuf;
  } else if (pc == path) { /* This is an absolute path without a drive letter */
    pOutbuf[0] = (char)(_getdrive() + 0x40);
    pOutbuf[1] = ':';
    if ((strlen(path) + 3) > PATH_MAX) goto realpath_failed;
    strcpy(pOutbuf+2, path);
    path = pOutbuf;
  }

  /* TO DO: Resolve substituted drives */

  /* TO DO: Convert short paths to long paths, and correct the name case */

  /* Remove useless parts in the absolute path */
  iErr = CompactPath(path, pOutbuf, PATH_MAX);
  if (iErr == -1) { /* CompactPath() sets errno */
    if (!outbuf) free(pOutbuf);
    return NULL;
  }

  if (!outbuf) pOutbuf = ShrinkBuf(pOutbuf, strlen(pOutbuf) + 1);
  return pOutbuf;
}

#endif


#ifdef _WIN32

#include <windows.h>	/* Also includes MsvcLibX' WIN32 UTF-8 extensions */
#include "reparsept.h" /* For the undocumented IO_REPARSE_TAG_APPEXECLINK, etc */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxResolveLinks					      |
|									      |
|   Description	    Resolve all link names within a pathname		      |
|									      |
|   Parameters	    const char *path	    Pathname to resolve 	      |
|		    char *buf		    Output buffer    		      |
|		    size_t bufsize	    Size of the output buffer         |
|									      |
|   Notes	    Description of the official path resolution process:      |
|		    http://man7.org/linux/man-pages/man7/path_resolution.7.html
|									      |
|		    This function always returns a valid string in the output |
|		    buffer, even when it fails. This allows displaying a clue |
|		    about at which stage of the resolution things went wrong. |
|									      |
|   Returns	    0 = success, or -1 = error, with errno set		      |
|									      |
|   History								      |
|    2014-02-07 JFL Created this routine                                      |
|    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      |
|    2017-03-20 JFL Bug fix: First convert relative paths to absolute paths.  |
|    2026-01-22 JFL Converted to a wide routine, M U A vers. depending on it. |
*									      *
\*---------------------------------------------------------------------------*/

/* Linked list of previous pathnames */
typedef struct _WNAMELIST {
  struct _WNAMELIST *prev;
  WCHAR *wpath;
} WNAMELIST;

/* Get the canonic name of a file, after resolving all links in its pathname */
int MlxResolveLinksW1(const WCHAR *wpath, WCHAR *wbuf, size_t bufsize, WNAMELIST *prev, int iDepth) {
  WCHAR *wtarget = NULL;
  int i;
  int iErr = 0;
  DWORD dwAttr;
  WCHAR wc = L'\0';
  int iPath = 0;
  int bFirst = 1;
  ssize_t nRead;
  int iBuf = 0;
  WNAMELIST list;
  WNAMELIST *pList;
  WCHAR *wpath0 = NULL;

  DEBUG_WENTER((L"MlxResolveLinks1(\"%s\", 0x%p, %lu, 0x%p, %d);\n", wpath, wbuf, (ULONG)bufsize, prev, iDepth));

  wtarget = malloc(sizeof(WCHAR) * WIDE_PATH_MAX);
  if (!wtarget) RETURN_INT_COMMENT(-1, ("Not enough memory\n"));

  /* Convert relative paths to absolute paths */
  if (!(   ((wpath[0] == L'\\') || (wpath[0] == L'/'))
        || (    wpath[0]
            && (wpath[1] == L':')
            && ((wpath[2] == L'\\') || (wpath[2] == L'/')))
      )) { /* If this is a relative pathname */
    wpath0 = malloc(sizeof(WCHAR) * WIDE_PATH_MAX);
    if (!wpath0) RETURN_INT(-1); /* errno = ENOMEM */
    if (!GetFullPathNameW(wpath, WIDE_PATH_MAX, wpath0, NULL)) {
      free(wtarget);
      free(wpath0);
      errno = EINVAL;
      RETURN_INT(-1);
    }
    wpath = wpath0;
    DEBUG_WPRINTF((L"path = \"%s\";\n", wpath));
  }
  /* Scan every part of the pathname for links */
  while (wpath[iPath]) {
    /* int iPath0 = iPath; */
    /* int iBuf0 = iBuf; */
    int iBuf1;
    /* Append the first name section to the output buffer */
    if (bFirst) {
      bFirst = 0;
      /* Special case of absolute pathnames */
      if (wpath[0] == L'\\') {
	if ((iBuf+1U) >= bufsize) {
resolves_too_long:
	  free(wtarget);
	  free(wpath0);
	  errno = ENAMETOOLONG;
	  RETURN_INT_COMMENT(-1, ("Name too long\n"));
	}
	wc = wbuf[iBuf++] = wpath[iPath++]; /* Copy the root \ . */
	/* DEBUG_PRINTF(("wbuf[%d] = %C\n", iBuf-1, wc)); */
      } else if (wpath[0] && (wpath[1] == L':')) {
	if ((iBuf+1U) >= bufsize) goto resolves_too_long;
	wc = wbuf[iBuf++] = wpath[iPath++]; /* Copy the drive letter */
	/* DEBUG_PRINTF(("wbuf[%d] = %C\n", iBuf-1, wc)); */
	if ((iBuf+1U) >= bufsize) goto resolves_too_long;
	wc = wbuf[iBuf++] = wpath[iPath++]; /*      and the : */
	/* DEBUG_PRINTF(("wbuf[%d] = %C\n", iBuf-1, wc)); */
	if (wpath[iPath] == L'\\') {
	  if ((iBuf+1U) >= bufsize) goto resolves_too_long;
	  wc = wbuf[iBuf++] = wpath[iPath++];
	  /* DEBUG_PRINTF(("wbuf[%d] = %C\n", iBuf-1, wc)); */
	}
      } /* Else it's a relative pathname, handled in the common code below */
    } else { /* It's a continuation section in the path */
      if ((iBuf+1U) >= bufsize) goto resolves_too_long;
      wbuf[iBuf++] = L'\\';
    }
    iBuf1 = iBuf; /* Index of the beginning of the node name we're about to add */
    for (i=0; (size_t)(iBuf+i) < (bufsize-1) ;i++) {
      wc = wpath[iPath+i];
      if (!wc) break;
      /* if (wc == L'/') break; */
      if (wc == L'\\') break;
      /* DEBUG_PRINTF(("wbuf[%d] = %C\n", iBuf-1, wc)); */
      wbuf[iBuf+i] = wc;
    }
    iBuf += i;
    wbuf[iBuf] = L'\0';
    while (wc && (/* (wc == L'/') || */ (wc == L'\\'))) wc=wpath[iPath + ++i]; /* Skip extra /, if any */
    iPath += i;
    /* DEBUG_PRINTF(("// Removed %d characters from path\n", iPath - iPath0)); */
    /* Get the file type */
    dwAttr = GetFileAttributesW(wbuf);
    DEBUG_WPRINTF((L"// \"%s\" is %S\n", wbuf, (dwAttr == INVALID_FILE_ATTRIBUTES)?"not found":(
					       (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)?"a reparse point":(
      					       (dwAttr & FILE_ATTRIBUTE_DIRECTORY)?"a directory":"a file"))));
    if (dwAttr == INVALID_FILE_ATTRIBUTES) {
      errno = ENOENT;
      if (wpath[iPath]) { /* Append the remainder of the input path, even though we know it's invalid */
      	if ((iBuf+1U+lstrlenW(wpath+iPath)) < bufsize) {
	  wbuf[iBuf++] = L'\\';
	  lstrcpyW(wbuf+iBuf, wpath+iPath);
	}
      }
      free(wtarget);
      free(wpath0);
      DEBUG_WLEAVE((L"return -1; // No such file: \"%s\"\n", wbuf));
      return -1;
    }
    if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
      nRead = readlinkW(wbuf, wtarget, WIDE_PATH_MAX);
      if (nRead == -1) {
	if (errno == EINVAL) { /* This is a reparse point, but not a symlink or a junction */
	  goto file_or_directory;
	}
	/* Anything else is a real error, and resolution cannot be done. */
	if (wpath[iPath]) { /* Append the remainder of the input path, even though we know it's invalid */
	  if ((iBuf+1U+lstrlenW(wpath+iPath)) < bufsize) {
	    wbuf[iBuf++] = L'\\';
	    lstrcpyW(wbuf+iBuf, wpath+iPath);
	  }
	}
	free(wtarget);
	free(wpath0);
	DEBUG_WLEAVE((L"return -1; // Dangling link: \"%s\"\n", wbuf));
	return -1;
      }
      if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) && !lstrcmpW(wbuf, wtarget)) {
      	/* This is probably a junction pointing to an otherwise inaccessible area.
      	   (See readlink() header in readlink.c for details about this case.)
      	   Handle this junction like if it were a real directory, instead of a symlink. */
	goto file_or_directory;
      }
      if (   ((wtarget[0] == L'\\') /* || (target[0] == L'/') */ )
	  || (wtarget[1] == L':')) { /* This is an absolute pathname */
	DEBUG_WPRINTF((L"// Absolute link to \"%s\"\n", wtarget));
	iBuf = lstrlenW(wtarget); /* Anticipate the new position after copying */
	if ((size_t)iBuf >= bufsize) goto resolves_too_long;
	lstrcpyW(wbuf, wtarget);
      } else { /* This is a relative pathname */
	DEBUG_WPRINTF((L"// Relative link to \"%s\"\n", wtarget));
	/* So it'll replace the tail name in the output path */
	iBuf = iBuf1; /* The index right after the last path separator */
	DEBUG_WPRINTF((L"// Base dir = \"%.*s\"\n", iBuf, wbuf));
	if (((size_t)iBuf+lstrlenW(wtarget)) >= bufsize) goto resolves_too_long;
	lstrcpyW(wbuf+iBuf, wtarget);
	iBuf = CompactPathW(wbuf, wbuf, bufsize);
      }
      /* Append the remainder of the input path, if any */
      DEBUG_WPRINTF((L"buf = \"%s\"; path tail = \"%s\";\n", wbuf, wpath+iPath));
      if (wpath[iPath]) {
	if (iBuf && (wbuf[iBuf-1] != L'\\')) {
	  if ((iBuf+1U) >= bufsize) goto resolves_too_long;
	  wbuf[iBuf++] = L'\\';
	}
	if (((size_t)iBuf+lstrlenW(wpath+iPath)) >= bufsize) goto resolves_too_long;
	lstrcpyW(wbuf+iBuf, wpath+iPath);
	iBuf = CompactPathW(wbuf, wbuf, bufsize);
      }
      /* Check for max depth */
      if (iDepth == SYMLOOP_MAX) {
	errno = ELOOP;
	free(wtarget);
	free(wpath0);
	DEBUG_WLEAVE((L"return -1; // Max symlink depth reached: \"%s\"\n", wbuf));
	return -1;
      }
      /* Check for loops */
      for (pList = prev ; pList; pList = pList->prev) {
      	if (!_wcsicmp(wbuf, pList->wpath)) {
      	  errno = ELOOP;
	  free(wtarget);
	  free(wpath0);
	  DEBUG_WLEAVE((L"return -1; // Loop found: \"%s\"\n", wbuf));
	  return -1;
	}
      }
      /* OK, no loop, so repeat the process for that new path */
      lstrcpyW(wtarget, wbuf); /* Keep that as a reference in the linked list, in case there are further links */
      wtarget = ShrinkBuf(wtarget, sizeof(WCHAR) * (lstrlenW(wtarget)+1));
      list.prev = prev;
      list.wpath = wtarget;
      iErr = MlxResolveLinksW1(wtarget, wbuf, bufsize, &list, iDepth+1);
      free(wtarget);
      free(wpath0);
      DEBUG_WLEAVE((L"return %d; // %S: \"%s\"\n", iErr, (iErr ? strerror(errno) : "Success"), wbuf));
      return iErr;
    } else { /* It's a normal file or directory */
file_or_directory:
      if ((wpath[iPath]) && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
      	errno = ENOTDIR;
	if ((iBuf+1U+lstrlenW(wpath+iPath)) < bufsize) {
	  wbuf[iBuf++] = L'\\';
	  lstrcpyW(wbuf+iBuf, wpath+iPath);
	}
	free(wtarget);
	free(wpath0);
	DEBUG_WLEAVE((L"return -1; // File where dir expected: \"%s\"\n", wbuf));
	return -1;
      }
    }
  }

  free(wtarget);
  free(wpath0);
  DEBUG_WLEAVE((L"return 0; // Success: \"%s\"\n", wbuf));
  return 0;
}

int MlxResolveLinksW(const WCHAR *wpath, WCHAR *wbuf, size_t bufsize) {
  WCHAR *wpath1;
  int nSize;
  WNAMELIST root;
  int iErr;

  DEBUG_WENTER((L"MlxResolveLinks(\"%s\", 0x%p, %ld);\n", wpath, wbuf, bufsize));

  wbuf[0] = '\0'; /* Always output a valid string */

  if (!*wpath) { /* Spec says an empty pathname is invalid */
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("Empty pathname\n"));
  }

  wpath1 = malloc(sizeof(WCHAR) * WIDE_PATH_MAX);
  if (!wpath1) RETURN_INT_COMMENT(-1, ("Not enough memory\n"));

  /* Normalize the input path, using a single \ as path separator */
  nSize = CompactPathW(wpath, wpath1, WIDE_PATH_MAX);
  if (nSize == -1) { /* CompactPath() already sets errno = ENAMETOOLONG */
    free(wpath1);
    RETURN_INT_COMMENT(-1, ("Path too long\n"));
  }
  if (wpath1[nSize-1] == L'\\') { /* Spec says resolution must implicitly add a trailing dot, */
    if (nSize == WIDE_PATH_MAX) {    /*  to ensure that the last component is a directory. */
      free(wpath1);
      errno = ENAMETOOLONG;
      RETURN_INT_COMMENT(-1, ("Path too long after adding .\n"));
    }
    wpath1[nSize++] = L'.'; /* So add it explicitly */
    wpath1[nSize] = L'\0';
  }
  wpath1 = ShrinkBuf(wpath1, sizeof(WCHAR) * (nSize+1)); /* Resize the buffer to fit the name length */
  root.wpath = wpath1;
  root.prev = NULL;
  iErr = MlxResolveLinksW1(wpath1, wbuf, bufsize, &root, 0);
  nSize = lstrlenW(wbuf);
  /* Remove the final dot added above, if needed. */
  if ((nSize >= 2) && (wbuf[nSize-2] == L'\\') && (wbuf[nSize-1] == L'.')) wbuf[--nSize] = '\0';
  free(wpath1);
  DEBUG_WLEAVE((L"return 0; // %S: \"%s\"\n", (iErr ? strerror(errno) : "Success"), wbuf));
  return 0;
}

/* Multibyte version of the same */
int MlxResolveLinksM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR *pwszPath;
  WCHAR wszBuf[WIDE_PATH_MAX];
  int iErr;

  /* Convert the pathname to a unicode string */
  pwszPath = MultiByteToNewWideString(cp, path);
  if (!pwszPath) return -1;

  /* Resolve all links */
  iErr = MlxResolveLinksW(pwszPath, wszBuf, WIDE_PATH_MAX);

  /* Convert the result back to multibyte */
  if (!iErr) {
    if (!WideCharToMultiByte(cp, 0, wszBuf, -1, buf, (int)bufsize, NULL, NULL)) {
      errno = Win32ErrorToErrno();
      iErr = -1;
    }
  }

  free(pwszPath);
  return iErr;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxResolveSubstDrives				      |
|									      |
|   Description	    Resolve all sustituted drives names within a pathname     |
|									      |
|   Parameters	    const char *path	    Pathname to resolve 	      |
|		    char *buf		    Output buffer    		      |
|		    size_t bufsize	    Size of the output buffer         |
|		    							      |
|   Notes	    Asserts the path is an absolute pathname.		      |
|		    							      |
|   Returns	    0 = success, or -1 = error, with errno set		      |
|		    							      |
|   History								      |
|    2026-01-22 JFL Created this routine                                      |
*									      *
\*---------------------------------------------------------------------------*/

int MlxResolveSubstDrivesW(const WCHAR *pwszPath, WCHAR *pwOutBuf, size_t lBuf) {
  WCHAR wcDrive;
  int iRoot, iOutRoot;
  int iLen;
  int i;

  DEBUG_WENTER((L"MlxResolveSubstDrives(\"%s\", 0x%p, %lu);\n", pwszPath, pwOutBuf, (ULONG)lBuf));

  if ((!pwszPath) || (!pwszPath[0]) || (!pwOutBuf)) {
    errno = EINVAL;
    RETURN_INT_COMMENT(-1, ("NULL or empty input or output\n"));
  }
  if ((pwszPath[0] == L'\\') && (pwszPath[1] == L'\\')) {
    errno = EINVAL;
    RETURN_INT_COMMENT(-1, ("Network paths not supported\n"));
  }
  /* Identify the drive name, which may not be specified */
  if (pwszPath[1] == L':') {
    wcDrive = pwszPath[0];
    if (wcDrive >= L'a') wcDrive -= ('a' - 'A'); /* Convert it to upper case */
    iRoot = 2;
  } else {
    wcDrive = L'@' + (WCHAR)_getdrive();
    iRoot = 0;
  }
  if (pwszPath[iRoot] != L'\\') {
    errno = EINVAL;
    RETURN_INT_COMMENT(-1, ("The path must be absolute\n"));
  }
  /* Identify the drive type */
  pwOutBuf[0] = wcDrive;
  pwOutBuf[1] = L':';
  pwOutBuf[2] = L'\0';
  QueryDosDeviceW(pwOutBuf, pwOutBuf, (DWORD)lBuf);
  DEBUG_WPRINTF((L"%c: = %s\n", wcDrive, pwOutBuf));
  if ((!wcsncmp(pwOutBuf, L"\\??\\", 4)) && pwOutBuf[4] && (pwOutBuf[5] == L':')) {
    /* Yes, this is a substituted drive. Check if there are further substitutions */
    WCHAR *pwszSubstition = _wcsdup(pwOutBuf+4);
    int iErr = MlxResolveSubstDrivesW(pwszSubstition, pwOutBuf, lBuf);
    if (iErr) RETURN_INT(iErr);
    free(pwszSubstition);
    iOutRoot = lstrlenW(pwOutBuf);	/* Append the path after the resolved drive path */
  } else { /* Not a substituted drive */
    pwOutBuf[0] = wcDrive;
    pwOutBuf[1] = L':';
    iOutRoot = 2;			/* Just copy the path after the drive */
  }
  if (pwOutBuf[iOutRoot-1] == L'\\') iOutRoot -= 1;
  /* Append the path after the resolved drive path */
  iLen = lstrlenW(pwszPath + iRoot);
  if ((size_t)(iOutRoot+iLen) >= lBuf) {
    errno = ENAMETOOLONG;
    RETURN_INT_COMMENT(-1, ("Output buffer too small\n"));
  }
  for (i=0; i<iLen; i++) pwOutBuf[iOutRoot+i] = pwszPath[iRoot+i];
  /* Remove a trailing \ except for the root */
  i = iOutRoot + iLen;	/* Offset of the end of string */
  if ((i > 3) && (pwOutBuf[i-1] == L'\\')) i -= 1;
  pwOutBuf[i] = L'\0';
  DEBUG_WLEAVE((L"return 0; \"%s\"\n", pwOutBuf));
  return 0;
}

/* Multibyte version of the same */
int MlxResolveSubstDrivesM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR *pwszPath;
  WCHAR wszBuf[WIDE_PATH_MAX];
  int iErr;

  /* Convert the pathname to a unicode string */
  pwszPath = MultiByteToNewWideString(cp, path);
  if (!pwszPath) return -1;

  /* Resolve the substituted drives */
  iErr = MlxResolveSubstDrivesW(pwszPath, wszBuf, WIDE_PATH_MAX);

  /* Convert the result back to multibyte */
  if (!iErr) {
    if (!WideCharToMultiByte(cp, 0, wszBuf, -1, buf, (int)bufsize, NULL, NULL)) {
      errno = Win32ErrorToErrno();
      iErr = -1;
    }
  }

  free(pwszPath);
  return iErr;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxGetFileName					      |
|									      |
|   Description	    Get the actual file name directly from Windows	      |
|									      |
|   Parameters	    const char *path	    Pathname to resolve 	      |
|		    char *buf		    Output buffer    		      |
|		    size_t bufsize	    Size of the output buffer         |
|		    							      |
|   Notes	    Asserts the path is an absolute pathname.		      |
|		    							      |
|   Returns	    0 = success, or -1 = error, with errno set		      |
|		    							      |
|   History								      |
|    2026-01-22 JFL Created this routine                                      |
|    2026-02-13 JFL Also resolve the tail name, not just the path.            |
*									      *
\*---------------------------------------------------------------------------*/

#pragma warning(disable:4996)       /* Ignore the deprecated name warning for GetVersion() */

int MlxGetFileNameW(const WCHAR *pwPath, WCHAR *pwOutBuf, size_t lBuf) {
  int lFni = sizeof(FILE_NAME_INFO) + (sizeof(WCHAR) * (WIDE_PATH_MAX - 1));
  FILE_NAME_INFO *pFni;
  WCHAR wcDrive;
  WCHAR wszDrive[4];
  int i, iLen, iRoot;
  HANDLE hFile;
  BOOL bDone;
  UINT uDrvType;

  DEBUG_WENTER((L"MlxGetFileName(\"%s\", 0x%p, %lu);\n", pwPath, pwOutBuf, (ULONG)lBuf));

  if (!HasGetFileInformationByHandleEx() || (LOBYTE(LOWORD(GetVersion())) < 6)) {
    errno = ENOSYS; /* Supported in Windows Vista and later */
    RETURN_CONST_COMMENT(-1, ("Unsupported on this system\n"));
  }
  /* Allocate a buffer for the output structure and real pathname */
  pFni = (FILE_NAME_INFO *)malloc(lFni);
  if (!pFni) RETURN_CONST_COMMENT(-1, ("Out of memory\n"));
  hFile = CreateFileW(pwPath,			/* lpFileName */
		      0,			/* dwDesiredAccess - 0 = Neither read or write = Only metadata access */
		      0,			/* dwShareMode - Others can do anything */
		      NULL,			/* lpSecurityAttributes */
		      OPEN_EXISTING,		/* dwCreationDisposition */
		      FILE_FLAG_BACKUP_SEMANTICS, /* dwFlagsAndAttributes - Allow opening directories too */
		      NULL			/* hTemplateFile */
		      );
  if (hFile == INVALID_HANDLE_VALUE) {
    errno = Win32ErrorToErrno();
    free(pFni);
    RETURN_CONST_COMMENT(-1, ("%s\n", strerror(errno)));
  }
  /* GetFileInformationByHandleEx() is supported in Windows Vista and later */
  DEBUG_PRINTF(("GetFileInformationByHandleEx(hFile, FileNameInfo, pFni, buflen=%d);\n", lFni));
  bDone = pGetFileInformationByHandleEx(hFile, FileNameInfo, pFni, (DWORD)lFni);
  CloseHandle(hFile);
  if (!bDone) {
    errno = Win32ErrorToErrno();
    free(pFni);
    RETURN_CONST_COMMENT(-1, ("%s\n", strerror(errno)));
  }
  iLen = (int)(pFni->FileNameLength / 2);
  DEBUG_WPRINTF((L"  return \"%.*s\"\n", iLen, pFni->FileName));
  /* Identify the drive name, as it's not returned by the above call */
  if (pwPath[0] && (pwPath[1] == L':')) {
    wcDrive = pwPath[0];
    if (wcDrive >= L'a') wcDrive -= ('a' - 'A'); /* Convert it to upper case */
  } else {
    wcDrive = L'@' + (WCHAR)_getdrive();
  }
  /* Identify the drive type */
  wszDrive[0] = wcDrive;
  wszDrive[1] = L':';
  wszDrive[2] = L'\\';
  wszDrive[3] = L'\0';
  uDrvType = GetDriveTypeW(wszDrive);
  DEBUG_PRINTF(("DriveType=%u;\n", uDrvType));
  /* Rebuild the full pathname ahead of the beginning of the output buffer */
  if (uDrvType == DRIVE_REMOTE) { 	/* It's a network drive */
    pwOutBuf[0] = L'\\';		/* Make it start with a double \ */
    iRoot = 1;
  } else {				/* It's a local drive */
    /* Resolve (possibly multiply) substituted drives */
    int iErr = MlxResolveSubstDrivesW(wszDrive, pwOutBuf, lBuf);
    if (iErr) RETURN_CONST_COMMENT(-1, ("%s\n", strerror(errno)));
    iRoot = 2; /* Keep only the drive name, as FileNameInfo already contains the initial path */
  }
  /* Append the absolute path returned by Windows */
  if ((iRoot + iLen) >= (int)lFni) {
    errno = ENAMETOOLONG;
    free(pFni);
    RETURN_INT_COMMENT(-1, ("Output buffer too small\n"));
  }
  for (i=0; i<iLen; i++) pwOutBuf[iRoot+i] = pFni->FileName[i];
  pwOutBuf[iRoot+iLen] = L'\0';
  free(pFni);
  DEBUG_WLEAVE((L"return 0; // \"%s\"\n", pwOutBuf));
  return 0;
}

/* Multibyte version of the same */
int MlxGetFileNameM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR *pwszPath;
  WCHAR wszBuf[WIDE_PATH_MAX];
  int iErr;

  /* Convert the pathname to a unicode string */
  pwszPath = MultiByteToNewWideString(cp, path);
  if (!pwszPath) return -1;

  /* Resolve the substituted drives */
  iErr = MlxGetFileNameW(pwszPath, wszBuf, WIDE_PATH_MAX);

  /* Convert the result back to multibyte */
  if (!iErr) {
    if (!WideCharToMultiByte(cp, 0, wszBuf, -1, buf, (int)bufsize, NULL, NULL)) {
      errno = Win32ErrorToErrno();
      iErr = -1;
    }
  }

  free(pwszPath);
  return iErr;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    realpath						      |
|									      |
|   Description	    Standard C library realpath()			      |
|									      |
|   Parameters	    const char *path	    Pathname to resolve 	      |
|		    char *buf		    Output buffer, length >= PATH_MAX |
|									      |
|   Notes	    Windows Vista and later have a built-in resolution	      |
|		    mechanism. See routine MlxGetFileName() above.	      |
|		    For Windows XP, we resolve links ourselves.		      |
|		    For Windows 95/98, there are no links, so nothing to do.  |
|		    							      |
|   Returns	    0 = success, or -1 = error, with errno set		      |
|		    							      |
|   History								      |
|    2026-01-22 JFL Created this routine                                      |
|    2026-02-13 JFL Fixed memory leaks in case of error.                      |
*									      *
\*---------------------------------------------------------------------------*/

WCHAR *realpathW(const WCHAR *pwPath, WCHAR *pwOutBuf) {
  int iErr;
  WCHAR *pwPath2 = NULL;
  WCHAR *pwOutBuf0 = pwOutBuf;
  WCHAR *pwResult = NULL;
  int n;
  DWORD dwTag;

  DEBUG_WENTER((L"realpathW(\"%s\", 0x%p);\n", pwPath, pwOutBuf));

  /* Extension: Optionally, dynamically allocate the output buffer */
  if (!pwOutBuf) pwOutBuf = malloc(sizeof(WCHAR) * WIDE_PATH_MAX);
  if (!pwOutBuf) RETURN_CONST_COMMENT(NULL, ("Out of memory\n"));

  /* Gotcha: If the algorithm is changed below, make sure to update truename.c accordingly */
  /* TO DO: To avoid having to synchronize the two sources, add a realpathWX() routine,
  	    with an additional flags argument, allowing to select only some of the
  	    operations below. And use that in truename.c */

  /* First try letting Windows do the resolution itself */
  iErr = MlxGetFileNameW(pwPath, pwOutBuf, WIDE_PATH_MAX);
  if (iErr && (errno == ENOSYS)) {
    /* The OS does not support name resolution. Do it ourselves */
    pwPath2 = malloc(sizeof(WCHAR) * WIDE_PATH_MAX);
    if (!pwPath2) {
      DEBUG_LEAVE(("return NULL; // Out of memory\n"));
      goto cleanup_and_return;
    }
    iErr = MlxResolveSubstDrivesW(pwPath, pwPath2, WIDE_PATH_MAX);
    if (!iErr) iErr = MlxResolveLinksW(pwPath2, pwOutBuf, WIDE_PATH_MAX);
    free(pwPath2);
  }
  if (iErr && (errno == EAGAIN)) {
    /* Returned for all reparse points that aren't resolved by MlxGetFileNameW */
    dwTag = MlxGetReparseTagW(pwOutBuf) & IO_REPARSE_TAG_TYPE_BITS;
    /* Special case for AppExecLinks, which we can resolve another way */
    if (dwTag == IO_REPARSE_TAG_APPEXECLINK) {
      pwPath2 = _wcsdup(pwOutBuf);
      if (!pwPath2) {
	DEBUG_LEAVE(("return NULL; // Out of memory\n"));
	goto cleanup_and_return;
      }
      n = MlxReadAppExecLinkW(pwPath2, pwOutBuf, WIDE_PATH_MAX);
      if (!n) {
	DEBUG_WLEAVE((L"return NULL; // Failed to resolve AppExecLink \"%s\". %S\n", pwPath2, strerror(errno)));
	goto cleanup_and_return;
      }
      dwTag = MlxGetReparseTagW(pwOutBuf); /* Check if the target exists */
      if (!dwTag && (errno == ENOENT)) { /* It does not */
	DEBUG_WLEAVE((L"return NULL; // Dangling AppExecLink \"%s\". %S\n", pwPath2, strerror(errno)));
	errno = EXDEV; /* "Improper link" */
	goto cleanup_and_return;
      }
      free(pwPath2);
      /* Repeat the resolution for that target file */
      pwPath2 = _wcsdup(pwOutBuf);
      if (!pwPath2)  {
	DEBUG_LEAVE(("return NULL; // Out of memory\n"));
	goto cleanup_and_return;
      }
      pwResult = realpathW(pwPath2, pwOutBuf);
      /* free(pwPath2); // Done again in the common exit */
      goto cleanup_and_return;
    } else if (dwTag) { /* Any other type of reparse point */
      errno = EXDEV; /* "Improper link" */
    }
  }
  if (iErr) {
    DEBUG_LEAVE(("return NULL; // Resolution failed. %s\n", strerror(errno)));
    goto cleanup_and_return;
  }

  /* Change short names to long names, and correct the name case */
  n = GetLongPathNameW(pwPath2, pwOutBuf, WIDE_PATH_MAX);
  free(pwPath2);
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_LEAVE(("return NULL; // Can't get long pathnames. %s\n", strerror(errno)));
    goto cleanup_and_return;
  }

  pwResult = pwOutBuf;
  DEBUG_WLEAVE((L"return \"%s\"\n", pwResult));

cleanup_and_return:
  free(pwPath2);

  /* If we dynamically allocated the result, resize it to avoid wasting space */
  if (!pwOutBuf0) {
    if (pwResult) {
      pwResult = ShrinkBuf(pwOutBuf, sizeof(WCHAR) * (lstrlenW(pwOutBuf) + 1));
    } else {
      free(pwOutBuf);
    }
  }

  return pwResult;
}

/* Multibyte version of the same */
char *realpathM(const char *path, char *buf, UINT cp) {
  char *buf0 = buf;
  WCHAR *pwszPath = NULL;
  WCHAR wszBuf[WIDE_PATH_MAX];
  WCHAR *pwszBuf;
  char *pszBuf = NULL;

  /* Extension: Optionally, dynamically allocate the output buffer */
  if (!buf) buf = malloc(UTF8_PATH_MAX); /* Assume no other encoding requires a larger buffer */
  if (!buf) goto cleanup_and_return;

  /* Convert the pathname to a unicode string */
  pwszPath = MultiByteToNewWideString(cp, path);
  if (!pwszPath) goto cleanup_and_return;

  /* Resolve the real file name */
  pwszBuf = realpathW(pwszPath, wszBuf);
  if (!pwszBuf) goto cleanup_and_return;

  /* Convert the result back to multibyte */
  if (!WideCharToMultiByte(cp, 0, wszBuf, -1, buf, UTF8_PATH_MAX, NULL, NULL)) {
    errno = Win32ErrorToErrno();
    goto cleanup_and_return;
  }

  /* If we dynamically allocated the result, resize it to avoid wasting space */
  if (!buf0) buf = ShrinkBuf(buf, lstrlen(buf) + 1);
  pszBuf = buf;

cleanup_and_return:
  if ((!pszBuf) && !buf0) free(buf);
  free(pwszPath);
  return pszBuf;
}

#endif /* defined(_WIN32) */

