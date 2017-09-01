/*****************************************************************************\
*                                                                             *
*   Filename	    lstat.c						      *
*									      *
*   Description     WIN32 port of standard C library's lstat()		      *
*                                                                             *
*   Notes 	    TO DO: Make 3 versions for Windows: ANSI, WSTR, UTF8      *
*		    							      *
*   History								      *
*    2014-02-06 JFL Created this module.				      *
*    2014-02-12 JFL Added code to filter reparse points, and keep only        *
*		    real junctions and symlinks.                              *
*    2014-02-13 JFL Moved dirent2stat() from dirent.c, as there must actually *
*		    be 4 WIN32 versions, for the four versions of struct stat.*
*    2014-02-28 JFL Added support for UTF-8 pathnames.                 	      *
*    2014-03-24 JFL Renamed "statx.h" as the standard <sys/stat.h>.	      *
*    2014-06-30 JFL Added support for 32K Unicode paths.           	      *
*    2017-03-18 JFL Moved unlink() and rmdir() to their own source files.     *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
#include <string.h>
/* MsvcLibX library extensions */
#include "msvclibx.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h> /* For ResolveLinks() definition */
#include "debugm.h"
#include <stdint.h>

#if defined(_MSDOS)
/* Make sure it's only defined it in one of the lstatxxx versions */
#if !defined(_USE_32BIT_TIME_T) && (_FILE_OFFSET_BITS == 32)

int dirent2stat(_dirent *pDirent, struct _stat *pStat) {
  memset(pStat, 0, sizeof(struct stat));

  /* Set times */
  pStat->st_mtime = Filetime2Timet(pDirent->d_date, pDirent->d_time);
  /* Size */
  pStat->st_size = pDirent->d_filesize;
  /* Standard attributes */
  pStat->st_mode |= (pDirent->d_type << 12); /* Set the 4-bit type field */
  pStat->st_mode |= _S_IREAD | _S_IWRITE | _S_IEXEC; /* Assume it's fully accessible */
  if (pDirent->d_attribs & _A_RDONLY) pStat->st_mode &= ~_S_IWRITE;
  /* DOS-specific attributes */
  if (pDirent->d_attribs & _A_HIDDEN) pStat->st_mode |= S_HIDDEN;
  if (pDirent->d_attribs & _A_ARCH) pStat->st_mode |= S_ARCHIVE;
  if (pDirent->d_attribs & _A_SYSTEM) pStat->st_mode |= S_SYSTEM;

  return 0;
}

#endif /* defined(_USE_32BIT_TIME_T) && (_FILE_OFFSET_BITS == 32) */
#endif /* defined(_MSDOS) */


#ifdef _WIN32

/* ------------ Display the *stat* macro values at compile time ------------ */

#pragma message(MACRODEF(_MSVC_stat))
#pragma message(MACRODEF(_MSVC_fstat))
#pragma message(MACRODEF(_MSVC_lstat))
#pragma message(MACRODEF(_MSVC_stat64))

#if _MSVCLIBX_STAT_DEFINED
  #pragma message(MACRODEF(_LIBX_stat))
  #pragma message(MACRODEF(_LIBX_stat64))
#endif

#pragma message(MACRODEF(stat))
#pragma message(MACRODEF(fstat))
#pragma message(MACRODEF(lstat))

#if 0
#pragma message(MACRODEF(_lstat))
#pragma message(MACRODEF(_lstati64))
#if _MSVCLIBX_STAT_DEFINED
  #pragma message(MACRODEF(_lstat_ns))
  #pragma message(MACRODEF(_lstati64_ns))
#endif
#endif

#if defined(_LARGEFILE_SOURCE64)
  #pragma message(MACRODEF(stat64))
  #pragma message(MACRODEF(fstat64))
  #pragma message(MACRODEF(lstat64))
#endif

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    lstat						      |
|									      |
|   Description	    Common definition of all _lstatXY() functions             |
|									      |
|   Parameters      const char *path		The symlink name	      |
|		    struct stat *buf		Output buffer		      |
|									      |
|   Returns	    0 = Success, -1 = Failure				      |
|									      |
|   Notes	    See sys\stat.h for a description of how the stat and      |
|		    lstat macros work.					      |
|									      |
|   History								      |
|    2014-02-06 JFL Created this routine                               	      |
|    2014-02-28 JFL Added support for UTF-8 pathnames.                 	      |
*									      *
\*---------------------------------------------------------------------------*/

