/*****************************************************************************\
*                                                                             *
*   Filename	    fileid.c						      *
*									      *
*   Description     Get file IDs for DOS and Windows			      *
*                                                                             *
*   Notes 	     							      *
*		    							      *
*   History								      *
*    2022-01-05 JFL Created this module.				      *
*    2025-12-06 JFL Extend MlxGetFileID() as MlxGetFileAttributesAndID().     *
*    2026-01-14 JFL Extracted new routine HasGetFileInformationByHandleEx()   *
*		    out of MlxGetFileAttributesAndID().			      *
*                   Moved GetFileInformationByHandleEx() access definitions   *
*		    to sys/stat.h.					      *
*    2026-02-03 JFL Added new routine MlxAttrAndTag2Type().		      *
*    2026-02-04 JFL Merged the two versions from readdir() and lstat().	      *
*    2026-02-06 JFL Added some debugging output.			      *
*    2026-02-19 JFL Added a workaround for Win7 VMware VMs reading null	      *
*		    reparse tags on their host's shared folders.	      *
*                   							      *
*         © Copyright 2022 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>	/* Public definitions for this module */
/* MsvcLibX library extensions */
#include <unistd.h>
#include "msvclibx.h"
#include "debugm.h"

#ifdef _WIN32

#include <windows.h>
#include "reparsept.h" /* For the undocumented IO_REPARSE_TAG_LX_SYMLINK, etc */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    HasGetFileInformationByHandleEx			      |
|									      |
|   Description     Init access to GetFileInformationByHandleEx, if available |
|									      |
|   Parameters      None						      |
|		    							      |
|   Returns 	    TRUE: Available; FALSE: Not available		      |
|		    							      |
|   Notes	    Allows building applications that run in older versions   |
|		    of Windows, and use GetFileInformationByHandleEx() in     |
|		    newer versions where it's available.		      |
|		    							      |
|   History								      |
|    2026-01-04 JFL Created this routine                               	      |
*									      *
\*---------------------------------------------------------------------------*/

/* The GetFileInformationByHandleEx() function is not available in Windows 95 */
/* The FileIdInfo class info is only available in Windows 8 and later */
/* https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileinformationbyhandle */
/* https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getfileinformationbyhandleex */
/* https://docs.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-file_id_info */
static int bGetFileInformationByHandleExLoaded = 0; /* TRUE if the following pointer has been initialized */
static int bHasFileIdInfo = -1; /* -1=Unknown; 0=No; 1=Yes, Windows supports GetFileInformationByHandleEx(FileIdInfo) */
/* Pointer to GetFileInformationByHandleEx() if the OS implements it */
PGETFILEINFORMATIONBYHANDLEEX pGetFileInformationByHandleEx = NULL;

