/*****************************************************************************\
*                                                                             *
*   Filename	    getenv.c						      *
*									      *
*   Description     Redefinitions of standard C library's getenv()	      *
*                                                                             *
*   Notes	    							      *
*                                                                             *
*   History								      *
*    2025-08-07 JFL Created this module.				      *
*    2025-08-13 JFL Made the dictionary public, for shared use by setenv().   *
*                   Use the dict. for every string, not just non-ASCII ones.  *
*    2025-08-15 JFL Declare the dict. data destructor when creating the dict. *
*                                                                             *
*		  © Copyright 2025 Jean-François Larvoire		      *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define  _CRT_SECURE_NO_WARNINGS /* Prevent "This function or variable may be unsafe" warnings */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
/* SysToolsLib include files */
#include "debugm.h"	/* SysToolsLib debug macros. Include first. */

#if defined(_WIN32)

#include "iconv.h"	/* MsvcLibX encoding conversion routines */
#include "dict.h"	/* SysToolsLib Manage a dictionary */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    getenvM						      |
|									      |
|   Description	    Define a code-page dependant getenv() function            |
|									      |
|   Parameters	    char *pszName	Name of the variable to get	      |
|		    UINT uCP		The selected code page		      |
|		    							      |
|   Returns	    The string converted to the given code page, or NULL      |
|		    							      |
|   Notes	    							      |
|		    							      |
|   History								      |
|    2025-08-07 JFL Created this routine                             	      |
|    2025-08-10 JFL Use a dictionary to avoid a memory leak.         	      |
|    2025-08-13 JFL Use the dict. for every string, not just non-ASCII ones.  |
*									      *
\*---------------------------------------------------------------------------*/

dict_t *mlxEnvDict = NULL;

char *getenvM(const char *pszName, UINT uCP) {
  WCHAR *pwszName = NULL;
  WCHAR *pwszValue;
  char *pszValue = NULL;

  DEBUG_ENTER(("getenvM(\"%s\", %u)\n", pszName, uCP));

  /* First check if it's already in the dictionnary */
  if (!mlxEnvDict) {	/* Create an empty dictionary */
    mlxEnvDict = NewDict(free); /* Values must be freed when nodes are overwritten or deleted */
  } else {		/* Search in the existing dictionary */
    pszValue = DictValue(mlxEnvDict, pszName);
    if (pszValue) RETURN_STRING(pszValue);
  }

  /* Convert the name to a unicode string */
  pwszName = MultiByteToNewWideString(uCP, pszName);
  if (!pwszName) RETURN_CONST(NULL);

  /* Get the Unicode value */
  pwszValue = _wgetenv(pwszName);
  if (!pwszValue) goto cleanup_and_exit;

  /* Convert the value to a new multibyte string */
  pszValue = WideToNewMultiByteString(uCP, pwszValue);
  if (!pszValue) goto cleanup_and_exit;

  /* Add it to the dictionary */
  NewDictValue(mlxEnvDict, pszName, pszValue);

cleanup_and_exit:
  free(pwszName);
  RETURN_STRING(pszValue);
}

#endif /* defined(_WIN32) */
