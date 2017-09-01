/*****************************************************************************\
*                                                                             *
*   Filename:	    debugm.h						      *
*                                                                             *
*   Description:    Debug macros					      *
*                                                                             *
*   Notes:	    OS-independant macros for managing distinct debug and     *
*		    release versions of a C or C++ program.                   * 
*                   The debug version is generated if the _DEBUG constant     *
*                   is defined. Else the release version is generated.        *
*                   These macros produce no extra code in the release version,*
*                   and thus have no overhead in that release version.	      *
*                   Even in the debug version, the debug output is disabled   *
*                   by default. It must be enabled by using DEBUG_ON().       *
*                                                                             *
*                   Usage:                                                    *
*                   - One source file must instanciate DEBUG_GLOBALS.         *
*                   - The source file parsing the arguments must look for one *
*                     argument (Ex: --debug) and enable the debug mode. Ex:   *
*			DEBUG_CODE(		                              *
*			  if (!strcmp(arg, "--debug")) DEBUG_ON();            *
*			)                                                     *
*                   - Insert DEBUG_ENTER() calls at the beginning of all      *
*                     routines that should be traced, and replace all their   *
*		      return instructions with RETURN_XXX() macros.	      *
*                   - Pepper the sources with DEBUG_PRINTF() calls displaying *
*                     critical intermediate values.                           *
*                                                                             *
*		    The debug output will show the function call stack by     *
*		    indenting traced subroutines proportionally to their call *
*                   depth.						      *
*		    To make the debug output more readable, it is recommended *
*		    to format it so that it looks like valid C code.	      *
*                                                                             *
*                   The macros actually support multiple debug levels.        *
*                   Level 1 is the normal debug mode.                         *
*                   Level 2 is the eXtra debug mode. Use it for displaying    *
*                    more detailed debug information, that is not needed in   *
*                    normal debugging sessions.                               *
*                   More levels could be used if desired.                     *
*                                                                             *
*    DEBUG_GLOBALS              Define global functions and variables used by *
*				macros below. Don't use the variables directly.
*    int iDebug = FALSE;        Global variable enabling debug output if TRUE.*
*    int iIndent = 0;           Global variable controlling debug indentation.*
*    int (*pdputs)(const char *)Pointer to the puts() routine for debug output.
*                                                                             *
*    DEBUG_ON()			Turn the debug mode on (Enables debug output) *
*    DEBUG_MORE()		Increase the debug level                      *
*    DEBUG_LESS()		Decrease the debug level                      *
*    DEBUG_OFF()		Turn the debug mode off                       *
*                                                                             *
*    DEBUG_IS_ON()		Test if the debug mode is enabled             *
*    XDEBUG_ON()		Turn eXtra debug mode on <==> 2*DEBUG_MORE()  *
*    XDEBUG_IS_ON()		Test if the eXtra debug mode is enabled       *
*                                                                             *
*    DEBUG_CODE(code)		Define code only in the _DEBUG version        *
*    DEBUG_CODE_IF_ON(code)	Debug code executed if debug mode is on       *
*    XDEBUG_CODE_IF_ON(code)	Debug code executed if eXtra debug mode is on *
*                                                                             *
*    SET_DEBUG_PUTS(pFunc)	Set the puts routine to use for debug output. *
*                               Useful to replace the default puts() with a   *
*                               thread-safe routine in multi-threaded programs.
*                                                                             *
*    DEBUG_PRINTF((format, ...))	Print a debug string if debug is on.  *
*                                       The double parenthesis are necessary  *
*                                       because C90 does not support macros   *
*                                       with variable list of arguments.      *
*    DEBUG_FPRINTF((iFile, fmt, ...))	Print a debug string to a stream      *
*                                                                             *
*        Important: Any call to DEBUG_ENTER MUST be matched by one call to    *
*                   DEBUG_LEAVE or RETURN_... when the function returns.      *                                                                             *
*                                                                             *
*    DEBUG_ENTER((format, ...))		Print a function name and arguments.  *
*                                       Increase indentation of further calls.*
*                                       It's the caller's responsibility to   *
*                                       format the routine name and arguments.*
*    DEBUG_LEAVE((format, ...))		Print a function return value.        *
*                                       Decrease indentation of further calls.*
*                                       It's the caller's responsibility to   *
*                                       format the return instruction & value.*
*    RETURN()				Leave and trace return		      *
*    RETURN_CONST(value)		Leave and trace return constant	      *
*    RETURN_BOOL(b)			Leave and trace return boolean	      *
*    RETURN_INT(i)			Leave and trace return integer	      *
*    RETURN_CHAR(c)			Leave and trace return character      *
*    RETURN_STRING(s)			Leave and trace return string	      *
*                                                                             *
*    RETURN_COMMENT((format, ...))	Leave, print comment and return	      *
*    RETURN_CONST_COMMENT(value, (...)) Leave, print comment & return a const.*
*    RETURN_BOOL_COMMENT(b, (...))	Leave, print comment & return a bool. *
*    RETURN_INT_COMMENT(i, (...))	Leave, print comment & return an int. *
*                                                                             *
*    Windows only:                                                            *
*    char *pszUtf8 = DEBUG_WSTR2NEWUTF8(pszUtf16);  Create a new UTF-8 string *
*    DEBUG_FREEUTF8(pszUtf8);			    Free the UTF-8 string     *
*                                                                             *
*   History:								      *
*    2012-01-16 JFL jf.larvoire@hp.com created this file.                     *
*    2012-02-03 JFL Renamed DEBUG_IF_IS_ON DEBUG_CODE_IF_ON.                  *
*		    Renamed file from debug.h to debugm.h because of a file   *
*		    name collision with another library on my PC.	      *
*    2014-02-10 JFL Added macros for an extra debug mode.		      *
*    2014-07-02 JFL renamed macro RETURN() as RETURN_CONST(), and defined     *
*		    new macro RETURN() to return nothing.		      *
*		    Idem for RETURN_COMMENT() as RETURN_CONST_COMMENT().      *
*    2016-09-09 JFL Flush every DEBUG_PRINTF output, to make sure to see      *
*		    every debug string printed before a program crash.	      *
*    2016-09-13 JFL Added macros DEBUG_WSTR2NEWUTF8() and DEBUG_FREEUTF8().   *
*    2016-10-04 JFL Added macros DEBUG_OFF(), DEBUG_MORE(), DEBUG_LESS().     *
*		    Allow using DEBUG_ON()/MORE()/LESS()/OFF() in release mode.
*    2017-03-22 JFL Rewrote DEBUG_PRINTF() and similar macros to generate a   *
*		    string, and output it in a single call to puts().         *
*		    This is useful for multi-threaded programs, that need     *
*		    to use a semaphore for synchronizing debug output from    *
*		    multiple threads.					      *
*		    The cost to pay for all this is that DEBUG_GLOBALS now    *
*		    defines several functions in addition to global variables.*
*    2017-03-24 JFL Added macro SET_DEBUG_PUTS() to hide the global variable. *
*    2017-03-29 JFL Revert to the old implementation for DOS, as the big      *
*		    DEBUG_GLOBALS macro sometimes chokes the DOS compiler.    *
*    2017-08-25 JFL Added an OS identification string definition.	      *
*		    Bug fix in _VSNPRINTF_EMULATION.			      *
*		    							      *
*        (C) Copyright 2016 Hewlett Packard Enterprise Development LP         *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef	_DEBUGM_H
#define	_DEBUGM_H	1
                                                                  
#include <stdio.h>	/* Macros use printf */
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