int lstat(const char *path, struct stat *pStat) {
  BOOL bDone;
  DWORD dwAttr;
  WIN32_FILE_ATTRIBUTE_DATA fileData;
  unsigned __int64 qwSize;
  int bIsJunction = FALSE;
  int bIsMountPoint = FALSE;
  DWORD dwTag = 0;
  DEBUG_CODE(
  char szTime[100];
  )
  WCHAR wszName[UNICODE_PATH_MAX];
  int n;

  DEBUG_ENTER((STRINGIZE(lstat) "(\"%s\", 0x%p);\n", path, pStat));

#if USE_MSVC_STAT
  dwAttr = GetFileAttributes(path);
  DEBUG_PRINTF(("GetFileAttributes() = 0x%lX\n", dwAttr));
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("File does not exist\n"));
  }

  if (!(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
    int iErr = stat(path, pStat);
    RETURN_INT(iErr);
  }
#endif

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszName,		/* lpWideCharStr, */
			  UNICODE_PATH_MAX	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));
  }

  bDone = GetFileAttributesExW(wszName, GetFileExInfoStandard, &fileData);
  if (!bDone) {
    errno = Win32ErrorToErrno();
    RETURN_INT_COMMENT(-1, ("GetFileAttributesEx(); // Failed\n"));
  }
  XDEBUG_PRINTF(("GetFileAttributesEx(); // Success\n"));
  dwAttr = fileData.dwFileAttributes;
  XDEBUG_PRINTF(("dwFileAttributes = 0x%lX\n", dwAttr));
  DEBUG_CODE_IF_ON(Filetime2String(&fileData.ftLastWriteTime, szTime, sizeof(szTime)););
  XDEBUG_PRINTF(("ftLastWriteTime = %s\n", szTime));
  qwSize = ((unsigned __int64)fileData.nFileSizeHigh << 32) | fileData.nFileSizeLow;
  XDEBUG_PRINTF(("nFileSize = %I64d\n", qwSize));

  ZeroMemory(pStat, sizeof(struct stat));
  /* Set times */
#if _MSVCLIBX_STAT_DEFINED
  Filetime2Timespec(&fileData.ftCreationTime, &(pStat->st_ctim));  /* Windows = Create time; Unix = Permissions change time */
  Filetime2Timespec(&fileData.ftLastWriteTime, &(pStat->st_mtim));
  Filetime2Timespec(&fileData.ftLastAccessTime, &(pStat->st_atim));
#else
  Filetime2Timet(&fileData.ftCreationTime, &(pStat->st_ctime));  /* Windows = Create time; Unix = Permissions change time */
  Filetime2Timet(&fileData.ftLastWriteTime, &(pStat->st_mtime));
  Filetime2Timet(&fileData.ftLastAccessTime, &(pStat->st_atime));
#endif
  /* Size */
  /* NOTE: There is loss of data here if the file size is > 2GB, and off_t is 32-bits */
  pStat->st_size = (off_t)qwSize;
#if (_STAT_FILE_SIZE < 64)
#define _MAX_FILE_SIZE 0x7FFFFFFFL
  if (qwSize > _MAX_FILE_SIZE) pStat->st_size = (off_t)_MAX_FILE_SIZE;
#endif
  /* Standard attributes */
  /* File type */
