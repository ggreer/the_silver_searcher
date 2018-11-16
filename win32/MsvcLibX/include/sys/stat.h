/*****************************************************************************\
*                                                                             *
*   Filename:	    stat.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to sys/stat.h.                        *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-02-06 JFL Moved stat extensions from dirent.h.		      *
*    2014-02-26 JFL Added proprietary routines for managing file times.       *
*    2014-03-24 JFL Added a mechanism to also include MSVC's own sys/stat.h.  *
*                   Renamed this file from statx.h to sys/stat.h.             *
*    2014-05-27 JFL Added dummy definitions for S_TYPEISSHM() & S_TYPEISTMO().*
*    2014-06-03 JFL Moved struct timespec definition to sys/time.h.           *
*    2014-06-06 JFL Moved mode_t & off*_t definition to sys/types.h.	      *
*		    Moved fseeko* & ftello* functions definitions to stdio.h. *
*    2014-06-24 JFL Added fstat and fstat64 external references.	      *
*    2015-11-15 JFL Visual Studio 2015 moved this file to the Windows Kit UCRT.
*    2016-09-15 JFL Fixed a warning in Visual Studio 2015.		      *
*    2017-03-20 JFL Moved unlink & rmdir definitions for unistd.h.	      *
*    2017-09-01 JFL Bug fix: Sockets and Fifos ARE supported in WIN32. Enable *
*		    macros S_ISSOCK and S_ISFIFO.			      *
*    2018-05-31 JFL Changed dirent2stat() first arg to (const _dirent *).     *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_STAT_H
#define	_MSVCLIBX_STAT_H	1

#include "msvclibx.h"

#include <sys/types.h>
#include UCRT_INCLUDE_FILE(sys\stat.h) /* Include MSVC's own <sys/stat.h> file */
#include <dirent.h> /* For dirent2stat() arguments definitions */
#include <time.h> /* for time_t definition */
#include <sys/time.h> /* for timespec definition */
/* Include MsvcLibX's <direct.h> override, to avoid conflict with the standard mkdir defined here,
   should <direct.h> be manually included later on in the C source */
#include <direct.h>

