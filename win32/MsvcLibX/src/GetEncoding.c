/*****************************************************************************\
*                                                                             *
*   Filename	    GetEncoding.c					      *
*									      *
*   Description     Heuristics for detecting the text encoding in a buffer    *
*                                                                             *
*   Notes	    Choose among the most common cases in Windows:	      *
*		    - The Windows System Encoding (CP_ACP)		      *
*		    - ASCII						      *
*		    - UTF-8						      *
*		    - UTF-16						      *
*		    - UTF-32						      *
*		    							      *
*   History								      *
*    2021-05-18 JFL Created this module, based on code in conv.c.	      *
*    2021-06-01 JFL Improved the heuristic, making it more rigorous.	      *
*                                                                             *
*         © Copyright 2021 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

/* Microsoft C libraries include files */
#include <errno.h>
#include <stdio.h>
#include <string.h>
/* MsvcLibX library extensions */
#include "iconv.h"
#include "debugm.h"

#ifdef _MSDOS

#pragma warning(disable:4001) /* Ignore the "nonstandard extension 'single line comment' was used" warning */

/* TO DO: Add support for DOS code pages! */

#endif

#ifdef _WIN32

#include <WinNLS.h>	/* Actually already included by iconv.h */

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function	    GetBufferEncoding					      |
|									      |
|   Description     Use heuristics to detect the encoding of a text buffer    |
|									      |
|   Parameters      const char *pszBuffer	The unidentified text	      |
|		    size_t nBufSize		The size of that text	      |
|		    DWORD dwFlags		What to look for	      |
|		    				0 = Every supported encoding  |
|		    				Else any combination of:      |
|		    				BE_TEST_BINARY -> CP_UNDEFINED|
|		    				BE_TEST_SYSTEM -> CP_ACP      |
|		    				BE_TEST_ASCII -> CP_ASCII     |
|		    				BE_TEST_UTF8 -> CP_UTF8	      |
|		    				BE_TEST_UTF16 -> CP_UTF16     |
|		    				BE_TEST_UTF32 -> CP_UTF32     |
|		    							      |
|   Returns	    The likely text encoding code page			      |
|		    0 = CP_ACP = The Windows System Code Page		      |
|		    		 or the current console code page	      |
|		    		 or any other non-UTF8 MBCS code page	      |
|		    							      |
|   Notes	    It's impossible to detect any encoding with certainty.    |
|		    The goal is to be fast, with a reasonably good accuracy.  |
|		    Limiting the list of encodings improves the search speed. |
|		    							      |
|		    Limited to the most common cases in Windows:	      |
|		    - The Windows System Encoding (CP_ACP)		      |
|		    - ASCII						      |
|		    - UTF-8						      |
|		    - UTF-16						      |
|		    - UTF-32						      |
|		    							      |
|		    							      |
|   History								      |
|    2021-05-18 JFL Created this routine, based on code in conv.c.	      |
*									      *
\*---------------------------------------------------------------------------*/

