/*****************************************************************************\
*                                                                             *
*   Filename	    readlink.c						      *
*									      *
*   Description:    WIN32 port of standard C library's readlink()	      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-02-03 JFL Created this module.				      *
*    2014-02-27 JFL Changed the output name encoding to UTF-8.		      *
*    2014-03-02 JFL Split the functions into a WSTR and an UTF-8 version.     *
*    2014-03-11 JFL Bug fix in junctions targets relativization.	      *
*    2014-03-13 JFL Allow reading junctions targets in first level shares.    *
*    2014-03-19 JFL Split routine ReadReparsePointW() from readlinkW().       *
*    2014-03-20 JFL Restructured Windows readlink function into Wide and      *
*		    MultiByte versions, and changed the Unicode and Ansi      *
*		    versions to macros.					      *
*    2014-07-03 JFL Added support for pathnames >= 260 characters. 	      *
*    2016-09-09 JFL Fixed a crash in debug mode, due to stack overflows.      *
*    2017-03-22 JFL Added routines TrimTailSlashesW() and ResolveTailLinks*().*
*    2017-05-31 JFL Get strerror() prototype from string.h.                   *
*    2017-06-27 JFL Decode the new reparse point types defined in reparsept.h.*
*    2018-04-24 JFL Changed PATH_MAX to WIDE_PATH_MAX for wide bufs.	      *
*    2020-12-11 JFL Added the ability to read IO_REPARSE_TAG_APPEXECLINK links.
*    2020-12-14 JFL Changed readlink to also read these APPEXEC links.        *
*    2020-12-15 JFL Added debug descriptions for all known tag types.         *
*                   Changed readlink to also read these LX_SYMLINK links.     *
*    2021-11-28 JFL Moved the junction base path heuristic to an outside      *
*                   subroutine, shared between readlink() and junction().     *
*                   Renamed MsvcLibX-specific ReadLink*() as MlxReadLink*().  *
*                   Likewise, renamed GetReparseTag*() as MlxGetReparseTag*(),*
*                   Resolve*Links*() as MlxResolve*Links*(), etc.             *
*    2025-08-03 JFL Added MlxReadWci(), etc.				      *
*		    Added MlxSetProcessPlaceholderCompatibilityMode(), etc.   *
*    2026-01-03 JFL Fixed readlinkM() when the target is an empty "" string.  *
*    2026-01-27 JFL Make sure MlxGetReparseTag() always sets errno on error.  *
*    2026-02-06 JFL Decode junction targets raw names beginning with          *
*                   '\??\Global\C:', etc, or'\??\UNC\server\share\dir', etc.  *
*    2026-02-17 JFL Added, then commented-out, a tentative fix for Win7 VMs   *
*                   reading null reparse tags on their host's shared folders. *
*    2026-02-19 JFL Return errno=ENXIO to distinguish that particular problem.*
*                   Prevent a misleading error with root directories.	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "debugm.h"

#ifdef _WIN32

#include <windows.h>
#include "iconv.h"
#define USE_TAG_0_FIX 0 /* 1 = Include a workaround for the (dwTag == 0) issue on Win7 VMs */
#if USE_TAG_0_FIX
#include "sys/stat.h"
#endif
#include "reparsept.h" /* For the undocumented IO_REPARSE_TAG_LX_SYMLINK, etc */

#pragma warning(disable:4201) /* Ignore the "nonstandard extension used : nameless struct/union" warning */
#include <Shlwapi.h> /* For PathFindFileName() */
#pragma comment(lib, "Shlwapi.lib")

/* Get the Reparse Point Tag for a mount point - Wide char version */
/* See http://msdn.microsoft.com/en-us/library/windows/desktop/aa365511(v=vs.85).aspx */
DWORD MlxGetReparseTagW(const WCHAR *pwszPath) {
  HANDLE hFind;
  WIN32_FIND_DATAW findFileData;
  DWORD dwTag = 0;

  XDEBUG_WPRINTF((L"MlxGetReparseTagW(\"%s\");\n", pwszPath));

  /* Prevent a misleading error in FindFirstFile() with root directories */
  if ((!wcscmp(pwszPath, L"\\")) || (pwszPath[0] && (!wcscmp(pwszPath+1, L":\\")))) {
    errno = EBADF; /* Not a reparse point */
    XDEBUG_PRINTF(("  return 0; // %s\n", strerror(errno)));
    return 0;
  }

  hFind = FindFirstFileW(pwszPath, &findFileData);
  if (hFind == INVALID_HANDLE_VALUE ) {
    XDEBUG_PRINTF(("  return 0; // FindFirstFile() failed\n"));
    errno = Win32ErrorToErrno();
    return 0;
  }
  CloseHandle(hFind);
  if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
    dwTag = findFileData.dwReserved0;
#if USE_TAG_0_FIX /* The following code works, but does not fix the (dwTag == 0) issue on Win7 VMs */
    if (!dwTag) { /* This is incoherent, but it happens in Win7 VMware VMs accessing their host's shared folders */
      FILE_ATTRIBUTE_TAG_INFO fati;
      int lFni = sizeof(FILE_ATTRIBUTE_TAG_INFO);
      HANDLE hFile;
      BOOL bDone;
      if (!HasGetFileInformationByHandleEx() || (LOBYTE(LOWORD(GetVersion())) < 6)) {
	errno = ENOSYS; /* Supported in Windows Vista and later */
	XDEBUG_PRINTF(("  return 0; // Unsupported on this system\n"));
	return 0;
      }
      hFile = CreateFileW(pwszPath,		/* lpFileName */
			  0,			/* dwDesiredAccess - 0 = Neither read or write = Only metadata access */
			  0,			/* dwShareMode - Others can do anything */
			  NULL,			/* lpSecurityAttributes */
			  OPEN_EXISTING,	/* dwCreationDisposition */
			  FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, /* dwFlagsAndAttributes - Allow opening directories too */
			  NULL			/* hTemplateFile */
			  );
      if (hFile == INVALID_HANDLE_VALUE) {
	errno = Win32ErrorToErrno();
	XDEBUG_PRINTF(("  return 0; // %s\n", strerror(errno)));
	return 0;
      }
      /* GetFileInformationByHandleEx() is supported in Windows Vista and later */
      DEBUG_PRINTF(("GetFileInformationByHandleEx(hFile, FileAttributeTagInfo, fati, buflen=%d);\n", lFni));
      bDone = pGetFileInformationByHandleEx(hFile, FileAttributeTagInfo, &fati, (DWORD)lFni);
      CloseHandle(hFile);
      if (!bDone) {
	errno = Win32ErrorToErrno();
	XDEBUG_PRINTF(("  return 0; // %s\n", strerror(errno)));
	return 0;
      }
      dwTag = fati.ReparseTag;
      XDEBUG_PRINTF(("  return (fati.ReparseTag = 0x%X);\n", dwTag));
    } else
#endif
    {
      XDEBUG_PRINTF(("  return (dwReserved0 = 0x%X);\n", dwTag));
    }
    if (!dwTag) { /* This is incoherent, but it happens in Win7 VMware VMs accessing their host's shared folders */
      /* Return a unique error code, to allow the caller to distinguish this error case */
      errno = ENXIO; /* "No such device or address" */
    }
  } else {
    XDEBUG_PRINTF(("  return 0; // Not a reparse point\n"));
    errno = EBADF; /* Not a reparse point */
  }
  return dwTag;
}

