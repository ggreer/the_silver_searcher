/*****************************************************************************\
*                                                                             *
*   Filename	    iconv.c						      *
*									      *
*   Description:    WIN32 port of standard C library's iconv()		      *
*                                                                             *
*   Notes:	    Define here a number of routines, that will eventually    *
*		    be used by iconv().					      *
*		    							      *
*		    Also defines routines adding support for UTF-8 in any     *
*		    console code page. (Enabled by defining _UTF8_SOURCE.)    *
*		    							      *
*   History:								      *
*    2014-02-27 JFL Created this module.				      *
*    2015-12-09 JFL Added routines fputsU and vfprintfU.		      *
*    2016-09-13 JFL Fixed warnings in fputsU. Do not change the input buffer. *
*    2017-03-03 JFL Added routine ConvertBuf(), and the fputc() series.	      *
*    2017-03-05 JFL Rewrote fputcU() & fputsU() to write UTF16 to the console.*
*    2017-03-12 JFL Restructured the UTF16 writing mechanism.		      *
*    2017-03-20 JFL Bug fix: _setmodeX() now checks its input values validity.*
*    2017-03-22 JFL Bug fix: Static variables in fputcM must be thread-local. *
*    2017-04-12 JFL Added puts().                                             *
*    2017-05-11 JFL Fixed fputc() for files in binary mode.                   *
*    2017-08-09 JFL Added fprintfM() and printfM().                           *
*    2017-09-27 JFL Added standard C library routines iconv(), etc.	      *
*    2018-04-24 JFL Added fputsW, vfprintfW(), fprintfW() and printfW().      *
*    2018-04-27 JFL Added MultiByteToNewWideString() from iconv.c.            *
*    2021-05-17 JFL Added two new arguments to ConvertBuf() etc.              *
*    2021-05-28 JFL Added conversions from/to UTF-16 and UTF-32.              *
*                   Added a third argument to ConvertBuf() etc; Renamed them  *
*                   with an Ex suffix; And added macros with the old name     *
*                   without the extra three arguments.                        *
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

#pragma warning(disable:4001) /* Ignore the "nonstandard extension 'single line comment' was used" warning */

/* TO DO: Add support for DOS code pages! */

#endif /* defined(_MSDOS) */


#ifdef _WIN32

#include <windows.h>
#include <io.h>         /* For _setmode() */
#include <fcntl.h>      /* For I/O modes */
#include "unistd.h"	/* For isatty() */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    ConvertString					      |
|									      |
|   Description:    Convert a string from one encoding to another	      |
|									      |
|   Parameters:     char *buf	     Buffer containg a NUL-terminated string  |
|		    size_t nBytes    Buffer size			      |
|		    UINT cpFrom	     Initial Windows code page identifier     |
|		    UINT cpTo	     Final Windows code page identifier	      |
|		    LPCSTR lpDfltC   Pointer to the Default Character to use  |
|		    		     (NULL = Use the default default!)        |
|		    LPBOOL lpUsedDef If !NULL, tell if the dflt char was used.|
|		    							      |
|   Returns:	    The converted string size. -1=error, and errno set.	      |
|		    							      |
|   Notes:	    See the list of Windows code page identifiers there:      |
|    http://msdn.microsoft.com/en-us/library/windows/desktop/dd317756(v=vs.85).aspx
|		    							      |
|		    WARNING: Do not insert debug_printf etc calls, as these   |
|		    	     functions are indirectly invoked by debug_printf.|
|		    							      |
|   History:								      |
|    2014-02-27 JFL Created this routine				      |
|    2021-05-17 JFL Added the lpUsedDef argument.			      |
|    2021-05-26 JFL Added conversions from/to UTF-16 and UTF-32.              |
*									      *
\*---------------------------------------------------------------------------*/

#include <conio.h>	// For _cprintf()