check_attr_again:
  if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
    /* JUNCTIONs and SYMLINKDs both have the FILE_ATTRIBUTE_DIRECTORY flag also set.
    // Test the FILE_ATTRIBUTE_REPARSE_POINT flag first, to make sure they're seen as symbolic links.
    //
    // All symlinks are reparse points, but not all reparse points are symlinks. */
    dwTag = GetReparseTagU(path);
    switch (dwTag) {
      case IO_REPARSE_TAG_MOUNT_POINT:	/* NTFS junction or mount point */
	{ /* We must read the link to distinguish junctions from mount points. */
	WCHAR wbuf[UNICODE_PATH_MAX];
	ssize_t n;
	bIsMountPoint = TRUE;
	n = readlinkW(wszName, wbuf, UNICODE_PATH_MAX);
	/* Junction targets are absolute pathnames, starting with a drive letter. Ex: C: */
	/* readlink() fails if the reparse point does not target a valid pathname */
	if (n < 0) goto this_is_not_a_symlink; /* This is not a junction. */
	bIsJunction = TRUE; /* Else this is a junction. Fall through to the symlink case. */
	} 	      
      case IO_REPARSE_TAG_SYMLINK:		/* NTFS symbolic link */
	pStat->st_mode |= S_IFLNK;		/* Symbolic link */
	break;
      default:	/* Anything else is definitely not like a Unix symlink */
this_is_not_a_symlink:
	dwAttr &= ~FILE_ATTRIBUTE_REPARSE_POINT;
	goto check_attr_again;
    }
  } else if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
    pStat->st_mode |= S_IFDIR;		/* Subdirectory */
  else if (dwAttr & FILE_ATTRIBUTE_DEVICE)
    pStat->st_mode |= S_IFCHR;		/* Device (we don't know if character or block) */
  else
    pStat->st_mode |= S_IFREG;		/* A normal file by default */
  /* pStat->st_mode |= (pDirent->d_type << 12); /* Set the 4-bit type field */
  pStat->st_mode |= _S_IREAD | _S_IWRITE | _S_IEXEC; /* Assume it's fully accessible */
  if (dwAttr & FILE_ATTRIBUTE_READONLY) pStat->st_mode &= ~_S_IWRITE;
  /* DOS-specific attributes */
  if (dwAttr & FILE_ATTRIBUTE_HIDDEN) pStat->st_mode |= S_HIDDEN;
  if (dwAttr & FILE_ATTRIBUTE_ARCHIVE) pStat->st_mode |= S_ARCHIVE;
  if (dwAttr & FILE_ATTRIBUTE_SYSTEM) pStat->st_mode |= S_SYSTEM;
  /* Windows-specific attributes */
  if (dwAttr & FILE_ATTRIBUTE_COMPRESSED) pStat->st_mode |= S_COMPRESSED;
  if (dwAttr & FILE_ATTRIBUTE_ENCRYPTED) pStat->st_mode |= S_ENCRYPTED;
  if (dwAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) pStat->st_mode |= S_NOT_CONTENT_INDEXED;
  if (dwAttr & FILE_ATTRIBUTE_OFFLINE) pStat->st_mode |= S_OFFLINE;
  if (dwAttr & FILE_ATTRIBUTE_SPARSE_FILE) pStat->st_mode |= S_SPARSE_FILE;
  if (bIsMountPoint) pStat->st_mode |= S_MOUNT_POINT; /* Will allow to distinguish junctions from mount points */
  /* if (dwAttr & FILE_ATTRIBUTE_TEMPORARY) pStat->st_mode |= S_TEMPORARY; */
  /* if (dwAttr & FILE_ATTRIBUTE_VIRTUAL) pStat->st_mode |= S_VIRTUAL; */
#if _MSVCLIBX_STAT_DEFINED
  pStat->st_Win32Attrs = dwAttr;
  pStat->st_ReparseTag = dwTag;
#endif

  RETURN_INT_COMMENT(0, ("%s  mode = 0x%04X  size = %I64d bytes\n", szTime, pStat->st_mode, qwSize));
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    stat						      |
|									      |
|   Description	    Common definition of all _statXY() functions              |
|									      |
|   Parameters      const char *path		The symlink name	      |
|		    struct stat *buf		Output buffer		      |
|									      |
|   Returns	    0 = Success, -1 = Failure				      |
|									      |
|   Notes	    See sys\stat.h for a description of how the stat and      |
|		    lstat macros work.					      |
|									      |
|   History								      |
|    2014-02-06 JFL Created this routine                               	      |
|    2014-02-28 JFL Added support for UTF-8 pathnames.                 	      |
|    2017-03-22 JFL No need to fully resolve all links in the pathname.       |
|		    Resolving the tail links is sufficient, and much faster.  |
|    2017-03-24 JFL Only resolve links if it's a link.                        |
*									      *
\*---------------------------------------------------------------------------*/