/* Get the Reparse Point Tag for a mount point - MultiByte char version */
DWORD MlxGetReparseTagM(const char *path, UINT cp) {
  WCHAR wszPath[WIDE_PATH_MAX];
  int n;
  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszPath,		/* lpWideCharStr, */
			  COUNTOF(wszPath)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("MlxGetReparseTagM(\"%s\", %d); // Conversion to Unicode failed. errno=%d - %s\n", path, cp, errno, strerror(errno)));
    return 0;
  }
  return MlxGetReparseTagW(wszPath);
}

/* Trim trailing slashes or backslashes in pathname, except for the root directory */
int TrimTailSlashesW(WCHAR *pwszPath) {
  int l;
  int lDrive = 0;
  if (pwszPath[0] && (pwszPath[1] == L':')) {
    lDrive = 2;
    pwszPath += 2; /* Skip the drive name */
  }
  l = lstrlenW(pwszPath);
  /* Testing (l > 1) avoids trimming the root directory "\" */
  while ((l > 1) && ((pwszPath[l-1] == L'\\') || (pwszPath[l-1] == L'/'))) {
    pwszPath[--l] = L'\0'; /* Trim the trailing \ or / */
  }
  return lDrive + l; /* Length of the corrected pathname */
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    readlink						      |
|									      |
|   Description:    WIN32 port of standard C library's readlink()	      |
|									      |
|   Parameters:     const TCHAR *path		The link name		      |
|		    TCHAR *buf			Buffer for the link target    |
|		    size_t bufsize		Number of TCHAR in buf        |
|		    							      |
|   Returns:	    >0 = Link target size in TCHARS, Success, -1 = Failure    |
|		    Special errno values:                                     |
|		      EBADF	Unsupported link type (lacking ENOTSUP)	      |
|		      EINVAL	Unsupported Reparse Point Type		      |
|		      ELOOP	The link loops to itself                      |
|		      ENOENT	Dangling link                                 |
|									      |
|   Notes:	    Supports NTFS link types: symlink, symlinkd, junction.    |
|		    							      |
|		    Converts junction targets to relative links if possible.  |
|		    							      |
|		    On network drives, junctions that cannot be resolved on   |
|		    the client side are returned unchanged: readlink()        |
|		    returns the link name as its own target. This allows      |
|		    resolving pathnames with such junctions successfully,     |
|		    and still accessing files behind these junctions.	      |
|		    Note that this is incompatible with Unix, which fails     |
|		    with errno = ELOOP if a link points to itself.	      |
|		    Windows-aware applications supporting this can detect     |
|		    this case by comparing linkName and targetName when       |
|		    readlink() succeeds.				      |
|		    Function MlxResolveLinks() relies on this.		      |
|		    							      |
|		    Using XDEBUG macros to debug readlink() itself,	      |
|		    and DEBUG macros to display information useful for	      |
|		    debugging applications using readlink().		      |
|		    							      |
|   History:								      |
|    2014-02-04 JFL Created this routine                                      |
|    2014-02-18 JFL Fix junctions targets on network drives.                  |
|		    Convert junction targets to relative paths, if they're    |
|		    on the same drive as the junction itself.		      |
|    2014-03-11 JFL Bug fix in junctions targets relativization: Use a case-  |
|		    insensitive path comparison.			      |
|    2014-03-13 JFL Allow reading junctions targets in \\server\Public shares.|
|    2014-03-19 JFL Split routine ReadReparsePointW() from readlinkW().       |
|		    Fail in case a junction target is on another server drive.|
|    2021-11-29 JFL Renamed ReadReparsePoint*() as MlxReadReparsePoint*().    |
*									      *
\*---------------------------------------------------------------------------*/

#pragma warning(disable:4706) /* Ignore the "assignment within conditional expression" warning */

/* Get the reparse point data, and return the tag. 0=failure */
DWORD MlxReadReparsePointW(const WCHAR *path, char *buf, size_t bufsize) {
  DWORD dwAttr;
  HANDLE hLink;
  BOOL done;
  DWORD dwRead;
  DWORD dwFlagsAndAttributes;
  DWORD dwTag;
  PREPARSE_READ_BUFFER pIoctlBuf;
  DEBUG_CODE(
  char *pType = "";
  )

  DEBUG_WENTER((L"MlxReadReparsePoint(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

  dwAttr = GetFileAttributesW(path);
  XDEBUG_PRINTF(("GetFileAttributes() = 0x%lX\n", dwAttr));
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(0, ("File does not exist\n"));
  }

  if (!(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
    errno = EINVAL;
    RETURN_INT_COMMENT(0, ("File is not a reparse point\n"));
  }

  dwFlagsAndAttributes = FILE_FLAG_OPEN_REPARSE_POINT;
  if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) dwFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
  hLink = CreateFileW(path,					/* lpFileName, */
		      0,					/* dwDesiredAccess, */
		      FILE_SHARE_READ | FILE_SHARE_WRITE,	/* dwShareMode, */
		      NULL,					/* lpSecurityAttributes, */
		      OPEN_EXISTING,                    	/* dwCreationDisposition, */
		      dwFlagsAndAttributes,             	/* dwFlagsAndAttributes, */
		      NULL                              	/* hTemplateFile */
		     );
  XDEBUG_PRINTF(("CreateFile() = 0x%lX\n", hLink));
  if (hLink == INVALID_HANDLE_VALUE) {
    errno = EPERM;
    RETURN_INT_COMMENT(0, ("Cannot open the reparse point\n"));
  }

  done = DeviceIoControl(hLink,				/* hDevice, */
    			 FSCTL_GET_REPARSE_POINT,       /* dwIoControlCode, */
    			 NULL,				/* lpInBuffer, */
    			 0,				/* nInBufferSize, */
    			 buf,				/* lpOutBuffer, */
    			 (DWORD)bufsize,		/* nOutBufferSize, */
    			 &dwRead,			/* lpBytesReturned, */
    			 NULL				/* lpOverlapped */
    			);
  CloseHandle(hLink);

  if (!done) {
    errno = EPERM;
    RETURN_INT_COMMENT(0, ("DeviceIoControl() failed\n"));
  }

  XDEBUG_PRINTF(("DeviceIoControl() returned %d bytes\n", dwRead));

  /* Make sur the header tag & length fields are valid */
  if (dwRead < 8) {
    errno = EBADF;
    RETURN_INT_COMMENT(0, ("Invalid reparse data buffer\n"));
  }
  pIoctlBuf = (PREPARSE_READ_BUFFER)buf;
  dwTag = pIoctlBuf->ReparseTag;
  DEBUG_CODE_IF_ON(
    switch (dwTag & IO_REPARSE_TAG_TYPE_BITS) {
    case IO_REPARSE_TAG_RESERVED_ZERO:		/* 0x00000000 */ pType = "Reserved"; break;	
    case IO_REPARSE_TAG_RESERVED_ONE:		/* 0x00000001 */ pType = "Reserved"; break;
    case IO_REPARSE_TAG_RESERVED_TWO:		/* 0x00000002 */ pType = "Reserved"; break;
    case IO_REPARSE_TAG_MOUNT_POINT:		/* 0xA0000003 */ pType = "Mount point or junction"; break;
    case IO_REPARSE_TAG_HSM:			/* 0xC0000004 */ pType = "Hierarchical Storage Manager"; break;
    case IO_REPARSE_TAG_DRIVE_EXTENDER:		/* 0x80000005 */ pType = "Home server drive extender"; break;
    case IO_REPARSE_TAG_HSM2:			/* 0x80000006 */ pType = "Hierarchical Storage Manager Product #2"; break;
    case IO_REPARSE_TAG_SIS:			/* 0x80000007 */ pType = "Single-instance storage filter driver"; break;
    case IO_REPARSE_TAG_WIM:			/* 0x80000008 */ pType = "Windows boot Image File"; break;
    case IO_REPARSE_TAG_CSV:			/* 0x80000009 */ pType = "Cluster Shared Volume"; break;
    case IO_REPARSE_TAG_DFS:			/* 0x8000000A */ pType = "Distributed File System"; break;
    case IO_REPARSE_TAG_FILTER_MANAGER:		/* 0x8000000B */ pType = "Filter manager test harness"; break;
    case IO_REPARSE_TAG_SYMLINK:		/* 0xA000000C */ pType = "Symbolic link"; break;
    case IO_REPARSE_TAG_IIS_CACHE:		/* 0xA0000010 */ pType = "Internet Information Services cache"; break;
    case IO_REPARSE_TAG_DFSR:			/* 0x80000012 */ pType = "Distributed File System R filter"; break;
    case IO_REPARSE_TAG_DEDUP:			/* 0x80000013 */ pType = "Deduplicated file"; break;
    case IO_REPARSE_TAG_NFS:			/* 0x80000014 */ pType = "NFS symbolic link"; break;
    case IO_REPARSE_TAG_APPXSTREAM:		/* 0xC0000014 */ pType = "APPXSTREAM (Not used?)"; break;
    case IO_REPARSE_TAG_FILE_PLACEHOLDER:	/* 0x80000015 */ pType = "Placeholder for a OneDrive file"; break;
    case IO_REPARSE_TAG_DFM:			/* 0x80000016 */ pType = "Dynamic File filter"; break;
    case IO_REPARSE_TAG_WOF:			/* 0x80000017 */ pType = "Windows Overlay Filesystem compressed file"; break;
    case IO_REPARSE_TAG_WCI:			/* 0x80000018 */ pType = "Windows Container Isolation filter"; break;
    case IO_REPARSE_TAG_GLOBAL_REPARSE:		/* 0xA0000019 */ pType = "NPFS server silo named pipe symbolic link into the host silo"; break;
    case IO_REPARSE_TAG_CLOUD:			/* 0x9000001A */ pType = "Cloud Files filter"; break;
    case IO_REPARSE_TAG_APPEXECLINK:		/* 0x8000001B */ pType = "Application Execution link"; break;
    case IO_REPARSE_TAG_PROJFS:			/* 0x9000001C */ pType = "Projected File System VFS filter, ex for git"; break;
    case IO_REPARSE_TAG_LX_SYMLINK:		/* 0xA000001D */ pType = "Linux Sub-System Symbolic Link"; break;
    case IO_REPARSE_TAG_STORAGE_SYNC:		/* 0x8000001E */ pType = "Azure File Sync (AFS) filter"; break;
    case IO_REPARSE_TAG_WCI_TOMBSTONE:		/* 0xA000001F */ pType = "Windows Container Isolation filter tombstone"; break;
    case IO_REPARSE_TAG_UNHANDLED:		/* 0xA0000020 */ pType = "Unhandled Windows Container Isolation filter"; break;
    case IO_REPARSE_TAG_ONEDRIVE:		/* 0xA0000021 */ pType = "One Drive (Not used?)"; break;
    case IO_REPARSE_TAG_PROJFS_TOMBSTONE:	/* 0xA0000022 */ pType = "Projected File System VFS filter tombstone, ex for git"; break;
    case IO_REPARSE_TAG_AF_UNIX:		/* 0xA0000023 */ pType = "Linux Sub-System Socket"; break;
    case IO_REPARSE_TAG_LX_FIFO:		/* 0xA0000024 */ pType = "Linux Sub-System FIFO"; break;
    case IO_REPARSE_TAG_LX_CHR:			/* 0xA0000025 */ pType = "Linux Sub-System Character Device"; break;
    case IO_REPARSE_TAG_LX_BLK:			/* 0xA0000026 */ pType = "Linux Sub-System Block Device"; break;
    case IO_REPARSE_TAG_WCI_LINK:		/* 0xA0000027 */ pType = "Windows Container Isolation filter Link"; break;
    default:					pType = "Unknown type! Please report its value and update reparsept.h & readlink.c."; break;
    }
    XDEBUG_PRINTF(("ReparseTag = 0x%04X; // %s\n", (unsigned)(dwTag), pType));
  )
  XDEBUG_PRINTF(("ReparseDataLength = 0x%04X\n", (unsigned)(pIoctlBuf->ReparseDataLength)));

  /* Dump the whole payload in extra-debug mode */
  XDEBUG_CODE_IF_ON({
    unsigned int ul;
    unsigned int u;
    unsigned int uMax;
    DEBUG_PRINTF(("ReparseDataBuffer =\n\
Offset    00           04           08           0C           0   4    8   C   \n\
--------  -----------  -----------  -----------  -----------  -------- --------\n\
"));

    for (ul = 0; ul < (unsigned)(pIoctlBuf->ReparseDataLength); ul += 16) {
      printf("%08X ", ul);

      uMax = (unsigned)(pIoctlBuf->ReparseDataLength) - ul;
      if (uMax > 16) uMax = 16;

      /* Display the hex dump */
      for (u=0; u<16; u++) {
	if (!(u&3)) printf(" ");
	if (u < uMax) {
	  printf("%02.2X ", ((unsigned char *)pIoctlBuf->DataBuffer)[ul + u]);
	} else {
	  printf("   ");
	}
      }

      /* Display the ASCII characters dump */
      for (u=0; u<16; u++) {
      	char c = ((char *)pIoctlBuf->DataBuffer)[ul + u];
	if (!(u&7)) printf(" ");
	if (c < ' ') c = ' ';
	if ((unsigned char)c > '\x7F') c = ' ';
	printf("%c", c);
      }

      printf("\n");
    }
  })

  RETURN_DWORD_COMMENT(dwTag, ("%s\n", pType));
}

/* Get the symlink or junction target. Returns the tag, or 0 on failure */
DWORD MlxReadLinkW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  char iobuf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  DWORD dwTag;
  PMOUNTPOINT_READ_BUFFER pMountpointBuf;
  PSYMLINK_READ_BUFFER pSymlinkBuf;
  PLX_SYMLINK_READ_BUFFER pLxSymlinkBuf;
  PAPPEXECLINK_READ_BUFFER pAppExecLinkBuf;
  PWCI_READ_BUFFER pWciLinkBuf; 
  WCHAR *pwStr = NULL;
  WCHAR *pwNewStr = NULL;
  unsigned short offset = 0, len = 0;
  
  DEBUG_WENTER((L"MlxReadLink(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

  dwTag = MlxReadReparsePointW(path, iobuf, sizeof(iobuf));
  if (!dwTag) RETURN_CONST(0);

  /* Process the supported tag types */
  switch (dwTag & IO_REPARSE_TAG_TYPE_BITS) {
    case IO_REPARSE_TAG_SYMLINK:
      pSymlinkBuf = (PSYMLINK_READ_BUFFER)iobuf;
      XDEBUG_PRINTF(("SubstituteNameOffset = 0x%04X\n", (unsigned)(pSymlinkBuf->SubstituteNameOffset)));
      XDEBUG_PRINTF(("SubstituteNameLength = 0x%04X\n", (unsigned)(pSymlinkBuf->SubstituteNameLength)));
      XDEBUG_PRINTF(("PrintNameOffset = 0x%04X\n", (unsigned)(pSymlinkBuf->PrintNameOffset)));
      XDEBUG_PRINTF(("PrintNameLength = 0x%04X\n", (unsigned)(pSymlinkBuf->PrintNameLength)));
      XDEBUG_PRINTF(("Flags = 0x%04X\n", (unsigned)(pSymlinkBuf->Flags)));

      pwStr = pSymlinkBuf->PathBuffer;
      offset = pSymlinkBuf->SubstituteNameOffset / 2; /* Convert byte offset to wide characters offset */
      len = pSymlinkBuf->SubstituteNameLength / 2; /* Convert bytes to wide characters count */
      break;

    case IO_REPARSE_TAG_MOUNT_POINT: /* aka. junctions */
      pMountpointBuf = (PMOUNTPOINT_READ_BUFFER)iobuf;
      XDEBUG_PRINTF(("SubstituteNameOffset = 0x%04X\n", (unsigned)(pMountpointBuf->SubstituteNameOffset)));
      XDEBUG_PRINTF(("SubstituteNameLength = 0x%04X\n", (unsigned)(pMountpointBuf->SubstituteNameLength)));
      XDEBUG_PRINTF(("PrintNameOffset = 0x%04X\n", (unsigned)(pMountpointBuf->PrintNameOffset)));
      XDEBUG_PRINTF(("PrintNameLength = 0x%04X\n", (unsigned)(pMountpointBuf->PrintNameLength)));

      pwStr = pMountpointBuf->PathBuffer;
      offset = pMountpointBuf->SubstituteNameOffset / 2; /* Convert byte offset to wide characters offset */
      len = pMountpointBuf->SubstituteNameLength / 2; /* Convert bytes to wide characters count */
      break;

    case IO_REPARSE_TAG_LX_SYMLINK: /* LinuX SubSystem symbolic links */
      pLxSymlinkBuf = (PLX_SYMLINK_READ_BUFFER)iobuf;
      if (pLxSymlinkBuf->FileType == 2) {
      	char *pszTarget;
      	len = pLxSymlinkBuf->ReparseDataLength - sizeof(pLxSymlinkBuf->FileType); /* The UTF-8 target string length */
      	pszTarget = malloc(len + 1); /* Room for the UTF-8 string plus a NUL */
        if (!pszTarget) RETURN_INT_COMMENT(0, ("Insufficient memory\n"));
      	CopyMemory(pszTarget, (char *)(pLxSymlinkBuf->PathBuffer), len);
      	pszTarget[len] = '\0';
      	pwNewStr = MultiByteToNewWideString(CP_UTF8, pszTarget);
      	free(pszTarget);
        if (!pwNewStr) RETURN_INT_COMMENT(0, ("Insufficient memory\n"));
        pwStr = pwNewStr;
      } else {
        errno = EBADF;
	RETURN_INT_COMMENT(0, ("Unsupported LXSS Symlink type = %d\n", (int)(pLxSymlinkBuf->FileType)));
      }
      break;
    
    case IO_REPARSE_TAG_APPEXECLINK: /* Ex: Empty *.exe in %LOCALAPPDATA%\Microsoft\WindowsApps */
      pAppExecLinkBuf = (PAPPEXECLINK_READ_BUFFER)iobuf;
      XDEBUG_PRINTF(("Version = 0x%04X\n", (unsigned)(pAppExecLinkBuf->Version)));
      XDEBUG_CODE_IF_ON({
      	WCHAR *pwStr0 = pwStr = pAppExecLinkBuf->StringList;
      	while((pwStr-pwStr0) < pAppExecLinkBuf->ReparseDataLength) {
      	  wprintf(L"%s\n", pwStr);
      	  pwStr += lstrlenW(pwStr) + 1;
      	}
      })
      
      if (pAppExecLinkBuf->Version == 3) {
      	unsigned short u;
	for (u=0, pwStr = pAppExecLinkBuf->StringList; u<2; u++) pwStr += lstrlenW(pwStr) + 1;
	len = (unsigned short)lstrlenW(pwStr);
      } else {
        errno = EBADF;
	RETURN_INT_COMMENT(0, ("Unsupported AppExecLink Version = %d\n", (int)(pAppExecLinkBuf->Version)));
      }
      break;

    case IO_REPARSE_TAG_CLOUD: /* Cloud File, ex. OneDrive */
      /* TO DO: Reverse-engineer and decode the data structure */
#pragma warning(disable:4428) /* Ignore Win95's "universal-character-name encountered in source" warning */
      pwStr = L"\u2601 "; /* Unicode cloud character, followed by a space */
#pragma warning(default:4428) /* Ignore Win95's "universal-character-name encountered in source" warning */
      offset = 0;
      len = 2;
      break;

    case IO_REPARSE_TAG_WCI: /* Windows Container Isolation */
      pWciLinkBuf = (PWCI_READ_BUFFER)iobuf;
      XDEBUG_PRINTF(("Version = 0x%04X\n", (unsigned)(pWciLinkBuf->Version)));

      pwStr = pWciLinkBuf->WciName;
      offset = 0;
      len = pWciLinkBuf->WciNameLength / 2; /* Convert bytes to wide characters count */
      break;

    default:
      errno = EINVAL;
      RETURN_INT_COMMENT(0, ("Unsupported reparse point type 0x%X\n", dwTag));
  }
  if (len) {
    if (len >= bufsize) {
      errno = ENAMETOOLONG;
      RETURN_INT_COMMENT(0, ("The output buffer is too small. The link size is %d bytes.\n", len));
    }
    CopyMemory(buf, pwStr+offset, len*sizeof(WCHAR));
  }
  buf[len] = L'\0';

  if (pwNewStr) free(pwNewStr);

  DEBUG_WLEAVE((L"return 0x%X; // \"%s\"\n", dwTag, buf));
  return dwTag;
}

/* Small UTF8 front end of the previous one */
DWORD MlxReadLinkU(const char *path, char *buf, size_t bufsize) {
  WCHAR wbuf[WIDE_PATH_MAX];
  WCHAR *wPath = NewWideCopy(path);
  DWORD dwTag = 0;
  if (wPath) {
    dwTag = MlxReadLinkW(wPath, wbuf, sizeof(wbuf));
    if (dwTag) {
      int n = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, (int)bufsize, NULL, NULL);
      if (!n) {
	errno = Win32ErrorToErrno();
      	dwTag = 0;
      }
    }
    free(wPath);
  }
  return dwTag;
}

/* Posix routine readlink - Wide char version. Returns the link size, or -1 on failure */
ssize_t readlinkW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  ssize_t nRead;
  UINT drvType;
  DWORD dwTag;

  DEBUG_WENTER((L"readlink(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

  /* TO DO: Fix readlinkW (And thus MlxReadReparsePointW) to return truncated links if the buffer is too small.
            Returning an ENAMETOOLONG or ENOMEM error as we do now is sane, but NOT standard */
  dwTag = MlxReadLinkW(path, buf, bufsize);
  if (!dwTag) {
    RETURN_INT_COMMENT(-1, ("MlxReadLink() failed.\n"));
  }

  /* Special case for junctions to other local directories: Remove their '\??\' header.
  // Note: Also seen once on a symlink. I don't know why in most cases symlinks don't have it.
  // Note: These junctions begin with '\??\C:\' (or another drive letter).
  // Other types of junctions/mount points do not continue with a drive letter.
  // For example: '\??\Volume{5e58015c-ba64-4048-928d-06aa03c983f9}\' */
  nRead = lstrlenW(buf);
  if ((nRead >= 7) && (!strncmpW(buf, L"\\??\\", 4))) {
    WCHAR *pwszPath = buf + 4;
    nRead -= 4;
    /* Some junction targets begin with '\??\Global\C:\...'
       Apparently it's even possible to have '\??\Global\Global\C:\...' */
    while ((!strncmpW(pwszPath, L"Global\\", 7))) {
      XDEBUG_PRINTF(("Removing \"Global\\\"\n"));
      pwszPath += 7;
      nRead -= 7;
    }
    if ((nRead > 2) && (!strncmpW(pwszPath+1, L":\\", 2))) {
      XDEBUG_WPRINTF((L"buf = \"%s\"; // Removed the \"%.*s\" prefix.\n", pwszPath, (int)(pwszPath - buf), buf));
      CopyMemory(buf, pwszPath, (nRead+1)*sizeof(WCHAR));
    } else if (!strncmpW(pwszPath, L"UNC\\", 4)) { /* This is a network share */
      /* Ex: '\??\UNC\server\share\dir' or '\??\Global\UNC\server\share\dir' */
      pwszPath += 3; /* Skip the 'UNC', but keep the '\' */
      nRead -= 3;
      XDEBUG_WPRINTF((L"buf = \"\\%s\"; // Removed the \"%.*s\" prefix.\n", pwszPath, (int)(pwszPath - buf), buf));
      CopyMemory(buf+1, pwszPath, (nRead+1)*sizeof(WCHAR)); /* Keep the initial '\', to generate '\\server\share\dir' */
      nRead += 1; /* For the initial \ */
    } else { /* Return an error for other types (Ex: Volumes), as Posix SW cannot handle them successfully. */
      errno = EINVAL;
      DEBUG_WLEAVE((L"return -1; // Unsupported mount point type: %s\n", buf+4));
      return -1;
    }
  }

  /* Fix junctions targets */
  /* Windows resolves junctions on the server side,
     but symlinks and symlinkds on the client side. */
  if (dwTag == IO_REPARSE_TAG_MOUNT_POINT) {
    char szRootDir[4] = "C:\\";
    WCHAR wszAbsPath[WIDE_PATH_MAX];
    WCHAR wszAbsPath2[WIDE_PATH_MAX];
    WCHAR *p1;
    WCHAR *p2;
    WCHAR *pc1 = L"A";
    WCHAR *pc2 = L"a";

    XDEBUG_WPRINTF((L"rawJunctionTarget = \"%s\"\n", buf));

    GetFullPathNameW(path, WIDE_PATH_MAX, wszAbsPath, NULL); /* Get the drive letter in the full path */
    szRootDir[0] = (char)(wszAbsPath[0]); /* Copy the drive letter */
    drvType = GetDriveType(szRootDir);
    XDEBUG_PRINTF(("GetDriveType(\"%s\") = %d // %s drive\n", szRootDir, drvType, (drvType == DRIVE_REMOTE) ? "Network" : "Local"));

    /* 1) On network drives, the target should reference the network drive itself,
          not a local drive on the remote machine */
    if (drvType == DRIVE_REMOTE) {
      /* Then check if the junction target is relative to the same network drive. (Not always true!) */
      int iTargetFound = FALSE;
      if (buf[0] && (buf[1] == L':')) {
	WCHAR wszLocalName[] = L"X:";
	WCHAR wszRemoteName[WIDE_PATH_MAX];
	DWORD dwErr;
	DWORD dwLength = WIDE_PATH_MAX;
	wszLocalName[0] = wszAbsPath[0];
	dwErr = WNetGetConnectionW(wszLocalName, wszRemoteName, &dwLength);
	if (dwErr == NO_ERROR) {
	  XDEBUG_WPRINTF((L"net use %c: %s\n", (char)(wszLocalName[0]), wszRemoteName));
	  if ((wszRemoteName[0] == L'\\') && (wszRemoteName[1] == L'\\')) {
	    WCHAR *pwszShareBasePath = MlxGetShareBasePathW(wszRemoteName);
	    if (pwszShareBasePath) {
	      int l = lstrlenW(pwszShareBasePath);
	      if (!_wcsnicmp(pwszShareBasePath, buf, l) && ((l == 3) || (buf[l] == L'\\') || !buf[l])) {
	      	/* Yes, it points at the same share */
	      	/* Replace the remote base dir by the local share drive root */
		buf[0] = szRootDir[0];
	      	if (l > 3) { /* If the remote base dir is not the root dir */
		  WCHAR *pwsz2 = buf + l;
		  MoveMemory(buf+2, pwsz2, (lstrlenW(pwsz2)+1)*sizeof(WCHAR));
		}
		XDEBUG_WPRINTF((L"buf = \"%s\"; // Substituted the share base path\n", buf));
		iTargetFound = TRUE;
	      }
	      free(pwszShareBasePath);
	    }
	  }
	}
      }
      if (!iTargetFound) {
	errno = EXDEV; /* Cross-device junction, with a target invalid in the context of the client */
	RETURN_INT_COMMENT(-1, ("Inaccessible junction target, on another server drive, or outside of the shared folder.\n"));
      }
    }

    /* 2) Convert absolute junction targets to relative links, if possible.
          This is useful because junctions are often used as substitutes
          for symlinkds. But Windows always records absolute target paths,
          even when relative paths were used for creating them. */
    TrimTailSlashesW(wszAbsPath);
    GetFullPathNameW(buf, WIDE_PATH_MAX, wszAbsPath2, NULL);
    XDEBUG_WPRINTF((L"szAbsPath = \"%s\"\n", wszAbsPath));
    XDEBUG_WPRINTF((L"szAbsPath2 = \"%s\"\n", wszAbsPath2));
    /* Find the first (case insensitive) difference */
    for (p1=wszAbsPath, p2=wszAbsPath2; (*pc1 = *p1) && (*pc2 = *p2); p1++, p2++) {
      CharLowerW(pc1);
      CharLowerW(pc2);
      if (*pc1 != *pc2) break;
    }
    if (p1 != wszAbsPath) { /* Both are on the same drive. Can be made relative. */
      WCHAR *pc;
      /* Backtrack to the last \ */
      for ( ; *(p1-1) != L'\\'; p1--, p2--) ;
      XDEBUG_WPRINTF((L"szRelPath1 = \"%s\"\n", p1));
      XDEBUG_WPRINTF((L"szRelPath2 = \"%s\"\n", p2));
      buf[0] = '\0';
      /* Count the # of parent directories that remain in path 1 */
      for (pc=p1; *pc; pc++) if (*pc == L'\\') lstrcatW(buf, L"..\\");
      /* Append what remains in path 2 */
      lstrcatW(buf, p2);
      if (!buf[0]) lstrcpyW(buf, L"."); /* If buf is empty, it means the target is the directory of p1 itself */
      /* That's the relative link */
      nRead = lstrlenW(buf);
    } /* Else the drives differ. Paths cannot be relative. Don't change buf. */
  }

  DEBUG_WLEAVE((L"return %d; // \"%s\"\n", (int)nRead, buf));
  return (int)nRead;
}

#pragma warning(default:4706)

/* Posix routine readlink - MultiByte char version. Returns the link size, or -1 on failure */
ssize_t readlinkM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR wszPath[WIDE_PATH_MAX];
  WCHAR wszTarget[WIDE_PATH_MAX];
  int n;
  ssize_t nResult;
  char *pszDefaultChar;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszPath,		/* lpWideCharStr, */
			  COUNTOF(wszPath)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("readlinkM(\"%s\", ...); // Conversion to Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return -1;
  }

  nResult = readlinkW(wszPath, wszTarget, WIDE_PATH_MAX);
  if (nResult == 0) buf[0] = '\0'; /* This can happen for WCI links, see https://github.com/JFLarvoire/SysToolsLib/issues/45 */
  if (nResult <= 0) return nResult;

  pszDefaultChar = (cp == CP_UTF8) ? NULL : "?";
  n = WideCharToMultiByte(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  wszTarget,		/* lpWideCharStr, */
			  (int)nResult + 1,	/* cchWideChar, */
			  buf,			/* lpMultiByteStr, */
			  (int)bufsize,		/* cbMultiByte, */
			  pszDefaultChar,	/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("readlinkM(\"%s\", ...); // Conversion back from Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return -1;
  }

  return n;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxResolveTailLinks					      |
|									      |
|   Description	    Resolve links in node names	(Ignore those in dir names)   |
|									      |
|   Parameters      const char *path	    The symlink name		      |
|		    char *buf		    Output buffer		      |
|		    size_t bufsize	    Output buffer size in characters  |
|									      |
|   Returns	    0 = Success, -1 = Failure and set errno		      |
|		    Special errno values:                                     |
|		      EBADF	Unsupported link type, ex: Linux symlink      |
|		      EINVAL	Unsupported Reparse Point Type		      |
|		      ELOOP	The link loops to itself                      |
|		      ENOENT	Dangling link                                 |
|		    							      |
|   Notes	    							      |
|		    							      |
|   History								      |
|    2017-03-22 JFL Created this routine                               	      |
|    2021-12-22 JFL Detect link loops                                  	      |
|    2022-01-12 JFL Fixed bug in last change in case: bin -> ..\bin -> ..\bin |
*									      *
\*---------------------------------------------------------------------------*/

/* Linked list of previous pathnames */
typedef struct _NAMELIST {
  struct _NAMELIST *prev;
  const WCHAR *path;
} NAMELIST;

int MlxResolveTailLinksW1(const WCHAR *path, WCHAR *buf, size_t bufsize, NAMELIST *prev, int iDepth) {
  DWORD dwAttr;
  size_t l;
  NAMELIST list;
  NAMELIST *pList;

  DEBUG_WENTER((L"MlxResolveTailLinks(\"%s\", %p, %ul);\n", path, buf, (unsigned long)bufsize));

  dwAttr = GetFileAttributesW(path);
  XDEBUG_PRINTF(("GetFileAttributes() = 0x%lX\n", dwAttr));
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("File does not exist\n"));
  }

  if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
    WCHAR wszBuf2[WIDE_PATH_MAX];
    WCHAR wszBuf3[WIDE_PATH_MAX];
    WCHAR *pwsz = wszBuf2;
    int iCDSize;
    int iRet;
    ssize_t nLinkSize = readlinkW(path, wszBuf2, WIDE_PATH_MAX); /* Corrects junction drive letters, etc */
    if (nLinkSize < 0) RETURN_INT(-1);
    if (wszBuf2[0] == L'/') { /* Most likely a Linux absolute symlink */
      errno = EBADF; /* We can't resolve such absolute links */
      RETURN_INT_COMMENT(-1, ("Can't resolve Linux absolute symlink"));
    }
    if (!(   (wszBuf2[0] == L'\\')
          || (wszBuf2[0] && (wszBuf2[1] == L':')))) { /* This is a relative path. We must compose it with the link dirname */
      lstrcpynW(wszBuf3, path, WIDE_PATH_MAX); /* May truncate the output string */
      wszBuf3[WIDE_PATH_MAX-1] = L'\0'; /* Make sure the string is NUL-terminated */
      TrimTailSlashesW(wszBuf3);
      iCDSize = lstrlenW(wszBuf3);

      lstrcpynW(wszBuf3+iCDSize, L"\\..\\", WIDE_PATH_MAX-iCDSize);	/* Remove the link name */
      wszBuf3[WIDE_PATH_MAX-1] = L'\0'; /* Make sure the string is NUL-terminated */
      iCDSize += lstrlenW(wszBuf3+iCDSize);

      lstrcpynW(wszBuf3+iCDSize, wszBuf2, WIDE_PATH_MAX-iCDSize);	/* Append the relative link target */
      wszBuf3[WIDE_PATH_MAX-1] = L'\0'; /* Make sure the string is NUL-terminated */
      iCDSize += lstrlenW(wszBuf3+iCDSize);

      CompactPathW(wszBuf3, wszBuf3, WIDE_PATH_MAX); /* Remove all useless . and .. */
      pwsz = wszBuf3;
    } else { /* This is an absolute path */
      if (!lstrcmpW(path, wszBuf2)) goto return_target_path; /* Junction to a server's external device. See readlink() header */
    }

    /* Check for the max link chain depth */
    if (iDepth == SYMLOOP_MAX) {
      errno = ELOOP;
      DEBUG_WLEAVE((L"return -1; // Max link chain depth reached: \"%s\"\n", pwsz));
      return -1;
    }
    /* Check if we've seen this path before */
    for (pList = prev; pList; pList = pList->prev) {
      if (!lstrcmpW(pwsz, pList->path)) {
	errno = ELOOP;
	DEBUG_WLEAVE((L"return -1; // Loop found: \"%s\"\n", pwsz));
	return -1;
      }
    }
    list.path = pwsz;
    list.prev = prev;
    iRet = MlxResolveTailLinksW1(pwsz, buf, bufsize, &list, iDepth+1);
    DEBUG_WLEAVE((L"return %d; // \"%s\"\n", iRet, buf));
    return iRet;
  }

return_target_path:
  l = lstrlenW(path);
  if (l >= bufsize) {
    errno = ENAMETOOLONG;
    RETURN_INT_COMMENT(-1, ("Buffer too small\n"));
  }
  lstrcpyW(buf, path);
  DEBUG_WLEAVE((L"return 0; // \"%s\"\n", buf));
  return 0;
}

int MlxResolveTailLinksW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  NAMELIST root;
  root.path = path;
  root.prev = NULL;
  return MlxResolveTailLinksW1(path, buf, bufsize, &root, 0);
}

