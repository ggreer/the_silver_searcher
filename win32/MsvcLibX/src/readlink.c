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
#include "reparsept.h" /* For the undocumented IO_REPARSE_TAG_LX_SYMLINK, etc */

#pragma warning(disable:4201) /* Ignore the "nonstandard extension used : nameless struct/union" warning */
#include <Shlwapi.h> /* For PathFindFileName() */
#pragma comment(lib, "Shlwapi.lib")

/* Get the Reparse Point Tag for a mount point - Wide char version */
/* See http://msdn.microsoft.com/en-us/library/windows/desktop/aa365511(v=vs.85).aspx */
DWORD GetReparseTagW(const WCHAR *pwszPath) {
  HANDLE hFind;
  WIN32_FIND_DATAW findFileData;
  DWORD dwTag = 0;

  hFind = FindFirstFileW(pwszPath, &findFileData);
  if (hFind == INVALID_HANDLE_VALUE ) return 0;
  CloseHandle(hFind);
  if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
    dwTag = findFileData.dwReserved0;
  }
  return dwTag;
}

/* Get the Reparse Point Tag for a mount point - MultiByte char version */
DWORD GetReparseTagM(const char *path, UINT cp) {
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
    DEBUG_PRINTF(("GetReparseTagM(\"%s\", %d); // Conversion to Unicode failed. errno=%d - %s\n", path, cp, errno, strerror(errno)));
    return 0;
  }
  return GetReparseTagW(wszPath);
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
|		    Function ResolveLinks() in resolvelinks.c relies on this. |
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
|		    							      |
*									      *
\*---------------------------------------------------------------------------*/

#pragma warning(disable:4706) /* Ignore the "assignment within conditional expression" warning */

