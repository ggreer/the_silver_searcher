/*****************************************************************************\
*                                                                             *
*   Filename	    snprintf.c						      *
*									      *
*   Description     Fix old MSVC versions of snprintf()/vsnprintf()	      *
*                                                                             *
*   Notes	    All versions up to Visual Stdio 14/2015 were duplicates   *
*		    of the non-standard _snprintf()/_vsnprintf().	      *
*		    They return -1 if the buffer is too small; And, they      *
*		    don't append a NUL if the buffer is the exact size!	      *
*		    							      *
*   History								      *
*    2020-07-27 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2020 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS /* Disable safety warning for _vsnprintf() */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#if !defined(_UCRT) /* Starting with Visual Stdio 14/2015 (_MSC_VER==1900) with the UCRT, these ARE standard */
 		    /* But older implementations are absent, or duplicate of the non-standard _snprintf(), etc */

int correct_vsnprintf(char *pBuf, size_t nBufSize, const char *pszFormat, va_list vl) {
  char *pBuf2;
  int iRet;
  va_list vl0;
  /* First try it with the original arguments */
  /* This consumes the vl arguments, which needs to be done once */
  /* This also optimizes the number of calls, in the normal case where the output buffer was sized correctly */
  va_copy(vl0, vl);	/* Save a copy of the caller's va_list */
  iRet = _vsnprintf(pBuf, nBufSize, pszFormat, vl);
  if (iRet >= 0) {	/* Success, the output apparently fits in the buffer */
    if ((size_t)iRet == nBufSize) if (pBuf && nBufSize) pBuf[nBufSize-1] = '\0'; /* Fix the missing NUL */
    va_end(vl0);
    return iRet;
  }
  /* OK, this does not fit. Try it with larger and larger buffers, until we know the full output size */
  iRet = vasprintf(&pBuf2, pszFormat, vl0);
  if (iRet >= 0) {	/*  Success at last, now we know the necessary size */
    if (pBuf && nBufSize) {	/* Copy whatever fits in the output buffer */
      if (nBufSize-1) memcpy(pBuf, pBuf2, nBufSize-1);
      pBuf[nBufSize-1] = '\0';	/* Make sure there's a NUL in the end */
    }
    free(pBuf2);
  }
  va_end(vl0);
  return iRet;
}

int correct_snprintf(char *pBuf, size_t nBufSize, const char *pszFormat, ...) {
  int iRet;
  va_list vl;
  va_start(vl, pszFormat);
  iRet = correct_vsnprintf(pBuf, nBufSize, pszFormat, vl);
  va_end(vl);
  return iRet;
}

#endif
