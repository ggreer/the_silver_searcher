/*****************************************************************************\
*                                                                             *
*   Filename	    GetShareBasePath.c					      *
*									      *
*   Description     Get the name of a network share, on the server side	      *
*                                                                             *
*   Notes	    Common heuristics routine, used by both:		      *
*		    - junction() for creating junctions on network shares     *
*		    - readlink() for reading junctions on network shares      *
*		    							      *
*		    Getting that name cannot be done reliably in all cases.   *
*		    Hence the need for heuristics, to make educated guesses   *
*		    in the cases that can't be done reliably.		      *
*		    							      *
*   History								      *
*    2021-11-24 JFL Created the GetShareBasePath() routine.		      *
*    2021-11-28 JFL Moved it to this new module.			      *
*    2026-02-16 JFL Tweaked the heuristic for network shares named "root".    *
*                                                                             *
*         © Copyright 2021 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#define _UTF8_LIB_SOURCE /* Generate the UTF-8 version of routines */

#include <unistd.h>
#include <iconv.h>
#include "debugm.h"

#ifndef _WIN32
#error "This is a Windows-specific source"
#endif

#include <windows.h>

#ifdef _MSC_VER
#pragma warning(disable:4428)	/* Ignore the "universal-character-name encountered in source" warning */
#endif

/* Read the first configuration line in a small configuration file */
/* Skip blank lines, and comment lines starting with a # */
/* Allow UTF-8 and UTF-16 files */
WCHAR *MlxReadConfigLineW(const WCHAR *pwszConfigFileName) {
  WCHAR *pwszConfigLine = NULL;
  WCHAR *pwsz;
  WCHAR *pwsz2;
  HANDLE hFile = INVALID_HANDLE_VALUE;
  WCHAR wbuf[1024];
  int lwBuf = sizeof(wbuf) / sizeof(WCHAR);
  DWORD dwRead;

  DEBUG_WENTER((L"MlxReadConfigLine(\"%s\");\n", pwszConfigFileName));

  hFile = CreateFileW(pwszConfigFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE) RETURN_CONST_COMMENT(NULL, ("File not found\n"));

  if (ReadFile(hFile, wbuf, (lwBuf-1) * sizeof(WCHAR), &dwRead, NULL)) {
    char *pBuf = (char *)wbuf;
    int l;
    *(WCHAR*)(pBuf+dwRead) = L'\0'; // Whether UTF8 or UTF16, the text buffer is now a NUL-terminated string
    DEBUG_PRINTF(("Read %u bytes\n", dwRead));
    if (pBuf[1]) { // This is UTF8
      pwsz = MultiByteToNewWideString(CP_UTF8, pBuf);
      l = lstrlenW(pwsz);
      XDEBUG_PRINTF(("Converted to %u WCHARs\n", l));
      if (l >= lwBuf) { // Trim it so that it fits in wbuf
      	l = lwBuf - 1;
        pwsz[l] = L'\0';
      }
      CopyMemory(wbuf, pwsz, (l+1)*sizeof(WCHAR));
      free(pwsz);
    }
    XDEBUG_WPRINTF((L"wbuf contents:\n%s~~~~~~~~~~\n", wbuf));
    pwsz = wbuf;
    if (pwsz[0] == L'\uFEFF') pwsz += 1;	// Skip the BOM, if any
    for ( ; pwsz && *pwsz; pwsz++) {
      if (*pwsz == L' ') continue;
      if (*pwsz == L'\t') continue;
      if (*pwsz == L'\r') continue;
      if (*pwsz == L'\n') continue;
      if (*pwsz == L'#') {
	XDEBUG_PRINTF(("Skipping a comment line\n"));
      	pwsz = wcschr(pwsz, L'\n');
      	continue;
      }
      XDEBUG_PRINTF(("Found the configuration line\n"));
      break; // OK, we've found the configuration line
    }
    // Search the end of the configuration line
    pwsz2 = wcschr(pwsz, L'\r');
    if (!pwsz2) pwsz2 = wcschr(pwsz, L'\n');
    if (pwsz2) *pwsz2 = L'\0';
    l = lstrlenW(pwsz);
    // Trim trailing spaces
    while (l && ((pwsz[l-1] == L' ') || (pwsz[l-1] == L'\t'))) pwsz[--l] = L'\0';
    pwszConfigLine = _wcsdup(pwsz);
  }

  CloseHandle(hFile);
  DEBUG_CODE(
    if (!pwszConfigLine) {
      RETURN_CONST_COMMENT(NULL, ("Config line not found\n"));
    } else
  )
  DEBUG_WLEAVE((L"return \"%s\";\n", pwszConfigLine));
  return pwszConfigLine;
}

