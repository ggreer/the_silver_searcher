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
#include "reparsept.h"

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
  WCHAR wszPath[PATH_MAX];
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

/* Get the reparse point target, and return the tag. 0=failure */
DWORD ReadReparsePointW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  DWORD dwAttr;
  HANDLE hLink;
  BOOL done;
  DWORD dwRead;
  char iobuf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  DWORD dwFlagsAndAttributes;
  DWORD dwTag;
  WCHAR *pwStr;
  PREPARSE_READ_BUFFER pIoctlBuf;
  PMOUNTPOINT_READ_BUFFER pMountpointBuf;
  PSYMLINK_READ_BUFFER pSymlinkBuf;
  unsigned short offset, len = 0;
  DEBUG_CODE(
  char *pszUtf8;
  )

  DEBUG_WSTR2NEWUTF8(path, pszUtf8);
  DEBUG_ENTER(("ReadReparsePointW(\"%s\", 0x%p, %d);\n", pszUtf8, buf, bufsize));
  DEBUG_FREEUTF8(pszUtf8);

  dwAttr = GetFileAttributesW(path);
  XDEBUG_PRINTF(("GetFileAttributes() = 0x%lX\n", dwAttr));
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(0, ("File does not exist\n"));
  }

  if (!(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
    errno = EINVAL;
    RETURN_INT_COMMENT(0, ("File is not a link\n"));
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
    RETURN_INT_COMMENT(0, ("Cannot open the link\n"));
  }

  done = DeviceIoControl(hLink,				/* hDevice, */
    			 FSCTL_GET_REPARSE_POINT,       /* dwIoControlCode, */
    			 NULL,				/* lpInBuffer, */
    			 0,				/* nInBufferSize, */
    			 iobuf,				/* lpOutBuffer, */
    			 sizeof(iobuf),			/* nOutBufferSize, */
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
  pIoctlBuf = (PREPARSE_READ_BUFFER)iobuf;
  dwTag = pIoctlBuf->ReparseTag;
  DEBUG_CODE_IF_ON(
    char *pType = "";
    switch (dwTag) {
    case IO_REPARSE_TAG_RESERVED_ZERO:		pType = "Reserved"; break;	
    case IO_REPARSE_TAG_RESERVED_ONE:		pType = "Reserved"; break;
    case IO_REPARSE_TAG_RESERVED_TWO:		pType = "Reserved"; break;
    case IO_REPARSE_TAG_MOUNT_POINT:		pType = "Mount point or junction"; break;
    case IO_REPARSE_TAG_HSM:			pType = "Hierarchical Storage Manager"; break;
    case IO_REPARSE_TAG_DRIVE_EXTENDER:		pType = "Home server drive extender"; break;
    case IO_REPARSE_TAG_HSM2:			pType = "Hierarchical Storage Manager Product #2"; break;
    case IO_REPARSE_TAG_SIS:			pType = "Single-instance storage filter driver"; break;
    case IO_REPARSE_TAG_WIM:			pType = "Windows boot Image File"; break;
    case IO_REPARSE_TAG_CSV:			pType = "Cluster Shared Volume"; break;
    case IO_REPARSE_TAG_DFS:			pType = "Distributed File System"; break;
    case IO_REPARSE_TAG_FILTER_MANAGER:		pType = "Filter manager test harness"; break;
    case IO_REPARSE_TAG_SYMLINK:		pType = "Symbolic link"; break;
    case IO_REPARSE_TAG_IIS_CACHE:		pType = "Internet Information Services cache"; break;
    case IO_REPARSE_TAG_DFSR:			pType = "Distributed File System R filter"; break;
    case IO_REPARSE_TAG_DEDUP:			pType = "Deduplicated file"; break;
    case IO_REPARSE_TAG_NFS:			pType = "NFS symbolic link"; break;
    case IO_REPARSE_TAG_APPXSTREAM:		pType = "APPXSTREAM (?)"; break;
    case IO_REPARSE_TAG_FILE_PLACEHOLDER:	pType = "Placeholder for a OneDrive file"; break;
    case IO_REPARSE_TAG_DFM:			pType = "DFM (?)"; break;
    case IO_REPARSE_TAG_WOF:			pType = "Windows Overlay Filesystem compressed file"; break;
    case IO_REPARSE_TAG_WCI:			pType = "Windows Container Image?"; break;
    case IO_REPARSE_TAG_GLOBAL_REPARSE:		pType = "GLOBAL_REPARSE (?)"; break;
    case IO_REPARSE_TAG_CLOUD:			pType = "CLOUD (?)"; break;
    case IO_REPARSE_TAG_APPEXECLINK:		pType = "APPEXECLINK (?)"; break;
    case IO_REPARSE_TAG_GVFS:			pType = "GVFS (?)"; break;
    case IO_REPARSE_TAG_LX_SYMLINK:		pType = "Linux Sub-System Symbolic Link"; break;
    default:					pType = "Unknown type! Please report its value and update readlink.c."; break;
    }
    DEBUG_PRINTF(("ReparseTag = 0x%04X; // %s\n", (unsigned)(dwTag), pType));
  )
  XDEBUG_PRINTF(("ReparseDataLength = 0x%04X\n", (unsigned)(pIoctlBuf->ReparseDataLength)));

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

  DEBUG_WSTR2NEWUTF8(buf, pszUtf8);
  DEBUG_LEAVE(("return 0x%X; // \"%s\"\n", dwTag, pszUtf8));
  DEBUG_FREEUTF8(pszUtf8);
  return dwTag;
}

/* Posix routine readlink - Wide char version */
ssize_t readlinkW(const WCHAR *path, WCHAR *buf, size_t bufsize) {
  ssize_t nRead;
  UINT drvType;
  DWORD dwTag;
  DEBUG_CODE(
  char *pszUtf8;
  )

  DEBUG_WSTR2NEWUTF8(path, pszUtf8);
  DEBUG_ENTER(("readlink(\"%s\", 0x%p, %d);\n", pszUtf8, buf, bufsize));
  DEBUG_FREEUTF8(pszUtf8);

  /* TO DO: Fix readlinkW (And thus ReadReparsePointW) to return truncated links if the buffer is too small.
            Returning an ENAMETOOLONG or ENOMEM error as we do now is sane, but NOT standard */
  dwTag = ReadReparsePointW(path, buf, bufsize);
  if (!dwTag) {
    RETURN_INT_COMMENT(-1, ("ReadReparsePointW() failed.\n"));
  }

  /* Special case for junctions to other local directories: Remove their '\??\' header.
  // Note: Also seen once on a symlink. I don't know why in most cases symlinks don't have it.
  // Note: These junctions begin with '\??\C:\' (or another drive letter).
  // Other types of junctions/mount points do not continue with a drive letter.
  // For example: '\??\Volume{5e58015c-ba64-4048-928d-06aa03c983f9}\' */
  nRead = lstrlenW(buf);
#define strncmpW(s1, s2, l) (CompareStringW(LOCALE_INVARIANT, 0, s1, l, s2, l)-2)
  if ((nRead >= 7) && (!strncmpW(buf, L"\\??\\", 4))) {
    if (!strncmpW(buf+5, L":\\", 2)) {
      nRead -= 4;
      CopyMemory(buf, buf+4, (nRead+1)*sizeof(WCHAR));
      DEBUG_WSTR2NEWUTF8(buf, pszUtf8);
      XDEBUG_PRINTF(("buf = \"%s\"; // Removed '\\\\?\\': \n", pszUtf8));
      DEBUG_FREEUTF8(pszUtf8);
    } else { /* Return an error for other types, as Posix SW cannot handle them successfully. */
      errno = EINVAL;
      DEBUG_WSTR2NEWUTF8(buf+4, pszUtf8);
      DEBUG_LEAVE(("return -1; // Unsupported mount point type: %s\n", pszUtf8));
      DEBUG_FREEUTF8(pszUtf8);
      return -1;
    }
  }

  /* Fix junctions targets */
  /* Windows resolves junctions on the server side,
     but symlinks and symlinkds on the client side. */
  if (dwTag == IO_REPARSE_TAG_MOUNT_POINT) {
    char szRootDir[4] = "C:\\";
    WCHAR wszAbsPath[PATH_MAX];
    WCHAR wszAbsPath2[PATH_MAX];
    WCHAR *p1;
    WCHAR *p2;
    WCHAR *pc1 = L"A";
    WCHAR *pc2 = L"a";

    DEBUG_WSTR2NEWUTF8(buf, pszUtf8);
    XDEBUG_PRINTF(("rawJunctionTarget = \"%s\"\n", pszUtf8));
    DEBUG_FREEUTF8(pszUtf8);

    GetFullPathNameW(path, PATH_MAX, wszAbsPath, NULL); /* Get the drive letter in the full path */
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
	WCHAR wszRemoteName[PATH_MAX];
	DWORD dwErr;
	DWORD dwLength = PATH_MAX;
	wszLocalName[0] = wszAbsPath[0];
	dwErr = WNetGetConnectionW(wszLocalName, wszRemoteName, &dwLength);
	if (dwErr == NO_ERROR) {
	  WCHAR *pwsz;
	  DEBUG_WSTR2NEWUTF8(wszRemoteName, pszUtf8);
	  XDEBUG_PRINTF(("net use %c: %s\n", (char)(wszLocalName[0]), pszUtf8));
	  DEBUG_FREEUTF8(pszUtf8);
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
		DEBUG_WSTR2NEWUTF8(buf, pszUtf8);
		XDEBUG_PRINTF(("GetFileAttributes(\"%s\") = 0x%lX\n", pszUtf8, dwAttr));
		DEBUG_FREEUTF8(pszUtf8);
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
		    DEBUG_WSTR2NEWUTF8(buf, pszUtf8);
		    XDEBUG_PRINTF(("GetFileAttributes(\"%s\") = 0x%lX\n", pszUtf8, dwAttr));
		    DEBUG_FREEUTF8(pszUtf8);
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
    GetFullPathNameW(buf, PATH_MAX, wszAbsPath2, NULL);
    DEBUG_WSTR2NEWUTF8(wszAbsPath, pszUtf8);
    XDEBUG_PRINTF(("szAbsPath = \"%s\"\n", pszUtf8));
    DEBUG_FREEUTF8(pszUtf8);
    DEBUG_WSTR2NEWUTF8(wszAbsPath2, pszUtf8);
    XDEBUG_PRINTF(("szAbsPath2 = \"%s\"\n", pszUtf8));
    DEBUG_FREEUTF8(pszUtf8);
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
      DEBUG_WSTR2NEWUTF8(p1, pszUtf8);
      XDEBUG_PRINTF(("szRelPath1 = \"%s\"\n", pszUtf8));
      DEBUG_FREEUTF8(pszUtf8);
      DEBUG_WSTR2NEWUTF8(p2, pszUtf8);
      XDEBUG_PRINTF(("szRelPath2 = \"%s\"\n", pszUtf8));
      DEBUG_FREEUTF8(pszUtf8);
      buf[0] = '\0';
      /* Count the # of parent directories that remain in path 1 */
      for (pc=p1; *pc; pc++) if (*pc == L'\\') lstrcatW(buf, L"..\\");
      /* Append what remains in path 2 */
      lstrcatW(buf, p2);
      /* That's the relative link */
      nRead = lstrlenW(buf);
    } /* Else the drives differ. Paths cannot be relative. Don't change buf. */
  }

  DEBUG_WSTR2NEWUTF8(buf, pszUtf8);
  DEBUG_LEAVE(("return %d; // \"%s\"\n", (int)nRead, pszUtf8));
  DEBUG_FREEUTF8(pszUtf8);
  return (int)nRead;
}

#pragma warning(default:4706)

/* Posix routine readlink - MultiByte char version */
ssize_t readlinkM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR wszPath[PATH_MAX];
  WCHAR wszTarget[PATH_MAX];
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

  nResult = readlinkW(wszPath, wszTarget, PATH_MAX);
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
  DEBUG_CODE( /* TO DO: Use DEBUG_WSTR2NEWUTF8() / DEBUG_FREEUTF8() to avoid wasting stack space */
  char szUtf8[UTF8_PATH_MAX];
  )
  size_t l;

  DEBUG_WSTR2UTF8(path, szUtf8, sizeof(szUtf8));
  DEBUG_ENTER(("ResolveTailLinks(\"%s\", %p, %ul);\n", szUtf8, buf, (unsigned long)bufsize));

  dwAttr = GetFileAttributesW(path);
  XDEBUG_PRINTF(("GetFileAttributes() = 0x%lX\n", dwAttr));
  if (dwAttr == INVALID_FILE_ATTRIBUTES) {
    errno = ENOENT;
    RETURN_INT_COMMENT(-1, ("File does not exist\n"));
  }

  if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
    WCHAR wszBuf2[PATH_MAX];
    WCHAR wszBuf3[PATH_MAX];
    WCHAR *pwsz = wszBuf2;
    int iCDSize;
    int iRet;
    ssize_t nLinkSize = readlinkW(path, wszBuf2, PATH_MAX); /* Corrects junction drive letters, etc */
    if (nLinkSize < 0) RETURN_INT(-1);
    if (!(   (wszBuf2[0] == L'\\')
          || (wszBuf2[0] && (wszBuf2[1] == L':')))) { /* This is a relative path. We must compose it with the link dirname */
      lstrcpynW(wszBuf3, path, PATH_MAX); /* May truncate the output string */
      wszBuf3[PATH_MAX-1] = L'\0'; /* Make sure the string is NUL-terminated */
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
      lstrcpynW(wszBuf3+iCDSize, wszBuf2, PATH_MAX-iCDSize); /* May truncate the output string */
      wszBuf3[PATH_MAX-1] = L'\0'; /* Make sure the string is NUL-terminated */
      /* CompactpathW(wszBuf3, wszBuf2, PATH_MAX); // We don't care as we're only interested in the tail */
      pwsz = wszBuf3;
    }
    iRet = ResolveTailLinksW(pwsz, buf, bufsize);
    DEBUG_CODE(
      szUtf8[0] = '\0';
      if (iRet >= 0) DEBUG_WSTR2UTF8(buf, szUtf8, sizeof(szUtf8));
    )
    RETURN_INT_COMMENT(iRet, ("\"%s\"\n", szUtf8));
  }

  l = lstrlenW(path);
  if (l >= bufsize) {
    errno = ENAMETOOLONG;
    RETURN_INT_COMMENT(-1, ("Buffer too small\n"));
  }
  lstrcpyW(buf, path);
  RETURN_CONST_COMMENT(0, ("\"%s\"\n", szUtf8));
}

int ResolveTailLinksM(const char *path, char *buf, size_t bufsize, UINT cp) {
  WCHAR wszPath[PATH_MAX];
  WCHAR wszTarget[PATH_MAX];
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

  iErr = ResolveTailLinksW(wszPath, wszTarget, PATH_MAX);
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

#endif