int ConvertBufEx(const char *pFromBuf, size_t nFromBufSize, UINT cpFrom, char *pToBuf, size_t nToBufSize,
                 UINT cpTo, DWORD dwFlags, LPCSTR lpDefaultChar, LPBOOL lpUsedDef) { // ConvertBuf() sets these 3 arguments to 0
  int nOut;			/* Number of output bytes */
  int nWide;			/* Number of UTF-16 characters */
  BOOL bUsedDef1 = FALSE;	/* Did the first conversion use the default character? */
  BOOL bUsedDef2 = FALSE;	/* Did the second conversion use the default character? */
  WCHAR *pTempWBuf = NULL;	/* The intermediate buffer, should we need one */
  WCHAR *pWBuf;			/* The UTF-16 buffer actually used */
  int nWBufMax;			/* Number of UTF-16 characters that fit in the UTF-16 buffer */

  /* DO NOT INSERT DEBUG_PRINTF ETC CALLS, AS THIS FUNCTION IS INDIRECTLY INVOKED BY DEBUG_PRINTF */
  XDEBUG_CODE_IF_ON(
    _cprintf("ConvertBufEx(*%p, %ld, %u, *%p, %ld, %u, 0x%x, *%p, *%p)\n", \
   	     pFromBuf, (long)nFromBufSize, cpFrom, pToBuf, (long)nToBufSize, cpTo, dwFlags, lpDefaultChar, lpUsedDef);
  )

  if (!nFromBufSize) return 0;	// Nothing to convert

  if (cpFrom == cpTo) {		// No conversion needed, only a copy
    if (nToBufSize < nFromBufSize) {
      errno = EFBIG;
      nOut = -1;
    } else {
      if (pFromBuf != pToBuf) memcpy(pToBuf, pFromBuf, nFromBufSize);
      nOut = (int)nFromBufSize;
    }
    return nOut;
  }

  /* When do we need an intermediate UTF-16 buffer?
     From               Temp              To
     UTF-16 --------------------copy----> UTF-16
     UTF-16 --------------------convert-> !UTF-16
     !UTF-16 -convert-------------------> UTF16
     !UTF-16 -convert-> UTF-16 -convert-> !UTF16 */
  if (   ((cpFrom != CP_UTF16) && (cpTo != CP_UTF16))	// If (!UTF-16 -> !UTF-16)
      || (pFromBuf == pToBuf) ) {			// Or if converting in place
    nWBufMax = 2*(int)nFromBufSize; // Worst case: Each input byte generating a UTF-16 surrogate pair
    pTempWBuf = (WCHAR *)malloc(sizeof(WCHAR)*nWBufMax); // Enough for worst case
    if (!pTempWBuf) return -1; // errno=ENOMEM
    pWBuf = pTempWBuf;
  } else { // For the case (!UTF-16 -> UTF16); The inverse case (UTF-16 -> !UTF16) will override this in the first switch below
    nWBufMax = (int)(nToBufSize / 2);
    pWBuf = (WCHAR *)pToBuf;
  }

  /* Convert the original encoding to an intermediate UTF-16 version */
  switch (cpFrom) {
    case CP_UTF16:	/* Already UTF-16, so no conversion to do in that first stage */
      if (pFromBuf == pToBuf) { // In this case, we need to copy it, to avoid overwriting the source with the second stage output
      	memcpy(pWBuf, pFromBuf, nFromBufSize);
      } else {			// Else we have nothing to do at all
	pWBuf = (WCHAR *)pFromBuf;
      }
      nWide = (int)(nFromBufSize / 2); /* Drop a possible incomplete character in the end */
      break;
    case CP_UTF32: {	/* UTF-32 not supported by Windows MBCS API, so convert it ourselves */
      DWORD dwU32;
      const char *pIn;
      nWide = 0;
      for (pIn = pFromBuf; (size_t)(pIn - pFromBuf) < (nFromBufSize - 3); pIn += 4) {
	dwU32 = *(const DWORD *)pIn;
	if (dwU32 & 0xFFFF0000) { // Convert the UTF-32 code to two UTF-16 surrogate characters
	  DWORD dwOffset = dwU32 - 0x10000; // Offset beyond the base plane 0
	  if (dwOffset & 0xFFF00000) goto bad_unicode_char; // Can't have more than 20 bits
	  pWBuf[nWide++] = (WORD)(0xD800 + ((dwOffset >> 10) & 0x3FF));	// High 10 bits
	  pWBuf[nWide++] = (WORD)(0xDC00 + (dwOffset & 0x3FF));		// Low 10 bits
	} else { // This is a plane 0 character, that maps directly to a UTF-16 character
	  if ((dwU32 & 0xF800) == 0xD800) {	// Can't be a surrogate character
bad_unicode_char:
	    dwU32 = 0xFFFD; // The Unicode replacement character <?>
	    bUsedDef1 = TRUE;
	  }
	  pWBuf[nWide++] = (WORD)dwU32;
	}
      }
      if ((size_t)(pIn - pFromBuf) < nFromBufSize) { // There's an incomplete character in the end
      	pWBuf[nWide++] = (WORD)0xFFFD; // The Unicode replacement character <?>
	bUsedDef1 = TRUE;
      }
      break;
    }
    default:	/* Anything else is a single byte or multibyte encoding, so let Windows convert it */
      XDEBUG_CODE_IF_ON(
	_cprintf("MultiByteToWideChar(%u, 0x%x, *%p, %d, *%p, %lu)\n", \
		 cpFrom, 0, pFromBuf, (int)nFromBufSize, pWBuf, (long)nWBufMax);
      )
      nWide = MultiByteToWideChar(cpFrom,		/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
				  0,			/* dwFlags, */
				  pFromBuf,		/* lpMultiByteStr, */
				  (int)nFromBufSize,	/* cbMultiByte */
				  pWBuf,		/* lpWideCharStr, */
				  nWBufMax		/* cchWideChar, */
				  );
      break;
  }

  /* Convert from the intermediate UTF-16 version to the final encoding */
  nOut = 0;
  if (nWide) switch (cpTo) {
    case CP_UTF16:	/* Already UTF-16, so nothing to do */
      nOut = nWide * sizeof(WCHAR);
      break;
    case CP_UTF32: {	/* UTF-32 not supported by Windows MBCS API, so convert it ourselves */
      const WORD *pwIn = pWBuf;
      DWORD *pdwOut = (DWORD *)pToBuf;
      int i;
      for (i=0; i<nWide; i++) {
	DWORD dw;
	if ((size_t)((const char *)pdwOut - pToBuf) > (nToBufSize - 4)) {
	  free(pTempWBuf);
	  errno = EFBIG;
	  return -1; // No room for storing the output character
	}
	dw = *(pwIn++);
	if ((dw & 0xFC00) == 0xD800) { // This begins a surrogate pair
	  DWORD dw2 = ((++i) < nWide) ? *(pwIn++) : 0;
	  if ((dw2 & 0xFC00) == 0xDC00) {
	    dw = 0x10000 + (((dw & 0x3FF) << 10) | (dw2 & 0x3FF)); // Recompose the Unicode character
	  } else { // Missing end of a surrogate pair
	    dw = 0xFFFD;
	    bUsedDef2 = TRUE;
	  }
	} else if ((dw & 0xFC00) == 0xDC00) { // Unmatched end of a surrogate pair
	  dw = 0xFFFD;
	  bUsedDef2 = TRUE;
	}
	*(pdwOut++) = dw;
      }
      nOut = (int)((char *)pdwOut - (char *)pToBuf);
      break;
    }
    default: {	/* Anything else is a single byte or multibyte encoding, so let Windows convert it */
      LPBOOL lpUsedDef2 = ((cpTo == CP_UTF7) || (cpTo == CP_UTF8)) ? NULL : &bUsedDef2;
      XDEBUG_CODE_IF_ON(
	_cprintf("WideCharToMultiByte(%u, 0x%x, *%p, %d, *%p, %d, *%p, *%p)\n", \
		 cpTo, dwFlags, pWBuf, nWide, pToBuf, (int)nToBufSize, lpDefaultChar, lpUsedDef2);
      )
      nOut = WideCharToMultiByte(cpTo,			/* CodePage, (CP_ACP, CP_OEMCP, CP_UTF8, ...) */
				 dwFlags,		/* dwFlags, */
				 pWBuf,			/* lpWideCharStr, */
				 nWide,			/* cchWideChar, */
				 pToBuf,		/* lpMultiByteStr, */
				 (int)nToBufSize,	/* cbMultiByte, */
				 lpDefaultChar,		/* lpDefaultChar, */
				 lpUsedDef2		/* lpUsedDefaultChar */
				 );
    }
  }

  /* Cleanup */
  free(pTempWBuf);
  if (!nOut) {
    errno = Win32ErrorToErrno();
    return -1;
  }
  if (lpUsedDef) *lpUsedDef = (bUsedDef1 || bUsedDef2);
  return nOut;
}

