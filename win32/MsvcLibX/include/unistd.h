/*****************************************************************************\
*                                                                             *
*   Filename:	    unistd.h						      *
*                                                                             *
*   Description:    DOS/WIN32 port of standard C library's unistd.h.	      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2012-01-18 JFL Created this file.                                        *
*    2012-10-17 JFL Added standard functions getcwd, chdir, getpid, access.   *
*                   Added Microsoft-specific functions getdrive, chdrive.     *
*    2013-03-27 JFL Updated getpid() and added getppid().                     *
*    2014-02-03 JFL Added readlink().                                         *
*    2014-03-20 JFL Restructured Windows link management functions into Wide  *
*		    and MultiByte versions, and changed the Unicode and Ansi  *
*		    versions to macros.					      *
*    2014-06-30 JFL Moved PATH_MAX definition to limits.h.		      *
*    2016-08-25 JFL Implemented ResolveLinksA().			      *
*    2017-03-01 JFL Added more standard routines MS thinks are proprietary.   *
*		    Added routine getpagesize().			      *
*    2017-03-18 JFL Added rmdir() & unlink().                                 *
*    2017-03-24 JFL Added ResolveTailLinks*() prototypes.                     *
*    2018-04-26 JFL Added getcwdW() and ConcatPathW() prototypes.             *
*    2020-12-11 JFL Added ReadAppExecLink*() prototypes.                      *
*    2021-11-29 JFL Prefix MsvcLibX-specific WIN32 public routines with Mlx.  *
*    2025-08-03 JFL Added routines MlxReadWci*() and routines for managing    *
*		    the Place Holder Compatibility Mode.                      *
*    2025-11-11 JFL For DOS, redefine the getcwd() macro as our getcwdX(),    *
*		    and added a chdir() macro defined as the new chdirX().    *
*    2025-12-03 JFL Added routine getcwd0().                       	      *
*    2026-01-22 JFL Added MlxResolveSubstDrives(), MlxGetFileName().	      *
*    2026-01-23 JFL Moved realpath() definitions to stdlib.h.		      *
*		    							      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _UNISTD_H
#define _UNISTD_H

#ifndef _MSC_VER
#error This include file is designed for use with the Microsoft C tools only.
#endif

#include "msvclibx.h"	/* Generate a library search record to load MsvcLibX.lib. */
#include "sys/types.h"	/* Define pid_t and getppid(). */
#include "dirent.h"	/* Define pid_t and getppid(). */
#include <direct.h>	/* For functions like _chdir() and _getcwd() */
#include <process.h>	/* For _getpid() */
#include <io.h>		/* For low level I/O functions like read() & write() */

/* Microsoft tools think these are non standard, but they are standard! */
#define close	_close
#define dup	_dup
#define dup2	_dup2
#define isatty	_isatty
#define read	_read
#define write	_write
/* #define getcwd	 _getcwd */
/* #define chdir	 _chdir */
/* Actually use the improved versions in MsvcLibX */
#undef getcwd /* MSVC _does_ define a getcwd macro, in addition to the getcwd function! */
#undef chdir
#if defined(_MSDOS)
char *getcwdX(char *pszBuf, int iBufLen);	/* MsvcLibX' modified getcwd */
#define getcwd(path, size) getcwdX(path, (int)(size)) /* Either _getcwd = MSVC's version, or getcwdX = ours */
int chdirX(const char *path);			/* MsvcLibX' modified chdir */
#define chdir(path) chdirX(path) /* Either _chdir = MSVC's version, or chdirX = ours */
#elif defined(_WIN32)
#if defined(_UTF8_SOURCE)
#define getcwd getcwdU
#define _getdcwd _getdcwdU
#define chdir chdirU
#define rmdir rmdirU
#define unlink unlinkU
#else /* _ANSI_SOURCE */
#define getcwd getcwdA
#define _getdcwd _getdcwdA
#define chdir chdirA
#define rmdir rmdirA
#define unlink unlinkA
#endif
char *getcwdA(char *buf, size_t bufSize); /* Can't use the getcwd name, as MSVC has an incompatible prototype for it */
char *_getdcwdA(int iDrive, char *buf, int iBuflen);
char *getcwdU(char *buf, size_t bufSize); /* Can't use the getcwd name, as MSVC has an incompatible prototype for it */
char *_getdcwdU(int iDrive, char *buf, int iBuflen);
WCHAR *getcwdW(WCHAR *pwBuf, size_t bufSize);
int chdirA(const char *path);
int chdirU(const char *path);
int chdirM(const char *path, UINT cp);
int chdirW(const WCHAR *pwszPath);
int rmdirA(const char *path);
int rmdirU(const char *path);
int unlinkA(const char *path);
int unlinkU(const char *path);
#endif /* defined(_WIN32) */
char *getcwd0(void);	/* Allocate a new string containing the current directory */

