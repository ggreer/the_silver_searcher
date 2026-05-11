/*****************************************************************************\
*                                                                             *
*   Filename	    symlink.c						      *
*									      *
*   Description:    WIN32 port of standard C library's symlink()	      *
*                                                                             *
*   Notes:	    Requires SE_CREATE_SYMBOLIC_LINK_NAME privilege.          *
*		    							      *
*		    							      *
*   History:								      *
*    2014-02-05 JFL Created this module.				      *
*    2014-03-02 JFL Split the functions into a WSTR and an UTF-8 version.     *
*    2014-03-20 JFL Restructured Windows link management functions into Wide  *
*		    and MultiByte versions, and changed the Unicode and Ansi  *
*		    versions to macros.					      *
*    2014-07-03 JFL Added support for pathnames >= 260 characters. 	      *
*    2015-12-14 JFL Added a workaround allowing to link support for symlinks  *
*                   in all apps, even when targeting XP or older systems that *
*                   do not support symlinks.                                  *
*    2016-08-25 JFL Fixed two warnings.                                       *
*    2017-05-03 JFL Dynamically allocate debug strings in junction().         *
*    2018-10-01 JFL Dynamically allocate path buffers in all routines.        *
*    2021-11-24 JFL Added an optional _Base_Path.txt file at the root of      *
*                   network shares, to provide a known good Base Path to use  *
*                   for creating remote junctions.                            *
*    2021-11-28 JFL Moved the junction base path heuristic to an outside      *
*                   subroutine, shared between readlink() and junction().     *
*    2021-11-29 JFL Use the wide-string debug macros.                         *
*                   Do not allow setting an external target on a netw. share. *
*                   Allow passing native target names to junction().          *
*    2026-02-10 JFL Fixed the creation of junctions with targets > 260 chars. *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

#include <unistd.h>
#include <errno.h>
#include <iconv.h>
#include "debugm.h"

#ifdef _MSC_VER
#pragma warning(disable:4001)	/* Ignore the // C++ comment warning */
#endif

#ifdef _WIN32

#include <windows.h>

#include "reparsept.h"

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    junction						      |
|									      |
|   Description:    Create an NTFS junction                                   |
|									      |
|   Parameters:     const char *targetName	The junction target name      |
|		    const char *junctionName	The junction name	      |
|									      |
|   Returns:	    0 = Success, -1 = Failure				      |
|									      |
|   Notes:	    Uses the undocumented FSCTL_SET_REPARSE_POINT structure   |
|		    Win2K uses for mount points and junctions.                |
|									      |
|   History:								      |
|    2014-02-05 JFL Adapted from Mark Russinovitch CreateJuction sample	      |
*									      *
\*---------------------------------------------------------------------------*/