int MlxResolveTailLinksM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR wszPath[WIDE_PATH_MAX];
  WCHAR wszTarget[WIDE_PATH_MAX];
  int n;
  int iErr;
  char *pszDefaultChar;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszPath,		/* lpWideCharStr, */
			  COUNTOF(wszPath)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("MlxResolveTailLinksM(\"%s\", ...); // Conversion to Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return -1;
  }

  iErr = MlxResolveTailLinksW(wszPath, wszTarget, WIDE_PATH_MAX);
  if (iErr < 0) return iErr;

  pszDefaultChar = (cp == CP_UTF8) ? NULL : "?";
  n = WideCharToMultiByte(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  wszTarget,		/* lpWideCharStr, */
			  -1,			/* cchWideChar, */
			  buf,			/* lpMultiByteStr, */
			  (int)bufsize,		/* cbMultiByte, */
			  pszDefaultChar,	/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("MlxResolveTailLinksM(\"%s\", ...); // Conversion back from Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return -1;
  }

  return iErr;
}

int MlxResolveTailLinksA(const char *path, char *buf, size_t bufsize) {
  return MlxResolveTailLinksM(path, buf, bufsize, CP_ACP);
}

int MlxResolveTailLinksU(const char *path, char *buf, size_t bufsize) {
  return MlxResolveTailLinksM(path, buf, bufsize, CP_UTF8);
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxReadAppExecLink					      |
|									      |
|   Description	    Get the AppExecLink target, and return its size           |
|									      |
|   Parameters      const char *path	    The AppExecLink name              |
|		    char *buf		    Output buffer		      |
|		    size_t bufsize	    Output buffer size in characters  |
|									      |
|   Returns	    >0 = Success, 0 = Failure and set errno		      |
|		    							      |
|   Notes	    							      |
|									      |
|   History								      |
|    2020-12-11 JFL Created this routine                               	      |
|    2025-08-03 JFL Simplified to match improvements in MlxReadWci().  	      |
*									      *
\*---------------------------------------------------------------------------*/

/* Get the AppExecLink target, and return its size. 0=failure */
int MlxReadAppExecLinkW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  char *iobuf;
  DWORD dwTag;
  PAPPEXECLINK_READ_BUFFER pAppExecLinkBuf;
  WCHAR *pwStr = NULL;
  unsigned short n, offset = 0, len = 0;

  DEBUG_WENTER((L"MlxReadAppExecLink(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

  iobuf = malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
  if (!iobuf) RETURN_CONST_COMMENT(0, ("Out of memory\n"));

  dwTag = MlxReadReparsePointW(path, iobuf, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
  if (!dwTag) {
    free(iobuf);
    RETURN_CONST_COMMENT(0, ("This is not a reparse point\n"));
  }

  if (dwTag != IO_REPARSE_TAG_APPEXECLINK) {
    free(iobuf);
    errno = EINVAL;
    RETURN_CONST_COMMENT(0, ("This is not an AppExecLink reparse point\n"));
  }

  /* This is an AppExecLink. Ex: Empty *.exe in %LOCALAPPDATA%\Microsoft\WindowsApps */
  pAppExecLinkBuf = (PAPPEXECLINK_READ_BUFFER)iobuf;
  XDEBUG_PRINTF(("Version = 0x%04X\n", (unsigned)(pAppExecLinkBuf->Version)));
  if (pAppExecLinkBuf->Version != 3) {
    free(iobuf);
    RETURN_CONST_COMMENT(0, ("Unexpected AppExecLink Version = %d\n", (int)(pAppExecLinkBuf->Version)));
  }

  /* This is an AppExecLink that we partially know how to decode */
  XDEBUG_CODE_IF_ON({
    WCHAR *pwStr0 = pwStr = pAppExecLinkBuf->StringList;
    while((pwStr-pwStr0) < pAppExecLinkBuf->ReparseDataLength) {
      wprintf(L"%s\n", pwStr);
      pwStr += lstrlenW(pwStr) + 1;
    }
  })

  /* Skip the first two strings, the third one being the link we want */
  for (n=0, pwStr = pAppExecLinkBuf->StringList; n<2; n++) pwStr += lstrlenW(pwStr) + 1;
  offset = 0;
  len = (unsigned short)lstrlenW(pwStr);

  if (len) {
    if (len >= bufsize) {
      free(iobuf);
      errno = ENAMETOOLONG;
      RETURN_INT_COMMENT(0, ("The output buffer is too small. The link size is %d bytes.\n", len));
    }
    CopyMemory(buf, pwStr+offset, len*sizeof(WCHAR));
  }
  buf[len] = L'\0';

  free(iobuf);
  DEBUG_WLEAVE((L"return 0x%X; // \"%s\"\n", len, buf));
  return len;
}

/* Get the AppExecLink target, and return its size. 0 = failure */
int MlxReadAppExecLinkM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR wszPath[WIDE_PATH_MAX];
  WCHAR wszTarget[WIDE_PATH_MAX];
  int n;
  char *pszDefaultChar;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszPath,		/* lpWideCharStr, */
			  COUNTOF(wszPath)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("MlxReadAppExecLinkM(\"%s\", ...); // Conversion to Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return 0;
  }

  n = MlxReadAppExecLinkW(wszPath, wszTarget, WIDE_PATH_MAX);
  if (n <= 0) return n;

  pszDefaultChar = (cp == CP_UTF8) ? NULL : "?";
  n = WideCharToMultiByte(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  wszTarget,		/* lpWideCharStr, */
			  n + 1,		/* cchWideChar, */
			  buf,			/* lpMultiByteStr, */
			  (int)bufsize,		/* cbMultiByte, */
			  pszDefaultChar,	/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("MlxReadAppExecLinkM(\"%s\", ...); // Conversion back from Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
  }

  return n;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxReadWci						      |
|									      |
|   Description	    Get the WCI target, and return its size                   |
|									      |
|   Parameters      const char *path	    The WCI name                      |
|		    char *buf		    Output buffer		      |
|		    size_t bufsize	    Output buffer size in characters  |
|									      |
|   Returns	    >0 = Success, 0 = Failure and set errno		      |
|		    							      |
|   Notes	    							      |
|		    							      |
|   History								      |
|    2025-08-03 JFL Created this routine                               	      |
*									      *
\*---------------------------------------------------------------------------*/

/* Get the WCI target, and return its size. 0=failure */
int MlxReadWciW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  char *iobuf;
  DWORD dwTag;
  PWCI_READ_BUFFER pWciBuf;
  WCHAR *pwStr = NULL;
  unsigned short len = 0;

  DEBUG_WENTER((L"MlxReadWci(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

  iobuf = malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
  if (!iobuf) RETURN_CONST_COMMENT(0, ("Out of memory\n"));

  dwTag = MlxReadReparsePointW(path, iobuf, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
  if (!dwTag) {
    free(iobuf);
    RETURN_CONST_COMMENT(0, ("This is not a reparse point\n"));
  }

  if (dwTag != IO_REPARSE_TAG_WCI) {
    free(iobuf);
    errno = EINVAL;
    RETURN_CONST_COMMENT(0, ("This is not a WCI reparse point\n"));
  }

  /* This is a WCI place holder */
  pWciBuf = (PWCI_READ_BUFFER)iobuf;
  XDEBUG_PRINTF(("WCI Version = 0x%04X\n", (unsigned)(pWciBuf->Version)));
  if (pWciBuf->Version != 1) {
    free(iobuf);
    errno = EBADF;
    RETURN_CONST_COMMENT(0, ("Unexpected WCI Version = %d\n", (int)(pWciBuf->Version)));
  }

  /* This is a WCI place holder that we partially know how to decode */
  XDEBUG_CODE_IF_ON({	/* Dump the target GUID */
    unsigned short i;
    char *pc = (char *)&(pWciBuf->LookupGuid);
    DEBUG_PRINTF(("GUID = ")); /* Indents the string as needed */
    for (i = 0; i < 16; i++) printf("%02X", pc[i] & 0xFF);
    printf("\n");
  })
  XDEBUG_PRINTF(("Length = %u\n", (unsigned)(pWciBuf->WciNameLength)));
  XDEBUG_CODE_IF_ON({	/* Display the target path */
    unsigned short i;
    unsigned short l = pWciBuf->WciNameLength / 2;
    DEBUG_WPRINTF((L"Target = \"")); /* Indents the string as needed */
    for (i = 0; i < l; i++) wprintf(L"%c", pWciBuf->WciName[i]);
    wprintf(L"\"\n");
  })

  pwStr = pWciBuf->WciName;
  len = (unsigned short)(pWciBuf->WciNameLength) / 2;
  if (len) {
    if (len >= bufsize) {
      free(iobuf);
      errno = ENAMETOOLONG;
      RETURN_CONST_COMMENT(0, ("The output buffer is too small. The link size is %d bytes.\n", len));
    }
    CopyMemory(buf, pwStr, len*sizeof(WCHAR));
  }
  buf[len] = L'\0';

  free(iobuf);
  DEBUG_WLEAVE((L"return 0x%X; // \"%s\"\n", len, buf));
  return len;
}

/* Get the WCI target, and return its size. 0 = failure */
int MlxReadWciM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR wszPath[WIDE_PATH_MAX];
  WCHAR wszTarget[WIDE_PATH_MAX];
  int n;
  char *pszDefaultChar;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  n = MultiByteToWidePath(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    			  path,			/* lpMultiByteStr, */
			  wszPath,		/* lpWideCharStr, */
			  COUNTOF(wszPath)	/* cchWideChar, */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("MlxReadWciM(\"%s\", ...); // Conversion to Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return 0;
  }

  n = MlxReadWciW(wszPath, wszTarget, WIDE_PATH_MAX);
  if (n <= 0) return n;

  pszDefaultChar = (cp == CP_UTF8) ? NULL : "?";
  n = WideCharToMultiByte(cp,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  wszTarget,		/* lpWideCharStr, */
			  n + 1,		/* cchWideChar, */
			  buf,			/* lpMultiByteStr, */
			  (int)bufsize,		/* cbMultiByte, */
			  pszDefaultChar,	/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) {
    errno = Win32ErrorToErrno();
    DEBUG_PRINTF(("MlxReadWciM(\"%s\", ...); // Conversion back from Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
  }

  return n;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    MlxSetProcessPlaceholderCompatibilityMode		      |
|									      |
|   Description     Call RtlSetProcessPlaceholderCompatibilityMode()          |
|		    							      |
|   Parameters	    							      |
|		    							      |
|   Returns	    Last PCHM state, or < 0 for error			      |
|		    							      |
|   Notes	    Necessary to expose cloud links. Else they're shown by    |
|		    default as normal files or directories.		      |
|		    							      |
|		    https://stackoverflow.com/questions/59152220/cant-get-reparse-point-information-for-the-onedrive-folder
|		    https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-rtlsetprocessplaceholdercompatibilitymode
|		    							      |
|   History	    							      |
|    2025-07-29 JFL Created this routine.				      |
*		    							      *
\*---------------------------------------------------------------------------*/

#pragma warning(disable:4100) /* Ignore the "unreferenced formal parameter" warning */
static CHAR MlxFailProcessPlaceholderCompatibilityMode(CHAR cMode) {
  return (CHAR)-1;
}
#pragma warning(default:4100) /* Ignore the "unreferenced formal parameter" warning */

int MlxSetProcessPlaceholderCompatibilityMode(int cMode) {
  HMODULE hModule;
  static PSPPHCMPROC pRtlSetProcessPlaceholderCompatibilityMode = NULL;

  DEBUG_ENTER(("RtlSetProcessPlaceholderCompatibilityMode(%d);\n", (int)cMode));

  if (!pRtlSetProcessPlaceholderCompatibilityMode) {
    hModule = LoadLibrary("ntdll.dll");
    if (!hModule) { /* Very unlikely to fail */
      pRtlSetProcessPlaceholderCompatibilityMode = MlxFailProcessPlaceholderCompatibilityMode;
      RETURN_INT_COMMENT(-3, ("Error 0x%X loading ntdll.dll\n", GetLastError()));
    }

    pRtlSetProcessPlaceholderCompatibilityMode = (PSPPHCMPROC)GetProcAddress(hModule, "RtlSetProcessPlaceholderCompatibilityMode");
    if (!pRtlSetProcessPlaceholderCompatibilityMode) {
      pRtlSetProcessPlaceholderCompatibilityMode = MlxFailProcessPlaceholderCompatibilityMode;
      RETURN_INT_COMMENT(-4, ("Error 0x%X getting RtlSetProcessPlaceholderCompatibilityMode() address\n", GetLastError()));
    }
  }

  RETURN_INT((int)(pRtlSetProcessPlaceholderCompatibilityMode((CHAR)cMode)));
}
	
#endif /* _WIN32 */

