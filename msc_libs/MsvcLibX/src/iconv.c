/*****************************************************************************\
*                                                                             *
*   Filename	    iconv.c						      *
*									      *
*   Description:    WIN32 port of standard C library's iconv()		      *
*                                                                             *
*   Notes:	    Define here a number of routines, that will eventually    *
*		    be used by iconv().					      *
*		    							      *
*   History:								      *
*    2014-02-27 JFL Created this module.				      *
*    2015-12-09 JFL Added routines fputsU and vfprintfU.		      *
*    2016-09-13 JFL Fixed warnings in fputsU. Do not change the input buffer. *
*    2017-03-03 JFL Added routine ConvertBuf(), and the fputc() series.	      *
*    2017-03-05 JFL Rewrote fputcU() & fputsU() to write UTF16 to the console.*
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
/* MsvcLibX library extensions */
#include "iconv.h"
#include "debugm.h"

#if defined(_MSDOS)

/* TO DO: Add support for DOS code pages! */

#endif /* defined(_MSDOS) */


#ifdef _WIN32

#include <windows.h>
#include <io.h>         /* For _setmode() */
#include <fcntl.h>      /* For _setmode() */
#include "unistd.h"	/* For isatty() */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    ConvertString					      |
|									      |
|   Description:    Convert a string from one MBCS encoding to another        |
|									      |
|   Parameters:     char *buf	    Buffer containg a NUL-terminated string   |
|		    size_t nBytes   Buffer size				      |
|		    UINT cpFrom	    Initial Windows code page identifier      |
|		    UINT cpTo	    Final Windows code page identifier	      |
|		    LPCSTR lpDfltC  Pointer to the Default Character to use   |
|		    			(NULL = Use the default default!)     |
|		    							      |
|   Returns:	    The converted string size. -1=error, and errno set.	      |
|		    							      |
|   Notes:	    See the list of Windows code page identifiers there:      |
|    http://msdn.microsoft.com/en-us/library/windows/desktop/dd317756(v=vs.85).aspx
|		    							      |
|   History:								      |
|    2014-02-27 JFL Created this routine                               	      |
*									      *
\*---------------------------------------------------------------------------*/

int ConvertBuf(const char *pFromBuf, size_t nFromBufSize, UINT cpFrom, char *pToBuf, size_t nToBufSize, UINT cpTo, LPCSTR lpDefaultChar) {
  int n = (int)nFromBufSize;
  WCHAR *pWBuf = (WCHAR *)malloc(sizeof(WCHAR)*n);
  if (!pWBuf) {
    errno = ENOMEM;
    return -1;
  }
  n = MultiByteToWideChar(cpFrom,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pFromBuf,		/* lpMultiByteStr, */
			  n,			/* cbMultiByte */
			  pWBuf,		/* lpWideCharStr, */
			  n			/* cchWideChar, */
			  );
  n = WideCharToMultiByte(cpTo,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,			/* dwFlags, */
			  pWBuf,		/* lpWideCharStr, */
			  n,			/* cchWideChar, */
			  pToBuf,		/* lpMultiByteStr, */
			  (int)nToBufSize,	/* cbMultiByte, */
			  lpDefaultChar,	/* lpDefaultChar, */
			  NULL		/* lpUsedDefaultChar */
			  );
  free(pWBuf);
  if (!n) {
    errno = Win32ErrorToErrno();
    return -1;
  }
  return n;
}

int ConvertString(char *buf, size_t nBufSize, UINT cpFrom, UINT cpTo, LPCSTR lpDefaultChar) {
  int n = lstrlen(buf) + 1;
  if (cpFrom != cpTo) {
    n = ConvertBuf(buf, n, cpFrom, buf, nBufSize, cpTo, lpDefaultChar);
  }
  if (n > 0) { /* If the conversion succeeded */
    n -= 1;	  /* Output string size, not counting the final NUL */
  }
  return n;
}

