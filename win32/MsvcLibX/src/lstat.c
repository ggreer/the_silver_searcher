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
*    2017-10-05 JFL Removed one huge buffer on stack, to lower stack usage.   *
*    2018-05-31 JFL Changed dirent2stat() first arg to (const _dirent *).     *
*    2020-12-15 JFL Added support for IO_REPARSE_TAG_APPEXECLINK.             *
*    2021-11-29 JFL Prefixed MsvcLibX-specific WIN32 public routines with Mlx.*
*    2025-12-06 JFL stat() and lstat() are now front ends to the new lstatX(),*
*                   extending the old lstat() with MlxGetFileAttributesAndID()*
*                   instead of GetFileAttributesEx() and MlxGetFileID().      *
*    2026-02-04 JFL Moved the conversion of the Win32 attributes and reparse  *
*		    tag to a C file type to new routine MlxAttrAndTag2Type(). *
*    2026-02-11 JFL Decode st_mode flags in the debugging output.             *
*                   Fixed the setting of st_mode flag S_MOUNT_POINT.	      *
*                   Moved the APPEXECLINK fix to MlxGetFileAttributesAndIDW().*
*    2026-02-19 JFL Added a workaround for Win7 VMware VMs reading null	      *
*		    reparse tags on their host's shared folders.	      *
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
#include <unistd.h> /* For MlxResolveLinks() definition */
#include "debugm.h"
#include <stdint.h>

#if defined(_MSDOS)
/* Make sure it's only defined it in one of the lstatxxx versions */
#if !defined(_USE_32BIT_TIME_T) && (_FILE_OFFSET_BITS == 32)

int dirent2stat(const _dirent *pDirent, struct _stat *pStat) {
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

#include "reparsept.h" /* For the undocumented IO_REPARSE_TAG_LX_SYMLINK, etc */

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
|   Function	    lstatX						      |
|									      |
|   Description	    Common definition of all _statXY() & _lstatXY() functions |
|									      |
|   Parameters      const char *path		The symlink name	      |
|		    struct stat *buf		Output buffer		      |
|		    BOOL is_lstat		1 = lstat(); 0 = stat()	      |
|		    							      |
|   Returns	    0 = Success, -1 = Failure				      |
|									      |
|   Notes	    See sys\stat.h for a description of how the stat and      |
|		    lstat macros work.					      |
|									      |
|   History								      |
|    2014-02-06 JFL Created this routine                               	      |
|    2014-02-28 JFL Added support for UTF-8 pathnames.                 	      |
|    2026-02-11 JFL Moved the APPEXECLINK fix to MlxGetFileAttributesAndIDW().|
*									      *
\*---------------------------------------------------------------------------*/

#define CONCAT_TOKENS(a, b) a##b

#define CONCAT_TOKENS_VALUE(a, b) CONCAT_TOKENS(a, b)

#define lstatX CONCAT_TOKENS_VALUE(lstat, X)

int lstatX(const char *path, struct stat *pStat, BOOL is_lstat) {
  BOOL bDone;
  DWORD dwAttr;
  WIN32_FILE_ATTRIBUTE_DATA fileData;
  unsigned __int64 qwSize;
  DWORD dwTag = 0;
  DEBUG_CODE(
  char szTime[100];
  char szAttrs[250]; /* Enough space for 13 attribute names, averaging 10 chars. each */
  )
  WCHAR *pwszName = NULL;
  FILE_ID fid;
  unsigned short wType;

  /* If (is_lstat is TRUE), the function name is "lstatXXX", else it's "statXXX" */
  DEBUG_ENTER(("%s(\"%s\", 0x%p);\n", (STRINGIZE(lstat) + !is_lstat), path, pStat));

#if USE_MSVC_STAT
  dwAttr = GetFileAttributes(path);
  DEBUG_PRINTF(("GetFileAttributes() = 0x%lX\n", dwAttr));
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("errno = %d; // %s\n", errno, strerror(errno)));
  }

  if (!(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
    int iErr = stat(path, pStat);
    RETURN_INT_COMMENT(iErr, ("errno = %d; // %s\n", errno, strerror(errno)));
  }
#endif

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(CP_UTF8, path);
  if (!pwszName) RETURN_INT_COMMENT(-1, ("errno=%d - %s\n", errno, strerror(errno)));

  bDone = MlxGetFileAttributesAndIDW(pwszName, &fileData, &fid, is_lstat); /* Superset of GetFileAttributesEx() */
  if (!bDone) {
    free(pwszName);
    RETURN_INT_COMMENT(-1, ("MlxGetFileAttributesAndID(); // Failed\n"));
  }
  dwAttr = fileData.dwFileAttributes;
  XDEBUG_PRINTF(("dwFileAttributes = 0x%lX\n", dwAttr));
  DEBUG_CODE_IF_ON(Filetime2String(&fileData.ftLastWriteTime, szTime, sizeof(szTime)););
  XDEBUG_PRINTF(("ftLastWriteTime = %s\n", szTime));
  qwSize = ((unsigned __int64)fileData.nFileSizeHigh << 32) | fileData.nFileSizeLow;
  XDEBUG_PRINTF(("nFileSize = %I64d\n", qwSize));

  if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
    dwTag = MlxGetReparseTagW(pwszName);
    if (!dwTag) dwAttr &= ~FILE_ATTRIBUTE_REPARSE_POINT; /* Workaround for Win7 VMware VMs reading null reparse tags on their host's shared folders */
  }

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
  /* Standard attributes - Make sure that what is set here is consistent with what readdir() sets in dirent.c */
  pStat->st_mode = 0;
  /* File type */
  wType = MlxAttrAndTag2Type(pwszName, NULL, dwAttr, dwTag);
  pStat->st_mode |= wType << 12; /* Set the 4-bit type field */
  /* File mode bits */
  pStat->st_mode |= _S_IREAD | _S_IWRITE | _S_IEXEC; /* Assume it's fully accessible */
  if (dwAttr & FILE_ATTRIBUTE_READONLY) pStat->st_mode &= ~_S_IWRITE;
  /* TO DO: Correct the read & exec mode bits */
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
  if ((dwTag & IO_REPARSE_TAG_TYPE_BITS) == IO_REPARSE_TAG_MOUNT_POINT) {
    pStat->st_mode |= S_MOUNT_POINT; /* Will allow to distinguish junctions from symlinkds and mount points */
  }
  /* if (dwAttr & FILE_ATTRIBUTE_TEMPORARY) pStat->st_mode |= S_TEMPORARY; */
  /* if (dwAttr & FILE_ATTRIBUTE_VIRTUAL) pStat->st_mode |= S_VIRTUAL; */
