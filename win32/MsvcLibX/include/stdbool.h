/*****************************************************************************\
*                                                                             *
*   Filename:	    stdbool.h						      *
*                                                                             *
*   Description:    Standard boolean definitions			      *
*                                                                             *
*   Notes:	    Set standard values. May need to be adapted for C++?      *
*                                                                             *
*   History:								      *
*    2012-10-17 JFL Created this file.					      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _STDBOOL_H
#define _STDBOOL_H

/* #define bool int /* Spec says _Bool */
typedef int bool;
#define true 1
#define false 0
#define __bool_true_false_are_defined 1

#endif