/* MsvcLibX-specific routine to create an NTFS junction - Wide char version */
int junctionW(const WCHAR *targetName, const WCHAR *junctionName) {
  WCHAR *wszReparseBuffer = NULL;
  WCHAR  wszVolumeName[] = L"X:\\";
  WCHAR *wszJunctionFullName = NULL;
  WCHAR *wszFileSystem = NULL;
  WCHAR *wszTargetTempName = NULL;
  WCHAR *wszTargetFullName = NULL;
  WCHAR *wszTargetNativeName = NULL;
  WCHAR *wszRemoteName = NULL;
  WCHAR *pwszFilePart;
  size_t lNativeName;
  HANDLE hFile;
  DWORD  dwReturnedLength;
  PMOUNTPOINT_WRITE_BUFFER reparseInfo;
  UINT uiDriveType;
  DWORD dwFileSystemFlags;
  int iRet = -1;	/* Assume failure */

  DEBUG_WENTER((L"junction(\"%s\", \"%s\");\n", targetName, junctionName));

  wszReparseBuffer = malloc(sizeof(WCHAR)*PATH_MAX*3);
  if (!wszReparseBuffer) goto junctionW_exit;
  reparseInfo = (PMOUNTPOINT_WRITE_BUFFER)wszReparseBuffer;
  wszJunctionFullName = malloc(sizeof(WCHAR)*PATH_MAX);
  if (!wszJunctionFullName) goto junctionW_exit;
#define SIZEOF_wszFileSystem (sizeof(WCHAR)*PATH_MAX)
  wszFileSystem = malloc(SIZEOF_wszFileSystem);
  if (!wszFileSystem) goto junctionW_exit;
  wszTargetTempName = malloc(sizeof(WCHAR)*PATH_MAX);
  if (!wszTargetTempName) goto junctionW_exit;
  wszTargetFullName = malloc(sizeof(WCHAR)*PATH_MAX);
  if (!wszTargetFullName) goto junctionW_exit;
  wszTargetNativeName = malloc(sizeof(WCHAR)*PATH_MAX);
  if (!wszTargetNativeName) goto junctionW_exit;

  /* Get the full path of the junction */
  if (!GetFullPathNameW(junctionName, PATH_MAX, wszJunctionFullName, &pwszFilePart)) {
    errno = Win32ErrorToErrno();
    DEBUG_WLEAVE((L"return -1; // %s is an invalid junction name\n", junctionName));
    goto junctionW_exit;
  }

  /* Do we have a native name already? */
  if (!wcsncmp(targetName, L"\\??\\", 4)) { /* We have the native name already */
    lstrcpyW(wszTargetNativeName, targetName);
    lNativeName = lstrlenW(wszTargetNativeName);
    goto junctionW_create;
  }

  /* Convert relative paths to absolute paths relative to the junction. */
  /* Note: I tested creating relative targets: They can be created, but Windows can't follow them. */
  if ((targetName[0] != L'\\') && (targetName[1] != L':')) {
    size_t lTempName = PATH_MAX;
    lstrcpyW(wszTargetTempName, wszJunctionFullName);
    lTempName -= lstrlenW(wszJunctionFullName);
    if (lTempName < (size_t)(lstrlenW(targetName) + 5)) {
      errno = ENAMETOOLONG;
      DEBUG_LEAVE(("return -1; // Intermediate target name too long\n"));
      goto junctionW_exit;
    }
    lstrcatW(wszTargetTempName, L"\\..\\");
    lstrcatW(wszTargetTempName, targetName);
    if (!GetFullPathNameW(wszTargetTempName, PATH_MAX, wszTargetFullName, &pwszFilePart)) {
      errno = Win32ErrorToErrno();
      DEBUG_WLEAVE((L"return -1; // %s is an invalid target directory name\n", targetName));
      goto junctionW_exit;
    }
    XDEBUG_WPRINTF((L"wszTargetFullName = \"%s\"; // After absolutization relative to the junction\n", wszTargetFullName));
  } else { /* Already an absolute name. Just make sure it's canonic. (Without . or ..) */
    if (!GetFullPathNameW(targetName, PATH_MAX, wszTargetFullName, &pwszFilePart)) {
      errno = Win32ErrorToErrno();
      DEBUG_WLEAVE((L"return -1; // %s is an invalid target directory name\n", targetName));
      goto junctionW_exit;
    }
    XDEBUG_WPRINTF((L"wszTargetFullName = \"%s\"; // After direct reabsolutization\n", wszTargetFullName));
  }
  /* Make sure the junction and target drive letters are upper case */
#pragma warning(disable:4305) /* truncation from 'LPSTR' to 'WCHAR' */
#pragma warning(disable:4306) /* conversion from 'WCHAR' to 'WCHAR *' of greater size */
  wszJunctionFullName[0] = (WCHAR)CharUpperW((WCHAR *)(wszJunctionFullName[0]));
  wszTargetFullName[0]   = (WCHAR)CharUpperW((WCHAR *)(wszTargetFullName[0]));
#pragma warning(default:4706)
#pragma warning(default:4705)

  /* Make sure that the junction is on a file system that supports reparse points (Ex: NTFS) */
  wszVolumeName[0] = wszJunctionFullName[0];
  wszFileSystem[0] = L'\0'; /* wszFileSystem = L""; */
  GetVolumeInformationW(wszVolumeName, NULL, 0, NULL, NULL, &dwFileSystemFlags, wszFileSystem, SIZEOF_wszFileSystem/sizeof(WCHAR));
  if (!(dwFileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS)) {
    errno = EDOM;
    DEBUG_WLEAVE((L"return -1; // Junctions are not supported on %s volumes\n", wszFileSystem));
    goto junctionW_exit;
  }

  /* On network drives, make sure the target refers to the local drive on the server */
  /* Note: The local path on the server can be inferred in simple cases, but not in the general case */
  uiDriveType = GetDriveTypeW(wszVolumeName);
  XDEBUG_WPRINTF((L"GetDriveType(\"%s\") = %d // %s drive\n", wszVolumeName, uiDriveType, (uiDriveType == DRIVE_REMOTE) ? L"Network" : L"Local"));
  if (uiDriveType == DRIVE_REMOTE) {
    WCHAR  wszLocalName[] = L"X:";
    DWORD dwErr;
    DWORD dwLength = PATH_MAX;
    if (wszJunctionFullName[0] != wszTargetFullName[0]) { /* Note: They're both upper case now */
      errno = EXDEV;
      DEBUG_WLEAVE((L"return -1; // Junctions to external drives are not supported on network shares\n"));
      goto junctionW_exit;
    }
    wszRemoteName = malloc(sizeof(WCHAR)*PATH_MAX);
    if (!wszRemoteName) goto junctionW_exit;
    wszLocalName[0] = wszJunctionFullName[0];
    dwErr = WNetGetConnectionW(wszLocalName, wszRemoteName, &dwLength);
    if (dwErr == NO_ERROR) {
      XDEBUG_WPRINTF((L"net use %c: %s\n", (char)(wszLocalName[0]), wszRemoteName));
      if ((wszRemoteName[0] == L'\\') && (wszRemoteName[1] == L'\\')) {
      	WCHAR *pwszShareBasePath = MlxGetShareBasePathW(wszRemoteName);
      	if (pwszShareBasePath) {
      	  if ((lstrlenW(pwszShareBasePath) + lstrlenW(wszTargetFullName+2) + 1) < PATH_MAX) {
	    lstrcpyW(wszTargetTempName, pwszShareBasePath);
	    lstrcatW(wszTargetTempName, wszTargetFullName+2);
	    lstrcpyW(wszTargetFullName, wszTargetTempName);
	  }
	  free(pwszShareBasePath);
	}
      }
    } else {
      XDEBUG_WPRINTF((L"WNetGetConnection(\"%s\") failed: Error %d\n", wszVolumeName, dwErr));
    }
  }

  /* Make the native target name */
  lNativeName = wsprintfW(wszTargetNativeName, L"\\??\\%s", wszTargetFullName );
  if ( (wszTargetNativeName[lNativeName-1] == L'\\') &&
       (wszTargetNativeName[lNativeName-2] != L':')) {
    wszTargetNativeName[lNativeName-1] = L'\0';
    lNativeName -= 1;
  }

junctionW_create:
  /* Create the link - ignore errors since it might already exist */
  DEBUG_WPRINTF((L"// Creating junction \"%s\" -> \"%s\"\n", wszJunctionFullName, wszTargetNativeName));
  CreateDirectoryW(junctionName, NULL);
  hFile = CreateFileW(junctionName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		      FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, NULL );
  if (hFile == INVALID_HANDLE_VALUE) {
    errno = Win32ErrorToErrno();
    DEBUG_WLEAVE((L"return -1; // Error creating %s\n", wszJunctionFullName));
    goto junctionW_exit;
  }

  /* Build the reparse info */
  ZeroMemory(reparseInfo, sizeof( *reparseInfo ));
  reparseInfo->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
  reparseInfo->ReparseTargetLength = (WORD)(lNativeName * sizeof(WCHAR));
  reparseInfo->ReparseTargetMaximumLength = reparseInfo->ReparseTargetLength + sizeof(WCHAR);
  reparseInfo->ReparseDataLength          = reparseInfo->ReparseTargetLength + MOUNTPOINT_WRITE_BUFFER_HEADER_SIZE - 4;
  lstrcpynW(reparseInfo->ReparseTarget, wszTargetNativeName, PATH_MAX);

  /* Set the link */
  if (!DeviceIoControl(hFile, FSCTL_SET_REPARSE_POINT, reparseInfo,
		       /* reparseInfo->ReparseDataLength + 4, */
		       reparseInfo->ReparseDataLength + 8,
		       NULL, 0, &dwReturnedLength, NULL )) {
    errno = Win32ErrorToErrno();
    CloseHandle(hFile);
    RemoveDirectoryW(junctionName);
    DEBUG_WLEAVE((L"return -1; // Error setting junction for %s:\n", wszJunctionFullName));
    goto junctionW_exit;
  }

  CloseHandle(hFile);
  DEBUG_WLEAVE((L"return 0; // Created \"%s\" -> \"%s\"\n", wszJunctionFullName, wszTargetNativeName));
  iRet = 0; /* Success */

junctionW_exit:
  free(wszReparseBuffer);
  free(wszJunctionFullName);
  free(wszFileSystem);
  free(wszTargetTempName);
  free(wszTargetFullName);
  free(wszTargetNativeName);
  free(wszRemoteName);
  return iRet;
}