char *DupAndConvert(const char *string, UINT cpFrom, UINT cpTo, LPCSTR lpDefaultChar) {
  int nBytes;
  char *pBuf;
  nBytes = 4 * ((int)lstrlen(string) + 1); /* Worst case for the size needed */
  pBuf = (char *)malloc(nBytes);
  if (!pBuf) {
    errno = ENOMEM;
    return NULL;
  }
  lstrcpy(pBuf, string);
  nBytes = ConvertString(pBuf, nBytes, cpFrom, cpTo, lpDefaultChar);
  if (nBytes == -1) {
    free(pBuf);
    return NULL;
  }
  pBuf = realloc(pBuf, nBytes+1);
  return pBuf;
}

int CountCharacters(const char *string, UINT cp) {
  int n;
  WCHAR *pWBuf;

  n = (int)lstrlen(string);
  if (!n) return 0;

  pWBuf = (WCHAR *)malloc(sizeof(WCHAR)*n);
  if (!pWBuf) {
    errno = ENOMEM;
    return -1;
  }

  n = MultiByteToWideChar(cp,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
			  0,		/* dwFlags, */
			  string,	/* lpMultiByteStr, */
			  n,		/* cbMultiByte, */
			  pWBuf,	/* lpWideCharStr, */
			  n		/* cchWideChar, */
			  );
  free(pWBuf);
  if (!n) {
    errno = Win32ErrorToErrno();
    return -1;
  }
  return n;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Functions:	    fputsU, vfprintfU, fprintfU, printfU		      |
|									      |
|   Description:    Output UTF-8 strings				      |
|									      |
|   Parameters:     Same as fputs, vfprintf, fprintf, printf		      |
|		    							      |
|   Returns:	    Same as fputs, vfprintf, fprintf, printf		      |
|		    							      |
|   Notes:	    Converts the string to the output code page if needed.    |
|		    							      |
|   History:								      |
|    2014-02-27 JFL Created fprintfU and printfU.                	      |
|    2015-12-09 JFL Restructured them over vfprintfU, itself over fputsU.     |
|    2016-09-13 JFL Fixed warnings.					      |
|		    Do not change the input buffer.                           |
*									      *
\*---------------------------------------------------------------------------*/

/* The codePage global variables are initialized in the main.c _initU() routine */
UINT consoleCodePage = 0;	/* The current console code page (may change) */
UINT systemCodePage = 0;	/* The system code page (unchangeable) */
UINT codePage = 0;		/* The user-specified code page */
/* Heuristic:
   - If codePage is set, use it for encoding all writes.
   - If writing to the console, write in 16-bits mode to make all all Unicode characters get displayed correctly.
   - Else convert the output to the system code page.
*/

/* Make sure we're calling Microsoft routines, not our aliases */
#undef printf
#undef fprintf
#undef vfprintf
#undef fputs
#undef fputc
#undef fwrite

#define IS_ASCII(c) ((c&0x80) == 0)
#define IS_LEAD_BYTE(c) ((c&0xC0) == 0xC0)

/* Test if the output goes to the console, and if it's eligible to Unicode output, initialize it */
static char iWideConsoleInitialized[20] = {0};
int isWideConsole(int iFile) {
  if (codePage) return 0; /* The user wants otherwise, and he's always right */
  if (!isConsole(iFile)) return 0; /* It's not the console anyway */
  if ((iFile < sizeof(iWideConsoleInitialized)) && !iWideConsoleInitialized[iFile]) {
    _setmode(iFile, _O_U16TEXT);
    iWideConsoleInitialized[iFile]++;
  }
  return 1;
}

/* Write a UTF-8 byte, converting full UTF-8 characters to the console code page */
int fputcM(int c, FILE *f, UINT cp) {
  static char buf[5];
  static int nInBuf = 0;
  static int nExpected = 0;
  UINT outputCodePage = consoleCodePage;
  wchar_t wBuf[3];
  int n;
  int iRet;

  if (codePage) { /* The user is always right */
    outputCodePage = codePage;
  }

  if ((cp == CP_UTF8) && (!IS_ASCII(c))) { /* Make sure we got a complete character */
    if (IS_LEAD_BYTE(c)) {
      buf[0] = (char)c;
      nInBuf = 1;
      if ((c&0x20) == 0) {
	nExpected = 2;
      } else if ((c&0x10) == 0) {
	nExpected = 3;
      } else if ((c&0x08) == 0) {
	nExpected = 4;
      } else {
	return EOF;	/* Invalid UTF-8 byte */
      }
      return c;
    }
    /* Else this is a trail byte */
    buf[nInBuf++] = (char)c;
    if (nInBuf < nExpected) return c; /* The UTF-8 character is incomplete */
    /* The UTF-8 character is complete */
    buf[nInBuf++] = '\0';
  } else { /* ANSI or ASCII */
    buf[0] = (char)c;
    buf[1] = '\0';
    nInBuf = 2;
  }

  if (isWideConsole(fileno(f))) {
    /* Output a wide character to guaranty every Unicode character is displayed */
    n = MultiByteToWideChar(cp, 0, buf, (int)nInBuf, wBuf, sizeof(wBuf)/sizeof(wchar_t));
    iRet = fputws(wBuf, f);
  } else {
    if (outputCodePage != cp) { /* Convert the string encoding */
      /* Do not call DupAndConvert(), to avoid the overhead of buffer allocations, etc */
      n = MultiByteToWideChar(cp, 0, buf, (int)nInBuf, wBuf, sizeof(wBuf)/sizeof(wchar_t));
      n = WideCharToMultiByte(outputCodePage, 0, wBuf, n, buf, sizeof(buf), NULL, NULL);
    }
    iRet = fputs(buf, f);
  }
  if ((iRet >= 0) && DEBUG_IS_ON()) fflush(f); /* Slower, but ensures we get everything before crashes! */
  /* printf(" fputc('\\x%02.2X') ", c & 0xFF); */
  if (iRet < 0) return EOF;
  return c;
}

int fputcA(int c, FILE *f) {
  return fputcM(c, f, CP_ACP);
}

int fputcU(int c, FILE *f) {
  return fputcM(c, f, CP_UTF8);
}

/* fputs an MBCS string, converted to the output code page */
int fputsM(const char *buf, FILE *f, UINT cp) {
  int iRet;
  char *pBuf = NULL;
  UINT outputCodePage = consoleCodePage;

  if (codePage) { /* The user is always right */
    outputCodePage = codePage;
  }

  if (isWideConsole(fileno(f))) {
    /* Output a wide string to guaranty every Unicode character is displayed */
    size_t l = strlen(buf);
    int n;
    wchar_t *pwBuf = (wchar_t *)malloc(l * 4);
    if (!pwBuf) return -1;
    n = MultiByteToWideChar(cp, 0, buf, (int)(l+1), pwBuf, (int)(l*2));
    iRet = fputws(pwBuf, f);
    free(pwBuf);
  } else if (outputCodePage != cp) { /* Convert the string encoding and output it */
    pBuf = DupAndConvert(buf, cp, outputCodePage, NULL);
    if (!pBuf) return -1;
    iRet = fputs(pBuf, f);
    free(pBuf);
  } else { /* Output the string as it is */
    iRet = fputs(buf, f);
  }
  if ((iRet >= 0) && DEBUG_IS_ON()) fflush(f); /* Slower, but ensures we get everything before crashes! */
  return iRet; /* Return the error (n<0) or success (n>=0) */
}

int fputsA(const char *buf, FILE *f) {
  return fputsM(buf, f, CP_ACP);
}

int fputsU(const char *buf, FILE *f) {
  return fputsM(buf, f, CP_UTF8);
}

int vfprintfU(FILE *f, const char *pszFormat, va_list vl) { /* vfprintf UTF-8 strings */
  int n;
  char buf[UNICODE_PATH_MAX + 4096];

  n = _vsnprintf(buf, sizeof(buf), pszFormat, vl);
  if (n > 0) { /* If no error (n>=0), and something to output (n>0), do output */
    int iErr = fputsU(buf, f);
    if (iErr < 0) n = iErr;
  }

  return n;
}

int fprintfU(FILE *f, const char *pszFormat, ...) { /* fprintf UTF-8 strings */
  va_list vl;
  int n;

  va_start(vl, pszFormat);
  n = vfprintfU(f, pszFormat, vl);
  va_end(vl);

  return n;
}

int printfU(const char *pszFormat, ...) { /* printf UTF-8 strings */
  va_list vl;
  int n;

  va_start(vl, pszFormat);
  n = vfprintfU(stdout, pszFormat, vl);
  va_end(vl);

  return n;
}

#endif /* _WIN32 */

