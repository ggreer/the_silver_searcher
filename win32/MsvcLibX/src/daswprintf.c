/*****************************************************************************\
*                                                                             *
*   Filename	    daswprintf.c					      *
*									      *
*   Description     Wrapper around aswprintf() used for MsvcLibX debugging    *
*                                                                             *
*   Notes	    							      *
*		    							      *
*   History								      *
*    2020-07-24 JFL Moved internal routines from asprintf.c to this new file. *
*                                                                             *
*         © Copyright 2020 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS /* Disable safety warning for _vsnprintf() */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(_WIN32)

/* MsvcLibX-specific alternatives to aswprintf(), used internally for debugging */
wchar_t *dvaswprintf(const wchar_t *pwszFormat, va_list vl) {
  wchar_t *pwszBuf = NULL;
  vaswprintf(&pwszBuf, pwszFormat, vl); /* Updates pszBuf only if successful */
  return pwszBuf;
}

wchar_t *daswprintf(const wchar_t *pwszFormat, ...) {
  wchar_t *pwszBuf = NULL;
  va_list vl;
  va_start(vl, pwszFormat);
  vaswprintf(&pwszBuf, pwszFormat, vl); /* Updates pwszBuf only if successful */
  va_end(vl);
  return pwszBuf;
}

#endif /* defined(_WIN32) */

