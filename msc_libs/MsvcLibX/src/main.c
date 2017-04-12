/*****************************************************************************\
*                                                                             *
*   Filename	    main.c						      *
*									      *
*   Description:    Main routine for WIN32 UTF-8 programs		      *
*                                                                             *
*   Notes:	    TO DO: Also set the environment with _setenvp() ?	      *
*		    							      *
*   History:								      *
*    2014-03-03 JFL Created this module.				      *
*    2016-09-20 JFL Bug fix: Empty arguments "" did not get recorded.	      *
*    2017-02-05 JFL Redesigned to override libc's _setargv(). This avoids     *
*                   having to encapsulate the main() routine with one here.   *
*    2017-03-12 JFL Restructured the UTF16 writing mechanism.		      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_SOURCE
#define _CRT_SECURE_NO_WARNINGS /* Avoid depreciation warnings */

#include <stdio.h>
#include "msvclibx.h"

#ifdef _WIN32

#include <windows.h>
#include <iconv.h>	/* For MsvcLibX' codePage global variable */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        BreakArgLine                                              |
|                                                                             |
|   Description     Break the Windows command line into standard C arguments  |
|                                                                             |
|   Parameters      LPSTR pszCmdLine    NUL-terminated argument line          |
|                   char *pszArg[]      Array of arguments pointers           |
|                                                                             |
|   Returns         int argc            Number of arguments found. -1 = Error |
|                                                                             |
|   Notes           MSVC library startup \" parsing rule is:                  |
|                   2N backslashes + " ==> N backslashes and begin/end quote  |
|                   2N+1 backslashes + " ==> N backslashes + literal "	      |
|                   N backslashes ==> N backslashes                           |
|                                                                             |
|   History 								      |
|    1993-10-05 JFL Initial implementation within devmain().		      |
|    1994-04-14 JFL Extracted from devmain, and created this routine.	      |
|    1995-04-07 JFL Extracted from llkinit.c.				      |
|    1996-09-26 JFL Adapted to Win32 programs.				      |
|    1996-12-11 JFL Use Windows string routines.			      |
|    2001-09-18 JFL Set argv[0] with actual module file name.		      |
|                   Manage quoted strings as a single argument.               |
|    2001-09-25 JFL Only process \x inside strings.                           |
|    2014-03-04 JFL Removed the C-style \-quoting of characters, which was    |
|                   convenient, but incompatible with MSVC argument parsing.  |
|                   Removed the limitation on the # of arguments.             |
|                   Made the code compatible with ANSI and UTF-8 encodings.   |
|    2017-02-05 JFL Check memory allocation errors, and if so return -1.      |
*                                                                             *
\*---------------------------------------------------------------------------*/

