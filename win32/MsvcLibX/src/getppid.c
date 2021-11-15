/*****************************************************************************\
*                                                                             *
*   Filename	    getppid.c						      *
*									      *
*   Description:    WIN32 port of standard C library's getppid()	      *
*                                                                             *
*   Notes:	    TO DO: Add a DOS version, using the PSP for the pid.      *
*                                                                             *
*		    During the development of this routine, I found a bug     *
*		    in Windows SDK's tlhelp32.h include file in WIN64:        *
*		    If packcking has been changed somewhere before including  *
*		    tlhelp32.h, either due to a #pragma pack directive, or to *
*		    a compiler /Zp option, it will generate a PROCESSENTRY32  *
*		    structure with the wrong size. Then anything can happen,  *
*		    including Process32First failures, or even crashes.	      *
*		    							      *
*   History:								      *
*    2013-03-27 JFL Created this module.				      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#include <unistd.h>

#ifdef _WIN32

#include <windows.h>
#pragma pack(push,8) /* Work around a bug in tlhelp32.h in WIN64, which generates the wrong structure if packing has been changed */
#include <tlhelp32.h>
#pragma pack(pop)

/*
pid_t getpid(void) {
  return (pid_t)GetCurrentProcessId();
}
*/

pid_t getppid(void) {
  pid_t ppid = INVALID_PID;
  pid_t pid = getpid();
  HANDLE h;
  BOOL bFound;
  PROCESSENTRY32 pe = {0};

  h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (h == INVALID_HANDLE_VALUE) {
    return ppid;
  }

  pe.dwSize = sizeof(PROCESSENTRY32);
  for (bFound=Process32First(h, &pe); bFound; bFound=Process32Next(h, &pe)) {
    if ((pid_t)(pe.th32ProcessID) == pid) {
      ppid = pe.th32ParentProcessID;
      break;
    }
  }

  CloseHandle(h);

  return ppid;
}

#endif