/* MsvcLibX-specific routine to create an NTFS junction - MultiByte char version */
int junctionM(const char *targetName, const char *junctionName, UINT cp) {
  WCHAR *pwszJunction = NULL;
  WCHAR *pwszTarget = NULL;
  int iRet = -1;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszJunction = MultiByteToNewWidePath(cp,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
					junctionName	/* lpMultiByteStr, */
					);
  if (!pwszJunction) goto cleanup_and_exit; /* errno is already set by MultiByteToNewWidePath() */

  /* Convert the pathname to a unicode string */
  /* Do not prepend the '\?\' prefix for long paths, as junction targets will get their own '\??\' prefix. */
  pwszTarget = MultiByteToNewWideString(cp,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
    				        targetName	/* lpMultiByteStr, */
    				        );
  if (!pwszTarget) goto cleanup_and_exit; /* errno is already set by MultiByteToNewWidePath() */

  iRet = junctionW(pwszTarget, pwszJunction);

cleanup_and_exit:
  free(pwszJunction);
  free(pwszTarget);
  return iRet;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    symlink						      |
|									      |
|   Description:    Create an NTFS symbolic link                              |
|									      |
|   Parameters:     const TCHAR *targetName	The symlink target name       |
|		    const TCHAR *linkName	The symlink name	      |
|									      |
|   Returns:	    0 = Success, -1 = Failure				      |
|									      |
|   Notes:	    							      |
|									      |
|   History:								      |
|    2014-02-04 JFL Created this routine                                      |
*									      *
\*---------------------------------------------------------------------------*/

/* Symbolic links first appeared in Vista WINVER 0x600.
   Add support for both old and new versions, by using Windows' CreateSymbolicLink
   function if it exists, or else a short stub that fails every time if not. */
#if WINVER < 0x600 /* This mechanism is only necessary when targeting older versions of Windows */

typedef BOOLEAN (WINAPI *LPCREATESYMBOLICLINK)(LPCWSTR lpSymlinkName, LPCWSTR lpTargetName, DWORD dwFlags);

/* Default routine to use if Windows does not have CreateSymbolicLinkW */
#pragma warning(disable:4100) /* 'dwFlags' : unreferenced formal parameter */
BOOLEAN WINAPI DefaultCreateSymbolicLinkW(LPCWSTR lpSymlinkName, LPCWSTR lpTargetName, DWORD dwFlags) {
  DWORD dwAttr = GetFileAttributesW(lpTargetName);
  if (dwAttr != INVALID_FILE_ATTRIBUTES) { /* If the target exists */
    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) { /* And if the target is a directory */
      /* Try creating a junction as a workaround */
      int iRet = junctionW(lpTargetName, lpSymlinkName);	/* 0=Success, -1=Failure */
      return (BOOLEAN)(iRet + 1);				/* 1=Success,  0=Failure */
    }
  }
  /* Else fail, as symlinks to files aren't supported by this Windows version */
  SetLastError(ERROR_NOT_SUPPORTED);
  return FALSE;
}
#pragma warning(default:4105)