/* Look for a file called _Base_Path.txt in the share root */
WCHAR *MlxReadShareBasePathFileW(const WCHAR *pwszShareUNC) { // Name must be \\SERVER\SHARE[\SUBPATH]
  WCHAR *pwszFileName;
  WCHAR *pwszConfigLine;
  WCHAR *pwsz;

  pwszFileName = malloc(sizeof(WCHAR)*(lstrlenW(pwszShareUNC) + 20));
  if (!pwszFileName) return NULL;

  lstrcpyW(pwszFileName, pwszShareUNC);
  pwsz = wcschr(pwszFileName+2, L'\\');		// Point to the \ between SERVER and SHARE
  if (pwsz) pwsz = wcschr(pwsz+1, L'\\');	// Point to the \ between SHARE and SUBPATH
  if (pwsz) *pwsz = L'\0'; // Remove the SUBPATH
  lstrcatW(pwszFileName, L"\\_Base_Path.txt");

  pwszConfigLine = MlxReadConfigLineW(pwszFileName);

  free(pwszFileName);
  return pwszConfigLine;
}

WCHAR *MlxGetShareBasePathW(const WCHAR *pwszShareUNC) { // Name must be \\SERVER\SHARE[\SUBPATH]
  WCHAR *pwszShareBasePath = NULL;
  WCHAR *pwsz;
  WCHAR *pwsz2;

  DEBUG_WENTER((L"MlxGetShareBasePath(\"%s\");\n", pwszShareUNC));

  pwsz = wcschr(pwszShareUNC+2, L'\\');
  if (!pwsz) goto fail_no_mem;
  pwsz2 = wcschr(pwsz+1, L'\\');
  if ((pwsz[2] == L'$') && (pwsz2 || !pwsz[3])) { /* This is the root of a shared drive. Ex: \\server\D$ -> D: */
    /* Safe, fast, and simple case */
    DEBUG_PRINTF(("// Case 1: This is a raw drive share\n"));
    pwszShareBasePath = _wcsdup(L"C:");
    if (!pwszShareBasePath) goto fail_no_mem;
    pwszShareBasePath[0] = pwsz[1];	/* Local drive name on the server */
  } else if ((pwszShareBasePath = MlxReadShareBasePathFileW(pwszShareUNC)) != NULL) {
    /* Heuristic: Look for a MsvcLibX-specific file called _Base_Path.txt in the root of the share, containing the information we want */
    /* Note: This is relatively fast, so we do this first, before trying to use WMI to get the same information, as WMI is much slower */
    DEBUG_PRINTF(("// Case 2: Use the path from that _Base_Path.txt config file\n"));
/*} else if (MlxWmiGetShareInfo()) { // To do: Insert a new Case 3 here, using WMI to get the share base path */
  } else if ((pwsz2 || !pwsz[2])) { /* I sometimes do that on some of my servers, to share the root with restricted permissions */
    /* Heuristic: If the share name has a single letter, assume it's the shared drive letter. Ex: \\server\D -> D: */
    DEBUG_PRINTF(("// Case 3: Heuristic: This 1-letter share may be a raw drive share\n"));
    pwszShareBasePath = _wcsdup(L"C:");
    if (!pwszShareBasePath) goto fail_no_mem;
    pwszShareBasePath[0] = pwsz[1];	/* Local drive name on the server */
  } else { /* Often the case when used between home PCs */
    /* Heuristic: Assume the share name is a subdirectory name on the C: drive. Ex: \\server\Public -> C:\Public */
    DEBUG_PRINTF(("// Case 4: Heuristic: Assume it's a folder on the C: drive\n"));
    pwszShareBasePath = malloc(sizeof(WCHAR)*(lstrlenW(pwsz) + 4));
    if (!pwszShareBasePath) goto fail_no_mem;
    pwszShareBasePath[0] = L'C';	/* Local drive name on the server */
    pwszShareBasePath[1] = L':';
    lstrcpyW(pwszShareBasePath+2, pwsz);
    if (pwsz2) pwszShareBasePath[2 + (pwsz2 - pwsz)] = L'\0';
    /* Heuristic: If the share name is root, assume it refers to C:\, not to C:\root */
    if (!_wcsicmp(pwszShareBasePath+3, L"root")) pwszShareBasePath[3] = L'\0';
  }

  /* Now append the optional \SUBPATH */
  if (pwsz2) {
    int l = lstrlenW(pwszShareBasePath);
    if (l == 3) l -= 1; /* Remove the trailing / if it's the root directory */
    pwszShareBasePath = realloc(pwszShareBasePath, sizeof(WCHAR)*(l + lstrlenW(pwsz2) + 1));
    if (!pwszShareBasePath) goto fail_no_mem;
    lstrcpyW(pwszShareBasePath+l, pwsz2);
  }

  DEBUG_WLEAVE((L"return \"%s\";\n", pwszShareBasePath));
  return pwszShareBasePath;

fail_no_mem:
  RETURN_CONST_COMMENT(NULL, ("Not enough memory\n"));
}

