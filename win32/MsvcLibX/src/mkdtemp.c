/*****************************************************************************\
*                                                                             *
*   Filename	    mkdtemp.c						      *
*									      *
*   Description:    WIN32 port of standard C library's mkdtemp()	      *
*                                                                             *
*   Notes:	    							      *
*		    							      *
*   History:								      *
*    2014-02-13 JFL Created this module.				      *
*    2020-04-10 JFL Added mkdtmpA/U/M() definitions for Windows.	      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "debugm.h"	/* SysToolsLib debug macros */

#if defined(_MSDOS)

char *mkdtemp(char *pszName) {
  char *pszXXX, *pc;
  int iErr = 0;
  char *base32 = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

  DEBUG_ENTER(("mkdtemp(\"%s\");\n", pszName));

  /* Find the XXX placeholder at the end of the string */
  for (pszXXX=pszName; pszXXX && *pszXXX; pszXXX++) ;
  while ((pszXXX > pszName) && (*(pszXXX-1) == 'X')) pszXXX--;
  if (!pszXXX || (*pszXXX != 'X')) {
    errno = EINVAL;
    RETURN_PTR_COMMENT(NULL, ("Invalid name template\n"));
  }

  /* Seed the random number generator */
  srand((unsigned)getpid() + ((unsigned)time(NULL) << 10));

  /* Try random names until one directory gets successfully created, or we get an unrecoverable error */
  do {
    for (pc = pszXXX; *pc; pc++) {
      *pc = base32[rand() % 32];
    }
    iErr = _mkdir(pszName);
  } while (iErr && (errno == EEXIST)); /* Try again if this directory exists already */

  RETURN_PTR_COMMENT(iErr ? NULL : pszName, ("\"%s\"\n", pszName));
}

#endif

/*****************************************************************************/

#if defined(_WIN32)

char *mkdtempM(char *pszName, UINT cp) {
  char *pszXXX, *pc;
  int iErr = 0;
  const char *base32 = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

  DEBUG_ENTER(("mkdtemp(\"%s\");\n", pszName));

  /* Find the XXX placeholder at the end of the string */
  for (pszXXX=pszName; pszXXX && *pszXXX; pszXXX++) ;
  while ((pszXXX > pszName) && (*(pszXXX-1) == 'X')) pszXXX--;
  if (!pszXXX || (*pszXXX != 'X')) {
    errno = EINVAL;
    RETURN_PTR_COMMENT(NULL, ("Invalid name template\n"));
  }

  /* Seed the random number generator */
  srand((unsigned)getpid() + ((unsigned)time(NULL) << 10));

  /* Try random names until one directory gets successfully created, or we get an unrecoverable error */
  do {
    for (pc = pszXXX; *pc; pc++) {
      *pc = base32[rand() % 32];
    }
    iErr = mkdirM(pszName, S_IRWXU, cp);
  } while (iErr && (errno == EEXIST)); /* Try again if this directory exists already */

  RETURN_PTR_COMMENT(iErr ? NULL : pszName, ("\"%s\"\n", pszName));
}

char *mkdtempU(char *pszName) {
  return mkdtempM(pszName, CP_UTF8);
}

char *mkdtempA(char *pszName) {
  return mkdtempM(pszName, CP_ACP);
}

#endif
