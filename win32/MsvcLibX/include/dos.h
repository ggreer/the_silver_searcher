/********************** :encoding=ISO-8859-1:tabSize=8: **********************\
*                                                                             *
*   Filename:	    dos.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to dos.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2025-12-31 JFL Created this file.                                        *
*									      *
*                 © Copyright 2025 Jean-François Larvoire                     *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_DOS_H
#define	_MSVCLIBX_DOS_H	1

#include "msvclibx.h"

#include UCRT_INCLUDE_FILE(dos.h) /* Include MSVC's own <io.h> file */

/* Call MS-DOS function 47H "Get Current Directory" */
/* Args & return value compatible with those of MSVC's _getdcwd() */
/* The DOS error code is returned in _doserrno, and the C error code in errno */
char *dos_getcwd(int iDrive, char *pszBuf, int iBufLen);

typedef struct _dos_fs_info {
  int iFlags;		/* File system flags */
  int iNameLength;	/* Maximum length of file name [usually 255 for LFN] */
  int iPathLength;	/* Maximum length of path [usually 260] */
  char szFsType[32];	/* ASCIZ, e.g. "FAT","NTFS","CDFS" */
} dos_fs_info;

/* File system flags */
#define DOS_FS_CASE_SENSITIVE	0x0001	/* Searches are case sensitive */
#define DOS_FS_CASE_PRESERVED	0x0002	/* Preserves case in directory entries */
#define DOS_FS_UNICODE_NAMES	0x0004	/* Uses Unicode characters in file and directory names */
#define DOS_FS_DOS_LFN		0x4000	/* Supports DOS long filename functions */
#define DOS_FS_COMPRESSED	0x8000	/* Volume is compressed */

/* Call MS-DOS function 71A0H "Get Volume Information" */
/* Assumes the volume is FAT on old DOS versions <= 6 */
/* Returns the DOS error code, or 0 for success */
/* The DOS error code is returned in _doserrno, and the C error code in errno */
int dos_get_volume_info(char *pszRoot, dos_fs_info *pDosFsInfo);

#endif /* defined(_MSVCLIBX_DOS_H) */