/* Get the reparse point data, and return the tag. 0=failure */
DWORD ReadReparsePointW(const WCHAR *path, char *buf, size_t bufsize) {
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

  DEBUG_WENTER((L"ReadReparsePointW(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

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
    switch (dwTag) {
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
    DEBUG_PRINTF(("ReparseTag = 0x%04X; // %s\n", (unsigned)(dwTag), pType));
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
DWORD ReadLinkW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  char iobuf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  DWORD dwTag;
  PMOUNTPOINT_READ_BUFFER pMountpointBuf;
  PSYMLINK_READ_BUFFER pSymlinkBuf;
  PLX_SYMLINK_READ_BUFFER pLxSymlinkBuf;
  PAPPEXECLINK_READ_BUFFER pAppExecLinkBuf;
  WCHAR *pwStr = NULL;
  WCHAR *pwNewStr = NULL;
  unsigned short offset = 0, len = 0;
  
  DEBUG_WENTER((L"ReadLinkW(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

  dwTag = ReadReparsePointW(path, iobuf, sizeof(iobuf));
  if (!dwTag) RETURN_CONST(0);

  /* Process the supported tag types */
  switch (dwTag) {
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

/* Posix routine readlink - Wide char version. Returns the link size, or -1 on failure */
ssize_t readlinkW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  ssize_t nRead;
  UINT drvType;
  DWORD dwTag;

  DEBUG_WENTER((L"readlink(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

  /* TO DO: Fix readlinkW (And thus ReadReparsePointW) to return truncated links if the buffer is too small.
            Returning an ENAMETOOLONG or ENOMEM error as we do now is sane, but NOT standard */
  dwTag = ReadLinkW(path, buf, bufsize);
  if (!dwTag) {
    RETURN_INT_COMMENT(-1, ("ReadReparsePointW() failed.\n"));
  }

  /* Special case for junctions to other local directories: Remove their '\??\' header.
  // Note: Also seen once on a symlink. I don't know why in most cases symlinks don't have it.
  // Note: These junctions begin with '\??\C:\' (or another drive letter).
  // Other types of junctions/mount points do not continue with a drive letter.
  // For example: '\??\Volume{5e58015c-ba64-4048-928d-06aa03c983f9}\' */
  nRead = lstrlenW(buf);
  if ((nRead >= 7) && (!strncmpW(buf, L"\\??\\", 4))) {
    if (!strncmpW(buf+5, L":\\", 2)) {
      nRead -= 4;
      CopyMemory(buf, buf+4, (nRead+1)*sizeof(WCHAR));
      XDEBUG_WPRINTF((L"buf = \"%s\"; // Removed '\\\\?\\': \n", buf));
    } else { /* Return an error for other types, as Posix SW cannot handle them successfully. */
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
	WCHAR  wszLocalName[] = L"X:";
	WCHAR wszRemoteName[WIDE_PATH_MAX];
	DWORD dwErr;
	DWORD dwLength = WIDE_PATH_MAX;
	wszLocalName[0] = wszAbsPath[0];
	dwErr = WNetGetConnectionW(wszLocalName, wszRemoteName, &dwLength);
	if (dwErr == NO_ERROR) {
	  WCHAR *pwsz;
	  XDEBUG_WPRINTF((L"net use %c: %s\n", (char)(wszLocalName[0]), wszRemoteName));
	  if ((wszRemoteName[0] == L'\\') && (wszRemoteName[1] == L'\\')) {
	    pwsz = wcschr(wszRemoteName+2, L'\\');
	    if (pwsz) {
	      if ((pwsz[2] == L'$') && !pwsz[3]) { /* This is the root of a shared drive. Ex: \\server\D$ -> D: */
		char c1, c2;
		XDEBUG_PRINTF(("// Checking if it's the root of an X$ shared drive\n"));
		c1 = (char)toupper((char)pwsz[1]);	/* The server-side drive letter of the network share */
		c2 = (char)toupper((char)buf[0]);	/* The server-side drive letter of the junction target */
		if (c1 == c2) { /* OK, the target is in the same share drive */
		  buf[0] = wszLocalName[0];	/* Make the target accessible locally */
		  iTargetFound = TRUE;
		  XDEBUG_PRINTF(("// Confirmed it's the root of the shared drive\n"));
		} /* Else the target is not accessible locally via its target name */
	      } else { /* Heuristic: Assume the share name is an alias to the root on the network drive. Ex: \\server\DROOT -> D:\ */
		DWORD dwAttr;
		XDEBUG_PRINTF(("// Checking if it's an alias of the root of the shared drive\n"));
		buf[0] = wszAbsPath[0];
		dwAttr = GetFileAttributesW(buf);
		XDEBUG_WPRINTF((L"GetFileAttributes(\"%s\") = 0x%lX\n", buf, dwAttr));
		if (dwAttr != INVALID_FILE_ATTRIBUTES) {
		  iTargetFound = TRUE;
		  XDEBUG_PRINTF(("// Confirmed it's an alias of the root of the shared drive\n"));
		} else { /* Heuristic: Assume the share name is a subdirectory name on the network drive. Ex: \\server\Public -> C:\Public */
		  WCHAR *pwsz2;
		  XDEBUG_PRINTF(("// Checking if it's first level shared directory\n"));
		  pwsz2 = wcschr(buf+3, L'\\');
		  if (pwsz2) {
		    CopyMemory(buf+2, pwsz2, (lstrlenW(pwsz2)+1)*sizeof(WCHAR));
		    dwAttr = GetFileAttributesW(buf);
		    XDEBUG_WPRINTF((L"GetFileAttributes(\"%s\") = 0x%lX\n", buf, dwAttr));
		    if (dwAttr != INVALID_FILE_ATTRIBUTES) {
		      iTargetFound = TRUE;
		      XDEBUG_PRINTF(("// Confirmed it's a first level shared directory\n"));
		    }
		  }
		}
	      }
	      /* To do:
		 The above code works for network drives shared at the root level and one level below.
		 Ex: N: is \\server\C$
		     junction target C:\Public\Temp\target.txt on N:
		     Resolves to     N:\Public\Temp\target.txt
		 Ex: If N: is \\server\Public, which is shared directory C:\Public,
		     junction target C:\Public\Temp\target.txt on N:
		     Should resolve to N:\Temp\target.txt
		 Actually this could be extended by checking every possible parent path,
		 to support cases where the junction is on a shared level2 or below subdirectory.
		 Note that the share name is not always the same as the subdirectory
		 name, nor is it even in the server's root. 
	      */
	    }
	  }
	}
      }
      if (!iTargetFound) {
#if 0	/* Initial implementation, which would have cause problems */
	lstrcpynW(buf, path, (int)bufsize); /* Report the target as identical to the source, to allow resolving it on the server side */
	buf[bufsize-1] = L'\0';
	nRead = lstrlenW(path);
	RETURN_INT_COMMENT((int)nRead, ("Cannot get to the real target, which is on another server drive.\n"));
#else
	errno = EINVAL;
	RETURN_INT_COMMENT(-1, ("Inaccessible junction target, on another server drive.\n"));
#endif
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
|   Function	    ResolveTailLinks					      |
|									      |
|   Description	    Resolve links in node names	(Ignore those in dir names)   |
|									      |
|   Parameters      const char *path	    The symlink name		      |
|		    char *buf		    Output buffer		      |
|		    size_t bufsize	    Output buffer size in characters  |
|									      |
|   Returns	    0 = Success, -1 = Failure and set errno		      |
|		    							      |
|   Notes	    TO DO: Detect circular loops?			      |
|									      |
|   History								      |
|    2017-03-22 JFL Created this routine                               	      |
*									      *
\*---------------------------------------------------------------------------*/

int ResolveTailLinksW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  DWORD dwAttr;
  size_t l;

  DEBUG_WENTER((L"ResolveTailLinks(\"%s\", %p, %ul);\n", path, buf, (unsigned long)bufsize));

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
    if (!(   (wszBuf2[0] == L'\\')
          || (wszBuf2[0] && (wszBuf2[1] == L':')))) { /* This is a relative path. We must compose it with the link dirname */
      lstrcpynW(wszBuf3, path, WIDE_PATH_MAX); /* May truncate the output string */
      wszBuf3[WIDE_PATH_MAX-1] = L'\0'; /* Make sure the string is NUL-terminated */
      TrimTailSlashesW(wszBuf3);
      pwsz = PathFindFileNameW(wszBuf3);
      if (!lstrcmpW(pwsz, L"..")) { /* The link dirname is actually one level above */
      	lstrcatW(pwsz, L"\\..\\");
      } else if (!lstrcmpW(pwsz, L".")) { /* It's also one level above */
      	lstrcatW(pwsz, L".\\");			/* Change the . into a ..\ */
      } else if (lstrcmpW(pwsz, L"/")) { /* The value replaces the link node name */
      	*pwsz = L'\0';
      }
      iCDSize = lstrlenW(wszBuf3);
      lstrcpynW(wszBuf3+iCDSize, wszBuf2, WIDE_PATH_MAX-iCDSize); /* May truncate the output string */
      wszBuf3[WIDE_PATH_MAX-1] = L'\0'; /* Make sure the string is NUL-terminated */
      /* CompactpathW(wszBuf3, wszBuf2, WIDE_PATH_MAX); // We don't care as we're only interested in the tail */
      pwsz = wszBuf3;
    }
    iRet = ResolveTailLinksW(pwsz, buf, bufsize);
    DEBUG_WLEAVE((L"return %d; // \"%s\"\n", iRet, buf));
    return iRet;
  }

  l = lstrlenW(path);
  if (l >= bufsize) {
    errno = ENAMETOOLONG;
    RETURN_INT_COMMENT(-1, ("Buffer too small\n"));
  }
  lstrcpyW(buf, path);
  DEBUG_WLEAVE((L"return 0; // \"%s\"\n", buf));
  return 0;
}

int ResolveTailLinksM(const char *path, char *buf, size_t bufsize, UINT cp) {
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
    DEBUG_PRINTF(("ResolveTailLinksM(\"%s\", ...); // Conversion to Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return -1;
  }

  iErr = ResolveTailLinksW(wszPath, wszTarget, WIDE_PATH_MAX);
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
    DEBUG_PRINTF(("ResolveTailLinksM(\"%s\", ...); // Conversion back from Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return -1;
  }

  return iErr;
}

int ResolveTailLinksA(const char *path, char *buf, size_t bufsize) {
  return ResolveTailLinksM(path, buf, bufsize, CP_ACP);
}

int ResolveTailLinksU(const char *path, char *buf, size_t bufsize) {
  return ResolveTailLinksM(path, buf, bufsize, CP_UTF8);
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    ReadAppExecLink					      |
|									      |
|   Description	    Get the AppExecLink target, and return the tag            |
|									      |
|   Parameters      const char *path	    The AppExecLink name              |
|		    char *buf		    Output buffer		      |
|		    size_t bufsize	    Output buffer size in characters  |
|									      |
|   Returns	    >0 = Success, 0 = Failure and set errno		      |
|		    							      |
|   Notes	    TO DO: Detect circular loops?			      |
|									      |
|   History								      |
|    2020-12-11 JFL Created this routine                               	      |
*									      *
\*---------------------------------------------------------------------------*/

/* Get the AppExecLink target, and return its size. 0=failure */
int ReadAppExecLinkW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  char iobuf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  DWORD dwTag;
  PAPPEXECLINK_READ_BUFFER pAppExecLinkBuf;
  WCHAR *pwStr = NULL;
  unsigned short offset = 0, len = 0;

  DEBUG_WENTER((L"ReadAppExecLinkW(\"%s\", 0x%p, %d);\n", path, buf, bufsize));

  dwTag = ReadReparsePointW(path, iobuf, sizeof(iobuf));
  if (!dwTag) RETURN_CONST(0);

  /* Process the supported tag types */
  switch (dwTag) {
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
	offset = 0;
	len = (unsigned short)lstrlenW(pwStr);
      } else {
        DEBUG_PRINTF(("# WARNING: Unexpected AppExecLink Version = %d\n", (int)(pAppExecLinkBuf->Version)));
      }
      break;

    default:
      errno = EINVAL;
      RETURN_INT_COMMENT(0, ("Unsupported reparse point type\n"));
  }
  if (len) {
    if (len >= bufsize) {
      errno = ENAMETOOLONG;
      RETURN_INT_COMMENT(0, ("The output buffer is too small. The link size is %d bytes.\n", len));
    }
    CopyMemory(buf, pwStr+offset, len*sizeof(WCHAR));
  }
  buf[len] = L'\0';

  DEBUG_WLEAVE((L"return 0x%X; // \"%s\"\n", len, buf));
  return len;
}

/* Get the AppExecLink target, and return its size. 0 = failure */
int ReadAppExecLinkM(const char *path, char *buf, size_t bufsize, UINT cp) {
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
    DEBUG_PRINTF(("ReadAppExecLinkM(\"%s\", ...); // Conversion to Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
    return 0;
  }

  n = ReadAppExecLinkW(wszPath, wszTarget, WIDE_PATH_MAX);
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
    DEBUG_PRINTF(("ReadAppExecLinkM(\"%s\", ...); // Conversion back from Unicode failed. errno=%d - %s\n", path, errno, strerror(errno)));
  }

  return n;
}

#endif /* _WIN32 */

