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

int ConvertBuf(const char *pFromBuf, size_t nFromBufSize, UINT cpFrom, char *pToBuf, size_t nToBufSize, UINT cpTo, LPCSTR lpDefaultChar);
int ConvertString(char *buf, size_t nBufSize, UINT cpFrom, UINT cpTo, LPCSTR lpDefaultChar);
int CountCharacters(const char *string, UINT cp);
char *DupAndConvert(const char *string, UINT cpFrom, UINT cpTo, LPCSTR lpDefaultChar);

#endif /* defined(_WIN32) */

#endif /* !defined(_ICONV_H) */