int BreakArgLine(LPSTR pszCmdLine, char ***pppszArg) {
  int i, j;
  int argc = 0;
  char c, c0;
  char *pszCopy;
  int iString = FALSE;	/* TRUE = string mode; FALSE = non-string mode */
  int nBackslash = 0;
  char **ppszArg;
  int iArg = FALSE;	/* TRUE = inside an argument; FALSE = between arguments */

  ppszArg = (char **)malloc((argc+1)*sizeof(char *));
  if (!ppszArg) return -1;

  /* Make a local copy of the argument line */
  /* Break down the local copy into standard C arguments */

  pszCopy = malloc(lstrlen(pszCmdLine) + 1);
  if (!pszCopy) return -1;
  /* Copy the string, managing quoted characters */
  for (i=0, j=0, c0='\0'; ; i++) {
    c = pszCmdLine[i];
    if (!c) {		    /* End of argument line */
      for ( ; nBackslash; nBackslash--) pszCopy[j++] = '\\'; /* Output pending \s */
      pszCopy[j++] = c;
      break;
    }
    if ((!iArg) && (c != ' ') && (c != '\t')) { /* Beginning of a new argument */
      iArg = TRUE;
      ppszArg[argc++] = pszCopy+j;
      ppszArg = (char **)realloc(ppszArg, (argc+1)*sizeof(char *));
      if (!ppszArg) return -1;
      pszCopy[j] = c0 = '\0';
    }
    if (c == '\\') {	    /* Escaped character in string (maybe) */
      nBackslash += 1; 
      continue;
    }
    if (c == '"') {
      if (nBackslash & 1) { /* Output N/2 \ and a literal " */
      	for (nBackslash >>= 1; nBackslash; nBackslash--) pszCopy[j++] = '\\';
	pszCopy[j++] = c0 = c;
	continue;
      }
      if (nBackslash) {	    /* Output N/2 \ and switch string mode */
      	for (nBackslash >>= 1; nBackslash; nBackslash--) pszCopy[j++] = '\\';
      }
      iString = !iString;
      continue;
    }
    for ( ; nBackslash; nBackslash--) pszCopy[j++] = '\\'; /* Output pending \s */
    if ((!iString) && ((c == ' ') || (c == '\t'))) { /* End of an argument */
      iArg = FALSE;
      c = '\0';
    }
    pszCopy[j++] = c0 = c;
  }

  ppszArg[argc] = NULL;
  *pppszArg = ppszArg;

  return argc;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    _setargv						      |
|									      |
|   Description	    Msft standard CRT routine for parsing the command line.   |
|									      |
|   Parameters	    char *_acmdln	Command line parameters.	      |
|									      |
|   Returns	    __argc = Number of arguments. -1 = Error.		      |
|		    __argv = Array of arguments				      |
|		    _pgmptr = The program pathname			      |
|									      |
|   Notes	    When linked in, replaces the default routine from the CRT.|
|		    							      |
|   History								      |
|    2001-09-25 JFL Created this routine				      |
|    2016-12-31 JFL Changed the return type from void to int, else the WIN64  |
|                   version fails with message:				      |
|		    runtime error R6008 - not enough space for arguments      |
|    2017-02-05 JFL Adapted for UTF-8 arguments initialization.		      |
*									      *
\*---------------------------------------------------------------------------*/

/* Global CRT variables defined in stdlib.h */
/* Do not include stdlib.h here, to avoid getting unwanted macros hiding these */
_CRTIMP extern int __argc;
_CRTIMP extern char **__argv;
_CRTIMP extern char *_acmdln;
_CRTIMP extern char *_pgmptr;

int _initU(void); /* Forward reference */

int _setargv(void) {
  int err = _initU();
  if (err) return err;
  __argc = BreakArgLine(_acmdln, &__argv);
  _pgmptr = __argv[0];
  return __argc;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    _initU						      |
|									      |
|   Description	    UTF-8 program initializations			      |
|									      |
|   Parameters	    None						      |
|									      |
|   Returns	    0=Success. -1 = Error.				      |
|		    _acmdln = UTF-8 command line			      |
|		    codePage = Console Code Page			      |
|		    							      |
|   Notes	    Forcibly linked in C programs that define _UTF8_SOURCE,   |
|		    etc, which drags in _setargv() above with it.	      |
|		    							      |
|   History								      |
|    2017-02-05 JFL Adapted from the abandonned _mainU0 routine.	      |
|    2017-03-03 JFL Record both the console and system code pages.	      |
|    2017-04-12 JFL Bug fix: realloc() sometimes moves the _acmdln buffer.    |
*									      *
\*---------------------------------------------------------------------------*/

int _initU(void) {
  LPWSTR lpwCommandLine;
  int n;
  WCHAR wc;
  char *pNewBuf;

  /* Get the Unicode command line */  
  lpwCommandLine = GetCommandLineW();
  /* Trim tail spaces */
  n = lstrlenW(lpwCommandLine);
  while (n && ((wc = lpwCommandLine[n-1]) != L'\0') && ((wc == L' ') || (wc == L'\t'))) lpwCommandLine[--n] = L'\0';
  /* Allocate space for the UTF8 copy */
  n += 1;	/* Count the final NUL */
  _acmdln = malloc(4 * n); /* Worst case */
  if (!_acmdln) return -1; /* Memory allocation failed */
  /* Convert the Unicode command line to UTF-8 */
  n = WideCharToMultiByte(CP_UTF8,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  lpwCommandLine,	/* lpWideCharStr, */
			  n,			/* cchWideChar, */
			  _acmdln,		/* lpMultiByteStr, */
			  (4 * n),		/* cbMultiByte, */
			  NULL,			/* lpDefaultChar, */
			  NULL			/* lpUsedDefaultChar */
			  );
  if (!n) {
#undef fprintf /* Use the real fprintf, to avoid further conversion errors! */
    fprintf(stderr, "Warning: Can't convert the argument line to UTF-8\n");
    _acmdln[0] = '\0';
  }
  pNewBuf = realloc(_acmdln, n+1); /* Resize the memory block to fit the UTF-8 line */
  if (pNewBuf) _acmdln = pNewBuf; /* Should not fail since we make it smaller, but may move */

  /* Initialize the UTF8/UTF16 output mechanism */
  initWideFiles();
  /* NOTE: Can't use fprintf anymore after this. Use fprintfA or fprintfU instead. */

  return 0;
}

#endif /* defined(_WIN32) */