BOOL HasGetFileInformationByHandleEx(void) {
  /* Check once if GetFileInformationByHandleEx() is available */
  if (!bGetFileInformationByHandleExLoaded) {
    DWORD dwVersion = GetVersion();
    int iMajor = (int)(LOBYTE(LOWORD(dwVersion)));
    int iMinor = (int)(HIBYTE(LOWORD(dwVersion)));
    int iVersion = (iMajor << 8) | iMinor;
    char *pszLib = (iMajor < 6) ? "fileextd.dll" : "kernel32.dll" ;

    pGetFileInformationByHandleEx = (PGETFILEINFORMATIONBYHANDLEEX)GetProcAddress(
      GetModuleHandle(pszLib), "GetFileInformationByHandleEx"
    );
    bHasFileIdInfo = (pGetFileInformationByHandleEx && (iVersion >= 0x602)); /* Windows 8 = version 6.2 */
    DEBUG_PRINTF(("WinVer = 0x%X; pGetFileInformationByHandleEx = %p; bHasFileIdInfo = %d;\n",
		  iVersion, pGetFileInformationByHandleEx, bHasFileIdInfo));

    bGetFileInformationByHandleExLoaded = 1;
  }
  return (pGetFileInformationByHandleEx != NULL);
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxGetFileAttributesAndID				      |
|									      |
|   Description     Get the attributes and unique ID for a file or directory  |
|									      |
|   Parameters      const char *path	The file name			      |
|		    WIN32_FILE_ATTRIBUTE_DATA *pAttr  Where to store attribs. |
|		    			Optional. See GetFileAttributesEx().  |
|		    FILE_ID *pFID	Where to store the ID. Optional.      |
|		    			See GetFileInformationByHandle().     |
|		    BOOL bLink		1 = Open links; 0 = Open their targets|
|		    							      |
|   Returns	    BOOL bDone, and errno set if not done		      |
|		    							      |
|   Notes	    The ID is similar in concept to the Unix inode number:    |
|		    A number uniquely identifying a file or directory.	      |
|		    							      |
|		    To get a file ID at the Windows cmd prompt, run:	      |
|		    fsutil file queryfileid <filename>			      |
|		    							      |
|		    WIN32 APIs comparison:				      |
|		    GetFileInformationByHandle() GetFileInformationByHandleEx |
|		    				   (FileIdInfo)		      |
|		    In Windows 95 & later	 In Windows 8 & later	      |
|		    32-bits volume ID		 64-bits volume Serial Number |
|		    	(32-bits volume ID = Low 32 bits of Serial Number)    |
|		    64-bits file ID		 128-bits file ID	      |
|		    	(Both are identical on NTFS systems)		      |
|		    							      |
|   History								      |
|    2021-12-27 JFL Created this routine				      |
|    2025-12-06 JFL Renamed GetFileID() as MlxGetFileAttributesAndID(),	      |
|		    adding arguments, and redefining the old name as a macro. |
|		    This allows collecting WIN32_FILE_ATTRIBUTE_DATA at the   |
|		    same time as the device and file IDs, and also to select  |
|		    if we want the information for a link or its target.      |
|    2026-02-11 JFL Moved the APPEXECLINK workaround here from lstatX().      |
*									      *
\*---------------------------------------------------------------------------*/

/* #pragma warning(disable:4996)       /* Ignore the deprecated name warning */

BOOL MlxGetFileAttributesAndIDW(const WCHAR *pwszName, WIN32_FILE_ATTRIBUTE_DATA *pAttr, FILE_ID *pFID, BOOL bLink) {
  HANDLE hFile;
  BOOL bDone;
  BY_HANDLE_FILE_INFORMATION fi;
  int errno0 = errno;

  DEBUG_WENTER((L"MlxGetFileAttributesAndIDW(\"%s\", %p, %d);\n", pwszName, pFID, bLink));

  /* Initialize access to GetFileInformationByHandleEx(FileIdInfo), if it is available */
  HasGetFileInformationByHandleEx(); /* Also sets bHasFileIdInfo */

  hFile = CreateFileW(pwszName,
		      0, /* 0 = Neither read or write. Was GENERIC_READ. */
		      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		      NULL,
		      OPEN_EXISTING,
		      FILE_FLAG_BACKUP_SEMANTICS | (bLink ? FILE_FLAG_OPEN_REPARSE_POINT : 0),
		      NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    int errno1 = errno = Win32ErrorToErrno();
    if ((!bLink) && (errno == EAGAIN)) { /* This may happen with stat(AppExecLinks or WciLinks), when the target object can't be accessed */
      /* Check if it's a case we can resolve */
      DWORD dwTag = MlxGetReparseTagW(pwszName); /* Returns 0 if this isn't a reparse point */
      if ((dwTag & IO_REPARSE_TAG_TYPE_BITS) == IO_REPARSE_TAG_APPEXECLINK) {
	/* Yes, we can resolve that case: Get the AppExecLink target, and try again */
	WCHAR *pwszTarget = malloc(WIDE_PATH_MAX * sizeof(WCHAR));
	if (pwszTarget) {
	  int iRet = MlxReadAppExecLinkW(pwszName, pwszTarget, WIDE_PATH_MAX);
	  if (iRet >= 0) {
	    errno = errno0;
	    bDone = MlxGetFileAttributesAndIDW(pwszTarget, pAttr, pFID, bLink);
	  } else {
	    bDone = FALSE;
	  }
	  free(pwszTarget);
	  if (bDone) RETURN_BOOL_COMMENT(bDone, ("Pass it on\n"));
	  errno1 = errno; /* Ex: File not found */
      	}
      }
    }
    errno = errno1;
    DEBUG_WLEAVE((L"return FALSE; // Failed to open \"%s\"\n", pwszName));
    return 0;
  }

  XDEBUG_PRINTF(("GetFileInformationByHandle(%p, %p);\n", hFile, &fi));
  bDone = GetFileInformationByHandle(hFile, &fi);
  if (!bDone) {
    errno = Win32ErrorToErrno();
    CloseHandle(hFile);
    RETURN_BOOL_COMMENT(0, ("GetFileInformationByHandle() failed.\n", GetLastError()));
  }

  if (pAttr) {  
    pAttr->dwFileAttributes = fi.dwFileAttributes;
    pAttr->ftCreationTime   = fi.ftCreationTime;
    pAttr->ftLastAccessTime = fi.ftLastAccessTime;
    pAttr->ftLastWriteTime  = fi.ftLastWriteTime;
    pAttr->nFileSizeHigh    = fi.nFileSizeHigh;
    pAttr->nFileSizeLow     = fi.nFileSizeLow;
  }

  if (pFID) {
    if (bHasFileIdInfo) { /* If we have extended IDs (In Windows 8 and later) */
      XDEBUG_PRINTF(("GetFileInformationByHandleEx(%p, FileIdInfo, %p, %d);\n", hFile, pFID, sizeof(FILE_ID)));
      bDone = pGetFileInformationByHandleEx(hFile, FileIdInfo, pFID, sizeof(FILE_ID));
      if (bDone) goto cleanup_and_return_done;
      DEBUG_PRINTF(("  // GetFileInformationByHandleEx() failed. Win32 error 0x%lX. Using the base ID infos.\n", GetLastError()));
    } /* Else we only have short IDs (Up to Windows 7) */
    pFID->dwIDVol0 = fi.dwVolumeSerialNumber;
    pFID->dwIDVol1 = 0;
    pFID->dwIDFil0 = fi.nFileIndexLow;
    pFID->dwIDFil1 = fi.nFileIndexHigh;
    pFID->dwIDFil2 = 0;
    pFID->dwIDFil3 = 0;
  }

cleanup_and_return_done:
  CloseHandle(hFile);
  RETURN_BOOL_COMMENT(1, ("Volume ID 0x%08lX, File ID 0x%lX%07lX, Attributes 0x%04lX\n",
			  pFID->dwIDVol0, pFID->dwIDFil1, pFID->dwIDFil0, pAttr->dwFileAttributes));
}

BOOL MlxGetFileAttributesAndID(const char *pszName, WIN32_FILE_ATTRIBUTE_DATA *pAttr, FILE_ID *pFID, BOOL bLink) {
  WCHAR *pwszName;
  BOOL bDone;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(CP_UTF8, pszName);
  if (!pwszName) { /* errno set already */
    RETURN_BOOL_COMMENT(0, ("Not enough memory\n"));
  }

  bDone = MlxGetFileAttributesAndIDW(pwszName, pAttr, pFID, bLink);

  free(pwszName);

  return bDone;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxAttrAndTag2Type					      |
|									      |
|   Description     Find the file type, based on its WIN32 Attribs & Rep. Tag |
|									      |
|   Parameters      WCHAR *pwszDir	The file path or pathname	      |
|		    WCHAR *pwszName	Optional file name		      |
|		    DWORD dwAttr	The file attributes		      |
|		    DWORD dwTag		Optional reparse point tag	      |
|		    			Read if not provided.		      |
|		    							      |
|   Returns	    The file dirent type = Top 4 bits of the stat st_mode     |
|		    							      |
|   Notes	    This is a shared subroutine of lstat() and readdir(),     |
|		    ensuring that both set consistent file types.  	      |
|		    							      |
|		    It's located in this module, as lstat() also relies on    |
|		    MlxGetFileAttributesAndID(), and most programs that use   |
|		    readdir() will also call stat() or lstat().		      |
|		    							      |
|   History								      |
|    2026-02-04 JFL Created this routine, by merging the two versions in      |
|		    lstat() and readdir().				      |
|    2026-02-11 JFL Made szLibcFileTypes[] public, for debug use in lstat.c.  |
*									      *
\*---------------------------------------------------------------------------*/

#include <dirent.h>
#include <unistd.h>

#if _DEBUG
char *szLibcFileTypes[16] = {
  "UNKNOWN",	/*  0  */
  "FIFO",	/*  1  // Fifo (not used in DOS/Windows dirs) */
  "CHR",	/*  2  // Character device (not used in DOS/Windows) */
  "#3?",	/*  3  // Undefined */
  "DIR",	/*  4  // Directory */
  "#5?",	/*  5  // Undefined */
  "BLK",	/*  6  // Block device (not used in DOS/Windows) */
  "#7?",	/*  7  // Undefined */
  "REG",	/*  8  // Normal file */
  "#9?",	/*  9  // Undefined */
  "LNK",	/* 10  // Symbolic link */
  "#11?",	/* 11  // Undefined */
  "SOCK",	/* 12  // Socket (not used in DOS/Windows dirs) */
  "#13?",	/* 13  // Undefined */
  "#14?",	/* 14  // Undefined */
  "VOLID"	/* 15  // Volume ID (non-standard extension for MS-DOS FAT) */
};
#endif /* _DEBUG */

unsigned char MlxAttrAndTag2Type(WCHAR *pwszDir, WCHAR *pwszName, DWORD dwAttr, DWORD dwTag) {
  unsigned char bType = DT_UNKNOWN;
  WCHAR *pwszPathname = NULL;
  WCHAR *pwszBuf = NULL;

  DEBUG_WENTER((L"MlxAttrAndTag2Type(\"%s\", \"%s\", 0x%04X, 0x%04X);\n", pwszDir, pwszName, dwAttr, dwTag));

  if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
    if (pwszDir && pwszName) {
      pwszPathname = ConcatPathW(pwszDir, pwszName, NULL, 0);
      if (!pwszPathname) {
	DEBUG_LEAVE(("return 0; // ConcatPathW() failed\n"));
	goto cleanup_and_exit;
      }
      pwszName = pwszPathname;
    } else { /* Only one of the two is defined */
      pwszName = pwszDir ? pwszDir : pwszName;
    }
    /* JUNCTIONs and SYMLINKDs both have the FILE_ATTRIBUTE_DIRECTORY flag also set.
    // Test the FILE_ATTRIBUTE_REPARSE_POINT flag first, to make sure they're seen as symbolic links.
    //
    // All symlinks are reparse points, but not all reparse points are symlinks. */
    if (!dwTag) dwTag = MlxGetReparseTagW(pwszName); /* Make the tag optional */
    if (!dwTag) {
      if (errno == ENXIO ) { /* Workaround for Win7 VMware VMs reading null reparse tags on their host's shared folders */
      	dwAttr &= ~FILE_ATTRIBUTE_REPARSE_POINT;
      	goto skip_reparse_point_cases;
      }
      DEBUG_LEAVE(("return 0; // MlxGetReparseTagW() failed\n"));
      goto cleanup_and_exit;
    }
    switch (dwTag & IO_REPARSE_TAG_TYPE_BITS) {
      case IO_REPARSE_TAG_MOUNT_POINT: { /* NTFS junction or mount point */
	ssize_t n;
	pwszBuf = malloc(sizeof(WCHAR) * WIDE_PATH_MAX);
	if (!pwszBuf) {
	  DEBUG_LEAVE(("return 0; // malloc() failed\n"));
	  goto cleanup_and_exit;
	}
	/* We must read the link to distinguish junctions from mount points. */
	n = readlinkW(pwszName, pwszBuf, WIDE_PATH_MAX);
	/* Junction targets are absolute pathnames, starting with a drive letter. Ex: C: */
	/* readlink() fails if the reparse point does not target a valid WIN32 pathname */
	/* Special case: readlink() fails with EXDEV if a network junction target is on an inaccessible drive. Yet it's a valid junction */
	if ((n < 0) && (errno != EXDEV)) break; /* This is not a junction. */
	/* This is a junction. Fall through to the symlink case. */
      }
      case IO_REPARSE_TAG_SYMLINK:		/* NTFS symbolic link */
      case IO_REPARSE_TAG_APPEXECLINK:		/* UWP application execution link */
      	/* Only list here the link types for which we know how to locate the
      	   target in Windows file system. */
	bType = DT_LNK;				/* Symbolic link */
	break;
      case IO_REPARSE_TAG_AF_UNIX:		/* Linux Sub-System Socket */
	bType = DT_SOCK;
	break;
      case IO_REPARSE_TAG_LX_FIFO:		/* Linux Sub-System FIFO */
	bType = DT_FIFO;
	break;
      case IO_REPARSE_TAG_LX_CHR:		/* Linux Sub-System Character Device */
	bType = DT_CHR;
	break;
      case IO_REPARSE_TAG_LX_BLK:		/* Linux Sub-System Block Device */
	bType = DT_BLK;
	break;
      case IO_REPARSE_TAG_NFS:			/* NFS symbolic link */
      case IO_REPARSE_TAG_LX_SYMLINK:		/* LinuX subsystem symlink */
      	/* These are links in principle, but we don't know how to find their`
      	   targets in the Windows file system. So we have to treat them as
      	   normal files for now. Fall through to the default case. */
      default: /* Unknown reparse point type. Treat it as a normal file below */
	/* bType = DT_UNKNOWN;	// We don't know what this reparse point is */
	break;
    }
  }
skip_reparse_point_cases:
  if (bType == DT_UNKNOWN) {
    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
      bType = DT_DIR;		/* Subdirectory */
    else if (dwAttr & FILE_ATTRIBUTE_DEVICE)
      bType = DT_CHR;		/* Device (we don't know if character or block) */
    else
      bType = DT_REG;		/* A normal file by default */
  }

  DEBUG_LEAVE(("return %d; // %s\n", (int)bType, szLibcFileTypes[bType]));
cleanup_and_exit:
  free(pwszPathname);
  free(pwszBuf);
  return bType;
}

#endif /* defined(_WIN32) */

