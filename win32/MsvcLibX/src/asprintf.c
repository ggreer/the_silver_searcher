/*****************************************************************************\
*                                                                             *
*   Filename	    asprintf.c						      *
*									      *
*   Description     Reimplementation of GNU library's asprintf()	      *
*                                                                             *
*   Notes	    Uses _vsnprintf(), which is available in all MSVC versions.
*		    							      *
*   History								      *
*    2020-06-26 JFL Created this module, based on debugm.h macros.	      *
*                                                                             *
*         © Copyright 2020 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS /* Disable safety warning for _vsnprintf() */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Wrapper around _vsnprintf() that avoids consuming the va_list arguments */
static int try_vsnprintf(char *pBuf, int iBufSize, const char *pszFormat, va_list vl) {
  int iRet;
  va_list vl2;
  va_copy(vl2, vl);
  iRet = _vsnprintf(pBuf, iBufSize, pszFormat, vl2);
  va_end(vl2);
  return iRet;
}

/* Work around the theoric (but unlikely) possibility that shrinking a buffer can fail */
static char *try_shrink_buf(char *pBuf, int iSize) {
  char *p2 = realloc(pBuf, iSize);
  return p2 ? p2 : pBuf; /* If this failed, return the initial buffer */
}

/* Lower level version using a va_list */
int vasprintf(char **ppszBuf, const char *pszFormat, va_list vl) {
  char *pszBuf = NULL;
  int iRet = -1, nBufSize = 64;
  do {pszBuf = (char *)realloc(pszBuf, nBufSize *= 2);} while (
    pszBuf && ((iRet = try_vsnprintf(pszBuf, nBufSize, pszFormat, vl)) == -1)
  );
  if (!pszBuf || (iRet < 0)) {
    if (pszBuf) free(pszBuf);
    return -1;
  }
  *ppszBuf = try_shrink_buf(pszBuf, iRet+1); /* Updates *ppszBuf only if successful */
  return iRet;
}

/* Upper level version using ellipsis */
int asprintf(char **ppszBuf, const char *pszFormat, ...) {
  int iRet;
  va_list vl;
  va_start(vl, pszFormat);
  iRet = vasprintf(ppszBuf, pszFormat, vl);
  va_end(vl);
  return iRet;
}