#if _MSVCLIBX_STAT_DEFINED
  pStat->st_Win32Attrs = dwAttr;
  pStat->st_ReparseTag = dwTag;
#endif

  /* Finally set st_dev & st_ino fields from the volume and file IDs */
  pStat->st_dev = *(dev_t *)(&fid.dwIDVol0);
  pStat->st_ino = *(ino_t *)(&fid.dwIDFil0);
  if (*(ino_t *)(&fid.dwIDFil2)) *(ino_t *)(&fid.dwIDFil0) = 0; /* This is an ReFS ID that does not fit on 64 bits */

  free(pwszName);

  DEBUG_CODE({
    extern char *szLibcFileTypes[16];
    strcpy(szAttrs, szLibcFileTypes[pStat->st_mode >> 12]);
    if (pStat->st_mode & S_MOUNT_POINT) strcat(szAttrs, " MOUNT_POINT");
    if (pStat->st_mode & _S_IREAD) strcat(szAttrs, " READ");
    if (pStat->st_mode & _S_IWRITE) strcat(szAttrs, " WRITE");
    if (pStat->st_mode & _S_IEXEC) strcat(szAttrs, " EXEC");
    if (pStat->st_mode & S_HIDDEN) strcat(szAttrs, " HIDDEN");
    if (pStat->st_mode & S_ARCHIVE) strcat(szAttrs, " ARCHIVE");
    if (pStat->st_mode & S_SYSTEM) strcat(szAttrs, " SYSTEM");
    if (pStat->st_mode & S_COMPRESSED) strcat(szAttrs, " COMPRESSED");
    if (pStat->st_mode & S_ENCRYPTED) strcat(szAttrs, " ENCRYPTED");
    if (pStat->st_mode & S_NOT_CONTENT_INDEXED) strcat(szAttrs, " NOT_INDEXED");
    if (pStat->st_mode & S_OFFLINE) strcat(szAttrs, " OFFLINE");
    if (pStat->st_mode & S_SPARSE_FILE) strcat(szAttrs, " SPARSE");
  })

  RETURN_INT_COMMENT(0, ("%s  mode = 0x%04X = {%s}  size = %I64d bytes\n", szTime, pStat->st_mode, szAttrs, qwSize));
}

int lstat(const char *path, struct stat *pStat) {
  return lstatX(path, pStat, TRUE);
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
|    2023-11-23 JFL In debug mode, don't print errno if there's no error.     |
|    2026-02-10 JFL Set the st_dev and st_ino fields too.		      |
*									      *
\*---------------------------------------------------------------------------*/

#if !USE_MSVC_STAT
int stat(const char *path, struct stat *pStat) {
  return lstatX(path, pStat, FALSE);
}
#endif /* !USE_MSVC_STAT */

int dirent2stat(const _dirent *pDirent, struct stat *pStat) {
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
  /* Unique IDs */
  pStat->st_ino = pDirent->d_ino; /* Always 0 in MsvcLibX' struct dirent */
  pStat->st_dev = 0;		  /* No corresponding field in "" dirent */

  return 0;
}

#endif /* _WIN32 */
