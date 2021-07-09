/*****************************************************************************\
*                                                                             *
*   Filename	    lstat.c						      *
*									      *
*   Description:    Redefinitions of standard C library's lstat()	      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-02-14 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

/* st_*time is 64-bits __time64_t  &  st_size is 64-bits __off64_t */

#undef _USE_32BIT_TIME_T
#define _FILE_OFFSET_BITS 64

#include "lstat.c"