int ConvertStringEx(char *buf, size_t nBufSize, UINT cpFrom, UINT cpTo,
		    DWORD dwFlags, LPCSTR lpDefaultChar, LPBOOL lpUsedDef) { // ConvertString() sets these 3 arguments to 0
  int n;

  /* DO NOT INSERT DEBUG_PRINTF ETC CALLS, AS THIS FUNCTION IS INDIRECTLY INVOKED BY DEBUG_PRINTF */
  XDEBUG_CODE_IF_ON(
    _cprintf("ConvertStringEx(*%p, %ld, %u, %u, 0x%x, *%p, *%p)\n", \
   	     buf, (long)nBufSize, cpFrom, cpTo, dwFlags, lpDefaultChar, lpUsedDef);
  )

  switch (cpFrom) {
    case CP_UTF16:
      n = (lstrlenW((LPCWSTR)buf) + 1) * 2;
      break;
    case CP_UTF32:
      for (n=0; (size_t)n<(nBufSize-3); n+=4) {
      	if (!*(DWORD*)(buf+n)) {
      	  n += 4;
      	  break;
      	}
      }
      break;
    default:
      n = lstrlen(buf) + 1;
      break;
  }
  if (cpFrom != cpTo) {
    n = ConvertBufEx(buf, n, cpFrom, buf, nBufSize, cpTo, dwFlags, lpDefaultChar, lpUsedDef);
  }
  if (n > 0) { /* If the conversion succeeded */
    n -= 1;	  /* Output string size, not counting the final NUL */
  }
  return n;
}

