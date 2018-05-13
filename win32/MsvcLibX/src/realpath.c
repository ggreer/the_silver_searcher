/*****************************************************************************\
*                                                                             *
*   Filename	    realpath.c						      *
*									      *
*   Description     Resolve links and remove . and .. parts in pathnames      *
*                                                                             *
*   Notes	    TO DO: Make Wide & MultiByte versions for Windows	      *
*                                                                             *
*                   TO DO: Microsoft provides a convenient routine, for which *
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
#include "debugm.h"

#define TRUE 1
#define FALSE 0

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    CompactPath						      |
|									      |
|   Description	    Remove all ., .., and extra / or \ separators in a path   |
|									      |
|   Parameters	    const char *path	    Pathname to cleanup 	      |
|		    char *buf		    Output buffer    		      |
|		    size_t bufsize	    Size of the output buffer         |
|									      |
|   Notes	    Allows having path = outbuf, ie. in-place compacting.     |
|		    Supports both relative and absolute paths.		      |
|									      |
|   Returns	    The length of the output string			      |
|									      |
|   History								      |
|    2014-02-19 JFL Created this routine                                      |
*									      *
\*---------------------------------------------------------------------------*/

int CompactPath(const char *path, char *outbuf, size_t bufsize) {
  const char *pcIn;
  char *pcOut;
  int i, j, inSize, outSize;
  char c = '\0';
  char lastc = '\0';
#ifdef _WIN32 /* Don't use PATH_MAX for WIN32, as it's overkill for UTF8. Use the OS limit for UTF16 instead */
#define MAX_SUBDIRS (WIDE_PATH_MAX / 2) /* Worst case is: \1\1\1\1\1\1... */
#else
#define MAX_SUBDIRS (PATH_MAX / 2) /* Worst case is: \1\1\1\1\1\1... */
#endif
  const char *pParts[MAX_SUBDIRS];
  int lParts[MAX_SUBDIRS];
  int lPart = 0;
  int nParts = 0;
  int isAbsolute = FALSE;
  int nDotDotParts = 0;

  DEBUG_ENTER(("CompactPath(\"%s\", %p, %lu);\n", path, outbuf, (unsigned long)bufsize));

  pcIn = path;
  inSize = (int)strlen(path) + 1;
  pcOut = outbuf;
  outSize = (int)bufsize;

  if ((pcIn[0] == '\\') && (pcIn[1] == '\\')) { /* This is a \\server\share UNC path */
    *(pcOut++) = *(pcIn++); /* Copy the initial first \, so that the remainder looks like an absolute path */
    inSize -= 1;
    outSize -= 1;
  }

  if (*pcIn && (pcIn[1] == ':')) { /* There's a drive letter */
    *(pcOut++) = *(pcIn++);		/* Copy it */
    *(pcOut++) = *(pcIn++);
    inSize -= 2;
    outSize -= 2;
  }

  /* Scan the input pathname, recording pointers to every part of the pathname */
  for (i=0; i<inSize; i++) {
    c = pcIn[i];
    if (c == '/') c = '\\';
    if ((c == '\\') && (lastc == '\\')) continue; /* Condense multiple \ into one */
    if ((c == '\\') && (lastc == '\0')) { /* This is an absolute pathname */
      isAbsolute = TRUE;
      lastc = c;
      continue;
    }
    if ((c != '\\') && ((lastc == '\\') || (lastc == '\0'))) { /* Beginning of a new node */
      if (nParts == MAX_SUBDIRS) {
	errno = ENAMETOOLONG;
	RETURN_INT_COMMENT(-1, ("Name too long\n"));
      }
      pParts[nParts] = pcIn + i;
      lPart = 0;
    }
    if (c && (c != '\\')) {
      lPart += 1;
    } else { /* End of a node */
      lParts[nParts++] = lPart;
      XDEBUG_PRINTF(("pParts[%d] = \"%.*s\"\n", nParts-1, lPart, pParts[nParts-1]));
    }
    lastc = c;
    if (c == '\0') break;
  }

  /* Eliminate . and .. parts */
  for (i=0; i<nParts; i++) {
    XDEBUG_PRINTF(("for pParts[%d] = \"%.*s\"\n", i, lParts[i], pParts[i]));
    if ((pParts[i][0] == '.') && (lParts[i] == 1)) { /* It's a . part */
its_a_dot_part:
      XDEBUG_PRINTF(("It's a '.'. Removing part #%d\n", i));
      nParts -= 1;
      for (j=i; j<nParts; j++) {
      	pParts[j] = pParts[j+1];
      	lParts[j] = lParts[j+1];
      }
      i -= 1;
      continue;
    }
    if ((pParts[i][0] == '.') && (pParts[i][1] == '.') && (lParts[i] == 2)) { /* It's a .. part */
      if (i == nDotDotParts) {
	XDEBUG_PRINTF(("It's a '..', but it's at the root.\n"));
      	if (isAbsolute) goto its_a_dot_part; /* .. in root is like . */
      	nDotDotParts += 1; /* Else for relative paths, keep this .. */
      	continue;
      }
      XDEBUG_PRINTF(("It's a '..'. Removing parts #%d and #%d\n", i-1, i));
      nParts -= 2;
      for (j=i-1; j<nParts; j++) {
      	pParts[j] = pParts[j+2];
      	lParts[j] = lParts[j+2];
      }
      i -= 2;
      continue;
    }
  }

  /* Join all remaining parts into a single path */
  if (!outSize) {
buffer_is_too_small:
    errno = ENAMETOOLONG;
    RETURN_INT_COMMENT(-1, ("Name too long\n"));
  }
  outSize -= 1;	/* Leave room for the final NUL */
  if (isAbsolute) {
    if (!outSize) goto buffer_is_too_small;
    *(pcOut++) = '\\';
    outSize -= 1;
  }
  if (nParts) {
    /* Copy part 0 if it's not already at the right place */
    if (lParts[0] > outSize) goto buffer_is_too_small;
    if (pcOut != pParts[0]) strncpy(pcOut, pParts[0], lParts[0]);
    pcOut += lParts[0];
    outSize -= lParts[0];
  } else {
    if (pcOut == outbuf) {
      if (!outSize) goto buffer_is_too_small;
      *(pcOut++) = '.'; /* Special case for "subdir\..", which is "." and not "" */
      outSize -= 1;
    }
  }
  for (i=1; i<nParts; i++) {
    if (!outSize) goto buffer_is_too_small;
    *(pcOut++) = '\\';
    outSize -= 1;
    if (lParts[i] > outSize) goto buffer_is_too_small;
    if (pcOut != pParts[i]) strncpy(pcOut, pParts[i], lParts[i]);
    pcOut += lParts[i];
    outSize -= lParts[i];
  }
  *pcOut = '\0';

  RETURN_INT_COMMENT((int)(pcOut - outbuf), ("\"%s\"\n", outbuf));
}


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

  if (!outbuf) pOutbuf = realloc(pOutbuf, strlen(pOutbuf) + 1);
  return pOutbuf;
}