/* These are non standard indeed, but the leading _ is annoying */ 
#define getdrive _getdrive
#define chdrive	 _chdrive

/* Microsoft tools think access is non standard, but it is standard! */
#if defined(_MSDOS)
#define access _access
#elif defined(_WIN32)
#if defined(_UTF8_SOURCE)
#define access _accessU
int _accessU(const char *pszFilename, int iAccessMode);
#else /* _ANSI_SOURCE */
#define access _access
#endif
#endif
#define F_OK 0	/* test for the existence of the file */
#define X_OK 1	/* test for execute permission */
#define R_OK 2	/* test for read permission */
#define W_OK 4	/* test for read permission */

/* getpid() and getppid() */
#define getpid() ((pid_t)(_getpid()))
pid_t getppid(void);	/* Get parent PID */
/* Contrary to in Unix, the above functions can fail in Windows. In this case they return INVALID_PID. */ 
#define INVALID_PID ((pid_t)-1L)

/* Path management */
#if defined(_WIN32)
#if defined(_UTF8_SOURCE)
#define CompactPath CompactPathU
#else /* _ANSI_SOURCE */
#define CompactPath CompactPathA
#endif
int CompactPathW(const WCHAR *path, WCHAR *outbuf, size_t bufsize); /* A proprietary subroutine, that cleans up . and .. parts. */
WCHAR *ConcatPathW(const WCHAR *pwszHead, const WCHAR *pwszTail, WCHAR *pwszBuf, size_t lBuf); /* A proprietary subroutine, that concatenates two paths. */
#endif /* defined(_WIN32) */
int CompactPath(const char *path, char *outbuf, size_t bufsize); /* A proprietary subroutine, that cleans up . and .. parts. */

/* Signed size type */
#ifndef _SSIZE_T_DEFINED
#if defined(_MSDOS)
typedef int ssize_t;
#elif defined(_WIN32)
#include <stdint.h>
#ifdef  _WIN64
typedef __int64 ssize_t;
#else
typedef _W64 int ssize_t;
#endif /* defined(_WIN64) */
#endif /* defined(_WIN32) */
#define _SSIZE_T_DEFINED
#endif /* !_SSIZE_T_DEFINED */

