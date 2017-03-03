/*****************************************************************************\
*                                                                             *
*   Filename	    utime.c						      *
*									      *
*   Description	    Updated utime() and port of standard C library's lutime() *
*                                                                             *
*   Notes	    TO DO: Create W, A, U versions of ResolveLinks(), then    *
*		    	   create W, A, U versions of utime().		      *
*		    							      *
*   History								      *
*    2014-02-12 JFL Created this module.				      *
*    2014-06-04 JFL Fixed minors issues in debugging code.		      *
*    2014-07-02 JFL Added support for pathnames >= 260 characters. 	      *
*    2016-08-25 JFL Added missing routine utimeA().                	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_SOURCE /* Generate the UTF-8 version of Windows print routines */

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <errno.h>
#include <sys/time.h> /* Must be included before any direct or indirect <windows.h> inclusion */
#include <utime.h>

#include "debugm.h"

#if defined(_DEBUG)
#include <stdio.h>
#endif /* defined(_DEBUG) */

#ifdef _WIN32

#include <windows.h>
#include <io.h> /* For  MSVC's _get_osfhandle() */
#include <unistd.h> /* For MsvcLibX's ResolveLinks() */

/* Convert a Windows FILETIME to a Unix time_t.
   A FILETIME is the number of 100-nanosecond intervals since January 1, 1601.
   A time_t is the number of 1-second intervals since January 1, 1970. */

time_t Filetime2Time_t(FILETIME *pFT) {
  ULARGE_INTEGER ull;
  ull.LowPart = pFT->dwLowDateTime;
  ull.HighPart = pFT->dwHighDateTime;
  return (time_t)(ull.QuadPart / 10000000ULL - 11644473600ULL);
}

void Time_t2Filetime(time_t ft, FILETIME *pFT) {
  ULARGE_INTEGER ull;
  ull.QuadPart = ft + 11644473600ULL;
  ull.QuadPart *= 10000000UL;
  pFT->dwLowDateTime = ull.LowPart;
  pFT->dwHighDateTime = ull.HighPart;
}

DEBUG_CODE(
  int Utimbuf2String(char *buf, size_t bufsize, const struct utimbuf *times) {
    struct tm *pTime;
    int n;
    if (!times) {
      return _snprintf(buf, bufsize, "NULL");
    }
    pTime = localtime(&(times->modtime)); /* Time of last data modification */
    n = _snprintf(buf, bufsize, "{%4d-%02d-%02d %02d:%02d:%02d, ...}",
	         pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday,
	         pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
    return n;
  }
);

/* Low level subroutine used by all the other routines below. */
int hutime(HANDLE hFile, const struct utimbuf *times) {
  FILETIME ftLastAccess;  /* last access time */
  FILETIME ftLastWrite;   /* last write time */
  int iErr;
  struct utimbuf now;

  if (!times) {
    now.actime = now.modtime = time(NULL);
    times = &now;
  }

  Time_t2Filetime(times->actime, &ftLastAccess);
  Time_t2Filetime(times->modtime, &ftLastWrite);
  iErr = !SetFileTime(hFile, NULL, &ftLastAccess, &ftLastWrite);
  if (iErr) {
    errno = Win32ErrorToErrno();
    return -1;
  }
  return 0;
}

/* Same as 'utime', but does not follow symbolic links. */
int lutimeW(const WCHAR *path, const struct utimbuf *times) {
  DWORD dwAttr;
  DWORD dwFlagsAndAttributes;
  int iErr;
  HANDLE hLink;

  DEBUG_CODE({
    char buf[100];
    char szUtf8[UTF8_PATH_MAX];
    Utimbuf2String(buf, sizeof(buf), times);
    DEBUG_WSTR2UTF8(path, szUtf8, sizeof(szUtf8));
    DEBUG_ENTER(("lutime(\"%s\", %s);\n", szUtf8, buf));
  });

  dwAttr = GetFileAttributesW(path);
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("File does not exist\n"));
  }

  dwFlagsAndAttributes = FILE_FLAG_OPEN_REPARSE_POINT;
  if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) dwFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
  hLink = CreateFileW(path,					/* lpFileName, */
		      FILE_WRITE_ATTRIBUTES,			/* dwDesiredAccess, */
		      FILE_SHARE_READ | FILE_SHARE_WRITE,	/* dwShareMode, */
		      NULL,					/* lpSecurityAttributes, */
		      OPEN_EXISTING,                     	/* dwCreationDisposition, */
		      dwFlagsAndAttributes,              	/* dwFlagsAndAttributes, */
		      NULL                               	/* hTemplateFile */
		    );
  XDEBUG_PRINTF(("CreateFile() = 0x%p\n", hLink));
  if (hLink == INVALID_HANDLE_VALUE) {
    errno = Win32ErrorToErrno();
    RETURN_INT_COMMENT(-1, ("Cannot open the pathname\n"));
  }

  iErr = hutime(hLink, times);
  CloseHandle(hLink);
  RETURN_INT_COMMENT(iErr, ("errno = %d\n", iErr ? errno : 0));
}

int lutimeA(const char *path, const struct utimbuf *times) {
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
    DEBUG_ENTER(("lutimeA(\"%s\", %p);\n", path, times));
    RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));
  }
  return lutimeW(wszPath, times);
}

int lutimeU(const char *path, const struct utimbuf *times) {
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
    DEBUG_ENTER(("lutimeU(\"%s\", %p);\n", path, times));
    RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));
  }
  return lutimeW(wszPath, times);
}

/* Same as 'utime', but takes an open file descriptor instead of a name. */
int futime(int fd, const struct utimbuf *times) {
  return hutime((HANDLE)_get_osfhandle(fd), times);
}

/* Change the file access time to times->actime and its modification time to times->modtime. */
int utimeA(const char *file, const struct utimbuf *times) {
  char buf[PATH_MAX];
  int iErr;

  DEBUG_CODE({
    char buf[100];
    Utimbuf2String(buf, sizeof(buf), times);
    DEBUG_ENTER(("utime(\"%s\", %s);\n", file, buf));
  });

  iErr = ResolveLinksA(file, buf, sizeof(buf));
  if (iErr) RETURN_INT_COMMENT(iErr, ("Cannot resolve the link\n"));

  iErr = lutimeA(buf, times);
  RETURN_INT_COMMENT(iErr, ("errno = %d\n", iErr ? errno : 0));
}

/* Change the file access time to times->actime and its modification time to times->modtime. */
int utimeU(const char *file, const struct utimbuf *times) {
  char buf[UTF8_PATH_MAX];
  int iErr;

  DEBUG_CODE({
    char buf[100];
    Utimbuf2String(buf, sizeof(buf), times);
    DEBUG_ENTER(("utime(\"%s\", %s);\n", file, buf));
  });

  iErr = ResolveLinksU(file, buf, sizeof(buf));
  if (iErr) RETURN_INT_COMMENT(iErr, ("Cannot resolve the link\n"));

  iErr = lutimeU(buf, times);
  RETURN_INT_COMMENT(iErr, ("errno = %d\n", iErr ? errno : 0));
}

#endif /* defined(_WIN32) */

