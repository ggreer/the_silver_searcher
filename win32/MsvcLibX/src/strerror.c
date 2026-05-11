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
*    2015-05-31 JFL Get the strerror() prototype from string.h.		      *
*    2026-01-18 JFL Added a messsage for ENOTSUP.			      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <stdlib.h>
#include <errno.h>
#include "msvclibx.h"
#include <string.h>

#ifdef _WIN32

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function        strerrorX	                                              |
|                                                                             |
|   Description     Extend strerror() with additional error codes	      |
|                                                                             |
|   Parameters      int errnum		Error number			      |
|                                                                             |
|   Returns         Pointer to the corresponding error message.		      |
|                                                                             |
|   Notes           Workaround for the missing entries in MSVC list	      |
|                                                                             |
|   History								      |
|    2014-03-06 JFL Created this routine.                      		      |
*                                                                             *
\*---------------------------------------------------------------------------*/

#undef strerror

char *strerrorX(int errnum) {
  switch (errnum) {
    case ELOOP:
      return "Symbolic links loop found";
    case ENOTSUP:
      return "Operation not supported";
    default:
      return strerror(errnum);
  }
}

#endif /* defined(_WIN32) */