#ifdef _MSC_VER
#pragma warning(disable:4127) /* Avoid warnings on while(0) below */
#endif

#define DEBUG_DO(code) do {code} while (0)
#define DEBUG_DO_NOTHING() do {} while (0)

/* Define Thread Local Storage attributes */
#if defined(_MSC_VER) && defined(_WIN32)	/* Microsoft C in Windows */
#define DEBUG_TLS __declspec(thread)
#elif defined(__unix__)				/* Any Unix compiler */
#define DEBUG_TLS __thread
#else /* _MSDOS */				/* MS-DOS */
#define DEBUG_TLS
#endif

#ifndef _MSC_VER /* Emulate Microsoft's _vsnprintf() using the standard vsnprintf() */
#define _VSNPRINTF_EMULATION \
int _vsnprintf(char *pBuf, int iBufSize, const char *pszFormat, va_list vl) {  \
  int iRet = vsnprintf(pBuf, iBufSize, pszFormat, vl);                         \
  if (iRet > iBufSize) iRet = -1;                                              \
  return iRet;			                                               \
}
#else /* Use Microsoft's own */
#define _VSNPRINTF_EMULATION
#endif

/* Conditional compilation based on Microsoft's standard _DEBUG definition */
#if defined(_DEBUG)

#define DEBUG_VERSION " Debug"

