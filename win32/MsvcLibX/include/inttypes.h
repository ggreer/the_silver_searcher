/*****************************************************************************\
*                                                                             *
*   Filename        inttypes.h                                                *
*                                                                             *
*   Description     ISO C9x compliant inttypes.h for Microsoft Visual Studio  *
*                                                                             *
*   Notes           TO DO: Move imaxdiv to its own C file.                    *
*                                                                             *
*   History                                                                   *
*    2014-02-07 JFL Added definitions for PRIdMAX and PRIiMAX.                *
*    2016-01-07 JFL Restructured and improved support for MS-DOS.             *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _MSC_VER
#error "Use this header only with Microsoft Visual C++ compilers!"
#endif

#ifndef _MSC_INTTYPES_H_
#define _MSC_INTTYPES_H_

#include <stdint.h>

/*  7.8 Format conversion of integer types */

typedef struct {
  intmax_t quot;
  intmax_t rem;
} imaxdiv_t;

/*  7.8.1 Macros for format specifiers */

#if !defined(__cplusplus) || defined(__STDC_FORMAT_MACROS) /*  [   See footnote 185 at page 198 */

#if defined(_WIN64)
#define __I64_PREFIX "I64"
#define __I32_PREFIX "I32"
#define __PTR_PREFIX "I64"
#define __MAX_PREFIX "I64"
#elif defined(_WIN32)
#define __I64_PREFIX "I64"
#define __I32_PREFIX "I32"
#define __PTR_PREFIX "I32"
#define __MAX_PREFIX "I64"
#elif defined(_MSDOS)
#define __I64_PREFIX "ll" /* MSVC 1.5 actually ignores the second l */
#define __I32_PREFIX "l"
#if defined(_M_I86CM) || defined(_M_I86LM) || defined(_M_I86HM) /* Long pointer memory models */
#define __PTR_PREFIX "l"
#else	/* Short pointer memory models */
#define __PTR_PREFIX ""
#endif
#define __MAX_PREFIX "l"
#endif

/* printf macros for integers */
#define PRId8       "d"
#define PRIi8       "i"
#define PRIo8       "o"
#define PRIu8       "u"
#define PRIx8       "x"
#define PRIX8       "X"

#define PRIdLEAST8  "d"
#define PRIiLEAST8  "i"
#define PRIoLEAST8  "o"
#define PRIuLEAST8  "u"
#define PRIxLEAST8  "x"
#define PRIXLEAST8  "X"

#define PRIdFAST8   "d"
#define PRIiFAST8   "i"
#define PRIoFAST8   "o"
#define PRIuFAST8   "u"
#define PRIxFAST8   "x"
#define PRIXFAST8   "X"

#define PRId16       "hd"
#define PRIi16       "hi"
#define PRIo16       "ho"
#define PRIu16       "hu"
#define PRIx16       "hx"
#define PRIX16       "hX"

#define PRIdLEAST16  PRId16
#define PRIiLEAST16  PRIi16
#define PRIoLEAST16  PRIo16
#define PRIuLEAST16  PRIu16
#define PRIxLEAST16  PRIx16
#define PRIXLEAST16  PRIX16

#define PRIdFAST16   PRId16
#define PRIiFAST16   PRIi16
#define PRIoFAST16   PRIo16
#define PRIuFAST16   PRIu16
#define PRIxFAST16   PRIx16
#define PRIXFAST16   PRIX16

#define PRId32       __I32_PREFIX "d"
#define PRIi32       __I32_PREFIX "i"
#define PRIo32       __I32_PREFIX "o"
#define PRIu32       __I32_PREFIX "u"
#define PRIx32       __I32_PREFIX "x"
#define PRIX32       __I32_PREFIX "X"

#define PRIdLEAST32  PRId32
#define PRIiLEAST32  PRIi32
#define PRIoLEAST32  PRIo32
#define PRIuLEAST32  PRIu32
#define PRIxLEAST32  PRIx32
#define PRIXLEAST32  PRIX32

