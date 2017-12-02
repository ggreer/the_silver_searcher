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
*									      *
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
#if defined(_MSDOS)
#define getcwd(path, size) _getcwd(path, (int)(size)) /* Use MSVC LIB's own, changing argument 2 type */
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
int rmdirA(const char *path);
int rmdirU(const char *path);
int unlinkA(const char *path);
int unlinkU(const char *path);
#endif /* defined(_WIN32) */
int chdir(const char *path);

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
#define realpath realpathU
#define CompactPath CompactPathU
#else /* _ANSI_SOURCE */
#define realpath realpathA
#define CompactPath CompactPathA
#endif
int CompactPathW(const WCHAR *path, WCHAR *outbuf, size_t bufsize); /* A proprietary subroutine, that cleans up . and .. parts. */
#endif /* defined(_WIN32) */
char *realpath(const char *path, char *buf); /* Posix routine, normally defined in stdlib.h. Output buf must contain PATH_MAX bytes */
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
#define GetReparseTag GetReparseTagU
#define ResolveLinks ResolveLinksU
#define ResolveTailLinks ResolveTailLinksU
#else /* _ANSI_SOURCE */
#define readlink readlinkA
#define symlink symlinkA
#define symlinkd symlinkdA
#define junction junctionA
#define GetReparseTag GetReparseTagA
#define ResolveLinks ResolveLinksA
#define ResolveTailLinks ResolveTailLinksA
#endif
ssize_t readlinkW(const WCHAR *path, WCHAR *buf, size_t bufsiz);	    /* Posix routine readlink - Wide char version */
ssize_t readlinkM(const char *path, char *buf, size_t bufsiz, UINT cp);     /* Posix routine readlink - Multibyte char version */
#define readlinkA(path, buf, bufsiz) readlinkM(path, buf, bufsiz, CP_ACP)   /* Posix routine readlink - ANSI version */
#define readlinkU(path, buf, bufsiz) readlinkM(path, buf, bufsiz, CP_UTF8)  /* Posix routine readlink - UTF-8 version */
int symlinkW(const WCHAR *targetname, const WCHAR *newlinkname);	    /* Posix routine symlink - Wide char version */
int symlinkM(const char *targetname, const char *newlinkname, UINT cp);	    /* Posix routine symlink - Multibyte char version */
#define symlinkA(target, newlink) symlinkM(target, newlink, CP_ACP)	    /* Posix routine symlink - ANSI version */
#define symlinkU(target, newlink) symlinkM(target, newlink, CP_UTF8)	    /* Posix routine symlink - UTF-8 version */
int symlinkdW(const WCHAR *targetname, const WCHAR *newlinkname);	    /* MsvcLibX Create an NTFS symlinkd - Wide char version */
int symlinkdM(const char *targetname, const char *newlinkname, UINT cp);    /* MsvcLibX Create an NTFS symlinkd - Multibyte char version */
#define symlinkdA(target, newlink) symlinkdM(target, newlink, CP_ACP)	    /* MsvcLibX Create an NTFS symlinkd - ANSI version */
#define symlinkdU(target, newlink) symlinkdM(target, newlink, CP_UTF8)	    /* MsvcLibX Create an NTFS symlinkd - UTF-8 version */
int junctionW(const WCHAR *targetname, const WCHAR *junctionName);	    /* MsvcLibX Create an NTFS junction - Wide char version */
int junctionM(const char *targetname, const char *junctionName, UINT cp);   /* MsvcLibX Create an NTFS junction - Multibyte char version */
#define junctionA(target, newjunc) junctionM(target, newjunc, CP_ACP)	    /* MsvcLibX Create an NTFS junction - ANSI version */
#define junctionU(target, newjunc) junctionM(target, newjunc, CP_UTF8)	    /* MsvcLibX Create an NTFS junction - UTF-8 version */
DWORD GetReparseTagW(const WCHAR *path);				    /* MsvcLibX Get a Repase Point tag - Wide char version */
DWORD GetReparseTagM(const char *path, UINT cp);			    /* MsvcLibX Get a Repase Point tag - MultiByte char version */
#define GetReparseTagA(path) GetReparseTagM(path, CP_ACP)		    /* MsvcLibX Get a Repase Point tag - ANSI version */
#define GetReparseTagU(path) GetReparseTagM(path, CP_UTF8)		    /* MsvcLibX Get a Repase Point tag - ANSI version */
int ResolveLinksM(const char *path, char *buf, size_t bufsize, UINT cp);    /* Resolve pathnames with symlinks, symlinkds, and junctions */
int ResolveLinksA(const char *path, char *buf, size_t bufsize);		    /* Resolve pathnames with symlinks, symlinkds, and junctions */
int ResolveLinksU(const char *path, char *buf, size_t bufsize);		    /* Resolve pathnames with symlinks, symlinkds, and junctions */
int ResolveTailLinksW(const WCHAR *path, WCHAR *buf, size_t bufsize);	    /* Resolve node names with symlinks, symlinkds, and junctions */
int ResolveTailLinksM(const char *path, char *buf, size_t bufsize, UINT cp);/* Resolve node names with symlinks, symlinkds, and junctions */
int ResolveTailLinksA(const char *path, char *buf, size_t bufsize);	    /* Resolve node names with symlinks, symlinkds, and junctions */
int ResolveTailLinksU(const char *path, char *buf, size_t bufsize);	    /* Resolve node names with symlinks, symlinkds, and junctions */
#ifndef ELOOP
/*
// Unix defines ELOOP as errno 40.
// MS Visual C++ 1.52 for DOS is standard up to errno 34, then diverges up to errno 36.
//  Many errnos within the list are actually unused, and for them _sys_errlist[] = "".
// MS Visual C++ 9 for Windows is standard up to errno 34, then diverges up to errno 43.
//  Also MSVC9 does not define errno:
//   15      // The posix standard ENOTBLK "Block device required"
//   26      // The posix standard ETXTBSY "Text file busy"
//   35      // Positioned between standard ERANGE and EDEADLK
//   37      // Positioned between standard EDEADLK and ENAMETOOLONG
//   43      // Positioned last, after standard ENOTEMPTY
// The _sys_errlist[] pointer for all the above points to a single string "Unknown error".
*/
#define ELOOP  35  /* Using the first available slot */  /* Update _sys_errlist[ELOOP] accordingly in any routine that generates ELOOP! */
#endif /* !defined(ELOOP) */
#define SYMLOOP_MAX 31 /* Maximum depth of symbolic name resolution, to avoid stack overflows. Windows is documented to allow 31: http://msdn.microsoft.com/en-us/library/windows/desktop/aa365460(v=vs.85).aspx */
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

