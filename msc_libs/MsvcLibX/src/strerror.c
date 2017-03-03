/*****************************************************************************\
*                                                                             *
*   Filename	    strerror.c						      *
*									      *
*   Description:    WIN32 update of strerror				      *
*                                                                             *
*   Notes:	    MSVC defines error messages only up to errno 42 EILSEQ    *
*		    							      *
*   History:								      *
*    2014-03-06 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <stdlib.h>
#include <errno.h>
#include "msvclibx.h"

#ifdef _WIN32

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        strerror	                                              |
|                                                                             |
|   Description     UTF-8 version of strerror				      |
|                                                                             |
|   Parameters      int errnum		Error number			      |
|                                                                             |
|   Returns         Pointer to the corresponding error message.		      |
|                                                                             |
|   Notes                                                                     |
|                                                                             |
|   History								      |
|    2014-03-06 JFL Created this routine.                      		      |
*                                                                             *
\*---------------------------------------------------------------------------*/

#pragma warning(disable:4100) /* Ignore the "unreferenced formal parameter" warning */

char *strerror(int errnum) {
  switch (errnum) {
    case ELOOP:
      return "Symbolic links loop found"; /* Workaround for the missing entry in MSVC list */
    default:
      if (errnum > _sys_nerr) errnum = _sys_nerr;
      return _sys_errlist[errnum];
  }
}

#endif /* defined(_WIN32) */

