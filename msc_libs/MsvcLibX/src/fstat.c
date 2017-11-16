/*****************************************************************************\
*                                                                             *
*   Filename	    fstat.c						      *
*									      *
*   Description:    Redefinitions of standard C library's fstat()	      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-06-24 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
#include <string.h>
/* MsvcLibX library extensions */
#include <sys/stat.h>
#include "debugm.h"
#include <stdint.h>


#if defined(_MSDOS)

/* Nothing needs to be redefined */

#endif /* defined(_MSDOS) */


#if defined(_WIN32)

/* ------------ Display the *stat* macro values at compile time ------------ */

#pragma message(MACRODEF(_MSVC_stat))
#pragma message(MACRODEF(_MSVC_fstat))
#pragma message(MACRODEF(_MSVC_lstat))
#pragma message(MACRODEF(_MSVC_stat64))

#if _MSVCLIBX_STAT_DEFINED
  #pragma message(MACRODEF(_LIBX_stat))
  #pragma message(MACRODEF(_LIBX_stat64))
#endif

#pragma message(MACRODEF(stat))
#pragma message(MACRODEF(fstat))
#pragma message(MACRODEF(lstat))

#if defined(_LARGEFILE_SOURCE64)
  #pragma message(MACRODEF(stat64))
  #pragma message(MACRODEF(fstat64))
  #pragma message(MACRODEF(lstat64))
#endif

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    fstat						      |
|									      |
|   Description:    Redefine the standard _fstatXY() functions                |
|									      |
|   Parameters:     int nFile			The file handle		      |
|		    struct stat *buf		Output buffer		      |
|									      |
|   Returns:	    0 = Success, -1 = Failure				      |
|									      |
|   Notes:	    See sys/stat.h for a description of how the stat and fstat|
|		    macros work.					      |
|									      |
|   History:								      |
|    2014-06-24 JFL Created this routine                               	      |
*									      *
\*---------------------------------------------------------------------------*/

int fstat(int nFile, struct stat *pStat) {
  int iErr;
  struct _MSVC_stat msStat;
  DEBUG_CODE(
  struct tm *pTime;
  char szTime[100];
  )

  DEBUG_ENTER((STRINGIZE(fstat) "(%d, 0x%p);\n", nFile, pStat));

  iErr = _MSVC_fstat(nFile, &msStat);
  if (!iErr) {
    ZeroMemory(pStat, sizeof(struct stat));
    pStat->st_mode = msStat.st_mode;
    pStat->st_size = msStat.st_size;
    pStat->st_ctime = 
#undef st_ctime
    			msStat.st_ctime;
    pStat->st_mtime = 
#undef st_mtime
    			msStat.st_mtime;
    pStat->st_atime = 
#undef st_atime
    			msStat.st_atime;
    DEBUG_CODE(
      pTime = LocalFileTime(&(msStat.st_mtime));
      strftime(szTime, sizeof(szTime), "%Y-%m-%d %H:%M:%S", pTime);
    )
    RETURN_INT_COMMENT(0, ("%s  mode = 0x%04X  size = %I64d bytes\n", szTime, pStat->st_mode, (__int64)(pStat->st_size)));
  }
  /* TO DO: Get the nanosecond time resolution using Windows functions */

  RETURN_INT_COMMENT(iErr, ("%s\n", strerror(errno)));
}

#endif /* _WIN32 */

