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
*    2018-04-27 JFL Added MultiByteToNewWideString().			      *
*    2021-05-17 JFL Added two new arguments to ConvertBuf() etc.              *
*    2021-05-28 JFL Added a third argument to ConvertBuf() etc; Renamed them  *
*                   with an Ex suffix; And added macros with the old name     *
*                   without the extra three arguments.                        *
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
#ifndef WC_NO_BEST_FIT_CHARS	/* Not defined if targeting Windows 95, but we want it anyway */
#define WC_NO_BEST_FIT_CHARS      0x00000400  /* Do not use best fit chars */
#endif
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS      0x00000080  /* Fail the conversion if any invalid character is present */
#endif
int ConvertBufEx(const char *pFromBuf, size_t nFromBufSize, UINT cpFrom, char *pToBuf, size_t nToBufSize, UINT cpTo, DWORD dwFlags, LPCSTR lpDefaultChar, LPBOOL lpUsedDef);
#define ConvertBuf(pFromBuf, nFromBufSize, cpFrom, pToBuf, nToBufSize, cpTo) ConvertBufEx(pFromBuf, nFromBufSize, cpFrom, pToBuf, nToBufSize, cpTo, 0, NULL, NULL)
int ConvertStringEx(char *buf, size_t nBufSize, UINT cpFrom, UINT cpTo, DWORD dwFlags, LPCSTR lpDefaultChar, LPBOOL lpUsedDef);
#define ConvertString(buf, nBufSize, cpFrom, cpTo) ConvertStringEx(buf, nBufSize, cpFrom, cpTo, 0, NULL, NULL)
int CountCharacters(const char *string, UINT cp);
char *DupAndConvertEx(const char *string, UINT cpFrom, UINT cpTo, DWORD dwFlags, LPCSTR lpDefaultChar, LPBOOL lpUsedDef);
#define DupAndConvert(string, cpFrom, cpTo) DupAndConvertEx(string, cpFrom, cpTo, 0, NULL, NULL)
WCHAR *MultiByteToNewWideStringEx(UINT cp, DWORD dwFlags, const char *string);
#define MultiByteToNewWideString(cp, string) MultiByteToNewWideStringEx(cp, 0, string)
UINT GetBufferEncoding(const char *pszBuffer, size_t nBufSize, DWORD dwFlags); /* If dwFlags is 0, test everything, else test only the specified encodings + Windows' own */
#define BE_TEST_BINARY	0x0001	/* Test if the buffer contains binary data, and if so return CP_UNDEFINED */
#define BE_TEST_ASCII	0x0002	/* Test if the buffer contains ASCII text, and if so return CP_ASCII */
#define BE_TEST_SYSTEM	0x0004	/* Test if the buffer contains Windows text, and if so return CP_ACP */
#define BE_TEST_UTF8	0x0008	/* Test if the buffer contains UTF-8 text, and if so return CP_UTF8 */
#define BE_TEST_UTF16	0x0010	/* Test if the buffer contains UTF-16 text, and if so return CP_UTF16 */
#define BE_TEST_UTF32	0x0020	/* Test if the buffer contains UTF-32 text, and if so return CP_UTF32 */

/* MsvcLibX internal routines */
int initWideFiles(void);	/* Initialize the UTF-8 output handlers */
int isWideFile(int iFile);	/* Test if the file is open for writing in wide mode */
int isTranslatedFile(int iFile, UINT cp, UINT *pcpOut); /* Test if the file encoding is to be translated from cp, and if so into which other cp. */
#define isConsole(iFile) isatty(iFile)

extern UINT consoleCodePage;	/* The current console code page (may change) */
extern UINT systemCodePage;	/* The system code page (unchangeable) */
extern UINT codePage;		/* The user-specified code page */

#define CP_UNDEFINED ((UINT)-1)	/* Can't be 0, which is CP_ACP, a valid code page */
#define CP_UTF16    1200	/* The pseudo-code page 1200 is UTF-16 */
#define CP_UTF16LE  1200	/* The pseudo-code page 1200 is UTF-16 little endian*/
#define CP_UTF16BE  1201	/* The pseudo-code page 1201 is UTF-16 big endian */
#define CP_UTF32    12000	/* The pseudo-code page 12000 is UTF-32 */
#define CP_UTF32LE  12000	/* The pseudo-code page 12000 is UTF-32 little endian*/
#define CP_UTF32BE  12001	/* The pseudo-code page 12001 is UTF-32 big endian */
#define CP_ASCII    20127	/* The pure 7-bits US-ASCII code page */
#define CP_EBCDIC   37		/* IBM US EBCDIC code page */

#endif /* defined(_WIN32) */

#endif /* !defined(_ICONV_H) */

