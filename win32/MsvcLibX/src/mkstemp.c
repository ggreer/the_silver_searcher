/*****************************************************************************\
*                                                                             *
*   Filename	    mkstemp.c						      *
*									      *
*   Description:    WIN32 port of standard C library's mkstemp()	      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-03-03 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

int mkstemp(char *pszName) {
  char *pszXXX, *pc;
  int hFile = -1;
  char *base32 = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

  /* Find the XXX placeholder at the end of the string */
  for (pszXXX=pszName; pszXXX && *pszXXX; pszXXX++) ;
  while ((pszXXX > pszName) && (*(pszXXX-1) == 'X')) pszXXX--;
  if (!pszXXX || (*pszXXX != 'X')) {
    errno = EINVAL;
    return -1;
  }

  /* Seed the random number generator */
  srand((unsigned)getpid() + ((unsigned)time(NULL) << 10));

  /* Try random file names until one file gets successfully created */
  while (hFile == -1) {
    for (pc = pszXXX; *pc; pc++) {
      *pc = base32[rand() % 32];
    }
    hFile = _open(pszName, O_CREAT|O_EXCL|O_RDWR, S_IREAD|S_IWRITE);
  }

  return hFile;
}