char *DupAndConvertEx(const char *string, UINT cpFrom, UINT cpTo,
		      DWORD dwFlags, LPCSTR lpDefaultChar, LPBOOL lpUsedDef) { // DupAndConvert() sets these 3 arguments to 0
  int nBytes;
  char *pBuf;

  /* DO NOT INSERT DEBUG_PRINTF ETC CALLS, AS THIS FUNCTION IS INDIRECTLY INVOKED BY DEBUG_PRINTF */
  XDEBUG_CODE_IF_ON(
    _cprintf("DupAndConvertEx(*%p, %u, %u, 0x%x, *%p, *%p)\n", \
   	     string, cpFrom, cpTo, dwFlags, lpDefaultChar, lpUsedDef);
  )

  nBytes = 4 * ((int)lstrlen(string) + 1); /* Worst case for the size needed */
  pBuf = (char *)malloc(nBytes);
  if (!pBuf) {
    errno = ENOMEM;
    return NULL;
  }
  lstrcpy(pBuf, string);
  nBytes = ConvertStringEx(pBuf, nBytes, cpFrom, cpTo, dwFlags, lpDefaultChar, lpUsedDef);
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

/* Allocate a new wide string converted from the input multi-byte string */
/* In case of failure, sets errno */
WCHAR *MultiByteToNewWideStringEx(UINT cp, DWORD dwFlags, const char *string) {
  int l = lstrlen(string);
  int n = (2*l)+1;	/* Worst case for the number of WCHARs needed */
  WCHAR *pwBuf = (WCHAR *)malloc(n * sizeof(WCHAR));
  WCHAR *pwBuf2;
  if (!pwBuf) return NULL;
  n = MultiByteToWideChar(cp, dwFlags, string, (int)(l+1), pwBuf, n);
  if (!n) {
    errno = Win32ErrorToErrno();
    free(pwBuf);
    return NULL;
  }
  pwBuf2 = realloc(pwBuf, n * sizeof(WCHAR));
  if (pwBuf2) pwBuf = pwBuf2;
  return pwBuf;
}

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Functions:	    iconv						      |
|									      |
|   Description:    Standard C library's iconv				      |
|									      |
|   Parameters:     See Standard C library's iconv	 		      |
|		    							      |
|   Returns:	    See Standard C library's iconv			      |
|		    							      |
|   Notes:	    Use `locale -f` in a Unix box to list supported encodings.|
|		    Also `locale charmap` displays the current encoding.      |
|		    							      |
|   History:								      |
|    2017-09-27 JFL Created this routine.	                	      |
*									      *
\*---------------------------------------------------------------------------*/

static short GetEncodingCP(const char *pszEnc) {
  UINT cp = 0;
  CPINFOEX cpi = {0};
  if (!pszEnc[0]) {						/* Ex: "" */
    return (short)consoleCodePage;
  } else if ((pszEnc[0] == 'C') && (pszEnc[1] == 'P')) {	/* Ex: "CP1252" */
    cp = atoi(pszEnc+2);
  } else if ((pszEnc[0] || pszEnc[1]) && !pszEnc[2]) {		/* Ex: 1252L (Non-standard) (Will fail for multiples of 256) */
    cp = *(unsigned short *)pszEnc;
  } else if (!strcmp(pszEnc, "UTF-7")) {
    cp = CP_UTF7;	/* 65000 */
  } else if (!strcmp(pszEnc, "UTF-8")) {
    cp = CP_UTF8;	/* 65001 */
  } else if (!strcmp(pszEnc, "UTF-16")) {
    cp = CP_UTF16;	/* 1200 */
#if 0
  } else if (!strcmp(pszEnc, "UTF-16LE")) {
    cp = CP_UTF16LE;	/* 1200 */
  } else if (!strcmp(pszEnc, "UTF-16BE")) {
    cp = CP_UTF16BE;	/* 1201 */
#endif
  } else if (!strcmp(pszEnc, "UTF-32")) {
    cp = CP_UTF32;	/* 12000 */
  } else if (   (!strcmp(pszEnc, "ANSI_X3.4-1968"))
             || (!strcmp(pszEnc, "US-ASCII"))
             || (!strcmp(pszEnc, "ASCII"))) {
    cp = CP_ASCII;	/* 20127 */
  }
  if ((!cp) || !GetCPInfoEx(cp, 0, &cpi)) {
    errno = EINVAL;
    return 0;
  }
  return (short)cp;
}

#define WORD0(var) (((WORD *)&var)[0])
#define WORD1(var) (((WORD *)&var)[1])

iconv_t iconv_open(const char *toEnc, const char *fromEnc) {
  iconv_t cpFromTo;
  short cp;
  cp = GetEncodingCP(fromEnc);
  if (!cp) return (iconv_t)-1;
  WORD0(cpFromTo) = cp;
  cp = GetEncodingCP(toEnc);
  if (!cp) return (iconv_t)-1;
  WORD1(cpFromTo) = cp;
  return cpFromTo;
}

size_t iconv(iconv_t cpFromTo, char **inBuf, size_t *inBytesLeft, char **outBuf, size_t *outBytesLeft) {
  UINT cpFrom = WORD0(cpFromTo);
  UINT cpTo = WORD1(cpFromTo);
  char *pFromBuf = *inBuf;
  size_t nFromBufSize = *inBytesLeft;
  char *pToBuf = *outBuf;
  size_t nToBufSize = *outBytesLeft;
  int iToSize;
  if (((!inBuf) || (!*inBuf)) && (outBuf && *outBuf) && (outBytesLeft && *outBytesLeft)) {
    return 0; /* Return the output buffer to its default shift state */
  }
  iToSize = ConvertBuf(pFromBuf, nFromBufSize, cpFrom, pToBuf, nToBufSize, cpTo);
  /* TO DO: Manage the invalid character cases */
  /* TO DO: Manage the query cases, with NULL output pointers or 0 sizes */
  /* TO DO: Manage conversions from/to UTF-16 */
  /* TO DO: Manage conversions from/to UTF-32 */
  if (iToSize == -1) return (size_t)-1;
  *inBuf += *inBytesLeft;
  *inBytesLeft = 0;
  *outBuf += iToSize;
  *outBytesLeft -= iToSize;
  return iToSize;
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
UINT codePage = CP_UNDEFINED;	/* The user-specified code page */
/* Heuristic:
   - If codePage is set, use it for encoding all writes.
   - If writing to the console, write in 16-bits mode to make all all Unicode characters get displayed correctly.
   - Else convert the output to the system code page.
*/

/* Make sure we're calling Microsoft routines, not our aliases */
#undef _setmode

/* List of known file modes */
/* TO DO: This list should actually be dynamically allocated, as the low level
          I/O library supports an illimited number of files. */ 
int iWideFileMode[FOPEN_MAX] = {0};

#define _O_CONV 0x100000  /* Convert encoding from/to the console code page (default for console & pipes) */

/* Change the file translation mode, and record it in iWideFileMode[] */
int _setmodeX(int iFile, int iMode) {
  int iOldMode;
  /* Check the input values validity */
  if ((iFile < 0) || (iFile >= FOPEN_MAX)) {
    errno = EINVAL;
    return -1;
  }
  /* Workaround for an MS C library bug: it cannot switch back directly from WTEXT to BINARY */
  if ((iMode & _O_BINARY) && (iWideFileMode[iFile] & (_O_U16TEXT | _O_WTEXT))) {
    iOldMode = _setmode(iFile, _O_TEXT);
    _setmode(iFile, iMode);
  } else if (iMode) {
    iOldMode = _setmode(iFile, iMode); /* Returns the previous translation mode */
  } else { /* No mode requested. Just record the current one */
    iOldMode = iMode = _setmode(iFile, _O_TEXT); /* Any mode can change to text mode */
    _setmode(iFile, iOldMode);	/* Restore the initial mode */
  }
  iWideFileMode[iFile] = iMode;
  DEBUG_PRINTF(("_setmodeX(%d, 0x%X); // Old mode = 0x%X\n", iFile, iMode, iOldMode));
  return iOldMode;
}

/* Associate a stdio FILE to a low level I/O file. Typically to redirect stdout after a dup()/dup2() */
FILE *fdopenX(int iFile, const char *pszMode) {
  FILE *f = _fdopen(iFile, pszMode);
  if (f) {
    if (isConsole(iFile)) {
      _setmodeX(iFile, _O_WTEXT); /* Change the mode to a wide stream */
    } else {
      _setmodeX(iFile, 0); /* Record the current mode in iWideFileMode[] */
    }
  }
  return f;
}

/* Empty invalid parameter handler.
   Returns, which prevents file I/O functions from crashing when passed invalid file numbers.
   (See https://msdn.microsoft.com/en-us/library/a9yf33zb.aspx)
   The file I/O functions return an error and set errno with EBADF or EINVAL instead */
#undef wprintf
#pragma warning(disable:4100) /* Ignore the "unreferenced formal parameter" warning */
void voidInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved) {
#if 0  /* The following does not work. All arguments are NULL */
  DEBUG_CODE_IF_ON( /* Display error messages in debug mode only */
    wprintf(L"Invalid parameter detected in function %s. File: %s Line: %d\n", function, file, line);
    wprintf(L"Expression: %s\n", expression);
  )
#endif
  return;
}
#pragma warning(default:4100) /* Restore the "unreferenced formal parameter" warning */

/* Initialize the UTF8/UTF16 output mechanism. Run once at program startup */
int initWideFiles(void) {
  int i;
  _invalid_parameter_handler oldHandler;

  /* Record the console and system code pages, to allow converting the output accordingly */
  consoleCodePage = GetConsoleOutputCP();
  systemCodePage = GetACP();

  /* Scan all initial streams. Usually just 1=stdout and 2=stderr. */
  /* Change the invalid parameter handler, to avoid crashing when passing invalid file numbers, and set EBADF instead */
  oldHandler = _set_invalid_parameter_handler(voidInvalidParameterHandler);
  for (i=1; i<FOPEN_MAX; i++) {
    if (isConsole(i)) { /* Switch all console output to UTF-16 */
      /* Note: isatty() returns TRUE for the NUL device. This is just a performance issue? */
      fflush(stdout); /* In the unlikely case that the C library output an error message */
      fflush(stderr); /* Idem */
      _setmodeX(i, _O_WTEXT);
    } else if (_eof(i) >= 0) { /* The file exists, but is not a console */
      iWideFileMode[i] = _O_TEXT; /* Record that by default it's in text mode */
      /* Extra streams aren't detected. For example when invoked with 3>file.log. Why? */
    }
  }
  /* Don't restore the default handler: We want to return EBADF or EINVAL if this is the case */
  /* _set_invalid_parameter_handler(oldHandler); */

  return 0;
}

/* Test if the output goes to a wide file, eligible for writing UTF-16 text. */
/* Change the standard channels width, if the user set a specific output codePage
   that does not match the current width. */
int isWideFile(int iFile) {
  int isStdFile = ((iFile == 1) || (iFile == 2)); /* stdout & stderr */
  if (isStdFile && (iWideFileMode[iFile] & _O_TEXT) && (codePage == CP_UTF16)) { /* The user set the UTF-16 encoding */
    _setmodeX(iFile, _O_WTEXT); /* Change to wide text mode */
    return 1; /* It is a wide file */
  }
  if (iWideFileMode[iFile] & (_O_U16TEXT | _O_WTEXT)) {
    if (isStdFile && (codePage != CP_UNDEFINED) && (codePage != CP_UTF16)) { /* The user set a specific 8-bit encoding */
      _setmodeX(iFile, _O_TEXT); /* Change back to text mode */
      return 0; /* It's a narrow file again */
    }
    return 1; /* It is a wide file */
  }
  return 0; /* It was binary or text to begin with */
}

/* Test if the output goes to a translated stream, and if it's to be translated from cp, and if so to which one */
int isTranslatedFile(int iFile, UINT cp, UINT *pcpOut) {
  int isStdFile = ((iFile == 1) || (iFile == 2)); /* stdout & stderr */
  UINT outputCodePage = consoleCodePage;
  if (codePage != CP_UNDEFINED) { /* The user wants a specific encoding */
    outputCodePage = codePage;
  }
  if (isStdFile && (iWideFileMode[iFile] & _O_TEXT) && (outputCodePage != cp)) {
    if (pcpOut) *pcpOut = outputCodePage; /* It is to be translated to this cp */
    return 1; /* It is to be translated */
  }
  return 0;
}

/* Make sure we're calling Microsoft routines, not our aliases */
#undef printf
#undef fprintf
#undef vfprintf
#undef wprintf
#undef fputs
#undef fputc
#undef fwrite

#if _MSC_VER < 1500 /* Up to VS 8/2005, fputws() is broken. It outputs just the 1st character. */
/* Actually it's _setmode() that does not support _O_WTEXT, but the effect is the same */
int fputwsW(const wchar_t *pws, FILE *f) {
  wint_t wi;
  for ( ;*pws; pws++) {
    wi = fputwc(*pws, f);
    /* if (wi == WEOF) return EOF; */ /* We get a WEOF after each char, yet they do appear! */
  }
  return 0;
}
#endif

#define IS_ASCII(c) ((c&0x80) == 0)
#define IS_LEAD_BYTE(c) ((c&0xC0) == 0xC0)

#define STATIC __declspec(thread) static

/* Write a UTF-8 byte, converting full UTF-8 characters to the console code page */
int fputcM(int c, FILE *f, UINT cp) {
  /* TO DO: There should be one set of static buffers per FILE */
  STATIC char buf[5];
  STATIC int nInBuf = 0;
  STATIC int nExpected = 0;
  wchar_t wBuf[3];
  int n;
  int iRet;
  int iFile = fileno(f);

  if (iWideFileMode[iFile] & _O_BINARY) { /* For binary files, don't change anything */
    return fputc(c, f);
  }
  /* Else this is a text file. The character encoding may need to be converted */
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

  if (isWideFile(iFile)) {
    /* Output a wide character to guaranty every Unicode character is displayed */
    n = MultiByteToWideChar(cp, 0, buf, (int)nInBuf, wBuf, sizeof(wBuf)/sizeof(WCHAR));
    iRet = fputws(wBuf, f);
  } else {
    UINT cpOut;
    if (isTranslatedFile(iFile, cp, &cpOut)) { /* Convert the string encoding */
      /* Do not call DupAndConvert(), to avoid the overhead of buffer allocations, etc */
      n = MultiByteToWideChar(cp, 0, buf, (int)nInBuf, wBuf, sizeof(wBuf)/sizeof(WCHAR));
      n = WideCharToMultiByte(cpOut, 0, wBuf, n, buf, sizeof(buf), NULL, NULL);
    }
    iRet = fputs(buf, f);
  }
  if ((iRet >= 0) && DEBUG_IS_ON()) fflush(f); /* Slower, but ensures we get everything before crashes! */
  /* printf(" fputc('\\x%02X') ", c & 0xFF); */
  if (iRet < 0) return EOF;
  return c;
}

int fputcA(int c, FILE *f) {
  return fputcM(c, f, CP_ACP);
}

int fputcU(int c, FILE *f) {
  return fputcM(c, f, CP_UTF8);
}

/* fputs a Wide string, converted to the output code page */
int fputsW(const wchar_t *pWBuf, FILE *f) {
  int iRet;
  int iFile = fileno(f);

  if (iFile >= FOPEN_MAX) {
    DEBUG_PRINTF(("ERROR: File index too high: fputsW(..., %d)\n", iFile));
  }

  if (isWideFile(iFile)) {
    iRet = fputws(pWBuf, f);
  } else { /* Find the output file encoding */
    UINT cpOut = systemCodePage;
    size_t l = lstrlenW(pWBuf);
    int n = (int)((4 * l) + 1);
    char *pBuf = (char *)malloc(n);
    if (!pBuf) return -1;
    isTranslatedFile(iFile, cpOut, &cpOut); /* Change it to the console code page if needed */
    n = WideCharToMultiByte(cpOut, 0, pWBuf, (int)l, pBuf, n, NULL, NULL);
    pBuf[n] = '\0';
    iRet = fputs(pBuf, f);
    free(pBuf);
  }
  if ((iRet >= 0) && DEBUG_IS_ON()) fflush(f); /* Slower, but ensures we get everything before crashes! */
  return iRet; /* Return the error (n<0) or success (n>=0) */
}

/* fputs an MBCS string, converted to the output code page */
int fputsM(const char *buf, FILE *f, UINT cp) {
  int iRet;
  char *pBuf = NULL;
  UINT cpOut;
  int iFile = fileno(f);

  if (iFile >= FOPEN_MAX) {
    DEBUG_PRINTF(("ERROR: File index too high: fputs(..., %d)\n", iFile));
  }

  if (isWideFile(iFile)) {
    /* Output a wide string to guaranty every Unicode character is displayed */
    size_t l = strlen(buf);
    int n;
    WCHAR *pwBuf = (WCHAR *)malloc(l * 4);
    if (!pwBuf) return -1;
    n = MultiByteToWideChar(cp, 0, buf, (int)(l+1), pwBuf, (int)(l*2));
    iRet = fputws(pwBuf, f);
    free(pwBuf);
  } else if (isTranslatedFile(iFile, cp, &cpOut)) { /* Convert the string encoding and output it */
    pBuf = DupAndConvert(buf, cp, cpOut);
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

int putsA(const char *buf) {
  int iRet = fputsM(buf, stdout, CP_ACP);
  if (iRet >= 0) iRet = fputsM("\n", stdout, CP_ACP);
  return iRet;
}

int putsU(const char *buf) {
  int iRet = fputsM(buf, stdout, CP_UTF8);
  if (iRet >= 0) iRet = fputsM("\n", stdout, CP_UTF8);
  return iRet;
}

int vfprintfW(FILE *f, const wchar_t *pwszFormat, va_list vl) { /* vfprintf Wide strings */
  int n;
  wchar_t wbuf[WIDE_PATH_MAX + 4096];

  n = _vsnwprintf(wbuf, sizeof(wbuf)/sizeof(wchar_t), pwszFormat, vl);
  if (n > 0) { /* If no error (n>=0), and something to output (n>0), do output */
    int iErr = fputsW(wbuf, f);
    if (iErr < 0) n = iErr;
  }

  return n;
}

int vfprintfM(FILE *f, const char *pszFormat, va_list vl, UINT cp) { /* vfprintf MCBS strings */
  int n;
  char buf[WIDE_PATH_MAX + 4096];

  n = _vsnprintf(buf, sizeof(buf), pszFormat, vl);
  if (n > 0) { /* If no error (n>=0), and something to output (n>0), do output */
    int iErr = fputsM(buf, f, cp);
    if (iErr < 0) n = iErr;
  }

  return n;
}

int vfprintfA(FILE *f, const char *pszFormat, va_list vl) { /* vfprintf ANSI strings */
  return vfprintfM(f, pszFormat, vl, CP_ACP);
}

int vfprintfU(FILE *f, const char *pszFormat, va_list vl) { /* vfprintf UTF-8 strings */
  return vfprintfM(f, pszFormat, vl, CP_UTF8);
}

int fprintfW(FILE *f, const wchar_t *pwszFormat, ...) { /* fprintf Wide strings */
  va_list vl;
  int n;

  va_start(vl, pwszFormat);
  n = vfprintfW(f, pwszFormat, vl);
  va_end(vl);

  return n;
}

int fprintfM(UINT cp, FILE *f, const char *pszFormat, ...) { /* fprintf UTF-8 strings */
  va_list vl;
  int n;

  va_start(vl, pszFormat);
  n = vfprintfM(f, pszFormat, vl, cp);
  va_end(vl);

  return n;
}

int fprintfA(FILE *f, const char *pszFormat, ...) { /* fprintf UTF-8 strings */
  va_list vl;
  int n;

  va_start(vl, pszFormat);
  n = vfprintfM(f, pszFormat, vl, CP_ACP);
  va_end(vl);

  return n;
}

int fprintfU(FILE *f, const char *pszFormat, ...) { /* fprintf UTF-8 strings */
  va_list vl;
  int n;

  va_start(vl, pszFormat);
  n = vfprintfM(f, pszFormat, vl, CP_UTF8);
  va_end(vl);

  return n;
}

int printfW(const wchar_t *pwszFormat, ...) { /* printf Wide strings */
  va_list vl;
  int n;

  va_start(vl, pwszFormat);
  n = vfprintfW(stdout, pwszFormat, vl);
  va_end(vl);

  return n;
}

int printfM(UINT cp, const char *pszFormat, ...) { /* printf UTF-8 strings */
  va_list vl;
  int n;

  va_start(vl, pszFormat);
  n = vfprintfM(stdout, pszFormat, vl, cp);
  va_end(vl);

  return n;
}

int printfA(const char *pszFormat, ...) { /* printf UTF-8 strings */
  va_list vl;
  int n;

  va_start(vl, pszFormat);
  n = vfprintfM(stdout, pszFormat, vl, CP_ACP);
  va_end(vl);

  return n;
}

int printfU(const char *pszFormat, ...) { /* printf UTF-8 strings */
  va_list vl;
  int n;

  va_start(vl, pszFormat);
  n = vfprintfM(stdout, pszFormat, vl, CP_UTF8);
  va_end(vl);

  return n;
}

#endif /* _WIN32 */

/*---------------------------------------------------------------------------*/

#if NEEDED

#ifdef _WIN32
#undef fgetc

int fgetcM(FILE *hf, UINT cp) {
  static char szBuf[8];
  static int nBuf = 0;	/* nBuf characters available, starting at offset iBuf */
  static int iBuf = 0;
  int c;
  UINT inputCodePage = consoleCodePage;
  if (codePage != CP_UNDEFINED) { /* The user wants a specific encoding */
    inputCodePage = codePage;
  }
  if (iBuf >= nBuf) { /* If there is no character left in the static buffer */
    c = fgetc(hf);
    if (c == -1) return c;		/* Input error */
    if (cp == inputCodePage) return c;	/* No translation needed */
    nBuf = ConvertBuf((char *)&c, 1, inputCodePage, szBuf, sizeof(szBuf), cp);
    if (nBuf <= 0) return -1;		/* Conversion error */
    iBuf = 0;
  }
  return (int)(unsigned char)(szBuf[iBuf++]);
}

int fgetcU(FILE *hf) {
  return fgetcM(hf, CP_UTF8);
}

#endif /* defined(_WIN32) */

#endif
