/*****************************************************************************\
*                                                                             *
*   Filename	    dirname.c						      *
*                                                                             *
*   Description     Get the parent directory name of a file pathname	      *
*                                                                             *
*   Notes	    Uses a static buffer in some cases. Not thread safe!      *
*                   Posix spec authorizes this, and also authorizes to modify *
*		    the input string, which we do. => Always do an strdup()   *
*		    before calling dirname(), and call dirname(copy).	      *
*                                                                             *
*   History								      *
*    2016-09-08 JFL Created this file.                                        *
*		    							      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#include "msvclibx.h"

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ 2005 security warnings */

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0

char szLastDriveDirName[4] = "C:.";

char *dirname(char *pszPathname) {
  char *pszPath = pszPathname;
  size_t len;
  char *pc;
  char *pc2;

  /* A NULL pathname is assumed to refer to the current directory */
  if (!pszPathname) return szLastDriveDirName + 2;	/* "." */
  /* Skip the drive if present */
  len = strlen(pszPathname);
  if (!len) return szLastDriveDirName + 2;		/* "." */
  if ((len >= 2) && (pszPathname[1] == ':')) {
    pszPath += 2;
    len -= 2;
    if (!len) {
      szLastDriveDirName[0] = pszPathname[0]; /* Warning: Not thread safe! */
      return szLastDriveDirName;			/* "D:." */
    }
  }
  /* Remove trailing path separators */
  while ((len > 1) && ((pszPath[len-1] == '\\') || (pszPath[len-1] == '/'))) {
    pszPath[--len] = '\0';
  }
  /* Remove the file name */
  pc = strrchr(pszPath, '\\');
  pc2 = strrchr(pszPath, '/');
  if (pc2 > pc) pc = pc2;
  if (pc) pc += 1; else pc = pszPath;
  *pc = '\0';
  len = pc - pszPath;
  if (!len) {
    strcpy(pc, "."); /* Yes, the spec says that the dirname of .. is . */
    len = 1;
  }
  /* Remove trailing path separators */
  while ((len > 1) && ((pszPath[len-1] == '\\') || (pszPath[len-1] == '/'))) {
    pszPath[--len] = '\0';
  }
  /* Done */
  return pszPathname;
}
