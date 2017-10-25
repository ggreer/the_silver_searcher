/*****************************************************************************\
*                                                                             *
*   Filename	    iconv.h						      *
*									      *
*   Description:    WIN32 port of standard C library's iconv()		      *
*                                                                             *
*   Notes:	    Define here a number of routines, that will eventually    *
*		    be used by iconv().					      *
*		    							      *
*   History:								      *
*    2014-02-27 JFL Created this file.					      *
*    2017-03-03 JFL Added routine ConvertBuf().				      *
*    2017-03-12 JFL Restructured the UTF16 writing mechanism.		      *
*    2017-09-27 JFL Added standard C library routines iconv(), etc.	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_ICONV_H
#define	_ICONV_H	1

#include "msvclibx.h"

#include <stdlib.h>

#if defined(_MSDOS)

/* Count the number of characters (not bytes!) in a string */
/* For now, assume the # of characters is the same as the # of bytes */
/* TO DO: Add support for DOS code pages! */
#define CountCharacters(string, cp) (int)strlen(string)

#endif /* defined(_MSDOS) */


#if defined(_WIN32)

#include <windows.h>

/* Standard C library routines */
typedef long iconv_t;
iconv_t iconv_open(const char *toEnc, const char *fromEnc);
size_t iconv(iconv_t cpFromTo, char **inBuf, size_t *inBytesLeft, char **outBuf, size_t *outBytesLeft);
#define iconv_close(cpFromTo) 0

/* MsvcLibX public routines */
int ConvertBuf(const char *pFromBuf, size_t nFromBufSize, UINT cpFrom, char *pToBuf, size_t nToBufSize, UINT cpTo, LPCSTR lpDefaultChar);
int ConvertString(char *buf, size_t nBufSize, UINT cpFrom, UINT cpTo, LPCSTR lpDefaultChar);
int CountCharacters(const char *string, UINT cp);
char *DupAndConvert(const char *string, UINT cpFrom, UINT cpTo, LPCSTR lpDefaultChar);

/* MsvcLibX internal routines */
int initWideFiles(void);	/* Initialize the UTF-8 output handlers */
int isWideFile(int iFile);	/* Test if the file is open for writing in wide mode */
int isTranslatedFile(int iFile, UINT cp, UINT *pcpOut); /* Test if the file encoding is to be translated from cp, and if so into which other cp. */
#define isConsole(iFile) isatty(iFile)

extern UINT consoleCodePage;	/* The current console code page (may change) */
extern UINT systemCodePage;	/* The system code page (unchangeable) */
extern UINT codePage;		/* The user-specified code page */

#define CP_UNDEFINED ((UINT)-1)	/* 0 is CP_ACP, a valid code page */
#define CP_UTF16 1200		/* The pseudo-code page 1200 is UTF-16 */

#endif /* defined(_WIN32) */

#endif /* !defined(_ICONV_H) */

