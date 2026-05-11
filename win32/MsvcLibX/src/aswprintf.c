/*****************************************************************************\
*                                                                             *
*   Filename	    aswprintf.c						      *
*									      *
*   Description     Wide string version of GNU library's asprintf()	      *
*                                                                             *
*   Notes	    Uses _vsnwprintf(), which is available in all MSVC versions.
*		    							      *
*   History								      *
*    2020-07-29 JFL Created this module, based on asprintf.c.		      *
*                                                                             *
*         © Copyright 2020 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS /* Disable safety warning for _vsnwprintf() */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(_WIN32)

/* Work around the theoric (but unlikely) possibility that shrinking a buffer can fail */
_inline char *try_shrink_buf(void *pBuf, int iSize) {
  char *p2 = realloc(pBuf, iSize);
  return p2 ? p2 : pBuf; /* If this failed, return the initial buffer */
}

/* Lower level version using a va_list */
int vaswprintf(wchar_t **ppwszBuf, const wchar_t *pwszFormat, va_list vl) {
  wchar_t *pBuf, *pBuf2;
  int n, nBufSize = 64;
  va_list vl0, vl2;
  /* First try it once with the original va_list (When nBufSize == 128) */
  /* This consumes the vl arguments, which needs to be done once */
  va_copy(vl0, vl);	/* Save a copy of the caller's va_list */
  for (pBuf = NULL; (pBuf2 = (wchar_t *)realloc(pBuf, sizeof(wchar_t)*(nBufSize *= 2))) != NULL; ) {
    va_copy(vl2, vl0);
    n = _vsnwprintf(pBuf = pBuf2, nBufSize, pwszFormat, (nBufSize == 128) ? vl : vl2);
    va_end(vl2);
    if ((n >= 0) && (n < nBufSize)) { /* Success at last, now we know the necessary size */
      *ppwszBuf = (wchar_t *)try_shrink_buf(pBuf, sizeof(wchar_t)*(n+1)); /* Free the unused space in the end */
      va_end(vl0);
      return n;
    } /* Else if n == nBufSize, actually not success, as there's no NUL in the end */
  }
  va_end(vl0);
  return -1;
}

/* Upper level version using ellipsis */
int aswprintf(wchar_t **ppwszBuf, const wchar_t *pwszFormat, ...) {
  int iRet;
  va_list vl;
  va_start(vl, pwszFormat);
  iRet = vaswprintf(ppwszBuf, pwszFormat, vl);
  va_end(vl);
  return iRet;
}

#endif /* defined(_WIN32) */

