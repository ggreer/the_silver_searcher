/*****************************************************************************\
*                                                                             *
*   Filename:	    stdarg.h						      *
*                                                                             *
*   Description:    MsvcLibX extensions to stdarg.h.			      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2019-09-25 JFL Created this file.                                        *
*									      *
*         © Copyright 2019 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_MSVCLIBX_STDARG_H
#define	_MSVCLIBX_STDARG_H	1

#include "msvclibx.h"

#include MSVC_INCLUDE_FILE(stdarg.h) /* Include MSVC's own <stdarg.h> file */

/*
   Visual Studio 8 to 11 do not define va_copy
   Visual Studio 12 and later define it as a macro.
   That macro definition varies depending on cases, but it is hoped that in all
   cases except maybe _M_CEE (= .NET CLR target), the following definition will be sufficient.
*/
#ifndef va_copy
#define va_copy(destination, source) ((destination) = (source))
#endif

#endif /* defined(_MSVCLIBX_STDARG_H)  */

