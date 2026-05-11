/*****************************************************************************\
*                                                                             *
*   Filename	    setenv.c						      *
*									      *
*   Description     Redefinitions of standard C library's setenv()	      *
*                                                                             *
*   Notes	    							      *
*                                                                             *
*   History								      *
*    2022-12-01 JFL Created this module.				      *
*    2025-08-10 JFL Added a Windows-specific implementation.		      *
*    2025-08-13 JFL Fixed a bug when updating an existing non-ASCII variable. *
*    2025-08-15 JFL Declare the dict. data destructor when creating the dict. *
*                   Corrected the ability to delete a value.		      *
*                                                                             *
*		  © Copyright 2022 Jean-François Larvoire		      *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define  _CRT_SECURE_NO_WARNINGS /* Prevent "This function or variable may be unsafe" warnings */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "debugm.h"

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    setenv						      |
|									      |
|   Description	    Redefine the standard setenv() functions	              |
|									      |
|   Parameters	    char *pszName	Name of the variable to set	      |
|		    char *pszValue	Value to set, or NULL to delete it    |
|		    int iOverwrite	If TRUE, allow overwriting a previous |
|		    			value for that variable		      |
|		    							      |
|   Returns	    0 = Success, -1 = Failure				      |
|		    							      |
|   Notes	    The Windows-specific version updates the Unicode version  |
|		    of the environment, to make sure non-ASCII characters     |
|		    are preserved.					      |
|		    							      |
|   History								      |
|    2022-11-29 JFL Created this routine                             	      |
|    2025-08-10 JFL Added a Windows-specific implementation.		      |
*									      *
\*---------------------------------------------------------------------------*/

#if defined(_MSDOS)

/* Emulate the standard C library setenv(), based on Microsoft's _putenv() */
int setenv(const char *pszName, const char *pszValue, int iOverwrite) {
  int iErr = 0;

  DEBUG_ENTER(("setenv(\"%s\", \"%s\", %d)\n", pszName, pszValue, iOverwrite));

  if ((!pszName) || (!pszName[0]) || strchr(pszName, '=')) {
    errno = EINVAL;
    RETURN_CONST(-1);
  }

  if (!pszValue) pszValue = ""; /* Non standard, but will delete the value in MSDOS */

  if (iOverwrite || !getenv(pszName)) {
    /* Build the "VAR=VALUE" string expected by putenv() */
    size_t l = strlen(pszName);
    char *pszBuf = malloc(l+1+strlen(pszValue)+1);
    if (!pszBuf) RETURN_CONST(-1);
    strcpy(pszBuf, pszName);
    pszBuf[l] = '=';
    strcpy(pszBuf+l+1, pszValue);
    /* Update the environment */
    iErr = _putenv(pszBuf);
    /* Cleanup */
    free(pszBuf);
  }
  RETURN_INT(iErr);
}

#endif /* defined(_MSDOS) */

#if defined(_WIN32)

#include "iconv.h"	/* MsvcLibX encoding conversion routines */
#include "dict.h"	/* SysToolsLib Manage a dictionary */

#include <windows.h>

extern dict_t *mlxEnvDict; /* getenv() multibyte (Ex: UTF-8) environment strings */

/* Emulate the standard C library setenv(), based on Microsoft's _putenv() */
int setenvM(const char *pszName, const char *pszValue, int iOverwrite, UINT uCP) {
  int iErr = -1;

  DEBUG_ENTER(("setenv(\"%s\", \"%s\", %d)\n", pszName, pszValue, iOverwrite));

  if ((!pszName) || (!pszName[0]) || strchr(pszName, '=')) {
    errno = EINVAL;
    RETURN_CONST(-1);
  }

  if (!pszValue) pszValue = ""; /* Non standard, but will delete the value in Windows */

  if (iOverwrite || !getenv(pszName)) {
    WCHAR *pwszName = NULL;
    WCHAR *pwszValue = NULL;
    WCHAR *pwszBuf = NULL;
    size_t lName, lValue;
    /* Convert the name to a unicode string */
    pwszName = MultiByteToNewWideString(uCP, pszName);
    if (!pwszName) goto cleanup;
    /* Convert the value to a unicode string */
    pwszValue = MultiByteToNewWideString(uCP, pszValue);
    if (!pwszValue) goto cleanup;
    /* Build the "VAR=VALUE" string expected by putenv() */
    lName = lstrlenW(pwszName);
    lValue = lstrlenW(pwszValue);
    pwszBuf = malloc(sizeof(WCHAR)*(lName+1+lValue+1));
    if (!pwszBuf) goto cleanup;
    lstrcpyW(pwszBuf, pwszName);
    pwszBuf[lName] = L'=';
    lstrcpyW(pwszBuf+lName+1, pwszValue);
    /* Update the Unicode environment */
    iErr = _wputenv(pwszBuf);
    /* Update the Multibyte environment */
    if (!mlxEnvDict) {	/* Search for a previous value in the dictionary */
      mlxEnvDict = NewDict(free); /* Values must be freed when nodes are overwritten or deleted */
    }
    if (lValue) {
      SetDictValue(mlxEnvDict, pszName, strdup(pszValue));
    } else {
      DeleteDictValue(mlxEnvDict, pszName);
    }
    /* Cleanup */
cleanup:
    free(pwszName);
    free(pwszValue);
    free(pwszBuf);
  } else {
    iErr = 0;
  }
  RETURN_INT(iErr);
}

#endif /* defined(_WIN32) */

