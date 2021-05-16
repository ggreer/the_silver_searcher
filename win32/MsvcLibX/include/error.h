/*****************************************************************************\
*                                                                             *
*   Filename:	    error.h						      *
*                                                                             *
*   Description:    DOS/WIN32 port of the GNU CoreUtils library error funct.  *
*                                                                             *
*   Notes:	    Gotcha: 						      *
*		    The Windows SDK also contains a file called error.h	      *
*		    See C:\Program Files\Microsoft SDKs\Windows\v7.0\INCLUDE  *
*									      *
*   History:								      *
*    2012-10-21 JFL Created this file.                                        *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _ERROR_H_
#define _ERROR_H_

#include "msvclibx.h"

extern void error(int status, int errnum, const char *format, ...);

#endif
