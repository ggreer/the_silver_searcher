/*****************************************************************************\
*                                                                             *
*   Filename:	    sys/utsname.h					      *
*                                                                             *
*   Description:    Defines structure utsname and function uname().           *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-05-30 JFL Created this file.                    		      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_SYS_UTSNAME_H
#define	_SYS_UTSNAME_H	1

#include "msvclibx.h"

struct utsname {
  char *sysname;  /* Name of this implementation of the operating system */
  char *nodename; /* Name of this node on the network */
  char *release;  /* Current release level of this implementation */
  char *version;  /* Current version level of this release */
  char *machine;  /* Name of the hardware type on which the system is running */
};

int uname(struct utsname *);

#endif /* _SYS_UTSNAME_H */