#define DEBUG_GLOBAL_VARS \
int iDebug = 0;		/* Global variable enabling debug output if TRUE. */   \
int DEBUG_TLS iIndent = 0; /* Global variable controlling debug indentation. */\
int (*pdputs)(const char *) = puts; /* Debug output routine. Default: puts */  \

#if defined(_MSDOS)
#define DEBUG_GLOBALS DEBUG_GLOBAL_VARS
#else /* !defined(_MSDOS) */
#define DEBUG_GLOBALS \
DEBUG_GLOBAL_VARS							       \
_VSNPRINTF_EMULATION							       \
char *debug_vsprintf(char *pszFormat, va_list vl) {                            \
  char *pszBuf = NULL;		                                               \
  int n = 0, nBufSize = 64;				                       \
  do {pszBuf = (char *)realloc(pszBuf, nBufSize *= 2);} while (		       \
    pszBuf && ((n = _vsnprintf(pszBuf, nBufSize, pszFormat, vl)) == -1)        \
  );		                                                               \
  if (!pszBuf) return NULL;						       \
  if (n && (pszBuf[n-1] == '\n')) pszBuf[--n] = '\0';                          \
  return (char *)realloc(pszBuf, n+1);	                                       \
}									       \
char *debug_sprintf(char *pszFormat, ...) {                                    \
  char *pszBuf;			                                               \
  va_list vl;                                                                  \
  va_start(vl, pszFormat);                                                     \
  pszBuf = debug_vsprintf(pszFormat, vl);				       \
  va_end(vl);                                                                  \
  return pszBuf;		                                               \
}									       \
int debug_printf(char *pszFormat, ...) {                                       \
  char *pszBuf1 = NULL, *pszBuf2 = NULL;		                       \
  int n = 0;	                                                               \
  va_list vl;                                                                  \
  va_start(vl, pszFormat);                                                     \
  pszBuf1 = debug_vsprintf(pszFormat, vl);				       \
  va_end(vl);                                                                  \
  if (!pszBuf1) return 0;		                                       \
  pszBuf2 = debug_sprintf("%*s%s", iIndent, "", pszBuf1);                      \
  if (pszBuf2) n = (int)strlen(pszBuf2);                                       \
  pdputs(pszBuf2);	                                                       \
  fflush(stdout);	                                                       \
  free(pszBuf1); free(pszBuf2);                                                \
  return n;                                                                    \
}
#endif /* defined(_MSDOS) */