/* Initialization routine. Tries using Windows' CreateSymbolicLinkW if present, else uses our default above */
BOOLEAN WINAPI InitCreateSymbolicLink(LPCWSTR lpSymlinkName, LPCWSTR lpTargetName, DWORD dwFlags) {
  extern LPCREATESYMBOLICLINK lpCreateSymbolicLinkW;
  lpCreateSymbolicLinkW = (LPCREATESYMBOLICLINK) GetProcAddress(
    GetModuleHandle(TEXT("kernel32.dll")), "CreateSymbolicLinkW");
  if (!lpCreateSymbolicLinkW) { /* This is XP or older, not supporting symlinks */
    lpCreateSymbolicLinkW = DefaultCreateSymbolicLinkW;
  }
  return (*lpCreateSymbolicLinkW)(lpSymlinkName, lpTargetName, dwFlags);
}

LPCREATESYMBOLICLINK lpCreateSymbolicLinkW = InitCreateSymbolicLink;

/* Make sure all uses of CreateSymbolicLinkW below go through our static pointer above */
#undef CreateSymbolicLinkW
#define CreateSymbolicLinkW (*lpCreateSymbolicLinkW)

#endif /* WINVER < 0x600 */

/* Posix routine symlink - Wide char version */
int symlinkW(const WCHAR *targetName, const WCHAR *linkName) {
  DWORD dwAttr;
  BOOL done;
  DWORD dwFlags;
  int err;

  DEBUG_WENTER((L"symlink(\"%s\", \"%s\");\n", targetName, linkName));

  /* Work around an incompatibility between Unix and Windows:
  // Windows needs to know if the target is a file or a directory;
  // But Unix allows creating dangling links, in which case we cannot guess what type it'll be. */
  dwAttr = GetFileAttributesW(targetName);
  DEBUG_WPRINTF((L"GetFileAttributes(\"%ls\") = 0x%lX\n", targetName, dwAttr));
  dwFlags = 0;
  if (dwAttr != INVALID_FILE_ATTRIBUTES) {	/* File exists */
    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) dwFlags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
  } else { /* Target does not exst. Use a heuristic: Names with trailing / or \ are directories */
    size_t len = lstrlenW(targetName);
    if (len) {
      WCHAR c = targetName[len-1];
      if ((c == L'/') || (c == L'\\')) dwFlags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
    }
  }

  DEBUG_WPRINTF((L"CreateSymbolicLink(\"%ls\", \"%ls\", 0x%lX)\n", linkName, targetName, dwFlags));
  done = CreateSymbolicLinkW(linkName, targetName, dwFlags);

  if (done) {
    err = 0;
  } else {
    errno = Win32ErrorToErrno();
    err = -1;
  }
  RETURN_INT_COMMENT(err, ("%s\n", err?"Failed to create link":"Created link successfully"));
}

