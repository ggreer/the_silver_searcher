/*****************************************************************************\
*                                                                             *
*   Filename:	    strcasestr.c					      *
*									      *
*   Description:    Case-insensitive search for a sub-string		      *
*                                                                             *
*   Notes:	    							      *
*									      *
*   History:								      *
*    2012-02-02 JFL Created stristr.					      *
*    2023-02-17 JFL Renamed as the standard strcasestr, and moved to MsvLibX. *
*									      *
*                   © Copyright 2023 Jean-François Larvoire                   *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#include <string.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    strcasestr						      |
|									      |
|   Description:    Case-insensitive search for a sub-string		      |
|									      |
|   Parameters:     const char *pszString   Source string to search in	      |
|		    const char *pszSearch   String to search in pszString     |
|									      |
|   Returns:	    A pointer to the first substring found, else NULL.	      |
|									      |
|   History:								      |
|    2012-02-02 JFL Created this routine.				      |
*									      *
\*---------------------------------------------------------------------------*/

char *strcasestr(const char *pszString, const char *pszSearch) {
  size_t lString = strlen(pszString);
  size_t lSearch = strlen(pszSearch);
  const char *pc;
  for (pc = pszString; (lSearch <= lString) && *pc; pc++, lString--) {
    if (!_strnicmp(pc, pszSearch, lSearch)) return (char *)pc;
  }
  return NULL;
}

