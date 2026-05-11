/*****************************************************************************\
*                                                                             *
*   Filename:	    fnmatch.c						      *
*                                                                             *
*   Description:    DOS/WIN32 port of standard C library's fnmatch()	      *
*                                                                             *
*   Notes:	    TO DO: Manage FNM_PERIOD, FNM_LEADING_DIR.		      *
*                                                                             *
*   History:								      *
*    2012-01-17 JFL Created this file.					      *
*    2013-03-10 JFL In DOS/Windows, the pattern "*.*" actually means "*".     *
*    2014-02-13 JFL Removed warnings.                                         *
*    2014-02-14 JFL In DOS/Windows, the pattern "*." means no extension.      *
*    2014-02-17 JFL Wildcards match the empty string.                         *
*    2014-02-20 JFL Fixed "*." pattern matching.                              *
*    2014-02-28 JFL Added support for UTF-8 pathnames.                 	      *
*    2014-03-05 JFL In debug mode, hide recursive calls.               	      *
*    2026-04-15 JFL Added support for character ranges, like "[a-z]".  	      *
*    2026-04-16 JFL Added support for FNM_PATHNAME.		  	      *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

#include <ctype.h>
#include <string.h>
#include <malloc.h>

#include "fnmatch.h" /* Include our associated .h, in the same dir as this .c. Do not use <>. */
#include "debugm.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#pragma warning(disable:4706) /* Ignore the "assignment within conditional expression" warning */
int fnmatch(const char *pszPattern, const char *pszName, int iFlags) {
  char cp, cn;
  size_t l;
  const char *pc;
  int i;
  int iErr;
  DEBUG_CODE(
  int iHideRecursions = FALSE;
  )

  DEBUG_ENTER(("fnmatch(\"%s\", \"%s\", 0x%x);\n", pszPattern, pszName, iFlags));

  DEBUG_CODE(if(DEBUG_IS_ON() && !XDEBUG_IS_ON()) iHideRecursions = TRUE;)

  /* Special case in DOS/Windows: "*.*" will actually match any name, even without a dot */
  if (!strcmp(pszPattern, "*.*")) pszPattern = "*";

  /* Special case in DOS/Windows: "anything." means anything without extension */
  l = strlen(pszPattern);
  if (l && (pszPattern[l-1] == '.')) {
    int lName = (int)strlen(pszName);
    /* First eliminate the case of forbidden extensions <==> A '.' anywhere before the last character */
    for (i=0; i<(lName-1); i++) { /* Search for a '.' in the name */
      if (pszName[i] == '.') RETURN_CONST_COMMENT(FNM_NOMATCH, ("Forbidden extension found\n"));
    }
    /* If the name doesn't end with a dot, Remove the pattern's trailing '.' */
    if (lName && (pszName[lName-1] != '.')) {
      char *pszPattern2;
      pszPattern2 = _strdup(pszPattern);
      pszPattern2[--l] = '\0';
      /* Recursively do the pattern matching with the new pattern */
      DEBUG_CODE(if (iHideRecursions) iDebug -= 1;)
      iErr = fnmatch(pszPattern2, pszName, iFlags);
      DEBUG_CODE(if (iHideRecursions) iDebug += 1;)
      free(pszPattern2);
      RETURN_INT(iErr);
    }
  }

  for ( ; (cp = *pszPattern) && (cn = *pszName); pszPattern++, pszName++) {
    XDEBUG_PRINTF(("// cp='%c' cn='%c'\n", cp, cn));
    if (iFlags & FNM_CASEFOLD) cn = (char)toupper(cn);
    switch (cp) {
    case '?':
      if (cn == '.') RETURN_CONST_COMMENT(FNM_NOMATCH, ("? does not match a .\n"));
      if ((iFlags & FNM_PATHNAME) && (cn == '/' || cn == '\\')) {
        RETURN_CONST_COMMENT(FNM_NOMATCH, ("? does not match a / in FNM_PATHNAME\n"));
      }
      break;	/* Anything else matches. Continue analysing the pattern. */
    case '*':
      cp = *(++pszPattern);
      if (!cp) {
        if (iFlags & FNM_PATHNAME) { // Then * only matches if no / remain in pszName
	  if (strchr(pszName, '/') || strchr(pszName, '\\')) {
	    RETURN_CONST_COMMENT(FNM_NOMATCH, ("* cannot match across directories\n"));
	  }
        }
        RETURN_CONST_COMMENT(FNM_MATCH, ("* matches whatever remains in the string\n"));
      }
      for ( ; cn = *pszName; pszName++) {
        if ((iFlags & FNM_PATHNAME) && (cn == '/' || cn == '\\')) break; // * stops at the first /
	DEBUG_CODE(if (iHideRecursions) iDebug -= 1;)
	iErr = fnmatch(pszPattern, pszName, iFlags);
	DEBUG_CODE(if (iHideRecursions) iDebug += 1;)
      	if (iErr == FNM_MATCH) RETURN_CONST(FNM_MATCH);
      }
      RETURN_CONST_COMMENT(FNM_NOMATCH, ("No tail string matches the remainder of the pattern\n"));
    case '[': {	/* This is a character class */
      int iFirst;
      int iMatch = 0;
      int iNegate = 0;

      pc = pszPattern;
      pc++; // Skip the '['

      if (*pc == '!' || *pc == '^') { // Handle negative classes
        iNegate = 1;
        pc++;
      }

      // Walk through the character class, looking for the current character.
      // If ']' is the first character (possibly after !/^), treat it normally.
      for (iFirst=1; *pc && (*pc != ']' || iFirst); iFirst=0, pc++) {
        if (pc[1] == '-' && pc[2] && pc[2] != ']') { // It's a range like [a-z]
          char cFirst = pc[0];
          char cLast = pc[2];
          if (iFlags & FNM_CASEFOLD) {
            cFirst = (char)toupper(cFirst);
            cLast = (char)toupper(cLast);
          }
          if (cn >= cFirst && cn <= cLast) iMatch = 1;
          pc += 2; // Skip the '-' and the range end
        } else {				     // It's a simple character
          cp = *pc;
          if (iFlags & FNM_CASEFOLD) cp = (char)toupper(cp);
          if (cn == cp) iMatch = 1;
        }
      }

      if (*pc == ']') { // If the class is indeed closed
	if ((iFlags & FNM_PATHNAME) && (cn == '/' || cn == '\\')) {
	  RETURN_CONST_COMMENT(FNM_NOMATCH, ("Bracket expression cannot match / in FNM_PATHNAME\n"));
	}
        if (iMatch == iNegate) RETURN_CONST_COMMENT(FNM_NOMATCH, ("Character class mismatch\n"));
        // Else the match succeeded
	pszPattern = pc;	// Point it at the closing ']'
	break;			// The main loop will skip that ']'
      }
      // Else, this is just a normal '[' character
      cp = '['; // = *pszPattern; // Will not be affected by the toupper() below
      // Fall through to the default case
    }
    default:
      if (iFlags & FNM_CASEFOLD) cp = (char)toupper(cp);
      if (cp != cn) RETURN_CONST_COMMENT(FNM_NOMATCH, ("Character mismatch\n"));
      break;	/* The character matches. Continue analysing the pattern. */
    } // End switch
  } // End for
  /* '*' and '?' match the empty string */
  if (*pszPattern && !*pszName) {
    int bOnlyWildCards = TRUE;
    for (pc=pszPattern; cp=*pc; pc++) {
      if ((cp != '*') && (cp != '?')) {
	bOnlyWildCards = FALSE;
	break;
      }
    }
    if (bOnlyWildCards) {
      RETURN_CONST_COMMENT(FNM_MATCH, ("WildCards match the empty string\n"));
    }
  }

  /* Special case in DOS/Windows: trailing dot allowed in pattern */
  if ((*pszPattern == '.') && (!*(pszPattern+1)) && (!*pszName)) RETURN_CONST_COMMENT(FNM_MATCH, ("trailing dot matches empty string\n"));

  if (*pszPattern || *pszName) RETURN_CONST_COMMENT(FNM_NOMATCH, ("Something remains that did not match\n"));

  RETURN_CONST_COMMENT(FNM_MATCH, ("Complete match\n"));
}
#pragma warning(default:4706)