#endif


#ifdef _WIN32

#include <windows.h>	/* Also includes MsvcLibX' WIN32 UTF-8 extensions */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    CompactPathW					      |
|									      |
|   Description	    Remove all ., .., and extra / or \ separators in a path   |
|									      |
|   Parameters	    const WCHAR *path	  Pathname to cleanup	 	      |
|		    WCHAR *buf		  Output buffer    		      |
|		    size_t bufsize	  Size of the output buffer in WCHARs |
|									      |
|   Notes	    Allows having path = outbuf, ie. in-place compacting.     |
|		    Supports both relative and absolute paths.		      |
|									      |
|   Returns	    The length of the output string			      |
|									      |
|   History								      |
|    2014-02-19 JFL Created the 8-bit version of this routine.                |
|    2017-10-04 JFL Adapted to 16-bit characters.                             |
*									      *
\*---------------------------------------------------------------------------*/

int CompactPathW(const WCHAR *path, WCHAR *outbuf, size_t bufsize) {
  const WCHAR *pcIn;
  WCHAR *pcOut;
  int i, j, inSize, outSize;
  WCHAR c = L'\0';
  WCHAR lastc = L'\0';
#define MAX_SUBDIRS (WIDE_PATH_MAX / 2) /* Worst case is: \1\1\1\1\1\1... */
  const WCHAR *pParts[MAX_SUBDIRS];
  int lParts[MAX_SUBDIRS];
  int lPart = 0;
  int nParts = 0;
  int isAbsolute = FALSE;
  int nDotDotParts = 0;
  DEBUG_CODE(
    char *pszUtf8;
  )
  int iLen;

  DEBUG_WSTR2NEWUTF8(path, pszUtf8);
  DEBUG_ENTER(("CompactPathW(L\"%s\", 0x%p, %lu);\n", pszUtf8, outbuf, (unsigned long)bufsize));
  DEBUG_FREEUTF8(pszUtf8);

  pcIn = path;
  inSize = (int)lstrlenW(path) + 1;
  pcOut = outbuf;
  outSize = (int)bufsize;

  if ((pcIn[0] == L'\\') && (pcIn[1] == L'\\')) { /* This is a \\server\share UNC path */
    *(pcOut++) = *(pcIn++); /* Copy the initial first \, so that the remainder looks like an absolute path */
    inSize -= 1;
    outSize -= 1;
  }

  if (*pcIn && (pcIn[1] == L':')) { /* There's a drive letter */
    *(pcOut++) = *(pcIn++);		/* Copy it */
    *(pcOut++) = *(pcIn++);
    inSize -= 2;
    outSize -= 2;
  }

  /* Scan the input pathname, recording pointers to every part of the pathname */
  for (i=0; i<inSize; i++) {
    c = pcIn[i];
    if (c == L'/') c = L'\\';
    if ((c == L'\\') && (lastc == L'\\')) continue; /* Condense multiple \ into one */
    if ((c == L'\\') && (lastc == L'\0')) { /* This is an absolute pathname */
      isAbsolute = TRUE;
      lastc = c;
      continue;
    }
    if ((c != L'\\') && ((lastc == L'\\') || (lastc == L'\0'))) { /* Beginning of a new node */
      if (nParts == MAX_SUBDIRS) {
	errno = ENAMETOOLONG;
	RETURN_INT_COMMENT(-1, ("Name too long\n"));
      }
      pParts[nParts] = pcIn + i;
      lPart = 0;
    }
    if (c && (c != L'\\')) {
      lPart += 1;
    } else { /* End of a node */
      lParts[nParts++] = lPart;
      DEBUG_WSTR2NEWUTF8(pParts[nParts-1], pszUtf8);
      XDEBUG_PRINTF(("pParts[%d] = \"%.*s\"; l = %d\n", nParts-1, lPart, pszUtf8, lPart));
      DEBUG_FREEUTF8(pszUtf8);
    }
    lastc = c;
    if (c == L'\0') break;
  }

  /* Eliminate . and .. parts */
  for (i=0; i<nParts; i++) {
    DEBUG_WSTR2NEWUTF8(pParts[i], pszUtf8);
    XDEBUG_PRINTF(("for pParts[%d] = \"%.*s\"\n", i, lParts[i], pszUtf8));
    DEBUG_FREEUTF8(pszUtf8);
    if ((pParts[i][0] == L'.') && (lParts[i] == 1)) { /* It's a . part */
its_a_dot_part:
      XDEBUG_PRINTF(("It's a L'.'. Removing part #%d\n", i));
      nParts -= 1;
      for (j=i; j<nParts; j++) {
      	pParts[j] = pParts[j+1];
      	lParts[j] = lParts[j+1];
      }
      i -= 1;
      continue;
    }
    if ((pParts[i][0] == L'.') && (pParts[i][1] == L'.') && (lParts[i] == 2)) { /* It's a .. part */
      if (i == nDotDotParts) {
	XDEBUG_PRINTF(("It's a L'..', but it's at the root.\n"));
      	if (isAbsolute) goto its_a_dot_part; /* .. in root is like . */
      	nDotDotParts += 1; /* Else for relative paths, keep this .. */
      	continue;
      }
      XDEBUG_PRINTF(("It's a L'..'. Removing parts #%d and #%d\n", i-1, i));
      nParts -= 2;
      for (j=i-1; j<nParts; j++) {
      	pParts[j] = pParts[j+2];
      	lParts[j] = lParts[j+2];
      }
      i -= 2;
      continue;
    }
  }

  /* Join all remaining parts into a single path */
  if (!outSize) {
buffer_is_too_small:
    errno = ENAMETOOLONG;
    RETURN_INT_COMMENT(-1, ("Name too long\n"));
  }
  outSize -= 1;	/* Leave room for the final NUL */
  if (isAbsolute) {
    if (!outSize) goto buffer_is_too_small;
    *(pcOut++) = L'\\';
    outSize -= 1;
  }
  if (nParts) {
    /* Copy part 0 if it's not already at the right place */
    if (lParts[0] > outSize) goto buffer_is_too_small;
    if (pcOut != pParts[0]) lstrcpynW(pcOut, pParts[0], lParts[0]+1);
    pcOut += lParts[0];
    outSize -= lParts[0];
  } else {
    if (pcOut == outbuf) {
      if (!outSize) goto buffer_is_too_small;
      *(pcOut++) = L'.'; /* Special case for "subdir\..", which is "." and not "" */
      outSize -= 1;
    }
  }
  for (i=1; i<nParts; i++) {
    if (!outSize) goto buffer_is_too_small;
    *(pcOut++) = L'\\';
    outSize -= 1;
    if (lParts[i] > outSize) goto buffer_is_too_small;
    if (pcOut != pParts[i]) {
      lstrcpynW(pcOut, pParts[i], lParts[i]+1);
    }
    pcOut += lParts[i];
    outSize -= lParts[i];
  }
  *pcOut = L'\0';

  iLen = (int)(pcOut - outbuf);
  DEBUG_WSTR2NEWUTF8(outbuf, pszUtf8);
  DEBUG_LEAVE(("return %d; // L\"%s\"\n", iLen, pszUtf8));
  DEBUG_FREEUTF8(pszUtf8);
  return iLen;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    ConcatPathW						      |
|									      |
|   Description	    Concatenate two paths				      |
|									      |
|   Parameters	    const WCHAR *pwszHead   First Pathname	 	      |
|		    const WCHAR *pwszTail   Second Pathname	 	      |
|		    WCHAR *pwszBuf	    Output buffer. If NULL, alloc one.|
|		    size_t lBuf		    Output buffer size in WCHARS.     |
|									      |
|   Notes	    							      |
|		    							      |
|   Returns	    The output buffer address, or NULL & errno set if failed. |
|		    							      |
|   History								      |
|    2018-04-25 JFL Created this routine                                      |
*									      *
\*---------------------------------------------------------------------------*/

WCHAR *ConcatPathW(const WCHAR *pwszHead, const WCHAR *pwszTail, WCHAR *pwszBuf, size_t lBuf) {
  int iBuf = 0;
  int iAlloc = FALSE;	/* TRUE if pwszBuf was allocated here */
  int iReAlloc = FALSE;	/* TRUE if pwszBuf should be reallocated in the end */
  int l;

  DEBUG_WPRINTF((L"ConcatPathW(\"%s\", \"%s\", %p, %Iu);\n", pwszHead, pwszTail, pwszBuf, lBuf));

  if (!pwszBuf) {
    iAlloc = TRUE;
    if (!lBuf) {
      iReAlloc = TRUE;
      lBuf = WIDE_PATH_MAX;
    }
    pwszBuf = malloc(lBuf * sizeof(WCHAR));
    if (!pwszBuf) return NULL;
  }
  if (lBuf < 1) {
fail_no_space:
    errno = ENOSPC;
    if (iAlloc) free(pwszBuf);
    return NULL;
  }
  if (!pwszHead) pwszHead = L"";
  /* Skip the \\?\ prefix, if any */
  if (!strncmpW(pwszTail, L"\\\\?\\UNC\\", 8)) {
    if (lBuf < 3) goto fail_no_space;
    pwszTail += 8;	/* Remove the '\\?\UNC\' prefix */
    pwszBuf[iBuf++] = L'\\';
    pwszBuf[iBuf++] = L'\\';
    pwszHead = L"";	/* Throw away the irrelevant base path. */
    goto drive_copy_done;
  } else if (!strncmpW(pwszTail, L"\\\\?\\", 4)) {
    pwszTail += 4;
    pwszHead = L"";	/* Throw away the irrelevant base path. */
    /* Fall through, to copy the tail drive. */
  }
  /* First copy the drive */
  if (pwszTail[0] && (pwszTail[1] == L':')) { /* If the tail has a drive specified */
    if (lBuf < 3) goto fail_no_space;
    if (pwszHead[0] && (pwszHead[1] == L':')) { /* If the base also has one */
      WCHAR wcBaseDrive = pwszHead[0];	/* Then do a case-independant comparison */
      WCHAR wcTailDrive = pwszTail[0];
      if (wcBaseDrive >= L'a') wcBaseDrive -= (L'a' - L'A'); /* Convert to upper case */
      if (wcTailDrive >= L'a') wcTailDrive -= (L'a' - L'A'); /* Convert to upper case */
      if (wcTailDrive != wcBaseDrive) {
      	pwszHead = L"";	/* If they're different, throw away the irrelevant base path. */
      } else {
	pwszHead += 2;	/* Else skip the base drive */
      }
    }
    pwszBuf[iBuf++] = pwszTail[0];	/* Use the tail drive */
    pwszBuf[iBuf++] = L':';
    pwszTail += 2;
  } else if (pwszHead[0] && (pwszHead[1] == L':')) { /* If only the base has a drive */
    if (lBuf < 3) goto fail_no_space;
    pwszBuf[iBuf++] = pwszHead[0];	/* Then use the base drive */
    pwszBuf[iBuf++] = L':';
    pwszHead += 2;
  } /* Else neither the base nor the tail have a drive specified */
drive_copy_done:
  pwszBuf[iBuf] = L'\0';
  /* Next copy the path */
  if (pwszTail[0] != L'\\') {	/* The tail is a relative path */
    l = lstrlenW(pwszHead);
    if ((iBuf+l+2) >= (int)lBuf) goto fail_no_space;
    lstrcpyW(pwszBuf+iBuf, pwszHead);
    iBuf += l;
    pwszBuf[iBuf++] = L'\\';
  }
  l = lstrlenW(pwszTail);
  if ((iBuf+l+1) >= (int)lBuf) goto fail_no_space;
  lstrcpyW(pwszBuf+iBuf, pwszTail);
  iBuf += l;
  /* Then remove the . and .. parts */
  l = CompactPathW(pwszBuf, pwszBuf, lBuf);
  /* Finally, if it's a new buffer of unspecified size, minimize its size */
  if (iReAlloc) {
    WCHAR *pwszBuf2 = realloc(pwszBuf, (l + 1) * sizeof(WCHAR));
    if (pwszBuf2) pwszBuf = pwszBuf2;
  }
  return pwszBuf;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    ResolveLinks					      |
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
*									      *
\*---------------------------------------------------------------------------*/

/* Linked list of previous pathnames */
typedef struct _NAMELIST {
  struct _NAMELIST *prev;
  char *path;
} NAMELIST;

/* TODO: Create a real ResolveLinksM common routine, called by ResolveLinksA
         and ResolveLinksU, then remove this uglyness */
int ResolveLinksM(const char *path, char *buf, size_t bufsize, UINT cp) {
  switch (cp) {
    case CP_ACP:
      return ResolveLinksA(path, buf, bufsize);
    case CP_UTF8:
      return ResolveLinksU(path, buf, bufsize);
    default:
      errno = EINVAL;
      return -1;
  }
}

/* Get the canonic name of a file, after resolving all links in its pathname */
int ResolveLinksU1(const char *path, char *buf, size_t bufsize, NAMELIST *prev, int iDepth) {
  char *target = NULL;
  char *target2;
  int i;
  int iErr = 0;
  DWORD dwAttr;
  char c = '\0';
  int iPath = 0;
  int bFirst = 1;
  ssize_t nRead;
  int iBuf = 0;
  NAMELIST list;
  NAMELIST *pList;
  char *path0 = NULL;

  DEBUG_ENTER(("ResolveLinks1(\"%s\", 0x%p, %ld, 0x%p, %d);\n", path, buf, bufsize, prev, iDepth));

  target = malloc(UTF8_PATH_MAX);
  if (!target) RETURN_INT_COMMENT(-1, ("Not enough memory\n"));

  /* Convert relative paths to absolute paths */
  if (!(   ((path[0] == '\\') || (path[0] == '/'))
        || (    path[0]
            && (path[1] == ':')
            && ((path[2] == '\\') || (path[2] == '/')))
      )) { /* If this is a relative pathname */
    path0 = malloc(UTF8_PATH_MAX);
    if (!path0) RETURN_INT(-1); /* errno = ENOMEM */
    if (!GetFullPathNameU(path, UTF8_PATH_MAX, path0, NULL)) {
      free(target);
      free(path0);
      errno = EINVAL;
      RETURN_INT(-1);
    }
    path = path0;
  }
  /* Scan every part of the pathname for links */
  while (path[iPath]) {
    /* int iPath0 = iPath; */
    /* int iBuf0 = iBuf; */
    int iBuf1;
    /* Append the first name section to the output buffer */
    if (bFirst) {
      bFirst = 0;
      /* Special case of absolute pathnames */
      if (path[0] == '\\') {
	if ((iBuf+1U) >= bufsize) {
resolves_too_long:
	  free(target);
	  free(path0);
	  errno = ENAMETOOLONG;
	  RETURN_INT_COMMENT(-1, ("Name too long\n"));
	}
	c = buf[iBuf++] = path[iPath++]; /* Copy the root \ . */
	/* DEBUG_PRINTF(("buf[%d] = %c\n", iBuf-1, c)); */
      } else if (path[0] && (path[1] == ':')) {
	if ((iBuf+1U) >= bufsize) goto resolves_too_long;
	c = buf[iBuf++] = path[iPath++]; /* Copy the drive letter */
	/* DEBUG_PRINTF(("buf[%d] = %c\n", iBuf-1, c)); */
	if ((iBuf+1U) >= bufsize) goto resolves_too_long;
	c = buf[iBuf++] = path[iPath++]; /*      and the : */
	/* DEBUG_PRINTF(("buf[%d] = %c\n", iBuf-1, c)); */
	if (path[iPath] == '\\') {
	  if ((iBuf+1U) >= bufsize) goto resolves_too_long;
	  c = buf[iBuf++] = path[iPath++];
	  /* DEBUG_PRINTF(("buf[%d] = %c\n", iBuf-1, c)); */
	}
      } /* Else it's a relative pathname, handled in the common code below */
    } else { /* It's a continuation section in the path */
      if ((iBuf+1U) >= bufsize) goto resolves_too_long;
      buf[iBuf++] = '\\';
    }
    iBuf1 = iBuf; /* Index of the beginning of the node name we're about to add */
    for (i=0; (size_t)(iBuf+i) < (bufsize-1) ;i++) {
      c = path[iPath+i];
      if (!c) break;
      /* if (c == '/') break; */
      if (c == '\\') break;
      /* DEBUG_PRINTF(("buf[%d] = %c\n", iBuf+i, c)); */
      buf[iBuf+i] = c;
    }
    iBuf += i;
    buf[iBuf] = '\0';
    while (c && (/* (c == '/') || */ (c == '\\'))) c=path[iPath + ++i]; /* Skip extra /, if any */
    iPath += i;
    /* DEBUG_PRINTF(("// Removed %d characters from path\n", iPath - iPath0)); */
    /* Get the file type */
    dwAttr = GetFileAttributesU(buf);
    DEBUG_PRINTF(("// \"%s\" is %s\n", buf, (dwAttr == INVALID_FILE_ATTRIBUTES)?"not found":(
					    (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)?"a reparse point":(
      					    (dwAttr & FILE_ATTRIBUTE_DIRECTORY)?"a directory":"a file"))));
    if (dwAttr == INVALID_FILE_ATTRIBUTES) {
      errno = ENOENT;
      if (path[iPath]) { /* Append the remainder of the input path, even though we know it's invalid */
      	if ((iBuf+1U+lstrlen(path+iPath)) < bufsize) {
	  buf[iBuf++] = '\\';
	  lstrcpy(buf+iBuf, path+iPath);
	}
      }
      free(target);
      free(path0);
      RETURN_INT_COMMENT(-1, ("No such file: \"%s\"\n", buf));
    }
    if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
      nRead = readlinkU(buf, target, UTF8_PATH_MAX);
      if (nRead == -1) {
	if (errno == EINVAL) { /* This is a reparse point, but not a symlink or a junction */
	  goto file_or_directory;
	}
	/* Anything else is a real error, and resolution cannot be done. */
	if (path[iPath]) { /* Append the remainder of the input path, even though we know it's invalid */
	  if ((iBuf+1U+lstrlen(path+iPath)) < bufsize) {
	    buf[iBuf++] = '\\';
	    lstrcpy(buf+iBuf, path+iPath);
	  }
	}
	free(target);
	free(path0);
        RETURN_INT_COMMENT(-1, ("Dangling link: \"%s\"\n", buf));
      }
      if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) && !lstrcmp(buf, target)) {
      	/* This is probably a junction pointing to an otherwise inaccessible area.
      	   (See readlink() header in readlink.c for details about this case.)
      	   Handle this junction like if it were a real directory, instead of a symlink. */
	goto file_or_directory;
      }
      if (   ((target[0] == '\\') /* || (target[0] == '/') */ )
	  || (target[1] == ':')) { /* This is an absolute pathname */
	DEBUG_PRINTF(("// Absolute link to \"%s\"\n", target));
	iBuf = (int)lstrlen(target); /* Anticipate the new position after copying */
	if ((size_t)iBuf >= bufsize) goto resolves_too_long;
	lstrcpy(buf, target);
      } else { /* This is a relative pathname */
	DEBUG_PRINTF(("// Relative link to \"%s\"\n", target));
	/* So it'll replace the tail name in the output path */
	iBuf = iBuf1; /* The index right after the last path separator */
	DEBUG_PRINTF(("ResolveLinks1(\"%.*s\", \"%s\"); // Target tail\n", iBuf, buf, target));
	if (((size_t)iBuf+lstrlen(target)) >= bufsize) goto resolves_too_long;
	lstrcpy(buf+iBuf, target);
	iBuf = CompactPath(buf, buf, bufsize);
      }
      /* Append the remainder of the input path, if any */
      DEBUG_PRINTF(("ResolveLinks1(\"%s\", \"%s\"); // Path tail\n", buf, path+iPath));
      if (path[iPath]) {
	if (iBuf && (buf[iBuf-1] != '\\')) {
	  if ((iBuf+1U) >= bufsize) goto resolves_too_long;
	  buf[iBuf++] = '\\';
	}
	if (((size_t)iBuf+lstrlen(path+iPath)) >= bufsize) goto resolves_too_long;
	lstrcpy(buf+iBuf, path+iPath);
	iBuf = CompactPath(buf, buf, bufsize);
      }
      /* Check for max depth */
      if (iDepth == SYMLOOP_MAX) {
	errno = ELOOP;
	free(target);
	free(path0);
	RETURN_INT_COMMENT(-1, ("Max symlink depth reached: \"%s\"\n", buf));
      }
      /* Check for loops */
      for (pList = prev ; pList; pList = pList->prev) {
      	if (!lstrcmpi(buf, pList->path)) {
      	  errno = ELOOP;
	  free(target);
	  free(path0);
	  RETURN_INT_COMMENT(-1, ("Loop found: \"%s\"\n", buf));
	}
      }
      /* OK, no loop, so repeat the process for that new path */
      lstrcpy(target, buf); /* Keep that as a reference in the linked list, in case there are further links */
      target2 = realloc(target, strlen(target)+1);
      if (target2) target = target2;
      list.prev = prev;
      list.path = target;
      iErr = ResolveLinksU1(target, buf, bufsize, &list, iDepth+1);
      free(target);
      free(path0);
      RETURN_INT_COMMENT(iErr, ("\"%s\"\n", buf));
    } else { /* It's a normal file or directory */
file_or_directory:
      if ((path[iPath]) && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
      	errno = ENOTDIR;
	if ((iBuf+1U+lstrlen(path+iPath)) < bufsize) {
	  buf[iBuf++] = '\\';
	  lstrcpy(buf+iBuf, path+iPath);
	}
	free(target);
	free(path0);
	RETURN_INT_COMMENT(-1, ("File where dir expected: \"%s\"\n", buf));
      }
    }
  }

  free(target);
  free(path0);
  RETURN_INT_COMMENT(0, ("Success: \"%s\"\n", buf));
}

