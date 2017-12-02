/*****************************************************************************\
*                                                                             *
*   Filename:	    dirent.h						      *
*                                                                             *
*   Description:    DOS/WIN32 port of standard C library's dirent.h.	      *
*                                                                             *
*   Notes:	    There are also remains of an OS/2 implementation here.    *
*                   This code did work c. 1990, in an OS/2 1.3 port of dirc.c.*
*		    It's not maintained anymore, and unlikely to still work.  *
*		    Left in as a historic reference, in the very unlikely     *
*		    case somebody wants to revive it.			      *
*									      *
*   History:								      *
*    2012-01-09 JFL Created this file, based on dirc and other programs.      *
*    2013-03-09 JFL Rewrote the stat/fstat/lstat and fseeko/ftello definitions*
*                   based on _FILE_OFFSET_BITS, _LARGEFILE_SOURCE, and        *
*		    _LARGEFILE_SOURCE64 definitions.			      *
*    2014-02-06 JFL Moved stat extensions to statx.h.			      *
*    2014-02-27 JFL Use a WIN32_FIND_DATAW in Windows, to support UTF-8 names.*
*    2014-03-20 JFL Restructured Windows opendir and readdir functions into   *
*		    Wide and MultiByte versions, and changed the Unicode and  *
*		    Ansi versions to macros.				      *
*    2014-06-06 JFL Fixed macro _D_EXACT_NAMLEN().			      *
*    2015-12-07 JFL Added the conditional definition of symlink constants, so *
*		    that our code builds even in XP and older Windows SDKs.   *
*    2017-10-02 JFL Removed struct _dirhandle dependency on MAX_PATH.	      *
*		    Renamed it as struct _DIR.				      *
*		    							      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_DIRENT_H
#define	_DIRENT_H	1

#include "msvclibx.h"

#define _DIRENT_FOR_DOS_WINDOWS	/* Uniquely identify this module */

#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#ifndef ENAMETOOLONG /* Not defined in DOS' errno.h */
#define ENAMETOOLONG 38
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* Macros to extract size information from a struct dirent */
#define _D_EXACT_NAMLEN(d) (strlen((d)->d_name)) /* Name length, not including NUL */

#define _DIRENT_HAVE_D_TYPE	/* We commit to providing this field */

/************************ MS-DOS-specific definitions ************************/

#ifdef _MSDOS	/* Automatically defined when targeting an MS-DOS application */

#include <dos.h>

#define NAME_MAX 12

#pragma pack(1)
struct _fileinfo { /* MS-DOS structure returning file search results */
  uint8_t  fiReserved[21];
  uint8_t  fiAttribute;		/* Attributes of file found */
  uint16_t fiFileTime;		/* Time of last write */
  uint16_t fiFileDate;		/* Time of last write */
  uint32_t fiSize;		/* File Size */
  char     fiFileName[13];	/* File name and extension */
};
#pragma pack()
typedef struct _fileinfo fileinfo;

#pragma pack(1)
struct dirent { /* Standard C library structure returning directory entries. */
  /* Standard fields */
  _ino_t d_ino;		/* We don't need it, but it's required by the spec */
  unsigned char d_type;	/* File type. Values defined in macros DT_xxxx */
  /* unsigned char d_namlen; /* File name length, not including NUL */
  /* OS-specific extensions (allowed by the Posix specification) */
  /* Matches exactly the struc _fileinfo above, to avoid doing any copying */
  uint8_t  d_reserved[21];
  uint8_t  d_attribs;		/* Attribute of file found */
  uint16_t d_time;		/* Time of last write */
  uint16_t d_date;		/* Time of last write */
  uint32_t d_filesize;		/* File Size */
  /* Standard field: The file name, which must be last in the dirent structure */
  char d_name[NAME_MAX+1];	/* Null-terminated file name. Must be last */
};
#pragma pack()

#pragma pack(1)
struct _DIR { /* Private structure, not for use by users */
  struct dirent sDirent;
  /* char dta_buf[128];		/* Protection area, in case MS-DOS uses the whole DTA, and not just the struct _fileinfo ahead of it. */
  char wildcards[4];		/* Leave room for the "\*.*" that we append initially. This field MUST follow sDirent. */
  char first;			/* Tracks if this is the first search call */
};
#pragma pack()

