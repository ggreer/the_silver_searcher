/*****************************************************************************\
*                                                                             *
*   Filename:	    fadvise.h						      *
*                                                                             *
*   Description:    WIN32 makeshift version of Coreutils' fadvise.h.          *
*                                                                             *
*   Notes:	    Gives hints to the kernel about future uses of a file.    *
*                   Define constants, and replace functions by void macros.   *
*									      *
*   History:								      *
*    2012-10-17 JFL Created this file.                                        *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

enum fadvice_t { 
  FADVISE_NORMAL,
  FADVISE_SEQUENTIAL,
  FADVISE_NOREUSE,
  FADVISE_DONTNEED, 
  FADVISE_WILLNEED,
  FADVISE_RANDOM 
};
 
/* void	fdadvise(int fd, off_t offset, off_t len, fadvice_t advice) */
#define fdadvise(fd, offset, len, advice)

/* void fadvise(FILE *fp, fadvice_t advice) */
#define fadvise(fp, advice)