int ResolveLinksU(const char *path, char *buf, size_t bufsize) {
  char *path1;
  char *path2;
  int nSize;
  NAMELIST root;
  int iErr;

  DEBUG_ENTER(("ResolveLinks(\"%s\", 0x%p, %ld);\n", path, buf, bufsize));

  buf[0] = '\0'; /* Always output a valid string */

  if (!*path) { /* Spec says an empty pathname is invalid */
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("Empty pathname\n"));
  }

  path1 = malloc(UTF8_PATH_MAX);
  if (!path1) RETURN_INT_COMMENT(-1, ("Not enough memory\n"));

  /* Normalize the input path, using a single \ as path separator */
  nSize = CompactPath(path, path1, UTF8_PATH_MAX);
  if (nSize == -1) { /* CompactPath() already sets errno = ENAMETOOLONG */
    free(path1);
    RETURN_INT_COMMENT(-1, ("Path too long\n"));
  }
  if (path1[nSize-1] == '\\') { /* Spec says resolution must implicitly add a trailing dot, */
    if (nSize == UTF8_PATH_MAX) {    /*  to ensure that the last component is a directory. */
      free(path1);
      errno = ENAMETOOLONG;
      RETURN_INT_COMMENT(-1, ("Path too long after adding .\n"));
    }
    path1[nSize++] = '.'; /* So add it explicitely */
    path1[nSize] = '\0';
  }
  path2 = realloc(path1, nSize+1); /* Resize the buffer to fit the name length */
  if (path2) path1 = path2;
  root.path = path1;
  root.prev = NULL;
  iErr = ResolveLinksU1(path1, buf, bufsize, &root, 0);
  nSize = lstrlen(buf);
  /* Remove the final dot added above, if needed. */
  if ((nSize >= 2) && (buf[nSize-2] == '\\') && (buf[nSize-1] == '.')) buf[--nSize] = '\0';
  free(path1);
  RETURN_INT_COMMENT(iErr, ("\"%s\"\n", buf));
}