extern int iDebug;	/* Global variable enabling of disabling debug messages */
extern int (*pdputs)(const char *);	/* Pointer to the debug puts routine */
#if !defined(_MSDOS)
extern int debug_printf(char *fmt,...);	/* Print debug messages */
extern char *debug_sprintf(char *fmt,...); /* Print debug messages to a new buffer */
extern char *debug_vsprintf(char *fmt, va_list vl); /* Common subroutine of the previous two */
#endif /* !defined(_MSDOS) */
#define DEBUG_ON() iDebug = 1		/* Turn debug mode on */
#define DEBUG_MORE() iDebug += 1	/* Increase the debug level */
#define DEBUG_LESS() iDebug -= 1	/* Decrease the debug level */
#define DEBUG_OFF() iDebug = 0		/* Turn debug mode off */
#define DEBUG_IS_ON() (iDebug > 0)	/* Check if the debug mode is enabled */
#define XDEBUG_ON() iDebug = 2		/* Turn extra debug mode on. Same as calling DEBUG_MORE() twice. */
#define XDEBUG_IS_ON() (iDebug > 1)	/* Check if the extra debug mode is enabled */

#define DEBUG_CODE(code) code	/* Code included in the _DEBUG version only */
#define DEBUG_CODE_IF_ON(code) DEBUG_CODE(if (DEBUG_IS_ON()) {code}) /*
				   Debug code executed if debug mode is on */
#define XDEBUG_CODE_IF_ON(code) DEBUG_CODE(if (XDEBUG_IS_ON()) {code}) /*
				   Debug code executed if extra debug mode is on */
				   
extern DEBUG_TLS int iIndent;	/* Debug messages indentation. Thread local. */
#define DEBUG_INDENT_STEP 2	/* How many spaces to add for each indentation level */
#define DEBUG_PRINT_INDENT() printf("%*s", iIndent, "")

/* Debug code, conditionally printing a string based on global variable 'debug' */
/* The enter and leave variants print, then respectively increase or decrease indentation,
   to make recursive calls easier to review. */
#define DEBUG_FPRINTF(args) DEBUG_DO(if (DEBUG_IS_ON()) {DEBUG_PRINT_INDENT(); fprintf args;})
#if defined(_MSDOS)
#define DEBUG_PRINTF(args) DEBUG_DO(if (DEBUG_IS_ON()) {DEBUG_PRINT_INDENT(); printf args; fflush(stdout);})
#define XDEBUG_PRINTF(args) DEBUG_DO(if (XDEBUG_IS_ON()) {DEBUG_PRINT_INDENT(); printf args; fflush(stdout);})
#else /* !defined(_MSDOS) */
#define DEBUG_PRINTF(args) DEBUG_DO(if (DEBUG_IS_ON()) {debug_printf args;})
#define XDEBUG_PRINTF(args) DEBUG_DO(if (XDEBUG_IS_ON()) {debug_printf args;})
#endif /* defined(_MSDOS) */
#define DEBUG_ENTER(args)  DEBUG_DO(DEBUG_PRINTF(args); iIndent += DEBUG_INDENT_STEP;)
#define DEBUG_LEAVE(args)  DEBUG_DO(DEBUG_PRINTF(args); iIndent -= DEBUG_INDENT_STEP;)

#define DEBUG_RETURN_INT(i, comment) DEBUG_DO(int DEBUG_i = (i); \
  DEBUG_LEAVE(("return %d; // " comment "\n", DEBUG_i)); return DEBUG_i;)

/* print return instruction and decrease indent */
#define RETURN() DEBUG_DO(DEBUG_LEAVE(("return;\n")); return;)
#define RETURN_CONST(value) DEBUG_DO(DEBUG_LEAVE(("return %s;\n", #value)); return value;)
#define RETURN_INT(i) DEBUG_DO(int DEBUG_i = (i); \
  DEBUG_LEAVE(("return %d;\n", DEBUG_i)); return DEBUG_i;)
#define RETURN_STRING(s) DEBUG_DO(char *DEBUG_s = (s); \
  DEBUG_LEAVE(("return \"%s\";\n", DEBUG_s)); return DEBUG_s;)
#define RETURN_CHAR(c) DEBUG_DO(char DEBUG_c = (c); \
  DEBUG_LEAVE(("return '%c';\n", DEBUG_c)); return DEBUG_c;)
#define RETURN_BOOL(b) DEBUG_DO(int DEBUG_b = (b); \
  DEBUG_LEAVE(("return %s;\n", DEBUG_b ? "TRUE" : "FALSE")); return DEBUG_b;)