#define PRIdFAST32   PRId32
#define PRIiFAST32   PRIi32
#define PRIoFAST32   PRIo32
#define PRIuFAST32   PRIu32
#define PRIxFAST32   PRIx32
#define PRIXFAST32   PRIX32

#define PRId64       __I64_PREFIX "d"
#define PRIi64       __I64_PREFIX "i"
#define PRIo64       __I64_PREFIX "o"
#define PRIu64       __I64_PREFIX "u"
#define PRIx64       __I64_PREFIX "x"
#define PRIX64       __I64_PREFIX "X"

#define PRIdLEAST64  PRId64
#define PRIiLEAST64  PRIi64
#define PRIoLEAST64  PRIo64
#define PRIuLEAST64  PRIu64
#define PRIxLEAST64  PRIx64
#define PRIXLEAST64  PRIX64

#define PRIdFAST64   PRId64
#define PRIiFAST64   PRIi64
#define PRIoFAST64   PRIo64
#define PRIuFAST64   PRIu64
#define PRIxFAST64   PRIx64
#define PRIXFAST64   PRIX64

#define PRIdMAX     __MAX_PREFIX "d"
#define PRIiMAX     __MAX_PREFIX "i"
#define PRIoMAX     __MAX_PREFIX "o"
#define PRIuMAX     __MAX_PREFIX "u"
#define PRIxMAX     __MAX_PREFIX "x"
#define PRIXMAX     __MAX_PREFIX "X"

#define PRIdPTR     __PTR_PREFIX "d"
#define PRIiPTR     __PTR_PREFIX "i"
#define PRIoPTR     __PTR_PREFIX "o"
#define PRIuPTR     __PTR_PREFIX "u"
#define PRIxPTR     __PTR_PREFIX "x"
#define PRIXPTR     __PTR_PREFIX "X"

/* scanf macros for integers */
#define SCNd8       "d"
#define SCNi8       "i"
#define SCNo8       "o"
#define SCNu8       "u"
#define SCNx8       "x"
#define SCNX8       "X"

#define SCNdLEAST8  SCNd8
#define SCNiLEAST8  SCNi8
#define SCNoLEAST8  SCNo8
#define SCNuLEAST8  SCNu8
#define SCNxLEAST8  SCNx8
#define SCNXLEAST8  SCNX8

#define SCNdFAST8   SCNd8
#define SCNiFAST8   SCNi8
#define SCNoFAST8   SCNo8
#define SCNuFAST8   SCNu8
#define SCNxFAST8   SCNx8
#define SCNXFAST8   SCNX8

#define SCNd16       "hd"
#define SCNi16       "hi"
#define SCNo16       "ho"
#define SCNu16       "hu"
#define SCNx16       "hx"
#define SCNX16       "hX"

#define SCNdLEAST16  SCNd16
#define SCNiLEAST16  SCNi16
#define SCNoLEAST16  SCNo16
#define SCNuLEAST16  SCNu16
#define SCNxLEAST16  SCNx16
#define SCNXLEAST16  SCNX16

#define SCNdFAST16   SCNd16
#define SCNiFAST16   SCNi16
#define SCNoFAST16   SCNo16
#define SCNuFAST16   SCNu16
#define SCNxFAST16   SCNx16
#define SCNXFAST16   SCNX16

#define SCNd32       __I32_PREFIX "d"
#define SCNi32       __I32_PREFIX "i"
#define SCNo32       __I32_PREFIX "o"
#define SCNu32       __I32_PREFIX "u"
#define SCNx32       __I32_PREFIX "x"
#define SCNX32       __I32_PREFIX "X"

#define SCNdLEAST32  SCNd32
#define SCNiLEAST32  SCNi32
#define SCNoLEAST32  SCNo32
#define SCNuLEAST32  SCNu32
#define SCNxLEAST32  SCNx32
#define SCNXLEAST32  SCNX32