#ifdef  __cplusplus
extern "C" {
#endif

/************************ MS-DOS-specific definitions ************************/

#ifdef _MSDOS	/* Automatically defined when targeting an MS-DOS application */

/* Define stat and fstat to use 32 or 64 file lengths, as defined by _FILE_OFFSET_BITS */
/* #undef  stat		/* Not normally defined by MSVC */
/* #undef  fstat	/* Not normally defined by MSVC */
#if defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64)
  /* TO DO: Windows 95 has extended functions for handling 64-bits files sizes */
#else
#endif
/* For now, use the MSVC 32-bits functions in all cases */
#define stat _stat
#define fstat _fstat
#define lstat stat

/* Define standard 64-bits structures and functions */
#if defined(_LARGEFILE_SOURCE64)
  /* For now, hide the fact that DOS does not support 64-bits lengths */
  #define off64_t _off_t
  #define stat64 _stat
  #define fstat64 _fstat
  #define lstat64 stat64
#endif

/* Proprietary function for recovering available infos without calling stat */
extern int dirent2stat(const struct dirent *pDE, struct _stat *pStat);
#define dirent2stat64 dirent2stat /* The current versions are the same */
#define _DIRENT2STAT_DEFINED 1

/* MS-DOS always reports local file times. MsvcLibX DOS version generates a
   pseudo GMT time for files using mktime(), which can then be undone by localtime(). */
#define LocalFileTime localtime

/* Proprietary function for converting a DOS date/time to a Posix time_t */
extern time_t Filetime2Timet(uint16_t date, uint16_t time);
/* Proprietary function for generating a string with the local file time, in the ISO 8601 date/time format */
extern char *Filetime2String(uint16_t date, uint16_t time, char *pBuf, size_t nBufSize);

#endif /* defined(_MSDOS) */

/************************ Win32-specific definitions *************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

/* Visual C++ sys/stat.h defines a series of _statXY functions and structures:
    XY = 32	st_*time is 32-bits __time32_t	&  st_size is 32-bits _off_t
    XY = 32i64	st_*time is 32-bits __time32_t	&  st_size is 64-bits __int64
    XY = 64i32	st_*time is 64-bits __time64_t	&  st_size is 32-bits _off_t
    XY = 64	st_*time is 64-bits __time64_t	&  st_size is 64-bits __int64
   Then it defines additional _statZ macros generating _statXY, based on:
    XY = 32	if _USE_32BIT_TIME_T defined	&  Z is ""
    XY = 32i64	if _USE_32BIT_TIME_T defined	&  Z is i64
    XY = 64i32	if _USE_32BIT_TIME_T undefined	&  Z is ""
    XY = 64	if _USE_32BIT_TIME_T undefined	&  Z is i64
   Special case: In 64-bits windows, _USE_32BIT_TIME_T defined is ignored and #undef(ined)
*/

/* Define stat and fstat to use 32 or 64 file lengths, as defined by _FILE_OFFSET_BITS */
/* #undef  stat		/* Not normally defined by MSVC */
/* #undef  fstat	/* Not normally defined by MSVC */

#if defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64)
  #define _STAT_FILE_SIZE 64
  #define _STAT_SUFFIX i64
#else
  #define _STAT_FILE_SIZE 32
  #define _STAT_SUFFIX 
#endif

#define _USE_EXTENDED_STAT_STRUCT 1

#define _VALUEOF(a) a				/* Get the token value */
#define _CONCAT2T(a,b) a##b			/* Concatenate two tokens (does not expand their values) */
#define _CONCAT3T(a,b,c) a##b##c		/* Concatenate three tokens (does not expand their values) */
#define _CONCAT4T(a,b,c,d) a##b##c##d		/* Concatenate four tokens (does not expand their values) */
#define _CONCAT(a,b) _CONCAT2T(a,b)		/* Concatenate two tokens values */
#define _CONCAT3(a,b,c) _CONCAT3T(a,b,c)	/* Concatenate three tokens values */
#define _CONCAT4(a,b,c,d) _CONCAT4T(a,b,c,d)	/* Concatenate four tokens values */
#define _CON_STAT_CAT1(pre,post) pre##stat##post
#define _CON_STAT_CAT(pre,post) _CON_STAT_CAT1(pre,post)

/* Define what stat, fstat, lstat, stat64 macros would be,
   if we were to use MSVC's own stat structures and routines */
/* Avoid concatenating tokens _stat and _fstat, as these are already macros
   that would be expanded, and we do not want this as this stage. */ 
#define _MSVC_stat    _CON_STAT_CAT(_ , _STAT_SUFFIX)
#define _MSVC_fstat   _CON_STAT_CAT(_f, _STAT_SUFFIX)
#define _MSVC_lstat   _CON_STAT_CAT(_l, _STAT_SUFFIX)
#define _MSVC_stat64  _CON_STAT_CAT(_ , i64)
#define _MSVC_lstat64 _CON_STAT_CAT(_l, i64)

#ifdef _USE_32BIT_TIME_T
#define _lstat       _lstat32
#define _lstati64    _lstat32i64
#define _lstat_ns    _lstat32_ns
#define _lstati64_ns _lstat32i64_ns
#else
#define _lstat       _lstat64i32
#define _lstati64    _lstat64
#define _lstat_ns    _lstat64i32_ns
#define _lstati64_ns _lstat64_ns
#endif

#if !_USE_EXTENDED_STAT_STRUCT
  #define _NS_SUFFIX
#else /* _USE_EXTENDED_STAT_STRUCT */
  #define _MSVCLIBX_STAT_DEFINED 1
  #define _NS_SUFFIX _ns
  /* Define what stat, fstat, lstat, stat64 macros and structures would be,
     if we were to use MsvcLibX extended stat structures and routines */
  #define _LIBX_stat   _CONCAT(_MSVC_stat,_ns)
  #define _LIBX_stat64 _CONCAT(_MSVC_stat64,_ns)
#include "debugm.h"
#pragma message("Defining type struct " VALUEIZE(_LIBX_stat))
  struct _LIBX_stat {
    /* MSVC standard stat structure fields */
    _dev_t          st_dev;
    _ino_t          st_ino;
    unsigned short  st_mode;
    short           st_nlink;
    short           st_uid;
    short           st_gid;
    _dev_t          st_rdev;
    off_t           st_size;
    /* End of MSVC standard stat structure fields */
    struct timespec st_ctim;		/* File creation date/time,  w. ns resolution */
    struct timespec st_mtim;		/* File modification date/time,  w. ns resolution */
    struct timespec st_atim;		/* File access date/time,  w. ns resolution */
    unsigned int    st_Win32Attrs;	/* Win32 file attributes */
    unsigned int    st_ReparseTag;	/* Reparse point tag */
  };
  /* Compatibility with old Unix fields */
  #define st_ctime st_ctim.tv_sec
  #define st_mtime st_mtim.tv_sec
  #define st_atime st_atim.tv_sec
  /* Compatibility with intermediate Unix fields */
  #define st_ctimensec st_ctim.tv_nsec	/* Nanosecond part of the file creation time */
  #define st_mtimensec st_mtim.tv_nsec	/* Nanosecond part of the file modification time */
  #define st_atimensec st_atim.tv_nsec	/* Nanosecond part of the file access time */

  #if (_STAT_FILE_SIZE != 64) /* Else it's the same as struct stat */
#pragma message("Defining type struct " VALUEIZE(_LIBX_stat64))
    struct _LIBX_stat64 {
      /* MSVC standard stat structure fields */
      _dev_t          st_dev;
      _ino_t          st_ino;
      unsigned short  st_mode;
      short           st_nlink;
      short           st_uid;
      short           st_gid;
      _dev_t          st_rdev;
      off64_t         st_size;
      /* End of MSVC standard stat structure fields */
      struct timespec st_ctim;		/* File creation date/time,  w. ns resolution */
      struct timespec st_mtim;		/* File modification date/time,  w. ns resolution */
      struct timespec st_atim;		/* File access date/time,  w. ns resolution */
      unsigned int    st_Win32Attrs;	/* Win32 file attributes */
      unsigned int    st_ReparseTag;	/* Reparse point tag */
    };
  #endif
#endif /* !_USE_EXTENDED_STAT_STRUCT */

/* Define standard stat, fstat, lstat macros */
#define stat    _CONCAT(_MSVC_stat,_NS_SUFFIX)
#define fstat   _CONCAT(_MSVC_fstat,_NS_SUFFIX)
#define lstat   _CONCAT(_MSVC_lstat,_NS_SUFFIX)

/* Reference standard stat, fstat, lstat functions that we redefine */
#if _USE_EXTENDED_STAT_STRUCT /* Else we'll use MSVC's version */
extern int stat(const char *path, struct stat *buf);
extern int fstat(int nFile, struct stat *buf);
#endif
extern int lstat(const char *path, struct stat *buf);

/* Define standard 64-bits macros and functions */
#if defined(_LARGEFILE_SOURCE64)
  #define stat64 _CONCAT(_stati64,_NS_SUFFIX)
  #define fstat64 _CONCAT(_fstati64,_NS_SUFFIX)
  #define lstat64 _CONCAT(_lstati64,_NS_SUFFIX)

  #if (_STAT_FILE_SIZE != 64) /* Else they're the same as functions stat & lstat */
    #if _USE_EXTENDED_STAT_STRUCT /* Else we'll use MSVC's version */
      extern int stat64(const char *path, struct stat64 *buf);
      extern int fstat64(int nFile, struct stat *buf);
    #endif
    extern int lstat64(const char *path, struct stat64 *buf);
  #endif
#endif

/* Proprietary function for recovering dirent infos without calling stat */
/* Note: MSDOS has a single stat version, and its dirent2stat implementation is in dirent.c */
#ifdef _USE_32BIT_TIME_T
#if (_STAT_FILE_SIZE != 64) /* Else if _STAT_FILE_SIZE=64, then struct _stat32_ns isn't defined */
extern int _CONCAT(_dirent2_stat32,_NS_SUFFIX)(const struct dirent *pDE, struct _CONCAT(_stat32,_NS_SUFFIX) *pStat);
#endif
extern int _CONCAT(_dirent2_stat32i64,_NS_SUFFIX)(const struct dirent *pDE, struct _CONCAT(_stat32i64,_NS_SUFFIX) *pStat);
#else
#if (_STAT_FILE_SIZE != 64) /* Else if _STAT_FILE_SIZE=64, then struct _stat64i32_ns isn't defined */
extern int _CONCAT(_dirent2_stat64i32,_NS_SUFFIX)(const struct dirent *pDE, struct _CONCAT(_stat64i32,_NS_SUFFIX) *pStat);
#endif
extern int _CONCAT(_dirent2_stat64,_NS_SUFFIX)(const struct dirent *pDE, struct _CONCAT(_stat64,_NS_SUFFIX) *pStat);
#endif
#define dirent2stat _CONCAT(_dirent2,stat)
#define _DIRENT2STAT_DEFINED 1

/* Proprietary function for generating a printable local file time,
   using Windows' specific algorithm, to match cmd.exe dir output. */
extern struct tm *LocalFileTime(const time_t *pt);

/* Proprietary function for converting a Win32 FILETIME to a Posix time_t */
extern time_t Filetime2Timet(const FILETIME *pFT);
/* Proprietary function for converting a Posix time_t to a Win32 FILETIME */
extern void Timet2Filetime(time_t s, FILETIME *pFT);

/* Proprietary function for converting a Win32 FILETIME to a Posix {time_t,nanosecond} */
extern void Filetime2Timespec(const FILETIME *pFT, struct timespec *pTS);
/* Proprietary function for converting a Posix {time_t,nanosecond} to a Win32 FILETIME */
extern void Timespec2Filetime(const struct timespec *pTS, FILETIME *pFT);

/* Proprietary function for generating a string with the local file time, in the ISO 8601 date/time format */
extern char *Filetime2String(const FILETIME *pFT, char *pBuf, size_t nBufSize);

#endif /* defined(_WIN32) */

/************************* OS/2-specific definitions *************************/

#ifdef _OS2	/* Automatically defined when targeting an OS/2 application? */

/* Define stat and fstat to use 32 or 64 file lengths, as defined by _FILE_OFFSET_BITS */
/* #undef  stat		/* Not normally defined by MSVC */
/* #undef  fstat	/* Not normally defined by MSVC */
#if defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64)
  /* TO DO: Windows 95 has extended functions for handling 64-bits files sizes */
#else
#endif
/* For now, use the MSVC 32-bits functions in all cases */
#define off_t _off_t
#define stat _stat
#define fstat _fstat
#define lstat stat

/* Define standard 64-bits functions */
#if defined(_LARGEFILE_SOURCE64)
  /* For now, hide the fact that DOS does not support 64-bits lengths */
  #define off64_t _off_t
  #define stat64 _stat
  #define fstat64 _fstat
  #define lstat64 stat64
#endif

/* Proprietary function for recovering available infos without calling stat */
/* extern int dirent2stat(struct dirent *pDE, struct _stat *pStat); */
/* #define dirent2stat64 dirent2stat /* The current versions are the same */
/* #define _DIRENT2STAT_DEFINED 1 */

/* To do: Check if OS/2 uses local or GMT times, and implement LocalFileTime() if needed. */
#define LocalFileTime localtime

#endif /* defined(_OS2) */

/********************** End of OS-specific definitions ***********************/

#define lchmod chmod /* MSVC is buggy and chmod() applies to the link itself */

/* Structure stat standard st_mode values not defined in DOS/Windows' sys/stat.h */
/* Must match the d_type field types in dirent.h. */
/* #define S_IFMT  0xF000   /* Mask for the 4-bit file type                    */
#define S_IFIFO    0x1000   /* FIFO                                            */
/* #define S_IFCHR 0x2000   /* Character device                                */
/* #define S_IFDIR 0x4000   /* Directory                                       */
#define S_IFBLK    0x6000   /* Block device                                    */
/* #define S_IFREG 0x8000   /* Regular file                                    */
#define S_IFLNK    0xA000   /* Symbolic link                                   */
#define S_IFSOCK   0xC000   /* Socket                                          */

/* Structure stat standard st_mode type test macros */
/* The existence of these macros can be used to test for the OS support of the feature */
#define S_ISTYPE(m, TYPE) ((m & S_IFMT) == TYPE)
/* Both DOS and Windows support these file types */
#define S_ISCHR(m)  S_ISTYPE(m, S_IFCHR)  /* Test for a character device */
#define S_ISDIR(m)  S_ISTYPE(m, S_IFDIR)  /* Test for a directory */
#define S_ISREG(m)  S_ISTYPE(m, S_IFREG)  /* Test for a regular file */
#ifdef _WIN32	/* Only Windows supports these */
#define S_ISLNK(m)  S_ISTYPE(m, S_IFLNK)  /* Test for a symbolic link */
#define S_ISFIFO(m) S_ISTYPE(m, S_IFIFO)  /* Test for a pipe or FIFO */
#define S_ISSOCK(m) S_ISTYPE(m, S_IFSOCK) /* Test for a socket */
#else		/* DOS does not */
#define S_ISLNK(m) 0	/* Test for a symbolic link */
#define S_ISFIFO(m) 0	/* Test for a pipe or FIFO */
#define S_ISSOCK(m) 0	/* Test for a socket */
#endif
/* All the following aren't supported in DOS or WIN32 */
#define S_ISBLK(m)  0	/* Test for a block device */
#define S_ISCTG(m)  0	/* Test for a high performance ("contiguous data") file */
#define S_ISDOOR(m) 0	/* Test for a door */
#define S_ISMPB(m)  0	/* Test for a multiplexed block device */
#define S_ISMPC(m)  0	/* Test for a multiplexed character device */
#define S_ISNWK(m)  0	/* Test for a network special file (HP-UX) */
#define S_ISPORT(m) 0	/* Test for a port */
#define S_ISWHT(m)  0	/* Test for a whiteout (4.4BSD) */


/* Other optional file types that some operating systems support */ 
#define S_TYPEISMQ(pStat) 0	/* Test for a message queue */
#define S_TYPEISSEM(pStat) 0	/* Test for a semaphore */
#define S_TYPEISSHM(pStat) 0	/* Test for a shared memory object */
#define S_TYPEISTMO(pStat) 0	/* Test for a typed memory object */

/* Structure stat standard st_mode flags */
#define S_ISUID    04000    /* Set UID bit                                     */
#define S_ISGID    02000    /* Set-group-ID bit                                */
#define S_ISVTX    01000    /* Sticky bit                                      */

#define S_IRWXU    0700     /* Mask for file owner permissions                 */
#define S_IRUSR    0400     /* Owner has read permission.    Alias S_IREAD     */
#define S_IWUSR    0200     /* Owner has write permission.   Alias S_IWRITE    */
#define S_IXUSR    0100     /* Owner has execute permission. Alias S_IEXEC     */

#define S_IRWXG    070      /* Mask for group permissions                      */
#define S_IRGRP    040      /* Group has read permission                       */
#define S_IWGRP    020      /* Group has write permission                      */
#define S_IXGRP    010      /* Group has execute permission                    */

#define S_IRWXO    07       /* Mask for permissions for others (not in group)  */
#define S_IROTH    04       /* Others have read permission                     */
#define S_IWOTH    02       /* Others have write permission                    */
#define S_IXOTH    01       /* Others have execute permission                  */

#define S_IRWXUGO  00777    /* S_IRWXU|S_IRWXG|S_IRWXO */
#define S_IALLUGO  07777    /* S_ISUID|S_ISGID|S_ISVTX|S_IRWXUGO */
#define S_IRUGO    00444    /* S_IRUSR|S_IRGRP|S_IROTH */
#define S_IWUGO    00222    /* S_IWUSR|S_IWGRP|S_IWOTH */
#define S_IXUGO    00111    /* S_IXUSR|S_IXGRP|S_IXOTH */

/* Structure stat extensions for DOS/Windows file attributes */
#define S_IFVOLID  0xF000   /* Volume ID pseudo-file, defined in FAT root dir */
#define S_ISVOLID(m) S_ISTYPE(m, S_IFVOLID) /* Test for a FAT volume ID */
#define S_HIDDEN		01	/* Reuse permissions bit not used in DOS/Windows */
#define S_ARCHIVE		02	/* Reuse permissions bit not used in DOS/Windows */
#define S_SYSTEM		04	/* Reuse permissions bit not used in DOS/Windows */
#define S_COMPRESSED		010	/* Reuse permissions bit not used in DOS/Windows */
#define S_ENCRYPTED		020	/* Reuse permissions bit not used in DOS/Windows */
#define S_NOT_CONTENT_INDEXED	040	/* Reuse permissions bit not used in DOS/Windows */
#define S_OFFLINE		01000	/* Reuse sticky bit, not used in DOS/Windows */
#define S_SPARSE_FILE		02000	/* Reuse GID bit, not used in DOS/Windows */
#define S_MOUNT_POINT		04000	/* Reuse UID bit, not used in DOS/Windows */
/* #define S_TEMPORARY			/* Reuse UID bit, not used in DOS/Windows */
/* #define S_VIRTUAL			/* No bit left for this one */

#if defined(_MSDOS)
#define _mkdirx(path, mode) _mkdir(path)
#elif defined(_WIN32)
#if defined(_UTF8_SOURCE)
#define _mkdirx(path, mode) mkdirU(path, mode)
#else /* _ANSI_SOURCE */
#define _mkdirx(path, mode) mkdirA(path, mode)
#endif /* defined(_UTF8_SOURCE) */
extern int mkdirU(const char *path, mode_t mode);
extern int mkdirA(const char *path, mode_t mode);
#endif /* defined(_MSDOS) */
#define mkdir(path, mode) _mkdirx(path, mode)

/* Special values for futimens() and utimensat(). Must not be in [0...((10^9)-1)] */
#define UTIME_NOW  1000000001
#define UTIME_OMIT 1000000002

#ifdef  __cplusplus
}
#endif

#endif /* defined(_MSVCLIBX_STAT_H)  */