#if defined(_MSDOS)
#define RETURN_COMMENT(args) DEBUG_DO(DEBUG_LEAVE(("return; // ")); \
  if (DEBUG_IS_ON()) printf args; return;)
#define RETURN_CONST_COMMENT(value, args) DEBUG_DO(DEBUG_LEAVE(("return %s; // ", #value)); \
  if (DEBUG_IS_ON()) printf args; return value;)
#define RETURN_INT_COMMENT(i, args) DEBUG_DO(int DEBUG_i = (i); \
  DEBUG_LEAVE(("return %d; // ", DEBUG_i)); if (DEBUG_IS_ON()) printf args; return DEBUG_i;)
#define RETURN_BOOL_COMMENT(b, args) DEBUG_DO(int DEBUG_b = (b); \
  DEBUG_LEAVE(("return %s; // ", DEBUG_b ? "TRUE" : "FALSE")); if (DEBUG_IS_ON()) printf args; return DEBUG_b;)
#else /* !defined(_MSDOS) */
#define RETURN_COMMENT(args) DEBUG_DO(char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_sprintf args; DEBUG_LEAVE(("return; // %s", DEBUG_buf)); return;)
#define RETURN_CONST_COMMENT(value, args) DEBUG_DO(char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_sprintf args; DEBUG_LEAVE(("return %s; // %s", #value, DEBUG_buf)); free(DEBUG_buf); return value;)
#define RETURN_INT_COMMENT(i, args) DEBUG_DO(int DEBUG_i = (i); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_sprintf args; DEBUG_LEAVE(("return %d; // %s", DEBUG_i, DEBUG_buf)); free(DEBUG_buf); return DEBUG_i;)
#define RETURN_BOOL_COMMENT(b, args) DEBUG_DO(int DEBUG_b = (b); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_sprintf args; DEBUG_LEAVE(("return %s; // %s", DEBUG_b ? "TRUE" : "FALSE", DEBUG_buf)); free(DEBUG_buf); return DEBUG_b;)
#endif /* defined(_MSDOS) */

#define SET_DEBUG_PUTS(pFunc) pdputs = pFunc /* Set the debug puts routine */

#else /* !defined(_DEBUG) */

#define DEBUG_VERSION ""	/* Non debug version: Simply don't say it */

#define DEBUG_GLOBALS

#define DEBUG_ON() (void)0
#define DEBUG_MORE() (void)0
#define DEBUG_LESS() (void)0
#define DEBUG_OFF() (void)0
#define DEBUG_IS_ON() 0
#define XDEBUG_IS_ON() 0
#define DEBUG_CODE(code) 	/* Code included in _DEBUG version only */
#define DEBUG_CODE_IF_ON(code)	/* Code included in _DEBUG version only */
#define XDEBUG_CODE_IF_ON(code)	/* Code included in _DEBUG version only */

#define DEBUG_PRINT_INDENT() DEBUG_DO_NOTHING() /* Print call-depth spaces */

#define DEBUG_FPRINTF(args) DEBUG_DO_NOTHING()  /* Print a debug string to a stream */
#define DEBUG_PRINTF(args) DEBUG_DO_NOTHING()   /* Print a debug string to stdout */
#define XDEBUG_PRINTF(args) DEBUG_DO_NOTHING()  /* Print an extra debug string to stdout */
#define DEBUG_ENTER(args)  DEBUG_DO_NOTHING()   /* Print and increase indent */
#define DEBUG_LEAVE(args)  DEBUG_DO_NOTHING()   /* Print and decrease indent */

#define DEBUG_RETURN_INT(i, comment) return(i)

/* print return instruction and decrease indent */
#define RETURN() return
#define RETURN_CONST(value) return(value)
#define RETURN_INT(i) return(i)
#define RETURN_STRING(s) return(s)
#define RETURN_CHAR(c) return(c)
#define RETURN_BOOL(b) return(b)