/* Link management functions */
#if defined(_WIN32)
#if defined(_UTF8_SOURCE)
#define readlink readlinkU
#define symlink symlinkU
#define symlinkd symlinkdU
#define junction junctionU
#define MlxGetReparseTag MlxGetReparseTagU
#define MlxResolveLinks MlxResolveLinksU
#define MlxResolveTailLinks MlxResolveTailLinksU
#define MlxReadLink MlxReadLinkU
#define MlxReadAppExecLink MlxReadAppExecLinkU
#define MlxReadWci MlxReadWciU
#define MlxResolveSubstDrives MlxResolveSubstDrivesU
#define MlxGetFileName MlxGetFileNameU
#else /* _ANSI_SOURCE */
#define readlink readlinkA
#define symlink symlinkA
#define symlinkd symlinkdA
#define junction junctionA
#define MlxGetReparseTag MlxGetReparseTagA
#define MlxResolveLinks MlxResolveLinksA
#define MlxResolveTailLinks MlxResolveTailLinksA
#define MlxReadLink MlxReadLinkA
#define MlxReadAppExecLink MlxReadAppExecLinkA
#define MlxReadWci MlxReadWciA
#define MlxResolveSubstDrives MlxResolveSubstDrivesA
#define MlxGetFileName MlxGetFileNameA
#endif
ssize_t readlinkW(const WCHAR *path, WCHAR *buf, size_t bufsiz);		/* Posix routine readlink - Wide char version */
ssize_t readlinkM(const char *path, char *buf, size_t bufsiz, UINT cp);		/* Posix routine readlink - Multibyte char version */
#define readlinkA(path, buf, bufsiz) readlinkM(path, buf, bufsiz, CP_ACP)	/* Posix routine readlink - ANSI version */
#define readlinkU(path, buf, bufsiz) readlinkM(path, buf, bufsiz, CP_UTF8)	/* Posix routine readlink - UTF-8 version */
int symlinkW(const WCHAR *targetname, const WCHAR *newlinkname);		/* Posix routine symlink - Wide char version */
int symlinkM(const char *targetname, const char *newlinkname, UINT cp);		/* Posix routine symlink - Multibyte char version */
#define symlinkA(target, newlink) symlinkM(target, newlink, CP_ACP)		/* Posix routine symlink - ANSI version */
#define symlinkU(target, newlink) symlinkM(target, newlink, CP_UTF8)		/* Posix routine symlink - UTF-8 version */
int symlinkdW(const WCHAR *targetname, const WCHAR *newlinkname);		/* MsvcLibX Create an NTFS symlinkd - Wide char version */
int symlinkdM(const char *targetname, const char *newlinkname, UINT cp);	/* MsvcLibX Create an NTFS symlinkd - Multibyte char version */
#define symlinkdA(target, newlink) symlinkdM(target, newlink, CP_ACP)		/* MsvcLibX Create an NTFS symlinkd - ANSI version */
#define symlinkdU(target, newlink) symlinkdM(target, newlink, CP_UTF8)		/* MsvcLibX Create an NTFS symlinkd - UTF-8 version */
int junctionW(const WCHAR *targetname, const WCHAR *junctionName);		/* MsvcLibX Create an NTFS junction - Wide char version */
int junctionM(const char *targetname, const char *junctionName, UINT cp);	/* MsvcLibX Create an NTFS junction - Multibyte char version */
#define junctionA(target, newjunc) junctionM(target, newjunc, CP_ACP)		/* MsvcLibX Create an NTFS junction - ANSI version */
#define junctionU(target, newjunc) junctionM(target, newjunc, CP_UTF8)		/* MsvcLibX Create an NTFS junction - UTF-8 version */
DWORD MlxGetReparseTagW(const WCHAR *path);					/* MsvcLibX Get a Repase Point tag - Wide char version */
DWORD MlxGetReparseTagM(const char *path, UINT cp);				/* MsvcLibX Get a Repase Point tag - MultiByte char version */
#define MlxGetReparseTagA(path) MlxGetReparseTagM(path, CP_ACP)			/* MsvcLibX Get a Repase Point tag - ANSI version */
#define MlxGetReparseTagU(path) MlxGetReparseTagM(path, CP_UTF8)		/* MsvcLibX Get a Repase Point tag - ANSI version */
DWORD MlxReadLinkW(const WCHAR *path, WCHAR *buf, size_t bufsize);		/* MsvcLibX Read the raw target of an NTFS link - Wide char version */
DWORD MlxReadLinkU(const char *path, char *buf, size_t bufsize);		/* MsvcLibX Read the raw target of an NTFS link - UTF-8 version */
int MlxResolveLinksW(const WCHAR *path, WCHAR *buf, size_t bufsize);		/* Resolve pathnames with symlinks, symlinkds, and junctions - Wide char version */
int MlxResolveLinksM(const char *path, char *buf, size_t bufsize, UINT cp);	/* Resolve pathnames with symlinks, symlinkds, and junctions - MultiByte char version */
#define MlxResolveLinksA(p, buf, sz) MlxResolveLinksM(p, buf, sz, CP_ACP)	/* Resolve pathnames with symlinks, symlinkds, and junctions - ANSI version */
#define MlxResolveLinksU(p, buf, sz) MlxResolveLinksM(p, buf, sz, CP_UTF8)	/* Resolve pathnames with symlinks, symlinkds, and junctions - UTF-8 version */
int MlxResolveTailLinksW(const WCHAR *path, WCHAR *buf, size_t bufsize);	/* Resolve node names with symlinks, symlinkds, and junctions */
int MlxResolveTailLinksM(const char *path, char *buf, size_t bufsize, UINT cp);	/* Resolve node names with symlinks, symlinkds, and junctions */
int MlxResolveTailLinksA(const char *path, char *buf, size_t bufsize);		/* Resolve node names with symlinks, symlinkds, and junctions */
int MlxResolveTailLinksU(const char *path, char *buf, size_t bufsize);		/* Resolve node names with symlinks, symlinkds, and junctions */
int MlxReadAppExecLinkW(const WCHAR *path, WCHAR *buf, size_t bufsize);		/* Get the target of an appexeclink - Wide char version */
int MlxReadAppExecLinkM(const char *path, char *buf, size_t bufsize, UINT cp);	/* Get the target of an appexeclink - MultiByte char version */
#define MlxReadAppExecLinkA(p, buf, sz) MlxReadAppExecLinkM(p, buf, sz, CP_ACP) /* Get the target of an appexeclink - ANSI version */
#define MlxReadAppExecLinkU(p, buf, sz) MlxReadAppExecLinkM(p, buf, sz, CP_UTF8)/* Get the target of an appexeclink - UTF-8 version */
int MlxReadWciW(const WCHAR *path, WCHAR *buf, size_t bufsize);			/* Get the target of a WCI place holder - Wide char version */
int MlxReadWciM(const char *path, char *buf, size_t bufsize, UINT cp);		/* Get the target of a WCI place holder - MultiByte char version */
#define MlxReadWciA(p, buf, sz) MlxReadWciM(p, buf, sz, CP_ACP)			/* Get the target of a WCI place holder - ANSI version */
#define MlxReadWciU(p, buf, sz) MlxReadWciM(p, buf, sz, CP_UTF8)		/* Get the target of a WCI place holder - UTF-8 version */
WCHAR *MlxGetShareBasePathW(const WCHAR *pwszShareUNC);				/* MsvcLibX Guess the server-side path of \\SERVER\SHARE */
int MlxResolveSubstDrivesW(const WCHAR *path, WCHAR *buf, size_t bufsize);		/* Resolve substituted drives - Wide char version */
int MlxResolveSubstDrivesM(const char *path, char *buf, size_t bufsz, UINT cp);		/* Resolve substituted drives - MultiByte char version */
#define MlxResolveSubstDrivesA(p, buf, sz) MlxResolveSubstDrivesM(p, buf, sz, CP_ACP)	/* Resolve substituted drives - ANSI version */
#define MlxResolveSubstDrivesU(p, buf, sz) MlxResolveSubstDrivesM(p, buf, sz, CP_UTF8)	/* Resolve substituted drives - UTF-8 version */
int MlxGetFileNameW(const WCHAR *pwPath, WCHAR *pwOutBuf, size_t lBuf);		/* Get the actual file name directly from Windows - Wide char version */
int MlxGetFileNameM(const char *path, char *buf, size_t bufsz, UINT cp);	/* Get the actual file name directly from Windows - MultiByte char version */
#define MlxGetFileNameA(p, buf, sz) MlxGetFileNameM(p, buf, sz, CP_ACP)		/* Get the actual file name directly from Windows - ANSI version */
#define MlxGetFileNameU(p, buf, sz) MlxGetFileNameM(p, buf, sz, CP_UTF8)	/* Get the actual file name directly from Windows - UTF-8 version */