/* Macros to extract size information from a struct dirent */
#define _D_ALLOC_NAMLEN(d) 13 /* Upper bound of the block size to alloc for a name */

/* MS-DOS compatible functions for searching with a specific attribute */
int srch1st(char *pszFile, uint16_t wAttr, fileinfo *pFI); /* Search first matching file */
int srchnext(fileinfo *pFI);     /* Search next matching file */
#define DOS_SEARCH_FUNCTIONS_DEFINED 1

#endif /* defined(_MSDOS) */

/************************ Win32-specific definitions *************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#ifndef WINVER
#define WINVER 0x0400
#endif

#include <dos.h>
#include <windows.h>

/* Add symlink definitions, that may be missing in XP and older Windows SDKs */
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK 0xA000000CL
#endif
#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY 1
#endif
#ifndef FSCTL_SET_REPARSE_POINT
#define FSCTL_SET_REPARSE_POINT     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) 
#endif
#ifndef FSCTL_GET_REPARSE_POINT
#define FSCTL_GET_REPARSE_POINT     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#endif
#ifndef FSCTL_DELETE_REPARSE_POINT
#define FSCTL_DELETE_REPARSE_POINT  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) 
#endif

#define NAME_MAX (4 * FILENAME_MAX) /* Worst case using UTF-8 encoding: 4 bytes/WCHAR */

struct dirent { /* Structure used to return information about directory entries. */
  /* OS-specific extensions */
  uint32_t d_attribs;
  uint32_t d_ReparseTag;
  FILETIME d_CreationTime;
  FILETIME d_LastAccessTime;
  FILETIME d_LastWriteTime;
  uint64_t d_filesize;
  char     d_shortname[14*sizeof(WCHAR)];
  /* Standard fields */
  _ino_t d_ino;		/* We don't need it, but it's required by the spec */
  unsigned char d_type;	/* File type. Values defined in macros DT_xxxx */
  /* unsigned char d_namlen; /* File name length, not including NUL */
  char d_name[(NAME_MAX+1)*sizeof(WCHAR)]; /* Null-terminated file name */
};

struct _DIR { /* Private structure, not for use by users */
  struct dirent sDirent;
  WCHAR *pwszDirName;		/* Null-terminated directory name */
  HANDLE hFindFile;		/* Search handle */
  WIN32_FIND_DATAW wfd;		/* Where Win32 will store the file information */
};

/* Macros to extract size information from a struct dirent */
#define _D_ALLOC_NAMLEN(d) (_D_EXACT_NAMLEN(d)+1) /* Upper bound of the block size to alloc for a name */

#endif /* defined(_WIN32) */

/************************* OS/2-specific definitions *************************/

#ifdef _OS2	/* Automatically defined when targeting an OS/2 application? */

#include <dos.h>

#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_VIO
#include "os2.h"

#define NAME_MAX CCHMAXPATHCOMP

struct dirent { /* Structure used to return information about directory entries. */
  _ino_t d_ino;		/* We don't need it, but it's required by the spec */
  /* Non standard extensions, to ease adapting old DOS/WIN32 apps */
  uintmax_t d_filesize;	/* File size */
  uint16_t time;	/* MS-DOS time */
  uint16_t date;	/* MS-DOS date */
  uint8_t attribs;	/* Attributes, the MS-DOS way */
  /* Standard fields */
  unsigned char d_type;	/* File type. Values defined in macros DT_xxxx */
  /* unsigned char d_namlen; /* File name length, not including NUL */
  char d_name[NAME_MAX+1];	/* Null-terminated file name. Must be last */
};

struct _DIR { /* Private structure, not for use by users */
  struct dirent sDirent;
  short hDir; 			/* Directory handle */
  FILEFINDBUF buf;		/* Where OS/2 will store the file information */
};

/* Macros to extract size information from a struct dirent */
#define _D_ALLOC_NAMLEN(d) (_D_EXACT_NAMLEN(d)+1) /* Upper bound of the block size to alloc for a name */

