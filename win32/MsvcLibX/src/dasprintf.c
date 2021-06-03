/*****************************************************************************\
*                                                                             *
*   Filename	    dasprintf.c						      *
*									      *
*   Description     Wrapper around asprintf() used for MsvcLibX debugging     *
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

/* MsvcLibX-specific alternatives to asprintf(), used internally for debugging */
char *dvasprintf(const char *pszFormat, va_list vl) {
  char *pszBuf = NULL;
  vasprintf(&pszBuf, pszFormat, vl); /* Updates pszBuf only if successful */
  return pszBuf;
}

char *dasprintf(const char *pszFormat, ...) {
  char *pszBuf = NULL;
  va_list vl;
  va_start(vl, pszFormat);
  vasprintf(&pszBuf, pszFormat, vl); /* Updates pszBuf only if successful */
  va_end(vl);
  return pszBuf;
}