#define RETURN_COMMENT(args) return
#define RETURN_CONST_COMMENT(value, args) return(value)
#define RETURN_INT_COMMENT(i, args) return(i)
#define RETURN_BOOL_COMMENT(b, args) return(b)

#define SET_DEBUG_PUTS(pFunc) DEBUG_DO_NOTHING() /* Set the debug puts routine */

#endif /* defined(_DEBUG) */

#define STRINGIZE(s) #s            /* Convert a macro name to a string */
#define VALUEIZE(s) STRINGIZE(s)   /* Convert a macro value to a string */
#define MACRODEF(s) "#define " #s " " STRINGIZE(s)

/* Display a macro name and value. */
#define DEBUG_PRINT_MACRO(name) DEBUG_DO( \
  const char *pszName = #name; /* Don't use STRINGIZE because we're already inside a macro */ \
  const char *pszValue = STRINGIZE(name); /* Don't use VALUEIZE because we're already inside a macro */ \
  DEBUG_PRINT_INDENT(); \
  if (strcmp(pszName, pszValue)) { \
    printf("#define %s %s\n", pszName, pszValue); \
  } else { /* Not 100% certain, but most likely. */ \
    printf("#undef %s\n", pszName); \
  } \
)

#ifdef _WIN32

/* Helper macros for displaying Unicode strings */
#define DEBUG_WSTR2UTF8(from, to, toSize) DEBUG_CODE( \
  WideCharToMultiByte(CP_UTF8, 0, from, lstrlenW(from)+1, to, toSize, NULL, NULL); \
)

/* Dynamically allocate a new buffer, then convert a Unicode string to UTF-8 */
/* The dynamic allocation is useful in modules using lots of UTF-16 pathnames.
   This avoids having many local buffers of length UTF8_PATH_MAX, which may
   make the stack grow too large and overflow. */
#define DEBUG_WSTR2NEWUTF8(pwStr, pUtf8)	\
  DEBUG_CODE(					\
    do {					\
      int nUtf8 = (int)lstrlenW(pwStr) * 2 + 1;	\
      pUtf8 = malloc(nUtf8);			\
      DEBUG_WSTR2UTF8(pwStr, pUtf8, nUtf8);	\
    } while (0);				\
  ) /* DEBUG_FREE(pUtf8) MUST be used to free the UTF-8 string after use, else there will be a memory leak */

#define DEBUG_FREEUTF8(pUtf8) DEBUG_CODE(free(pUtf8))

#endif /* defined(_WIN32) */

/******************** OS identification string definition ********************/

#ifdef _MSDOS		/* Automatically defined when targeting an MS-DOS app. */
#  define EXE_OS_NAME "DOS"
#endif /* _MSDOS */

#ifdef _OS2		/* To be defined on the command line for the OS/2 version */
#  define EXE_OS_NAME "OS/2"
#endif /* _OS2 */

#ifdef _WIN32		/* Automatically defined when targeting a Win32 app. */
#  if defined(__MINGW64__)
#    define EXE_OS_NAME "MinGW64"
#  elif defined(__MINGW32__)
#    define EXE_OS_NAME "MinGW32"
#  elif defined(_WIN64)
#    define EXE_OS_NAME "Win64"
#  else
#    define EXE_OS_NAME "Win32"
#  endif
#endif /* _WIN32 */

#ifdef __unix__		/* Automatically defined when targeting a Unix app. */
#  if defined(__CYGWIN64__)
#    define EXE_OS_NAME "Cygwin64"
#  elif defined(__CYGWIN32__)
#    define EXE_OS_NAME "Cygwin"
#  elif defined(_TRU64)
#    define EXE_OS_NAME "Tru64"	/* 64-bits Alpha Tru64 */
#  elif defined(__linux__)
#    define EXE_OS_NAME "Linux"
#  else
#    define EXE_OS_NAME "Unix"
#  endif
#endif /* __unix__ */

/**************** End of OS identification string definition *****************/

#endif /* !defined(_DEBUGM_H) */