#define SCNdFAST32   SCNd32
#define SCNiFAST32   SCNi32
#define SCNoFAST32   SCNo32
#define SCNuFAST32   SCNu32
#define SCNxFAST32   SCNx32
#define SCNXFAST32   SCNX32

#define SCNd64       __I64_PREFIX "d"
#define SCNi64       __I64_PREFIX "i"
#define SCNo64       __I64_PREFIX "o"
#define SCNu64       __I64_PREFIX "u"
#define SCNx64       __I64_PREFIX "x"
#define SCNX64       __I64_PREFIX "X"

#define SCNdLEAST64  SCNd64
#define SCNiLEAST64  SCNi64
#define SCNoLEAST64  SCNo64
#define SCNuLEAST64  SCNu64
#define SCNxLEAST64  SCNx64
#define SCNXLEAST64  SCNX64

#define SCNdFAST64   SCNd64
#define SCNiFAST64   SCNi64
#define SCNoFAST64   SCNo64
#define SCNuFAST64   SCNu64
#define SCNxFAST64   SCNx64
#define SCNXFAST64   SCNX64

#define SCNdMAX     __MAX_PREFIX "d"
#define SCNiMAX     __MAX_PREFIX "i"
#define SCNoMAX     __MAX_PREFIX "o"
#define SCNuMAX     __MAX_PREFIX "u"
#define SCNxMAX     __MAX_PREFIX "x"
#define SCNXMAX     __MAX_PREFIX "X"

#define SCNdPTR     __PTR_PREFIX "d"
#define SCNiPTR     __PTR_PREFIX "i"
#define SCNoPTR     __PTR_PREFIX "o"
#define SCNuPTR     __PTR_PREFIX "u"
#define SCNxPTR     __PTR_PREFIX "x"
#define SCNXPTR     __PTR_PREFIX "X"

#endif /*  __STDC_FORMAT_MACROS ] */

/*  7.8.2 Functions for greatest-width integer types */

/*  7.8.2.1 imaxabs() */
#if defined(_WIN32)
#define imaxabs _abs64
#elif defined(_MSDOS)
#define imaxabs abs
#endif

/*  7.8.2.2 imaxdiv() */

/*  This is modified version of div() function from Microsoft's div.c found */
/*  in %MSVC.NET%\crt\src\div.c */
#if defined(_MSDOS) && !defined(STATIC_IMAXDIV) && !defined(__cplusplus)
/* MSVC 1.52 compiler for MS-DOS does not support inline for C */
extern imaxdiv_t __cdecl imaxdiv(intmax_t numer, intmax_t denom);
#else
#ifdef STATIC_IMAXDIV /*  [ */
static
#else /*  STATIC_IMAXDIV ][ */
_inline
#endif /*  STATIC_IMAXDIV ] */
imaxdiv_t __cdecl imaxdiv(intmax_t numer, intmax_t denom) {
   imaxdiv_t result;

   result.quot = numer / denom;
   result.rem = numer % denom;

   if (numer < 0 && result.rem > 0) {
      /*  did division wrong; must fix up */
      ++result.quot;
      result.rem -= denom;
   }

   return result;
}
#endif /* defined(_MSDOS) && !defined(STATIC_IMAXDIV) && !defined(__cplusplus) */

/*  7.8.2.3 strtoimax() and strtoumax() */
#if defined(_WIN32)
#define strtoimax _strtoi64
#define strtoumax _strtoui64
#elif defined(_MSDOS)
#define strtoimax strtol
#define strtoumax strtoul
#endif

/*  7.8.2.4 wcstoimax() and wcstoumax() */
#if defined(_WIN32)
#define wcstoimax _wcstoi64
#define wcstoumax _wcstoui64
#elif defined(_MSDOS)
#define wcstoimax wcstol
#define wcstoumax wcstoul
#endif

#endif /* _MSC_INTTYPES_H_ */