/* ANSI version of the same, built upon the UTF-8 version */
int ResolveLinksA(const char *path, char *buf, size_t bufsize) {
  char pathU[UTF8_PATH_MAX];
  char pathU2[UTF8_PATH_MAX];
  WCHAR wszPath[PATH_MAX];
  int n;
  int iErr;

  /* Convert the pathname to a unicode string */
  n = MultiByteToWideChar(CP_ACP, 0, path, (int)strlen(path) + 1, wszPath, PATH_MAX);
  /* Convert it back to UTF-8 characters */
  if (n) n = WideCharToMultiByte(CP_UTF8, 0, wszPath, n, pathU, UTF8_PATH_MAX, NULL, NULL);
  /* Check (unlikely) conversion errors */
  if (!n) {
    errno = Win32ErrorToErrno();
    RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));
  }

  /* Resolve the links */
  iErr = ResolveLinksU(pathU, pathU2, UTF8_PATH_MAX);

  /* Convert the result back to ANSI */
  if (!iErr) {
    n = MultiByteToWideChar(CP_UTF8, 0, pathU2, (int)strlen(pathU2) + 1, wszPath, PATH_MAX);
    if (n) n = WideCharToMultiByte(CP_ACP, 0, wszPath, n, buf, (int)bufsize, NULL, NULL);
    if (!n) {
      errno = Win32ErrorToErrno();
      RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));
    }
  }

  return iErr;
}

