/*****************************************************************************\
*                                                                             *
*   Filename	    utimes.c						      *
*									      *
*   Description:    WIN32 port of standard C library's *utimes()	      *
*                                                                             *
*   Notes:	    TO DO: Create W, A, U versions of ResolveLinks(), then    *
*		    	   create W, A, U versions of utimes().		      *
*		    							      *
*   History:								      *
*    2014-02-07 JFL Created this module.				      *
*    2014-03-24 JFL Renamed "statx.h" as the standard <sys/stat.h>.	      *
*    2014-06-03 JFL Added support for WIDE, ANSI and UTF8 versions.	      *
*    2014-06-04 JFL Added handling of UTIME_NOW and UTIME_OMIT.		      *
*    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      *
*    2017-05-31 JFL Get strerror() prototype from string.h.                   *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of Windows print routines */

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <errno.h>
#include <sys/time.h> /* Must be included before any direct or indirect <windows.h> inclusion */
#include <sys/stat.h>
#include <string.h>
#include "debugm.h"

#if defined(_DEBUG)
#include <stdio.h>
#endif /* defined(_DEBUG) */

#ifdef _WIN32

#include <windows.h>
#include <io.h> /* For MSVC's _get_osfhandle() */
#include <unistd.h> /* For MsvcLibX's ResolveLinks() */

DEBUG_CODE(
  int Timeval2String(char *buf, size_t bufsize, const struct timeval *tvp) {
    struct tm *pTime;
    int n;
    if (tvp->tv_usec == UTIME_NOW) {
      return _snprintf(buf, bufsize, "UTIME_NOW");
    }
    if (tvp->tv_usec == UTIME_OMIT) {
      return _snprintf(buf, bufsize, "UTIME_OMIT");
    }
    pTime = localtime(&(tvp->tv_sec)); /* Time of last data modification */
    n = _snprintf(buf, bufsize, "{%4d-%02d-%02d %02d:%02d:%02d.%06d, ...}",
	         pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday,
	         pTime->tm_hour, pTime->tm_min, pTime->tm_sec, tvp->tv_usec);
    return n;
  }
);

/* Convert a Windows FILETIME to a Unix timeval
   A FILETIME is the number of 100-nanosecond intervals since January 1, 1601.
   A time_t is the number of 1-second intervals since January 1, 1970. */

void Filetime2Timeval(const FILETIME *pFT, struct timeval *ptv) {
  ULARGE_INTEGER ull;
  ull.LowPart = pFT->dwLowDateTime;
  ull.HighPart = pFT->dwHighDateTime;
  ptv->tv_sec = ull.QuadPart / 10000000ULL - 11644473600ULL;
  ptv->tv_usec = (int32_t)((ull.QuadPart % 10000000ULL)/10);
}

void Timeval2Filetime(const struct timeval *ptv, FILETIME *pFT) {
  ULARGE_INTEGER ull;
  struct timeval now;
  if (ptv->tv_usec == UTIME_NOW) {
    gettimeofday(&now, NULL); /* Get the current time into a struct timeval */
    DEBUG_CODE({
      char buf[100];
      Timeval2String(buf, sizeof(buf), &now);
      DEBUG_PRINTF(("// UTIME_NOW -> %s\n", buf));
    });
    ptv = &now;
  }
  ull.QuadPart = ptv->tv_sec + 11644473600ULL;
  ull.QuadPart *= 1000000UL;
  ull.QuadPart += ptv->tv_usec;
  ull.QuadPart *= 10UL;
  pFT->dwLowDateTime = ull.LowPart;
  pFT->dwHighDateTime = ull.HighPart;
}

