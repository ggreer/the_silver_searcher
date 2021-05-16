/*****************************************************************************\
*                                                                             *
*   Filename	    getpagesize.c					      *
*									      *
*   Description:    WIN32 UTF-8 version of getpagesize			      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2017-02-27 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2017 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include "msvclibx.h"
#include "unistd.h"

#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        getpagesize	                                              |
|                                                                             |
|   Description     Get the virtual memory page size                          |
|                                                                             |
|   Parameters      None						      |
|                                                                             |
|   Returns         The page size					      |
|                                                                             |
|   Notes                                                                     |
|                                                                             |
|   History								      |
|    2017-02-27 JFL Created this routine.                      		      |
*                                                                             *
\*---------------------------------------------------------------------------*/

int getpagesize(void) {
  SYSTEM_INFO si;

  GetSystemInfo(&si);
  return (int)(si.dwPageSize);
}

#endif /* defined(_WIN32) */
