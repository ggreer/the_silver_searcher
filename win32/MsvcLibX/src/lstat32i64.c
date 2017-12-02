/*****************************************************************************\
*                                                                             *
*   Filename	    lstat32i64.c					      *
*									      *
*   Description:    Redefinitions of standard C library's lstat32i64()	      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-02-14 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef  _WIN64 /* WIN64 does not support 32-bits time_t */

/* st_*time is 32-bits __time32_t  &  st_size is 64-bits __off64_t */

#define _USE_32BIT_TIME_T
#define _FILE_OFFSET_BITS 64

#include "lstat.c"
#endif
