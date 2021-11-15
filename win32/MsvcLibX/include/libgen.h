/*****************************************************************************\
*                                                                             *
*   Filename:	    libgen.h						      *
*                                                                             *
*   Description:    DOS/WIN32 port of standard C library's libgen.h.	      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2016-09-08 JFL Created this file.					      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _LIBGEN_H
#define _LIBGEN_H

char *basename(char *pszPathname);	/* Modifies pathname; Not thread-safe */
char *dirname(char *pszPathname);	/* Modifies pathname; Not thread-safe */

#endif /* !defined(_LIBGEN_H) */
