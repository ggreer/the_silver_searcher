/*****************************************************************************\
*                                                                             *
*   Filename	    getline.c						      *
*									      *
*   Description     Read a line from a stream                                 *
*                                                                             *
*   Notes	                                                              *
*                                                                             *
*   History								      *
*    2022-12-11 JFL Created this module.                                      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include "debugm.h"

/*---------------------------------------------------------------------------*\
|		    							      |
|  Function         getline                                                   |
|		    							      |
|  Description      Read a line from a file, growing the buffer if needed     |
|		    							      |
|  Input            char **pBuf		Address of output buffer pointer      |
|                   size_t *pBufSize	Address of output buffer size	      |
|		    FILE *f		Input file handle		      |
|		    							      |
|  Output           The size of the line read, or -1 if EOF or error.	      |
|		    							      |
|  History                                                                    |
|    2017-01-06 JFL Initial implementation of getLine() in inicomp.c.         |
|    2022-12-11 JFL Converted it to the standard getline() routine.           |
|		    							      |
\*---------------------------------------------------------------------------*/

DEBUG_CODE(
/* Output debug messages with the line number.
   Having a linked list of line counters allows supporting recursive routines,
   each reading a file with getline() in a loop. */
typedef struct _sCounter {
  struct _sCounter *pNext;
  FILE *f;
  long lNumber;
} sCounter;
static sCounter *psCounter = NULL;
static int iDepth = -1;
static void CloseCounter(FILE *f) {
  if (psCounter && (f == psCounter->f)) {
    psCounter = psCounter->pNext;
    iDepth -= 1;
  }
}
)

#define FAILURE ((ssize_t)-1L)

ssize_t getline(char **ppBuf, size_t *pBufSize, FILE *f) {
  char *pszLine = *ppBuf;
  size_t offset = 0;
  size_t l;

  if ((!ppBuf) || (!pBufSize)) {
    errno = EINVAL;
    return FAILURE;
  }

  if (!pszLine) {
    pszLine = malloc(128);
    if (!pszLine) return FAILURE;
    *ppBuf = pszLine;
    *pBufSize = 128;
  }

  DEBUG_CODE(
    if ((!psCounter) || (f != psCounter->f)) { /* Open a new line counter */
      sCounter *psCounter2 = malloc(sizeof(sCounter));
      if (!psCounter2) return FAILURE;
      psCounter2->pNext = psCounter;
      psCounter2->f = f;
      psCounter2->lNumber = 0;
      psCounter = psCounter2;
      iDepth += 1;
    }
  )

  if (!fgets(pszLine, (int)*pBufSize, f)) {
    DEBUG_CODE(
      CloseCounter(f);
    )
    return FAILURE;
  }
  l = strlen(pszLine);

  DEBUG_CODE(
    psCounter->lNumber += 1; /* Increment the line number */
  )

  /* Check if the buffer is too small, and needs to be extended to get the full line */
  while ((!feof(f)) && ((offset+l) == (*pBufSize-1)) && (pszLine[l-1] != '\n')) {
    *pBufSize *= 2; /* Double its size, to avoid doing this too often */
    if (*pBufSize > SSIZE_MAX) {
      errno = EOVERFLOW;
      if (errno < _sys_nerr) _sys_errlist[errno] = "line too long";
      l = (size_t)FAILURE;
      break;
    }
    DEBUG_PRINTF(("%*sgetline() File #%d line %ld overflowed. Expanding buffer to %lu bytes\n",
		  iDepth*2, "", iDepth, psCounter->lNumber, (unsigned long)*pBufSize));
    *ppBuf = realloc(*ppBuf, *pBufSize);
    if (!*ppBuf) {
      l = (size_t)FAILURE;
      break;
    }
    pszLine = *ppBuf + offset;
    if (!fgets(pszLine+l, (int)(*pBufSize-(offset+l)), f)) break;
    l += strlen(pszLine+l);
  }

  DEBUG_CODE(
    if ((l == (size_t)FAILURE) || feof(f)) CloseCounter(f);
  )
  return (ssize_t)l;
}
