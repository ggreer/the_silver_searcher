/*****************************************************************************\
*                                                                             *
*   Filename:	    fnmatch.h						      *
*                                                                             *
*   Description:    DOS/WIN32 port of standard C library's fnmatch.h.	      *
*                                                                             *
*   Notes:	    Reference for fnmatch and glob:			      *
*    http://www.cs.utah.edu/dept/old/texinfo/glibc-manual-0.02/library_9.html *
*                                                                             *
*   History:								      *
*    2012-01-17 JFL Created this file.					      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _FNMATCH_H
#define _FNMATCH_H      1

#include "msvclibx.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* Bits set in the FLAGS argument to fnmatch() */
#define FNM_PATHNAME	0x01	/* Wildcards don't match '/' or '\\' */
#define FNM_FILE_NAME FNM_PATHNAME /* Equivalent GNU name */
#define FNM_NOESCAPE	0x02	/* Backslashes don't quote special chars (Irrelevant in DOS/Windows) */
#define FNM_PERIOD	0x04	/* Wildcards don't match leading dots */
#define FNM_LEADING_DIR	0x08	/* Match up to the first '/' or '\\' */
#define FNM_CASEFOLD	0x10	/* Case-insentitive comparison */

/* Values returned by fnmatch() */
#define FNM_MATCH	0	/* Non standard, but makes fnmatch.c more readable */ 
#define FNM_NOMATCH	1

/* Value returned by fnmatch() if unsupported */
#define FNM_NOSYS      (-1)

/* Match NAME against the filename pattern PATTERN,
   returning zero if it matches, FNM_NOMATCH if not.  */
extern int fnmatch(const char *pszPattern, const char *pszName, int iFlags);

#ifdef  __cplusplus
}
#endif

#endif /* !defined(_FNMATCH_H) */

