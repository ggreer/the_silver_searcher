/*****************************************************************************\
*                                                                             *
*   Filename	    sys/param.h						      *
*                                                                             *
*   Description     DOS/WIN32 port of standard C library's sys/param.h.	      *
*                                                                             *
*   Notes	                                                              *
*                                                                             *
*   History								      *
*    2014-06-10 JFL Created this file.                                        *
*		    							      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H


#ifdef _MSDOS

#define _POSIX_ARG_MAX 127	/* Maximum command line size */

#endif


#ifdef _WIN32

/* #include MSVC_INCLUDE_FILE(sys\param.h) /* Include MSVC's own <sys/param.h> file */

#define _POSIX_ARG_MAX 8191

#endif /* defined(_WIN32) */

#endif /* !defined(_SYS_PARAM_H) */