#endif /* defined(_OS2) */

/********************** End of OS-specific definitions ***********************/

typedef struct _DIR DIR;	/* Directory enumerator handle */
typedef struct dirent _dirent;	/* Directory entry */

#define MAXNAMELEN NAME_MAX /* Alias used by some Unix versions */

/* File types for struct dirent d_type. Must match the S_IFMT field in sys/stat.h. */
#define DT_UNKNOWN	 0
#define DT_FIFO		 1	/* Fifo (not used in DOS/Windows dirs) */
#define DT_CHR		 2	/* Character device (not used in DOS/Windows) */
#define DT_DIR		 4	/* Directory */
#define DT_BLK		 6	/* Block device (not used in DOS/Windows) */
#define DT_REG		 8	/* Normal file */
#define DT_LNK		10	/* Symbolic link */
#define DT_SOCK		12	/* Socket (not used in DOS/Windows dirs) */
#define DT_VOLID	15	/* Volume ID (non-standard extension for MS-DOS FAT) */

/* Functions operating on directories */
#if defined(_MSDOS)

extern DIR *opendir(const char *name);	    /* Open a directory */
extern int closedir(DIR *pDir);		    /* Close the directory. Return 0 if successful, -1 if not. */
extern _dirent *readdir(DIR *pDir);	    /* Read a directory entry. Return pDirEnt, or NULL for EOF or error. */

#elif defined(_WIN32)

#if defined(_UTF8_SOURCE)
#define opendir opendirU
#define readdir readdirU
#define scandir scandirU
#else /* _ANSI_SOURCE */
#define opendir opendirA
#define readdir readdirA
#define scandir scandirA
#endif
extern DIR *opendirW(const WCHAR *name);	    /* Open a directory - Wide char version */
extern DIR *opendirM(const char *name, UINT cp);    /* Open a directory - MultiByte char version */
#define opendirA(name) opendirM(name, CP_ACP)	    /* Open a directory - ANSI version */
#define opendirU(name) opendirM(name, CP_UTF8)	    /* Open a directory - UTF-8 version */
extern int closedir(DIR *pDir);			    /* Close the directory. Return 0 if successful, -1 if not. */
extern _dirent *readdirW(DIR *pDir);		    /* Read a directory entry. Return pDirEnt, or NULL for EOF or error. */
extern _dirent *readdirM(DIR *pDir, UINT cp);	    /* Read a directory entry. Return pDirEnt, or NULL for EOF or error. */
#define readdirA(pDir) readdirM(pDir, CP_ACP)	    /* Read a directory entry. Return pDirEnt, or NULL for EOF or error. */
#define readdirU(pDir) readdirM(pDir, CP_UTF8)	    /* Read a directory entry. Return pDirEnt, or NULL for EOF or error. */

#endif /* defined(_WIN32) */

/* extern int readdir_r(DIR *pDir, _dirent *__entry, _dirent **__result); /* Reentrant readdir */
/* extern void rewinddir(DIR *pDir); /* Rewind DIRP to the beginning of the directory. */
/* extern void seekdir(DIR *pDir, long lPos); /* Seek to position POS on DIRP. */
/* extern long telldir(DIR *pDir); /* Return the current position of DIRP. */
/* extern int dirfd(DIR *pDir); /* Return the file descriptor used by DIRP. */

/* Scan the directory dir, calling cbSelect() on each directory entry.
   Entries for which cbSelect() returns nonzero are individually malloc'd,
   sorted using qsort with cbCompare(), and collected in a malloc'd array in
   *namelist.  Returns the number of entries selected, or -1 on error. */
extern int scandir(const char *dir,
		   _dirent ***namelist,
		   int (*cbSelect) (const _dirent *),
		   int (__cdecl *cbCompare) (const _dirent **,
					     const _dirent **));

/* Function to compare two `struct dirent's alphabetically.  */
extern int __cdecl alphasort (const _dirent **ppDE1, const _dirent **ppDEe2);
/* extern int versionsort (const _dirent **ppDE1, const _dirent **ppDEe2); */

#ifdef  __cplusplus
}
#endif

#endif /* defined(_DIRENT_H)  */

