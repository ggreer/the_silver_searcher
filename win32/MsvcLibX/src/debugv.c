/*****************************************************************************\
*                                                                             *
*   Filename:	    debugv.c						      *
*                                                                             *
*   Description:    Define debug variables used by MsvcLibX.lib in debug mode *
*                                                                             *
*   Notes:	                                                              *
*                                                                             *
*   History:								      *
*    2013-03-27 JFL jf.larvoire@hp.com created this file.                     *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ 2005 security warnings */

#include "debugm.h"

#if defined(_DEBUG)

DEBUG_GLOBALS

#endif /* !defined(_DEBUG) */

