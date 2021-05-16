/*****************************************************************************\
*                                                                             *
*   Filename	    GetFileAttributes.c					      *
*									      *
*   Description:    UTF-8 version of WIN32's GetFileAttributes()	      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2016-09-12 JFL Created this file.					      *
*    2017-10-03 JFL Simplified the code by using MultiByteToNewWidePath().    *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifdef _WIN32	/* Automatically defined when targeting a Win32 application */

#include <windows.h>	/* Also includes MsvcLibX' WIN32 UTF-8 extensions */
#include <limits.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    GetFileAttributesU					      |
|                                                                             |
|   Description:    Get atributes for a UTF-8 path.			      |
|                                                                             |
|   Parameters:     See WIN32's GetFileAttributes()			      |
|                                                                             |
|   Return value:   The file attributes flags				      |
|                                                                             |
|   Notes:								      |
|                                                                             |
|   History:								      |
|    2014-07-02 JFL Created this routine                                      |
*                                                                             *
\*---------------------------------------------------------------------------*/

DWORD WINAPI GetFileAttributesU(LPCTSTR lpFileName) {
  WCHAR *pwszName;
  DWORD dwAttrs;

  /* Convert the pathname to a unicode string, with the proper extension prefixes if it's longer than 260 bytes */
  pwszName = MultiByteToNewWidePath(CP_UTF8, lpFileName);
  if (!pwszName) return 0;

  /* Get the file attributes, using the Unicode version of the function */
  dwAttrs = GetFileAttributesW(pwszName);

  /* Cleanup and return */
  free(pwszName);
  return dwAttrs;
}

#endif /* defined(_WIN32) */

