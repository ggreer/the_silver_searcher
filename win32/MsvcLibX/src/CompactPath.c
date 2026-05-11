/*****************************************************************************\
*                                                                             *
*   Filename	    CompactPath.c					      *
*									      *
*   Description     Remove . and .. parts in pathnames			      *
*                                                                             *
*   Notes	                                                              *
*                                                                             *
*   History								      *
*    2014-02-10 JFL Created realpath.c with routine ResolveLinks() for Win32. *
*    2014-02-19 JFL Added OS-independant CompactPath() subroutine. 	      *
*    2026-01-03 JFL Extracted CompactPath() & CompactPathW() off of realpath.c.
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
|    2026-01-23 JFL Simplified and fixed the debug output, w. DEBUG_W* macros.|
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
  int iLen;

  DEBUG_WENTER((L"CompactPathW(L\"%s\", 0x%p, %lu);\n", path, outbuf, (unsigned long)bufsize));

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
      XDEBUG_WPRINTF((L"pParts[%d] = \"%.*s\"; l = %d\n", nParts-1, lPart, pParts[nParts-1], lPart));
    }
    lastc = c;
    if (c == L'\0') break;
  }

  /* Eliminate . and .. parts */
  for (i=0; i<nParts; i++) {
    XDEBUG_WPRINTF((L"for pParts[%d] = \"%.*s\"\n", i, lParts[i], pParts[i]));
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
  DEBUG_WLEAVE((L"return %d; // L\"%s\"\n", iLen, outbuf));
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
  if (iReAlloc) pwszBuf = ShrinkBuf(pwszBuf, (l + 1) * sizeof(WCHAR));
  return pwszBuf;
}

#endif /* defined(_WIN32) */