/* Normally defined in stdlib.h. Output buf must contain PATH_MAX bytes */
char *realpathU(const char *path, char *outbuf) {
  char *pOutbuf = outbuf;
  char *pPath1 = NULL;
  char *pPath2 = NULL;
  int iErr;
  const char *pc;
  size_t nSize;
  DEBUG_CODE(
  char *pszCause = "Out of memory";
  )
  int n;

  DEBUG_ENTER(("realpath(\"%s\", 0x%p);\n", path, outbuf));

  if (!pOutbuf) pOutbuf = malloc(UTF8_PATH_MAX);
  if (!pOutbuf) {
realpathU_failed:
    if (!outbuf) free(pOutbuf);
    free(pPath1);
    free(pPath2);
    errno = ENOMEM;
    RETURN_CONST_COMMENT(NULL, ("%s\n", pszCause));
  }

  pPath1 = malloc(UTF8_PATH_MAX);
  if (!pPath1) goto realpathU_failed;

  pPath2 = malloc(UTF8_PATH_MAX);
  if (!pPath2) goto realpathU_failed;

  /* Convert relative paths to absolute paths */
  pc = path;
  if (pc[0] && (pc[1] == ':')) pc += 2; /* Skip the drive letter, if any */
  if ((*pc != '/') && (*pc != '\\')) { /* This is a relative path */
    int iDrive = 0;
    if (pc != path) { /* A drive was specified */
      iDrive = toupper(path[0]) - '@'; /* A=1, B=2, ... */
    }
    _getdcwdU(iDrive, pPath1, UTF8_PATH_MAX);
    nSize = UTF8_PATH_MAX - lstrlen(pPath1);
    if ((lstrlen(pc) + 2U) > nSize) {
      errno = ENAMETOOLONG;
      DEBUG_CODE(pszCause = "Path too long after concatenating current dir");
      goto realpathU_failed; 
    }
    strcat(pPath1, "\\");
    strcat(pPath1, pc);
    path = pPath1;
  } else if (pc == path) { /* This is an absolute path without a drive letter */
    pPath1[0] = (char)(_getdrive() + 0x40);
    pPath1[1] = ':';
    if (strlen(path) > (UTF8_PATH_MAX-3)) {
      errno = ENAMETOOLONG;
      DEBUG_CODE(pszCause = "Path too long after adding drive");
      goto realpathU_failed;
    }
    strcpy(pPath1+2, path);
    path = pPath1;
  }    

  /* Resolve links in the absolute path */
  iErr = ResolveLinksU(path, pPath2, UTF8_PATH_MAX);
  if (iErr == -1) {
    DEBUG_CODE(pszCause = "Resolution failed");
    goto realpathU_failed;
  }

  /* Change short names to long names, and correct the name case */
  n = GetLongPathNameU(pPath2, pOutbuf, UTF8_PATH_MAX); /* This will NOT correct long names case */
  if (!n) {
    DEBUG_CODE(pszCause = "Can't get long pathnames";)
    goto realpathU_failed;
  }

  DEBUG_LEAVE(("return 0x%p; // \"%s\"\n", pOutbuf, pOutbuf));
  if (!outbuf) pOutbuf = realloc(pOutbuf, strlen(pOutbuf) + 1);
  free(pPath1);
  free(pPath2);
  return pOutbuf;
}

#endif /* defined(_WIN32) */

