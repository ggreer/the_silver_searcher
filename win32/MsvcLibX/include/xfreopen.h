/*****************************************************************************\
*                                                                             *
*   Filename:	    error.h						      *
*                                                                             *
*   Description:    DOS/WIN32 port of the GNU CoreUtils library xfreopen func.*
*                                                                             *
*   Notes:	    msvclibx: Implement using freopen or _setmode.	      *
*									      *
*   History:								      *
*    2014-02-10 JFL Created this file.                                        *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#include "msvclibx.h" /* Generate a library search record to load MsvcLibX.lib. */

extern FILE *xfreopen(const char *filename, const char *mode, FILE *stream);