/* Posix routine symlink - MultiByte char version */
int symlinkM(const char *targetName, const char *linkName, UINT cp) {
  WCHAR *pwszLink;
  WCHAR *pwszTarget;
  int iRet;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszLink = MultiByteToNewWidePath(cp,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
				    linkName	/* lpMultiByteStr, */
				    );
  if (!pwszLink) {
    /* errno = Win32ErrorToErrno();		// errno is already set by MultiByteToNewWidePath() */
    return -1;
  }
  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszTarget = MultiByteToNewWidePath(cp,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
				      targetName	/* lpMultiByteStr, */
				      );
  if (!pwszTarget) {
    /* errno = Win32ErrorToErrno();		// errno is already set by MultiByteToNewWidePath() */
    free(pwszLink);
    return -1;
  }

  iRet = symlinkW(pwszTarget, pwszLink);
  free(pwszLink);
  free(pwszTarget);
  return iRet;  
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    symlinkd						      |
|									      |
|   Description:    Create an NTFS symbolic directory link                    |
|									      |
|   Parameters:     const TCHAR *targetName	The symlink target name       |
|		    const TCHAR *linkName	The symlink name	      |
|									      |
|   Returns:	    0 = Success, -1 = Failure				      |
|									      |
|   Notes:	    							      |
|									      |
|   History:								      |
|    2014-03-04 JFL Created this routine                                      |
*									      *
\*---------------------------------------------------------------------------*/

/* MsvcLibX-specific routine to create an NTFS symlinkd - Wide char version */
int symlinkdW(const WCHAR *targetName, const WCHAR *linkName) {
  BOOL done;
  int err;

  DEBUG_WENTER((L"symlinkd(\"%s\", \"%s\");\n", targetName, linkName));

  done = CreateSymbolicLinkW(linkName, targetName, SYMBOLIC_LINK_FLAG_DIRECTORY);

  if (done) {
    err = 0;
  } else {
    errno = Win32ErrorToErrno();
    err = -1;
  }
  RETURN_INT_COMMENT(err, ("%s\n", err?"Failed to create link":"Created link successfully"));
}

/* MsvcLibX-specific routine to create an NTFS symlinkd - MultiByte char version */
int symlinkdM(const char *targetName, const char *linkName, UINT cp) {
  WCHAR *pwszLink;
  WCHAR *pwszTarget;
  int iRet;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszLink = MultiByteToNewWidePath(cp,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
				    linkName	/* lpMultiByteStr, */
				    );
  if (!pwszLink) {
    /* errno = Win32ErrorToErrno();		// errno is already set by MultiByteToNewWidePath() */
    return -1;
  }
  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszTarget = MultiByteToNewWidePath(cp,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
				      targetName	/* lpMultiByteStr, */
				      );
  if (!pwszTarget) {
    /* errno = Win32ErrorToErrno();			// errno is already set by MultiByteToNewWidePath() */
    free(pwszLink);
    return -1;
  }

  iRet = symlinkdW(pwszTarget, pwszLink);
  free(pwszLink);
  free(pwszTarget);
  return iRet;
}

#endif

