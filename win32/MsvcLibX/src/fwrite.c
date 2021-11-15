/*****************************************************************************\
*                                                                             *
*   Filename	    fwrite.c						      *
*									      *
*   Description:    WIN32 UTF-8 versions of fwrite			      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2017-03-03 JFL Created this module.				      *
*    2017-03-05 JFL Rewrote fwriteU() to write UTF16 to the console.	      *
*    2017-03-12 JFL Restructured the UTF16 writing mechanism.		      *
*                                                                             *
*         � Copyright 2017 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of printf routines */

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include "stdio.h"
#include <errno.h>
#include "iconv.h"
#include "unistd.h"	/* For isatty() */
#include "msvclibx.h"
#include "debugm.h"

#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        fwrite*	                                              |
|                                                                             |
|   Description     UTF-8 versions of fwrite				      |
|                                                                             |
|   Parameters      const void *buf	Input buffer address		      |
|                   size_t itemSize	Size in bytes of each item to write   |
|                   size_t nItems	Number if items to write	      |
|                   FILE *f		Output file handle		      |
|                   UINT cp		Input buffer encoding (Code page)     |
|                   							      |
|   Returns         File handle						      |
|                   							      |
|   Notes           All versions below convert the input data encoding to     |
|                   match that of the current command console.                |
|                   							      |
|                   fwriteM	Generic MBCS version supporting any encoding  |
|                   fwriteA     Convert ANSI characters			      |
|                   fwriteU     Convert UTF-8 characters		      |
|                   fwriteUH    Convert UTF-8 characters to the console, or   |
|                   		send the raw data to any other type of stream.|
|                   							      |
|                   Known issue: The return value may be different from the   |
|		    actual # of items written. If the encoding conversion     |
|                   changed the number of bytes, they cannot be the same.     |
|		    This routine returns an estimated count, designed to      |
|		    minimize the risk of confusion of the caller:	      |
|		    * nItems	 => Everything was written		      |
|		    * 0		 => Nothing was written			      |
|		    * In between => The data was partially written	      |
|                   							      |
|   History								      |
|    2017-03-03 JFL Created this routine.                      		      |
*                                                                             *
\*---------------------------------------------------------------------------*/

/* Make sure we're calling Microsoft routines, not our aliases */
#undef printf
#undef fprintf
#undef vfprintf
#undef fputs
#undef fputc
#undef fwrite

/* Write MBCS characters, converted to the console code page */
size_t fwriteM(const void *buf, size_t itemSize, size_t nItems, FILE *f, UINT cp) {
  size_t nToWrite = itemSize * nItems;
  int iCharSize = 1;
  size_t nWritten;
  UINT cpOut;
  int iFile = fileno(f);

  if (isWideFile(iFile)) {
    /* Output a wide string to guaranty every Unicode character is displayed correctly */
    wchar_t *pwBuf = (wchar_t *)malloc(nToWrite * 4);
    int iRet;
    if (!pwBuf) return 0;
    nToWrite = MultiByteToWideChar(cp, 0, buf, (int)nToWrite, pwBuf, (int)(nToWrite*2));
    /* nWritten = fwrite(pwBuf, 2, nToWrite, f); // Crashes! */
    /* Workaround: Make is a string, and use putws() */
    pwBuf[nToWrite] = 0;
    iRet = fputws(pwBuf, f);
    free(pwBuf);
    if (iRet < 0) return 0;
    nWritten = nToWrite;
    iCharSize = 2;
  } else if (isTranslatedFile(iFile, cp, &cpOut)) {
    size_t nBufSize = 4 * nToWrite; /* Worst case for the size needed */
    char *pBuf;
    int n;
    pBuf = (char *)malloc(nBufSize);
    if (!pBuf) return 0; /* malloc sets errno = ENOMEM */
    n = ConvertBuf(buf, nToWrite, cp, pBuf, nBufSize, cpOut);
    if (n < 0) {
      free(pBuf);
      return 0;
    }
    nToWrite = n;
    nWritten = fwrite(pBuf, 1, nToWrite, f);
    free(pBuf);
  } else {
    nWritten = fwrite(buf, 1, nToWrite, f);
  }

  /* Now the problem is that the size written may not be what the user expected, due to the encoding conversion */
  if (nWritten == nToWrite) return nItems; /* Everything was written successfully */
  /* Not everything could be written. Find a reasonable equivalent. */
  nWritten = (nWritten * iCharSize) / itemSize;
  if (nWritten >= nItems) nWritten = nItems - 1;
  return nWritten;
}

/* Write ANSI characters, converted to the console code page */
size_t fwriteA(const void *buf, size_t itemSize, size_t nItems, FILE *f) {
  return fwriteM(buf, itemSize, nItems, f, CP_ACP);
}

/* Write UTF-8 characters, converted to the console code page */
size_t fwriteU(const void *buf, size_t itemSize, size_t nItems, FILE *f) {
  return fwriteM(buf, itemSize, nItems, f, CP_UTF8);
}

#endif /* defined(_WIN32) */