/* Low level subroutine used by all the other routines below. */
int hutimes(HANDLE hFile, const struct timeval tvp[2]) {
  FILETIME ftLastAccess;  /* last access time */
  FILETIME ftLastWrite;   /* last write time */
  FILETIME *pftLastAccess = &ftLastAccess;
  FILETIME *pftLastWrite = &ftLastWrite;
  int iErr;

  if (tvp[0].tv_usec != UTIME_OMIT) {
    Timeval2Filetime(tvp+0, pftLastAccess);
  } else {
    pftLastAccess = NULL;	/* Do not set this value */
  }
  if (tvp[1].tv_usec != UTIME_OMIT) {
    Timeval2Filetime(tvp+1, pftLastWrite);
  } else {
    pftLastWrite = NULL;	/* Do not set this value */
  }
  iErr = !SetFileTime(hFile, NULL, pftLastAccess, pftLastWrite);
  if (iErr) {
    errno = Win32ErrorToErrno();
    return -1;
  }
  return 0;
}

/* Same as 'utimes', but does not follow symbolic links. */
int lutimesW(const WCHAR *path, const struct timeval tvp[2]) {
  DWORD dwAttr;
  DWORD dwFlagsAndAttributes;
  int iErr;
  HANDLE hLink;

  DEBUG_CODE({
    char buf[100];
    char szUtf8[UTF8_PATH_MAX];
    Timeval2String(buf, sizeof(buf), tvp+1);
    DEBUG_WSTR2UTF8(path, szUtf8, sizeof(szUtf8));
    DEBUG_ENTER(("lutimes(\"%s\", %s);\n", szUtf8, buf));
  });

  dwAttr = GetFileAttributesW(path);
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("File does not exist\n"));
  }

  dwFlagsAndAttributes = FILE_FLAG_OPEN_REPARSE_POINT;
  if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) dwFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
  hLink = CreateFileW(path,					/* lpFileName,		  */
		      FILE_WRITE_ATTRIBUTES,			/* dwDesiredAccess, */
		      FILE_SHARE_READ | FILE_SHARE_WRITE,	/* dwShareMode,           */
		      NULL,					/* lpSecurityAttributes,  */
		      OPEN_EXISTING,                    	/* dwCreationDisposition, */
		      dwFlagsAndAttributes,             	/* dwFlagsAndAttributes,  */
		      NULL                              	/* hTemplateFile          */
		     );
  XDEBUG_PRINTF(("CreateFile() = 0x%p\n", hLink));
  if (hLink == INVALID_HANDLE_VALUE) {
    errno = Win32ErrorToErrno();
    RETURN_INT_COMMENT(-1, ("Cannot open the pathname\n"));
  }

  iErr = hutimes(hLink, tvp);
  CloseHandle(hLink);
  RETURN_INT_COMMENT(iErr, ("errno = %d\n", iErr ? errno : 0));
}

int lutimesA(const char *path, const struct timeval tvp[2]) {
  WCHAR wszPath[PATH_MAX];
  int n;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_ACP,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszPath,		/* lpWideCharStr, */
			  COUNTOF(wszPath)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_ENTER(("lutimesA(\"%s\", %p);\n", path, tvp));
    RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));
  }
  return lutimesW(wszPath, tvp);
}

int lutimesU(const char *path, const struct timeval tvp[2]) {
  WCHAR wszPath[PATH_MAX];
  int n;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszPath,		/* lpWideCharStr, */
			  COUNTOF(wszPath)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_ENTER(("lutimesU(\"%s\", %p);\n", path, tvp));
    RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));
  }
  return lutimesW(wszPath, tvp);
}

/* Same as 'utimes', but takes an open file descriptor instead of a name. */
int futimes(int fd, const struct timeval tvp[2]) {
  return hutimes((HANDLE)_get_osfhandle(fd), tvp);
}

/* Change the file access time to tvp[0] and its modification time to tvp[1]. */
int utimes(const char *file, const struct timeval tvp[2]) {
  char buf[UTF8_PATH_MAX];
  int iErr;

  DEBUG_CODE({
    char buf[100];
    Timeval2String(buf, sizeof(buf), tvp+1);
    DEBUG_ENTER(("utimes(\"%s\", %s);\n", file, buf));
  });

  iErr = ResolveLinksU(file, buf, sizeof(buf));
  if (iErr) RETURN_INT_COMMENT(iErr, ("Cannot resolve the link\n"));

  iErr = lutimesU(buf, tvp);
  RETURN_INT_COMMENT(iErr, ("errno = %d\n", iErr ? errno : 0));
}

#endif /* defined(_WIN32) */