#define SYMLOOP_MAX 31 /* Maximum depth of symbolic name resolution, to avoid stack overflows. Windows is documented to allow 31: http://msdn.microsoft.com/en-us/library/windows/desktop/aa365460(v=vs.85).aspx */

/* Front end to the RtlSetProcessPlaceholderCompatibilityMode() procedure */
typedef NTSYSAPI CHAR(*PSPPHCMPROC)(CHAR Mode); /* Pointer to the RtlSetProcessPlaceholderCompatibilityMode() procedure */
#ifndef PHCM_APPLICATION_DEFAULT		/* Normally defined in ntifs.h */
#define PHCM_APPLICATION_DEFAULT  ((CHAR)0)
#define PHCM_DISGUISE_PLACEHOLDER ((CHAR)1)
#define PHCM_EXPOSE_PLACEHOLDERS  ((CHAR)2)
#endif
int MlxSetProcessPlaceholderCompatibilityMode(int cMode);
#define MlxShowPlaceholders() MlxSetProcessPlaceholderCompatibilityMode((int)PHCM_EXPOSE_PLACEHOLDERS)
#define MlxHidePlaceholders() MlxSetProcessPlaceholderCompatibilityMode((int)PHCM_DISGUISE_PLACEHOLDER)

#endif /* defined(_WIN32) */

/* Standard file descriptor numbers, for low level I/O functions */
#define STDIN_FILENO    0       /* Standard input file number */
#define STDOUT_FILENO   1       /* Standard output file number */
#define STDERR_FILENO   2       /* Standard error file number */

/* Virtual Memory */
#if defined(_MSDOS)
#define getpagesize() 4096	/* No virtual memory. Return a reasonable default for the 80386. */
#elif defined(_WIN32)
int getpagesize(void);
#endif

#endif /* _UNISTD_H */