UINT GetBufferEncoding(const char *pszBuffer, size_t nBufSize, DWORD dwFlags) {
  UINT cp;
  DEBUG_CODE(char *pszMsg = NULL;)
  
  DEBUG_ENTER(("GetBufferEncoding(%p, %d, 0x%lx);\n", pszBuffer, nBufSize, (unsigned long)dwFlags));

  /* First look for a Unicode BOM: https://en.wikipedia.org/wiki/Byte_order_mark */
  if ((nBufSize >= 3) && !memcmp(pszBuffer, "\xEF\xBb\xBF", 3)) { /* UTF-8 BOM */
    cp = CP_UTF8;
    DEBUG_CODE(pszMsg = "Detected a UTF-8 BOM";)
  } else if ((nBufSize >= 4) && !memcmp(pszBuffer, "\xFF\xFE\x0\x0", 4)) { /* UTF-32 BOM */
    cp = CP_UTF32;
    DEBUG_CODE(pszMsg = "Detected a UTF-32 BOM";)
  } else if ((nBufSize >= 2) && !memcmp(pszBuffer, "\xFF\xFE", 2)) { /* UTF-16 BOM */
    cp = CP_UTF16;
    DEBUG_CODE(pszMsg = "Detected a UTF-16 BOM";)
#if 0
  } else if ((nBufSize >= 2) && !memcmp(pszBuffer, "\xFE\xFF", 2)) { /* UTF-16 BE BOM */
    cp = 1201;
    DEBUG_CODE(pszMsg = "Detected a UTF-16 BE BOM";)
  } else if ((nBufSize >= 5) && !memcmp(pszBuffer, "+/v", 3)) { /* UTF-7 BOM */
    /* The UTF-7 BOM is "+/v8-" if followed by self-representing ASCII characters, else "+/vXX...XX- */
    cp = CP_UTF7;
    DEBUG_CODE(pszMsg = "Detected a UTF-7 BOM";)
#endif
  } else { /* No Unicode BOM. Try detecting UTF-8 or UTF-16 or UTF-32 without BOM */
    size_t n = 0;
    size_t nMax;
    size_t nNonASCII = 0;
    int isASCII = FALSE;
    size_t nNUL8 = 0;
    size_t nValidUTF8 = 0;
    int isValidUTF8 = FALSE;
    size_t nNUL16 = 0;
    size_t nHiNUL8 = 0;
    size_t nValidUTF16 = 0;
    int isValidUTF16 = FALSE;
    size_t nNUL32 = 0;
    size_t nHiNUL16 = 0;
    size_t nValidUTF32 = 0;
    int isValidUTF32 = FALSE;
    size_t nBinary = 0;

    if (dwFlags == 0) dwFlags = ~dwFlags; // Search for every supported encoding

    /* Check if this may be UTF-8 */
    nMax = nBufSize;
    if (nMax && !pszBuffer[nMax-1]) nMax -= 1; // Support strings with a final NUL
    if (dwFlags & BE_TEST_UTF8) {
      isValidUTF8 = TRUE;
      for (n=0; n<nMax; n++) {
	char c = pszBuffer[n];
	if (!c) {
	  nNUL8 += 1;
	  if (nNUL8 > 0) {
	    isValidUTF8 = FALSE;
	    break;		// No need to scan any further, it's something else
	  }
	}
/* See https://en.wikipedia.org/wiki/UTF-8 */
#define IS_ASCII(c)     ((c&0x80) == 0)
#define IS_LEAD_BYTE(c) ((c&0xC0) == 0xC0)
#define IS_TAIL_BYTE(c) ((c&0xC0) == 0x80)
	if (IS_ASCII(c)) continue;
	nNonASCII += 1;
	if (IS_LEAD_BYTE(c)) {
	  int nTailBytesExpected = 0;
	  if ((c&0x20) == 0) {
	    nTailBytesExpected = 1;
	    if ((c == '\xC0') || (c == '\xC1')) isValidUTF8 = FALSE; /* Overlong encoding of 7-bits ASCII */
	  } else if ((c&0x10) == 0) {
	    nTailBytesExpected = 2;
	  } else if ((c&0x08) == 0) {
	    nTailBytesExpected = 3;
	    if ((c >= '\xF5') && (c <= '\xF7')) isValidUTF8 = FALSE; /* Encoding of invalid Unicode chars > \u10FFFF */
	  } else {	/* No valid Unicode character requires a 5-bytes or more encoding */
	    isValidUTF8 = FALSE;
	    break;		// No need to scan any further, it's something else
	  }
	  /* Then make sure that the expected tail bytes are all there */
	  for ( ; nTailBytesExpected && (++n < nBufSize); nTailBytesExpected--) {
	    c = pszBuffer[n];
	    if (!IS_ASCII(c)) nNonASCII += 1;
	    if (!IS_TAIL_BYTE(c)) { /* Invalid UTF-8 sequence */
	      isValidUTF8 = FALSE;
	      break;		// No need to scan any further, it's something else
	    }
	  }
	  if (nTailBytesExpected) isValidUTF8 = FALSE; /* Incomplete UTF-8 sequence at the end of the buffer */
	} else { /* Invalid UTF-8 tail byte not preceeded by a lead byte */
	  isValidUTF8 = FALSE;
	  break;		// No need to scan any further, it's something else
	} /* End if (IS_LEAD_BYTE(c)) */
	if (!isValidUTF8) break;	// No need to scan any further, it's something else
#define ENOUGH8 20 	// If we've seen that many, we're confident it UTF-8
	if (++nValidUTF8 > ENOUGH8) break;	// No need to scan any further, it's UTF-8
      } /* End for each byte in pszBuffer[] */
      DEBUG_PRINTF(("nNUL8 = %lu\n", (unsigned long)nNUL8));
      DEBUG_PRINTF(("nValidUTF8 = %d\n", nValidUTF8));
      DEBUG_PRINTF(("isValidUTF8 = %s\n", isValidUTF8 ? "TRUE" : "FALSE"));
      if (nValidUTF8 > ENOUGH8) goto it_is_utf8; // No need to scan any further, it's UTF-8
    }

    /* Check if this may be ASCII - Must immedately follow the test for UTF-8 */
    if (dwFlags & BE_TEST_ASCII) {
      isASCII = TRUE;
      if ((nNUL8 <= 0) && !nNonASCII) for ( ; n<nMax; n++) {
	char c = pszBuffer[n];
	if (!c) {
	  nNUL8 += 1;
	  if (nNUL8 > 0) break;	// No need to scan any further, it's something else
	}
	if (c & 0x80) {
	  nNonASCII += 1;
	  break;			// No need to scan any further, it's something else
	}
	// To be sure it's ASCII, we must scan to the very end
      }
      isASCII = isASCII && (nNUL8 <= 0) && !nNonASCII;
      DEBUG_PRINTF(("nNUL8 = %lu\n", (unsigned long)nNUL8));
      DEBUG_PRINTF(("nValidASCII = %d\n", n - nNonASCII));
      DEBUG_PRINTF(("isASCII = %s\n", isASCII ? "TRUE" : "FALSE"));
      if (isASCII) goto it_is_ascii; // No need to scan any further, it's ASCII
    }

    /* Check if this may be UTF-16 */
    if (dwFlags & BE_TEST_UTF16) {
      isValidUTF16 = TRUE;
      nMax = nBufSize & ~1;
      if (nMax && !*(WORD *)(pszBuffer+nMax-2)) nMax -= 2; // Support strings with a final NUL
      for (n=0; n < nMax; n+=2) {
	WORD w = *(WORD *)(pszBuffer+n);
	if (!w) {
	  nNUL16 += 1;
	  if (nNUL16 > 0) {
	    isValidUTF16 = FALSE;
	    break;		// No need to scan any further, it's something else
	  }
	}
	if (!(w & 0xFF00)) nHiNUL8 += 1;
	if (   (w == 0xFFFF) || (w == 0xFFFE)	// Forbidden values
	    || ((w & 0xFC00) == 0xDC00)		// Unmatched end of a surrogate pair
	    || (   ((w & 0xFC00) == 0xD800)       // This begins a surrogate pair
		&& (   (n >= (nBufSize-3))
		    || ((*(WORD *)(pszBuffer+(n+=2)) & 0xFC00) != 0xDC00)
		    )				// Missing end of a surrogate pair
		)
	    ) {
	  isValidUTF16 = FALSE;
	  break;			// No need to scan any further, it's something else
	}
#define ENOUGH16 10000 // If we've seen that many, we're confident it UTF-16
	if (++nValidUTF16 > ENOUGH16) break;	// No need to scan any further, it's UTF-16
      }
      DEBUG_PRINTF(("nNUL16 = %lu\n", (unsigned long)nNUL16));
      DEBUG_PRINTF(("nValidUTF16 = %d\n", nValidUTF16));
      DEBUG_PRINTF(("isValidUTF16 = %s\n", isValidUTF16 ? "TRUE" : "FALSE"));
      if (nHiNUL8 && (nValidUTF16 > ENOUGH16)) goto it_is_utf16; // No need to scan any further, it's UTF-16
    }

    /* Check if this may be UTF-32 */
    if (dwFlags & BE_TEST_UTF32) {
      isValidUTF32 = TRUE;
      nMax = nBufSize & ~3;
      if (nMax && !*(DWORD *)(pszBuffer+nMax-4)) nMax -= 4; // Support strings with a final NUL
      for (n=0; n < nMax; n+=4) {
	DWORD dw = *(DWORD *)(pszBuffer+n);
	if (!dw) {
	  nNUL32 += 1;
	  if (nNUL32 > 0) {
	    isValidUTF32 = FALSE;
	    break;		// No need to scan any further, it's something else
	  }
	}
	if (!(dw & 0xFFFF0000)) nHiNUL16 += 1;
	if (   (dw & 0xFFE00000)			// More than 21 bits
	    || (dw == 0xFFFF) || (dw == 0xFFFE)	// Forbidden values
	    || ((dw & 0xF800) == 0xD800)		// Reserved for UTF16 surrogate pairs
	    ) {
	  isValidUTF32 = FALSE;
	  break;			// No need to scan any further, it's something else
	}
#define ENOUGH32 10000 // If we've seen that many, we're confident it UTF-32
	if (++nValidUTF32 > ENOUGH32) break;	// No need to scan any further, it's UTF-32
      }
      DEBUG_PRINTF(("nNUL32 = %lu\n", (unsigned long)nNUL32));
      DEBUG_PRINTF(("nValidUTF32 = %d\n", nValidUTF32));
      DEBUG_PRINTF(("isValidUTF32 = %s\n", isValidUTF32 ? "TRUE" : "FALSE"));
      if (nHiNUL16 && (nValidUTF32 > ENOUGH32)) goto it_is_utf32; // No need to scan any further, it's UTF-32
    }

    /* Check if this may be binary */
    if (dwFlags & BE_TEST_BINARY) {
      nMax = nBufSize;
      if (nMax && !pszBuffer[nMax-1]) nMax -= 1; // Support strings with a final NUL
      for (n=0; n<nMax; n++) {
	char c = pszBuffer[n];
	if (  ((c >= '\x00') && (c < '\x07'))	// NUL <= c < BEL
	    || ((c > '\x0D') && (c < '\x1A'))	// CR  <  c < Ctrl-Z
	    || ((c > '\x1B') && (c < '\x20'))	// ESC <  c < Space
	    || (c == '\xFF')
	    ) {
#define ENOUGHBIN 10 // If we've seen that many, we're confident it's not an 8-bit text encoding
	  if (++nBinary > ENOUGHBIN) break;	// No need to scan any further, it's binary
	}
      }
    } else {
      nBinary = nNUL8;
    }
    DEBUG_PRINTF(("nBinary = %lu\n", (unsigned long)nBinary));

    /* Heuristics for identifying an encoding from the information gathered so far,
       if we have not found enough data to be sure alread
       Note that this choice is probabilistic. It may not be correct in all cases. */
    if (isASCII) {
it_is_ascii:
      cp = CP_ASCII;
      DEBUG_CODE(pszMsg = "Everything was plain ASCII";)
    } else if ((nHiNUL16 > 0) && isValidUTF32) {
it_is_utf32:
      cp = CP_UTF32; /* Assume it's UTF-32 */
      DEBUG_CODE(pszMsg = "Detected UTF-32 without BOM";)
    } else if ((nHiNUL8 > 0) && isValidUTF16) {
it_is_utf16:
      cp = CP_UTF16; /* Assume it's UTF-16 LE for now, which is the default in Windows */
      DEBUG_CODE(pszMsg = "Detected UTF-16 without BOM";)
    } else if ((nBinary == 0) && isValidUTF8) {
it_is_utf8:
      cp = CP_UTF8; /* We've verified this is valid UTF-8 */
      DEBUG_CODE(pszMsg = "Detected UTF-8 without BOM";)
    } else if ((nBinary == 0) && nNonASCII && (dwFlags & BE_TEST_SYSTEM)) {
      /* Recommended: Default to the current console code page for input from a pipe,
		      and to the Windows system code page for input from a file. */
      cp = CP_ACP; /* Default to the ANSI Code Page */
      DEBUG_CODE(pszMsg = "Detected Windows System code page";)
    } else {
      cp = CP_UNDEFINED;
      DEBUG_CODE(pszMsg = "Unrecognized encoding, possibly binary";)
    }
  }
  DEBUG_LEAVE(("return %u; // %s\n", cp, pszMsg));
  return cp;
}

#endif

