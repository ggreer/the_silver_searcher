/*****************************************************************************\
*                                                                             *
*   Filename	    filetime.c						      *
*									      *
*   Description:    MsvcLibX internal routines for managing file times	      *
*                                                                             *
*   Notes:	                                                              *
*		    							      *
*   History:								      *
*    2014-02-26 JFL Created this module.				      *
*    2014-03-24 JFL Renamed "statx.h" as the standard <sys/stat.h>.	      *
*    2014-07-03 JFL Filetime2String: Output time with µs precision if possib. *
*    2016-09-13 JFL Fixed a warning.					      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1 /* Avoid Visual C++ security warnings */

#include <time.h>	/* Define time_t */
#include <sys/stat.h>

#ifdef _MSDOS

/* 
   Convert a DOS date/time to a Unix time_t
   DOS dates start from January 1, 1980. DOS times have a 2-second resolution.
   A Unix time_t is the number of 1-second intervals since January 1, 1970.
   time_ts are expressed in the GMT time zone. DOS times in the current local time.
*/
time_t Filetime2Timet(uint16_t date, uint16_t time) {
  unsigned int year, month, day, hour, minute, second;
  struct tm stm;

  /* Decode fields */
  year = 1980 + ((date & 0xFE00) >> 9);
  month = (date & 0x1E0) >> 5;
  day = date & 0x1F;
  hour = (time & 0xF800) >> 11;
  minute = (time & 0x7E0) >> 5;
  second = 2 * (time & 0x1F);

  stm.tm_year = (int)year - 1900;
  stm.tm_mon  = (int)month - 1;
  stm.tm_mday = (int)day;
  stm.tm_hour = (int)hour;
  stm.tm_min  = (int)minute;
  stm.tm_sec  = (int)second;
  stm.tm_isdst = -1; /* Let mktime decide if DST is in effect or not */

  return mktime(&stm);
}

#if 0
/* Older version of the same, trying to generate the time_t manually.
   Did not handle DST well */
time_t Filetime2Timet(uint16_t date, uint16_t time) {
  unsigned int year, month, day, hour, minute, second;
  unsigned int olympiads; /* 4-year periods */
  unsigned long t = 0;

  /* Decode fields */
  year = 1980 + ((date & 0xFE00) >> 9);
  month = (date & 0x1E0) >> 5;
  day = date & 0x1F;
  hour = (time & 0xF800) >> 11;
  minute = (time & 0x7E0) >> 5;
  second = 2 * (time & 0x1F);

  /* Count days */
  year -= 1970; /* Start of Unix time_t epoch */
  olympiads = year / 4;
  year = year % 4;
  t = olympiads * (365 + 365 + 366 + 365);
  switch (year) {
    case 3: t += 366;
    case 2: t += 365;
    case 1: t += 365;
  }
  switch (month) {
    case 12: t += 30;
    case 11: t += 31;
    case 10: t += 30;
    case 9: t += 31;
    case 8: t += 31;
    case 7: t += 30;
    case 6: t += 31;
    case 5: t += 30;
    case 4: t += 31;
    case 3: t += (year == 2) ? 29 : 28;
    case 2: t += 31;
  }
  t += day-1;

  /* Count seconds */
  t *= 24;
  t += hour;
  t *= 60;
  t += minute;
  t *= 60;
  t += second;

  /* Correct for the timezone (As DOS returns local times, but time_t is UTC-based) */
  t += timezone;
  
  /* Still need correction for DST */

  return (time_t)t;
}
#endif

