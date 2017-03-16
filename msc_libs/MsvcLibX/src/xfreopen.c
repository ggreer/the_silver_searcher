/*****************************************************************************\
*                                                                             *
*   Filename	    xfreopen.c						      *
*									      *
*   Description:    WIN32 port of GNU CoreUtils library's xfreopen()	      *
*                                                                             *
*   Notes:	    The GNU CoreUtils library extends the freopen function.   *
*		    msvclibx: Use the standard freopen, or _setmode.	      *
*		    							      *
*   History:								      *
*    2014-03-03 JFL Created this module.				      *
*                                                                             *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <stdio.h>
#include <io.h>         /* For _setmode() */
#include <fcntl.h>      /* For I/O modes */
#include <string.h>
#include "xfreopen.h"

FILE *xfreopen(const char *filename, const char *mode, FILE *stream) {
  int iMode = 0;
  if (filename) return freopen(filename, mode, stream);
  if (strstr(mode, "r+")) {
    iMode = _O_RDWR;
  } else if (strstr(mode, "w+")) {
    iMode = _O_WRONLY | _O_CREAT | _O_TRUNC;
  } else if (strstr(mode, "a+")) {
    iMode = _O_RDWR | _O_CREAT | _O_APPEND;
  } else if (strchr(mode, 'r')) {
    iMode = _O_RDONLY;
  } else if (strchr(mode, 'w')) {
    iMode = _O_WRONLY | _O_CREAT;
  } else if (strchr(mode, 'a')) {
    iMode = _O_WRONLY | _O_CREAT | _O_APPEND;
  }
  if (strchr(mode, 'b')) {
    iMode = _O_BINARY;
  } else if (strchr(mode, 't')) {
    iMode = _O_TEXT;
  }
  _setmode(_fileno(stream), iMode);
  return stream;
}