#if !USE_MSVC_STAT
int stat(const char *path, struct stat *pStat) {
  char buf[UTF8_PATH_MAX];
  int iErr;
  DWORD dwAttr;

  DEBUG_ENTER((STRINGIZE(stat) "(\"%s\", 0x%p);\n", path, pStat));

  dwAttr = GetFileAttributes(path);
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("File does not exist\n"));
  }

  if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
    iErr = ResolveTailLinks(path, buf, sizeof(buf));
    path = buf;
  } else {
    iErr = 0;
  }

  if (!iErr) iErr = lstat(path, pStat);

  RETURN_INT(iErr);
}
#endif /* !USE_MSVC_STAT */

int dirent2stat(_dirent *pDirent, struct stat *pStat) {
  memset(pStat, 0, sizeof(struct stat));

  /* Set times */
#if _MSVCLIBX_STAT_DEFINED
  Filetime2Timespec(&pDirent->d_CreationTime, &(pStat->st_ctim));  /* Windows = Create time; Unix = Permissions change time */
  Filetime2Timespec(&pDirent->d_LastWriteTime, &(pStat->st_mtim));
  Filetime2Timespec(&pDirent->d_LastAccessTime, &(pStat->st_atim));
#else
  Filetime2Timet(&pDirent->d_CreationTime, &(pStat->st_ctime));  /* Windows = Create time; Unix = Permissions change time */
  Filetime2Timet(&pDirent->d_LastWriteTime, &(pStat->st_mtime));
  Filetime2Timet(&pDirent->d_LastAccessTime, &(pStat->st_atime));
#endif
  /* Size */
  /* NOTE: There is loss of data here if the file size is > 2GB, and off_t is 32-bits */
  pStat->st_size = (off_t)(pDirent->d_filesize);
#if (_STAT_FILE_SIZE < 64)
#define _MAX_FILE_SIZE 0x7FFFFFFFL
  if (pDirent->d_filesize > _MAX_FILE_SIZE) pStat->st_size = (off_t)_MAX_FILE_SIZE;
#endif
  /* Standard attributes */
  pStat->st_mode |= (pDirent->d_type << 12); /* Set the 4-bit type field */
  pStat->st_mode |= _S_IREAD | _S_IWRITE | _S_IEXEC; /* Assume it's fully accessible */
  if (pDirent->d_attribs & FILE_ATTRIBUTE_READONLY) pStat->st_mode &= ~_S_IWRITE;
  /* DOS-specific attributes */
  if (pDirent->d_attribs & FILE_ATTRIBUTE_HIDDEN) pStat->st_mode |= S_HIDDEN;
  if (pDirent->d_attribs & FILE_ATTRIBUTE_ARCHIVE) pStat->st_mode |= S_ARCHIVE;
  if (pDirent->d_attribs & FILE_ATTRIBUTE_SYSTEM) pStat->st_mode |= S_SYSTEM;
  /* Windows-specific attributes */
  if (pDirent->d_attribs & FILE_ATTRIBUTE_COMPRESSED) pStat->st_mode |= S_COMPRESSED;
  if (pDirent->d_attribs & FILE_ATTRIBUTE_ENCRYPTED) pStat->st_mode |= S_ENCRYPTED;
  if (pDirent->d_attribs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) pStat->st_mode |= S_NOT_CONTENT_INDEXED;
  if (pDirent->d_attribs & FILE_ATTRIBUTE_OFFLINE) pStat->st_mode |= S_OFFLINE;
  if (pDirent->d_attribs & FILE_ATTRIBUTE_SPARSE_FILE) pStat->st_mode |= S_SPARSE_FILE;
  /* Special case of junction and mount points */
  if (pDirent->d_ReparseTag) pStat->st_mode |= S_MOUNT_POINT;
  /* if (pDirent->d_attribs & FILE_ATTRIBUTE_TEMPORARY) pStat->st_mode |= S_TEMPORARY; */
  /* if (pDirent->d_attribs & FILE_ATTRIBUTE_VIRTUAL) pStat->st_mode |= S_VIRTUAL; */
#if _MSVCLIBX_STAT_DEFINED
  pStat->st_Win32Attrs = pDirent->d_attribs;
  pStat->st_ReparseTag = pDirent->d_ReparseTag;
#endif

  return 0;
}

#endif /* _WIN32 */

