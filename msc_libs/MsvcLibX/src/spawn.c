/*****************************************************************************\
*                                                                             *
*   Filename	    spawn.c						      *
*									      *
*   Description:    WIN32 UTF-8 version of spawn			      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-03-27 JFL Created this module.				      *
*    2014-07-03 JFL Added support for pathnames >= 260 characters. 	      *
*    2017-10-03 JFL Fixed support for pathnames >= 260 characters. 	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

/* Microsoft C libraries include files */
#include <stdio.h>
/* MsvcLibX library extensions */
#include "debugm.h"
#include "msvclibx.h"

#ifdef _WIN32

#include <windows.h>
#include <process.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        _spawnvpU	                                              |
|                                                                             |
|   Description     UTF-8 version of Microsoft's _spawnvp		      |
|                                                                             |
|   Parameters:     int iMode		Spawning mode. P_WAIT or P_NOWAIT     |
|		    char *pszCommand	Program to start		      |
|		    char **argv		List of arguments, terminated by NULL |
|									      |
|   Returns:	    The exit code (if P_WAIT) or the process ID (if P_NOWAIT) |
|                                                                             |
|   Notes                                                                     |
|                                                                             |
|   History								      |
|    2014-03-04 JFL Created this routine.                      		      |
*                                                                             *
\*---------------------------------------------------------------------------*/

intptr_t _spawnvpU(int iMode, const char *pszCommand, char *const *argv) {
  WCHAR *pwszCommand;
  WCHAR **wszArgv;
  int n;
  int nArgs;
  int iArg;
  intptr_t iRet;

  DEBUG_CODE({
    int i;
    DEBUG_PRINTF(("_spawnvpU(%d, \"%s\", {", iMode, pszCommand));
    if (DEBUG_IS_ON()) {
      for (i=0; argv[i]; i++) {
      	if (i) printf(", ");
      	printf("\"%s\"", argv[i]);
      }
      printf("});\n");
    }
  })

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszCommand = MultiByteToNewWidePath(CP_UTF8, pszCommand);
  if (!pwszCommand) return -1;

  for (nArgs=0; argv[nArgs]; nArgs++) ;	/* Count the number of arguments */
  wszArgv = (WCHAR **)malloc((nArgs+1) * sizeof(WCHAR *));
  if (!wszArgv) {
    free(pwszCommand);
    return -1;    /* errno already set by malloc */
  }

  for (iArg=0; argv[iArg]; iArg++) {	/* Convert every argument */
    int iArgBufSize = lstrlen(argv[iArg]) + 1;
    wszArgv[iArg] = malloc(sizeof(WCHAR)*iArgBufSize);
    if (!wszArgv[iArg]) {
      while (iArg) free(wszArgv[--iArg]);	/* Free the partial arg list */
      free(wszArgv);
      free(pwszCommand);
      return -1;      /* errno already set by malloc */
    }
    /* Convert the argument to a unicode string. This is not a pathname, so just do a plain conversion */
    n = MultiByteToWideChar(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			    0,			/* dwFlags, */
			    argv[iArg],		/* lpMultiByteStr, */
			    iArgBufSize,	/* cbMultiByte, */
			    wszArgv[iArg],	/* lpWideCharStr, */
			    iArgBufSize		/* cchWideChar, */
			    );
    if (!n) {
      errno = Win32ErrorToErrno();
      while (iArg >= 0) free(wszArgv[iArg--]);	/* Free the partial arg list */
      free(wszArgv);
      free(pwszCommand);
      return -1;
    }
  }
  wszArgv[nArgs] = NULL;

  iRet = _wspawnvp(iMode, pwszCommand, wszArgv);

  while (nArgs) free(wszArgv[--nArgs]);	/* Free the full arg list */
  free(wszArgv);
  free(pwszCommand);
  return iRet;
}

#endif /* defined(_WIN32) */

