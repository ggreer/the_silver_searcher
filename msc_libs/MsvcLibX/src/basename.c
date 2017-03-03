/*****************************************************************************\
*                                                                             *
*   Filename	    basename.c						      *
*                                                                             *
*   Description     Get the file name part of a file pathname		      *
*                                                                             *
*   Notes	    Uses a static buffer in some cases. Not thread safe!      *
*                   Posix spec authorizes this, and also authorizes to modify *
*		    the input string, which we do. => Always do an strdup()   *
*		    before calling basename(), and call basename(copy).	      *
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

char *basename(char *pszPathname) {
  char *pszPath = pszPathname;
  size_t len;
  char *pc;
  char *pc2;

  /* A NULL pathname is assumed to refer to the current directory */
  if (!pszPathname) return ".";
  /* Skip the drive if present */
  len = strlen(pszPathname);
  if ((len >= 2) && (pszPathname[1] == ':')) {
    pszPath += 2;
    len -= 2;
  }
  if (!len) return ".";
  /* Remove trailing path separators */
  while ((len > 1) && ((pszPath[len-1] == '\\') || (pszPath[len-1] == '/'))) {
    pszPath[--len] = '\0';
  }
  /* Find the beginning of the file name */
  pc = strrchr(pszPath, '\\');
  pc2 = strrchr(pszPath, '/');
  if (pc2 > pc) pc = pc2;
  if (pc) pc += 1; else pc = pszPath;
  if (!*pc) return pszPath; /* No file name left. This is the root directory */
  /* Done */
  return pc;
}