/* Generate a string with the local file time, in the ISO 8601 date/time format */
char *Filetime2String(uint16_t date, uint16_t time, char *pBuf, size_t nBufSize) {
  unsigned int year, month, day, hour, minute, second;

  /* Decode fields */
  year = 1980 + ((date & 0xFE00) >> 9);
  month = (date & 0x1E0) >> 5;
  day = date & 0x1F;
  hour = (time & 0xF800) >> 11;
  minute = (time & 0x7E0) >> 5;
  second = 2 * (time & 0x1F);

  if (nBufSize >= 20) {
    sprintf(pBuf, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
  } else {
    return "Buffer too small";
  }
  return pBuf;
}

#endif /* defined(_MSDOS) */

/*****************************************************************************/

#ifdef _WIN32

#include <windows.h>

/*---------------------------------------------------------------------------*\
*                                                                             *
|   Function:	    LocalFileTime					      |
|									      |
|   Description:    Convert a file GMT time to the local struct tm to display |
|									      |
|   Parameters:     const time_t *t		The file GMT time_t	      |
|									      |
|   Returns:	    struct tm *						      |
|									      |
|   Notes:	    This routine is a replacement for Posix' localtime(),     |
|		    using Windows' specific algorithm for generating local    |
|		    file times.						      |
|		    							      |
|		    Windows displays file times based on the current	      |
|		    difference between the local time and GMT.		      |
|		    Linux displays file times as the local time when the      |
|		    the file was created.				      |
|		    The two times shown may be different if DST was different |
|		    then and now. (Ex: Displaying in winter the date/time of  |
|		    a file created the previous summer.)		      |
|		    The advantage of Windows' method is that apparent	      |
|		    relative times will always be correct, even for files     |
|		    created around the winter/summer time transitions.	      |
|		    The advantage of Linux method is that the time displayed  |
|		    for a file never changes. The drawback is that files      |
|		    created 1 hour apart around the Winter/summer time 	      |
|		    transition may be shown with the exact same time.	      |
|		    							      |
|   History:								      |
|    2014-02-26 JFL Created this routine                               	      |
*									      *
\*---------------------------------------------------------------------------*/

/* 
   Convert a Windows FILETIME to a Unix time_t.
   A FILETIME is the number of 100-nanosecond intervals since January 1, 1601.
   A time_t is the number of 1-second intervals since January 1, 1970.
   Both Windows and Linux file times are expressed in the GMT time zone.
*/
time_t Filetime2Timet(const FILETIME *pFT) {
  ULARGE_INTEGER ull;
  ull.LowPart = pFT->dwLowDateTime;
  ull.HighPart = pFT->dwHighDateTime;
  return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

/* Convert a Unix time_t to a Windows FILETIME */
void Timet2Filetime(time_t t, FILETIME *pFT) {
  ULARGE_INTEGER ull;
  ull.QuadPart = (t * 10000000ULL) + 116444736000000000ULL;
  pFT->dwLowDateTime = ull.LowPart;
  pFT->dwHighDateTime = ull.HighPart;
  return;
}

/* 
   Convert a Windows FILETIME to a Unix struct timespec.
   A FILETIME is the number of 100-nanosecond intervals since January 1, 1601.
   A struct timespec contains a time_t and a number of nanoseconds.
   Both Windows and Linux file times are expressed in the GMT time zone.
*/
void Filetime2Timespec(const FILETIME *pFT, struct timespec *pTS) {
  ULARGE_INTEGER ull;
  ull.LowPart = pFT->dwLowDateTime;
  ull.HighPart = pFT->dwHighDateTime;
  pTS->tv_sec = (time_t)(ull.QuadPart / 10000000ULL - 11644473600ULL);
  pTS->tv_nsec = (int)(ull.QuadPart % 10000000ULL) * 100;
  return;
}

/* Convert a Unix time_t to a Windows FILETIME */
void Timespec2Filetime(const struct timespec *pTS, FILETIME *pFT) {
  ULARGE_INTEGER ull;
  ull.QuadPart = (pTS->tv_sec * 10000000ULL) + 116444736000000000ULL + (pTS->tv_nsec / 100);
  pFT->dwLowDateTime = ull.LowPart;
  pFT->dwHighDateTime = ull.HighPart;
  return;
}

/* Convert a file GMT time to a struct tm with the local time to display */
struct tm *LocalFileTime(const time_t *pt) {
  FILETIME ft, lft;
  time_t lt;

  Timet2Filetime(*pt, &ft);
  FileTimeToLocalFileTime(&ft, &lft);
  lt = Filetime2Timet(&lft);
  return gmtime(&lt);
}  

/* Generate a string with the local file time, in the ISO 8601 date/time format */
/* 2014-07-03 Output time with µs precision if possible */
char *Filetime2String(const FILETIME *pFT, char *pBuf, size_t nBufSize) {
  FILETIME lft;
  SYSTEMTIME sTime;

  FileTimeToLocalFileTime(pFT, &lft);
  FileTimeToSystemTime(&lft, &sTime);
  if (nBufSize >= 20) {
    wsprintf(pBuf, "%04d-%02d-%02d %02d:%02d:%02d", sTime.wYear, sTime.wMonth, sTime.wDay,
                   sTime.wHour, sTime.wMinute, sTime.wSecond);
    if (nBufSize >= 27) {
      ULARGE_INTEGER uli;
      int iFraction; /* Fraction of a second */
      uli.LowPart = lft.dwLowDateTime;
      uli.HighPart = lft.dwHighDateTime;
      iFraction = (int)(uli.QuadPart % 10000000); /* FILETIME has 100ns resolution */
      iFraction /= 10; /* Convert 100ns resolution to 1µs resolution */
      wsprintf(pBuf+19, ".%06d", iFraction);
    } else if (nBufSize >= 24) {
      wsprintf(pBuf+19, ".%03d", sTime.wMilliseconds);
    }
  } else {
    return NULL; /* Buffer too small */
  }
  return pBuf;
}

#endif /* defined(_WIN32) */

