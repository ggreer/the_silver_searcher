/*****************************************************************************\
*                                                                             *
*   Filename:	    uname.c						      *
*                                                                             *
*   Description:    Get the name of the current system.			      *
*                                                                             *
*   Notes:	    TO DO: Fix the Win32 windows version for Windows >= 8.1,  *
*                          as explained in MSDN's GetVersion function page.   *
*                                                                             *
*                   TO DO: Do not rely on PROCESSOR_ARCHITECTURE to get the   *
*                          processor architecture, because the WIN32 version  *
*			   always sees "x86", even on "AMD64" systems.        *
*                                                                             *
*                   TO DO: Implement sysinfo.c, moving some of the code from  *
*			   here to there, and use it.                         *
*                                                                             *
*   History:								      *
*    2014-05-30 JFL Created this file.                    		      *
*    2017-03-22 JFL Avoid getting Windows version twice to be thread safe.    *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#include "sys/utsname.h"
#include <stdlib.h>	/* For itoa() */

static char major[4] = {0};
static char minor[4] = {0};

#ifdef _MSDOS

#include <dos.h>

static char name[16] = {0};

int uname(struct utsname *pun) {
  unsigned int wVersion = _bdos(0x30, 0, 0); /* int 21H ah=30H Get DOS version */

  _itoa((int)(wVersion & 0x0F), major, 10);
  _itoa((int)((wVersion >> 8) & 0x0F), minor, 10);

  /* Use _bdos instead of _intdosx, because even in large memory mode, static variables will be allocated in the default data segment */ 
  _bdos(0x5E, (unsigned short)(unsigned long)(char far *)name, 0); /* int 21H ax=5E00H ds:dx=&buf Get Machine Name */

  pun->sysname = "MS-DOS"; /* Name of this operating system */
  pun->nodename = name; /* Name of this node on the network */ /* TO DO: Get it from LAN Manager */
  pun->release = major;  /* Current release level of this implementation */
  pun->version = minor;  /* Current version level of this release */
  pun->machine = "x86"; /* Name of the hardware type on which the system is running */

  return 0;
}

#endif /* defined(_MSDOS) */

/*---------------------------------------------------------------------------*/

#ifdef _WIN32

#include <windows.h>

#pragma warning(disable:4996) /* Ignore the "This function or variable may be unsafe" warning for itoa() and getenv() */

int uname(struct utsname *pun) {
  if (!major[0]) {
    DWORD dwVersion = GetVersion();
    _itoa((int)(dwVersion & 0x0F), major, 10);
    _itoa((int)((dwVersion >> 8) & 0x0F), minor, 10);
  }

  pun->sysname = getenv("OS"); /* Name of this operating system */
  pun->nodename = getenv("COMPUTERNAME"); /* Name of this node on the network */
  pun->release = major;  /* Current release level of this implementation */
  pun->version = minor;  /* Current version level of this release */
  pun->machine = getenv("PROCESSOR_ARCHITECTURE"); /* Name of the hardware type on which the system is running */

  return 0;
}

#endif /* defined(_WIN32) */
