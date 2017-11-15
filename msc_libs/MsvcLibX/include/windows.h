/*****************************************************************************\
*                                                                             *
*   Filename:	    windows.h						      *
*                                                                             *
*   Description:    Define MsvcLibX' extensions to the WIN32 API functions    *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2016-09-12 JFL Created this file.					      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_WINDOWS_H
#define	_MSVCLIBX_WINDOWS_H	1

#include "msvclibx.h"

#ifdef _WIN32

#include WINSDK_INCLUDE_FILE(windows.h) /* Include Windows SDK's own windows.h */

/****************** Define UTF-8 versions of WIN32 routines ******************/

#ifdef  __cplusplus
extern "C" {
#endif

DWORD WINAPI GetFileAttributesU(LPCTSTR lpFileName);
BOOL  WINAPI GetFileAttributesExU(LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
DWORD WINAPI GetFullPathNameU(LPCTSTR lpName, DWORD nBufferLength, LPTSTR lpBuf, LPTSTR *lpFilePart);
DWORD WINAPI GetLongPathNameU(LPCTSTR lpShortName, LPTSTR lpBuf, DWORD nBufferLength);

#ifdef  __cplusplus
}
#endif

/********** Redefine the legacy names to point to the UTF-8 version **********/

#if defined(_UTF8_SOURCE)

#undef GetFileAttributes
#define GetFileAttributes GetFileAttributesU

#undef GetFileAttributesEx
#define GetFileAttributesEx GetFileAttributesExU

#undef GetFullPathName
#define GetFullPathName GetFullPathNameU

#undef GetLongPathName
#define GetLongPathName GetLongPathNameU

#endif /* defined(_UTF8_SOURCE) ... */

#endif /* defined(_WIN32) */

#endif /* defined(_MSVCLIBX_WINDOWS_H) */

