/*****************************************************************************\
*									      *
*   Filename	    debugm.h						      *
*									      *
*   Description     Debug macros					      *
*		    							      *
*   Notes	    OS-independant macros for managing distinct debug and     *
*		    release versions of a C or C++ program.		      * 
*		    The debug version is generated if the _DEBUG constant     *
*		    is defined. Else the release version is generated.	      *
*		    These macros produce no extra code in the release version,*
*		    and thus have no overhead in that release version.	      *
*		    Even in the debug version, the debug output is disabled   *
*		    by default. It must be enabled by using DEBUG_ON().	      *
*		    							      *
*		    Usage:						      *
*		    - One source file must instanciate DEBUG_GLOBALS.	      *
*		    - The source file parsing the arguments must look for one *
*		      argument (Ex: --debug) and enable the debug mode. Ex:   *
*			DEBUG_CODE(					      *
*			  if (!strcmp(arg, "--debug")) DEBUG_ON();	      *
*			)						      *
*		    - Insert DEBUG_ENTER() calls at the beginning of all      *
*		      routines that should be traced, and replace all their   *
*		      return instructions with RETURN_XXX() macros.	      *
*		    - Pepper the sources with DEBUG_PRINTF() calls displaying *
*		      critical intermediate values.			      *
*		    							      *
*		    The debug output will show the function call stack by     *
*		    indenting traced subroutines proportionally to their call *
*		    depth.						      *
*		    To make the debug output more readable, it is recommended *
*		    to format it so that it looks like valid C code.	      *
*		    							      *
*		    The macros actually support multiple debug levels.	      *
*		    Level 1 is the normal debug mode.			      *
*		    Level 2 is the eXtra debug mode. Use it for displaying    *
*		     more detailed debug information, that is not needed in   *
*		     normal debugging sessions.				      *
*		    More levels could be used if desired.		      *
*		    							      *
*    DEBUG_GLOBALS		Define global functions and variables used by *
*    				macros below. Don't use the variables directly.
*    int iDebug = FALSE;	Global variable enabling debug output if TRUE.*
*    int iIndent = 0;		Global variable controlling debug indentation.*
*    int (*pdput)(const char *) Pointer to the put() routine for debug output.*
*    		    							      *
*    DEBUG_ON()			Turn the debug mode on (Enables debug output) *
*    DEBUG_MORE()		Increase the debug level		      *
*    DEBUG_LESS()		Decrease the debug level		      *
*    DEBUG_OFF()		Turn the debug mode off			      *
*    		    							      *
*    DEBUG_IS_ON()		Test if the debug mode is enabled	      *
*    XDEBUG_ON()		Turn eXtra debug mode on <==> 2*DEBUG_MORE()  *
*    XDEBUG_IS_ON()		Test if the eXtra debug mode is enabled	      *
*    		    							      *
*    DEBUG_CODE(code)		Define code only in the _DEBUG version	      *
*    DEBUG_CODE_IF_ON(code)	Debug code executed if debug mode is on	      *
*    XDEBUG_CODE_IF_ON(code)	Debug code executed if eXtra debug mode is on *
*    		    							      *
*    SET_DEBUG_PUT(pFunc)	Set the routine to use for debug output.      *
*    				Useful to replace the default routine with a  *
*    				thread-safe routine in multi-threaded programs.
*    		    							      *
*    DEBUG_PRINTF((format, ...))	Print a debug string if debug is on.  *
*    					The double parenthesis are necessary  *
*    					because C90 does not support macros   *
*    					with variable list of arguments.      *
*    DEBUG_FPRINTF((iFile, fmt, ...))	Print a debug string to a stream      *
*    		    							      *
*    	Important: Any call to DEBUG_ENTER MUST be matched by one call to     *
*    		   DEBUG_LEAVE or RETURN_... when the function returns .      *
*    		    							      *
*    DEBUG_ENTER((format, ...))		Print a function name and arguments.  *
*    					Increase indentation of further calls.*
*    					It's the caller's responsibility to   *
*    					format the routine name and arguments.*
*    DEBUG_LEAVE((format, ...))		Print a function return value.	      *
*    					Decrease indentation of further calls.*
*    					It's the caller's responsibility to   *
*    					format the return instruction & value.*
*    RETURN()				Leave and trace return		      *
*    RETURN_CONST(value)		Leave and trace return constant	      *
*    RETURN_BOOL(b)			Leave and trace return boolean	      *
*    RETURN_INT(i)			Leave and trace return integer	      *
*    RETURN_CHAR(c)			Leave and trace return character      *
*    RETURN_STRING(s)			Leave and trace return string	      *
*    RETURN_PTR(p)			Leave and trace return pointer	      *
*    RETURN_LONG(l)			Leave and trace return long	      *
*    RETURN_CSTRING(s)			Leave and trace return const. string  *
*    RETURN_CPTR(p)			Leave and trace return const. pointer *
*    									      *
*    RETURN_COMMENT((format, ...))	Leave, print comment and return	      *
*    RETURN_CONST_COMMENT(value, (...)) Leave, print comment & return a const.*
*    RETURN_BOOL_COMMENT(b, (...))	Leave, print comment & return a bool. *
*    RETURN_INT_COMMENT(i, (...))	Leave, print comment & return an int. *
*    	    								      *
*    Windows only:							      *
*    char *pszUtf8 = DEBUG_WSTR2NEWUTF8(pszUtf16);  Create a new UTF-8 string *
*    DEBUG_FREEUTF8(pszUtf8);			    Free the UTF-8 string     *
*		    							      *
*   History:								      *
*    2012-01-16 JFL jf.larvoire@hp.com created this file.		      *
*    2012-02-03 JFL Renamed DEBUG_IF_IS_ON DEBUG_CODE_IF_ON.		      *
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
*		    string, and output it in a single call to puts().	      *
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
*    2017-10-30 JFL Added macro DEBUG_QUIET_LEAVE().			      *
*    2018-02-02 JFL Added several missing DEBUG_xxx_COMMENT() macros.	      *
*    2018-04-25 JFL Added macro DEBUG_WPRINTF().			      *
*		    Added macros DEBUG_WENTER() and DEBUG_WLEAVE().	      *
*    2018-10-01 JFL DEBUG_FREEUTF8() now clears the buffer pointer.	      *
*    2019-04-15 JFL Changed the debug puts() routine to an fputs-based routine*
*		    which does not implicitely outputs an \n in the end.      *
*		    Likewise, renamed SET_DEBUG_PUTS() as SET_DEBUG_PUT().    *
*    2019-09-24 JFL Fixed bug in debug_vsprintf() using new try_vsnprintf().  *
*    2020-03-19 JFL Fixed DEBUG_PRINT_MACRO() which sometimes failed in DOS   *
*		    and WIN95 when used with undefined macros.		      *
*    2020-07-22 JFL Fixed bug in debug_vsprintf(): Make sure _vsnprintf()     *
*		    in try_vsnprintf() always appends a NUL to its output.    *
*    2020-07-24 JFL Rewrote debug_printf() to use the standard asprintf() as  *
*		    much as possible.					      *
*    2020-12-11 JFL Added XDEBUG_WPRINTF and RETURN_DWORD* macros.            *
*		    							      *
*	 (C) Copyright 2016 Hewlett Packard Enterprise Development LP	      *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _DEBUGM_H
#define _DEBUGM_H	1
								  
#include <stdio.h>	/* Macros use printf */
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>	/* Macros use malloc */

#ifdef __cplusplus
extern "C" {
#endif

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

#if defined(HAS_MSVCLIBX) || defined(GNU_SOURCE)  /* If we have MsvcLibX or GNU asprintf() functions, use them */
#define _DEBUG_USE_ASPRINTF 1
#endif

/* #undef _DEBUG_USE_ASPRINTF // For testing the alternate implementation */

#if _DEBUG_USE_ASPRINTF || (!defined(_MSC_VER)) || defined(_UCRT)  /* If this library has a standard vsnprintf() function, use it */
#define _DEBUG_VSNPRINTF_DEFINITION
#define debug_vsnprintf vsnprintf
#elif !defined(_MSDOS)	    /* Else redefine them for MSVC versions before the UCRT was introduced, except for MS-DOS which does not support very large macros like that */
#define _DEBUG_VSNPRINTF_DEFINITION									 	\
int debug_vsnprintf(char *pBuf, size_t nBufSize, const char *pszFormat, va_list vl) {                           \
  char *pBuf2;                                                                                                  \
  int iRet;                                                                                                     \
  va_list vl0;                                                                                                  \
  /* First try it with the original arguments */                                                                \
  /* This consumes the vl arguments, which needs to be done once */                                             \
  /* This also optimizes the number of calls, in the normal case where the output buffer was sized correctly */ \
  va_copy(vl0, vl);	/* Save a copy of the caller's va_list */                                               \
  iRet = _vsnprintf(pBuf, nBufSize, pszFormat, vl);                                                             \
  if (iRet >= 0) {	/* Success, the output apparently fits in the buffer */                                 \
    if ((size_t)iRet == nBufSize) if (pBuf && nBufSize) pBuf[nBufSize-1] = '\0'; /* Fix the missing NUL */      \
    va_end(vl0);                                                                                                \
    return iRet;                                                                                                \
  }                                                                                                             \
  /* OK, this does not fit. Try it with larger and larger buffers, until we know the full output size */        \
  iRet = vasprintf(&pBuf2, pszFormat, vl0);                                                                     \
  if (iRet >= 0) {	/*  Success at last, now we know the necessary size */                                  \
    if (pBuf && nBufSize) {	/* Copy whatever fits in the output buffer */                                   \
      if (nBufSize-1) memcpy(pBuf, pBuf2, nBufSize-1);                                                          \
      pBuf[nBufSize-1] = '\0';	/* Make sure there's a NUL in the end */                                        \
    }                                                                                                           \
    free(pBuf2);                                                                                                \
  }                                                                                                             \
  va_end(vl0);                                                                                                  \
  return iRet;                                                                                                  \
}
#endif

#if _DEBUG_USE_ASPRINTF  /* If we have MsvcLibX or GNU asprintf() functions, use them */
#define _DEBUG_ASPRINTF_DEFINITION
#define debug_vasprintf vasprintf
#define debug_asprintf  asprintf
#elif !defined(_MSDOS)	    /* Else redefine them, except for MS-DOS which does not support very large macros like that */
#define _DEBUG_ASPRINTF_DEFINITION							    \
int debug_vasprintf(char **ppszBuf, const char *pszFormat, va_list vl) {		    \
  char *pBuf, *pBuf2;									    \
  int n, nBufSize = 64;									    \
  va_list vl0, vl2;									    \
  /* First try it once with the original va_list (When nBufSize == 128) */		    \
  /* This consumes the vl arguments, which needs to be done once */			    \
  va_copy(vl0, vl);	/* Save a copy of the caller's va_list */			    \
  for (pBuf = NULL; (pBuf2 = (char *)realloc(pBuf, nBufSize *= 2)) != NULL; ) {		    \
    va_copy(vl2, vl0);									    \
    n = debug_vsnprintf(pBuf = pBuf2, nBufSize, pszFormat, (nBufSize == 128) ? vl : vl2);   \
    va_end(vl2);									    \
    if ((n >= 0) && (n < nBufSize)) { /* Success, now we know the necessary size */	    \
      pBuf2 = (char *)realloc(pBuf, n+1); /* Free the unused space in the end - May fail */ \
      *ppszBuf = pBuf2 ? pBuf2 : pBuf;	  /* Return the valid one */			    \
      va_end(vl0);									    \
      return n;										    \
    } /* Else if n == nBufSize, actually not success, as there's no NUL in the end */	    \
  }											    \
  va_end(vl0);										    \
  return -1;										    \
}											    \
int debug_asprintf(char **ppszBuf, const char *pszFormat, ...) {			    \
  int n;										    \
  va_list vl;										    \
  va_start(vl, pszFormat);								    \
  n = debug_vasprintf(ppszBuf, pszFormat, vl);						    \
  va_end(vl);										    \
  return n;										    \
}
#endif

#if defined(HAS_MSVCLIBX)  /* If we have the MsvcLibX dasprintf() functions, use it */
#define _DEBUG_DASPRINTF_DEFINITION
#define debug_dasprintf dasprintf
#else
#define _DEBUG_DASPRINTF_DEFINITION							\
char *debug_dasprintf(const char *pszFormat, ...) {					\
  char *pszBuf = NULL;									\
  va_list vl;										\
  va_start(vl, pszFormat);								\
  debug_vasprintf(&pszBuf, pszFormat, vl); /* Updates pszBuf only if successful */	\
  va_end(vl);										\
  return pszBuf;									\
}
#endif

/* Conditional compilation based on Microsoft's standard _DEBUG definition */
#if defined(_DEBUG)

#define DEBUG_VERSION " Debug"

#define DEBUG_GLOBAL_VARS \
int iDebug = 0;		/* Global variable enabling debug output if TRUE. */   \
int DEBUG_TLS iIndent = 0; /* Global variable controlling debug indentation. */\
int debug_put(const char *str) { return fputs(str, stdout); }		       \
int (*pdput)(const char *) = debug_put; /* Debug output routine. Default: debug_put */

#if defined(_MSDOS)
#define DEBUG_GLOBALS DEBUG_GLOBAL_VARS
#else /* !defined(_MSDOS) */
#define DEBUG_GLOBALS \
DEBUG_GLOBAL_VARS									    \
_DEBUG_VSNPRINTF_DEFINITION								    \
_DEBUG_ASPRINTF_DEFINITION								    \
_DEBUG_DASPRINTF_DEFINITION								    \
int debug_printf(const char *pszFormat, ...) {						    \
  char *pszBuf1, *pszBuf2;								    \
  int n;										    \
  va_list vl;										    \
  va_start(vl, pszFormat);								    \
  n = debug_vasprintf(&pszBuf1, pszFormat, vl);						    \
  va_end(vl);										    \
  if (n == -1) return -1; /* No memory for generating the debug output */		    \
  n = debug_asprintf(&pszBuf2, "%*s%s", iIndent, "", pszBuf1);				    \
  if (n == -1) goto abort_debug_printf;							    \
  pdput(pszBuf2); /* Output everything in a single system call. Useful if multithreaded. */ \
  fflush(stdout); /* Make sure we see the output. Useful to see everything before a crash */\
  free(pszBuf2);									    \
abort_debug_printf:									    \
  free(pszBuf1);									    \
  return n;										    \
}
#endif /* defined(_MSDOS) */

extern int iDebug;	/* Global variable enabling of disabling debug messages */
extern int (*pdput)(const char *);	/* Pointer to the debug puts routine */
#if !defined(_MSDOS)
extern int debug_printf(const char *fmt, ...);		  /* Print debug messages */
extern int debug_asprintf(char **, const char *fmt, ...); /* Print debug messages to a new buffer */
extern int debug_vasprintf(char **, const char *fmt, va_list vl); /* Common subroutine of the previous two */
extern char *debug_dasprintf(const char *fmt, ...);	  /* Shorter alternative used by other macros below */
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
#define DEBUG_WPRINTF(args) DEBUG_DO(if (DEBUG_IS_ON()) {DEBUG_PRINT_INDENT(); wprintf args;})
#define XDEBUG_WPRINTF(args) DEBUG_DO(if (XDEBUG_IS_ON()) {DEBUG_PRINT_INDENT(); wprintf args;})
#if defined(_MSDOS)
#define DEBUG_PRINTF(args) DEBUG_DO(if (DEBUG_IS_ON()) {DEBUG_PRINT_INDENT(); printf args; fflush(stdout);})
#define XDEBUG_PRINTF(args) DEBUG_DO(if (XDEBUG_IS_ON()) {DEBUG_PRINT_INDENT(); printf args; fflush(stdout);})
#else /* !defined(_MSDOS) */
#define DEBUG_PRINTF(args) DEBUG_DO(if (DEBUG_IS_ON()) {debug_printf args;})
#define XDEBUG_PRINTF(args) DEBUG_DO(if (XDEBUG_IS_ON()) {debug_printf args;})
#endif /* defined(_MSDOS) */
#define DEBUG_ENTER(args)   DEBUG_DO(DEBUG_PRINTF(args); iIndent += DEBUG_INDENT_STEP;)
#define DEBUG_WENTER(args)  DEBUG_DO(DEBUG_WPRINTF(args); iIndent += DEBUG_INDENT_STEP;)
#define DEBUG_LEAVE(args)   DEBUG_DO(DEBUG_PRINTF(args); iIndent -= DEBUG_INDENT_STEP;)
#define DEBUG_WLEAVE(args)  DEBUG_DO(DEBUG_WPRINTF(args); iIndent -= DEBUG_INDENT_STEP;)
#define DEBUG_QUIET_LEAVE() DEBUG_DO(iIndent -= DEBUG_INDENT_STEP;)

#define DEBUG_RETURN_INT(i, comment) DEBUG_DO(int DEBUG_i = (i); \
  DEBUG_LEAVE(("return %d; // " comment "\n", DEBUG_i)); return DEBUG_i;)

/* print return instruction and decrease indent */
#define RETURN() DEBUG_DO(DEBUG_LEAVE(("return;\n")); return;)
#define RETURN_CONST(k) DEBUG_DO(DEBUG_LEAVE(("return %s;\n", #k)); return k;)
#define RETURN_INT(i) DEBUG_DO(int DEBUG_i = (i); \
  DEBUG_LEAVE(("return %d;\n", DEBUG_i)); return DEBUG_i;)
#define RETURN_STRING(s) DEBUG_DO(char *DEBUG_s = (s); \
  DEBUG_LEAVE(("return \"%s\";\n", DEBUG_s)); return DEBUG_s;)
#define RETURN_CHAR(c) DEBUG_DO(char DEBUG_c = (c); \
  DEBUG_LEAVE(("return '%c';\n", DEBUG_c)); return DEBUG_c;)
#define RETURN_BOOL(b) DEBUG_DO(int DEBUG_b = (b); \
  DEBUG_LEAVE(("return %s;\n", DEBUG_b ? "TRUE" : "FALSE")); return DEBUG_b;)
#define RETURN_PTR(p) DEBUG_DO(void *DEBUG_p = (p); \
  DEBUG_LEAVE(("return %p;\n", DEBUG_p)); return DEBUG_p;)
#define RETURN_LONG(l) DEBUG_DO(long DEBUG_l = (l); \
  DEBUG_LEAVE(("return %ld;\n", DEBUG_l)); return DEBUG_l;)
#define RETURN_CSTRING(s) DEBUG_DO(const char *DEBUG_s = (s); \
  DEBUG_LEAVE(("return \"%s\";\n", DEBUG_s)); return DEBUG_s;)
#define RETURN_CPTR(p) DEBUG_DO(const void *DEBUG_p = (p); \
  DEBUG_LEAVE(("return %p;\n", DEBUG_p)); return DEBUG_p;)
#define RETURN_DWORD(dw) DEBUG_DO(DWORD DEBUG_dw = (dw); \
  DEBUG_LEAVE(("return 0x%lX;\n", DEBUG_dw)); return DEBUG_dw;)

#if defined(_MSDOS)
#define RETURN_COMMENT(args) DEBUG_DO(DEBUG_LEAVE(("return; // ")); \
  if (DEBUG_IS_ON()) printf args; return;)
#define RETURN_CONST_COMMENT(k, args) DEBUG_DO(DEBUG_LEAVE(("return %s; // ", #k)); \
  if (DEBUG_IS_ON()) printf args; return k;)
#define RETURN_INT_COMMENT(i, args) DEBUG_DO(int DEBUG_i = (i); \
  DEBUG_LEAVE(("return %d; // ", DEBUG_i)); if (DEBUG_IS_ON()) printf args; return DEBUG_i;)
#define RETURN_STRING_COMMENT(s, args) DEBUG_DO(char *DEBUG_s = (s); \
  DEBUG_LEAVE(("return \"%s\"; // \n", DEBUG_s)); if (DEBUG_IS_ON()) printf args; return DEBUG_s;)
#define RETURN_CHAR_COMMENT(c, args) DEBUG_DO(char DEBUG_c = (c); \
  DEBUG_LEAVE(("return '%c'; // \n", DEBUG_c)); if (DEBUG_IS_ON()) printf args; return DEBUG_c;)
#define RETURN_BOOL_COMMENT(b, args) DEBUG_DO(int DEBUG_b = (b); \
  DEBUG_LEAVE(("return %s; // ", DEBUG_b ? "TRUE" : "FALSE")); if (DEBUG_IS_ON()) printf args; return DEBUG_b;)
#define RETURN_PTR_COMMENT(p, args) DEBUG_DO(void *DEBUG_p = (p); \
  DEBUG_LEAVE(("return %p; // \n", DEBUG_p)); if (DEBUG_IS_ON()) printf args; return DEBUG_p;)
#define RETURN_LONG_COMMENT(l, args) DEBUG_DO(long DEBUG_l = (l); \
  DEBUG_LEAVE(("return %ld; // \n", DEBUG_l)); if (DEBUG_IS_ON()) printf args; return DEBUG_l;)
#define RETURN_CSTRING_COMMENT(s, args) DEBUG_DO(const char *DEBUG_s = (s); \
  DEBUG_LEAVE(("return \"%s\"; // \n", DEBUG_s)); if (DEBUG_IS_ON()) printf args; return DEBUG_s;)
#define RETURN_CPTR_COMMENT(p, args) DEBUG_DO(const void *DEBUG_p = (p); \
  DEBUG_LEAVE(("return %p; // \n", DEBUG_p)); if (DEBUG_IS_ON()) printf args; return DEBUG_p;)
#define RETURN_DWORD_COMMENT(dw, args) DEBUG_DO(DWORD DEBUG_dw = (dw); \
  DEBUG_LEAVE(("return 0x%lX; // \n", DEBUG_dw)); if (DEBUG_IS_ON()) printf args; return DEBUG_dw;)
#else /* !defined(_MSDOS) */
#define RETURN_COMMENT(args) DEBUG_DO(char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return; // %s", DEBUG_buf)); free(DEBUG_buf); return;)
#define RETURN_CONST_COMMENT(k, args) DEBUG_DO(char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %s; // %s", #k, DEBUG_buf)); free(DEBUG_buf); return k;)
#define RETURN_INT_COMMENT(i, args) DEBUG_DO(int DEBUG_i = (i); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %d; // %s", DEBUG_i, DEBUG_buf)); free(DEBUG_buf); return DEBUG_i;)
#define RETURN_STRING_COMMENT(s, args) DEBUG_DO(char *DEBUG_s = (s); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %s; // %s", DEBUG_s, DEBUG_buf)); free(DEBUG_buf); return DEBUG_s;)
#define RETURN_CHAR_COMMENT(c, args) DEBUG_DO(char DEBUG_c = (c); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %c; // %s", DEBUG_c, DEBUG_buf)); free(DEBUG_buf); return DEBUG_c;)
#define RETURN_BOOL_COMMENT(b, args) DEBUG_DO(int DEBUG_b = (b); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %s; // %s", DEBUG_b ? "TRUE" : "FALSE", DEBUG_buf)); free(DEBUG_buf); return DEBUG_b;)
#define RETURN_PTR_COMMENT(p, args) DEBUG_DO(void *DEBUG_p = (p); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %p; // %s", DEBUG_p, DEBUG_buf)); free(DEBUG_buf); return DEBUG_p;)
#define RETURN_LONG_COMMENT(l, args) DEBUG_DO(long DEBUG_l = (l); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %l; // %s", DEBUG_l, DEBUG_buf)); free(DEBUG_buf); return DEBUG_l;)
#define RETURN_CSTRING_COMMENT(s, args) DEBUG_DO(const char *DEBUG_s = (s); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %s; // %s", DEBUG_s, DEBUG_buf)); free(DEBUG_buf); return DEBUG_s;)
#define RETURN_CPTR_COMMENT(p, args) DEBUG_DO(const void *DEBUG_p = (p); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return %p; // %s", DEBUG_p, DEBUG_buf)); free(DEBUG_buf); return DEBUG_p;)
#define RETURN_DWORD_COMMENT(dw, args) DEBUG_DO(DWORD DEBUG_dw = (dw); char *DEBUG_buf = NULL; \
  if (DEBUG_IS_ON()) DEBUG_buf = debug_dasprintf args; DEBUG_LEAVE(("return 0x%X; // %s", DEBUG_dw, DEBUG_buf)); free(DEBUG_buf); return DEBUG_dw;)
#endif /* defined(_MSDOS) */

#define SET_DEBUG_PUT(pFunc) pdput = pFunc /* Set the debug put routine */

#else /* !defined(_DEBUG) */

#define DEBUG_VERSION ""	/* Non debug version: Simply don't say it */

#define DEBUG_GLOBALS

#define DEBUG_ON() (void)0
#define DEBUG_MORE() (void)0
#define DEBUG_LESS() (void)0
#define DEBUG_OFF() (void)0
#define DEBUG_IS_ON() 0
#define XDEBUG_IS_ON() 0
#define DEBUG_CODE(code)	/* Code included in _DEBUG version only */
#define DEBUG_CODE_IF_ON(code)	/* Code included in _DEBUG version only */
#define XDEBUG_CODE_IF_ON(code) /* Code included in _DEBUG version only */

#define DEBUG_PRINT_INDENT() DEBUG_DO_NOTHING() /* Print call-depth spaces */

#define DEBUG_FPRINTF(args)  DEBUG_DO_NOTHING()	/* Print a debug string to a stream */
#define DEBUG_WPRINTF(args)  DEBUG_DO_NOTHING()	/* Print a wide debug string to stdout */
#define DEBUG_PRINTF(args)   DEBUG_DO_NOTHING()	/* Print a debug string to stdout */
#define XDEBUG_PRINTF(args)  DEBUG_DO_NOTHING()	/* Print an extra debug string to stdout */
#define XDEBUG_WPRINTF(args) DEBUG_DO_NOTHING()	/* Print an extra debug string to stdout */
#define DEBUG_ENTER(args)    DEBUG_DO_NOTHING()	/* Print and increase indent */
#define DEBUG_WENTER(args)   DEBUG_DO_NOTHING()	/* Print and increase indent */
#define DEBUG_LEAVE(args)    DEBUG_DO_NOTHING()	/* Print and decrease indent */
#define DEBUG_WLEAVE(args)   DEBUG_DO_NOTHING()	/* Print and decrease indent */
#define DEBUG_QUIET_LEAVE()  DEBUG_DO_NOTHING()	/* Print and decrease indent */

#define DEBUG_RETURN_INT(i, comment) return(i)

/* print return instruction and decrease indent */
#define RETURN() return
#define RETURN_CONST(k) return(k)
#define RETURN_INT(i) return(i)
#define RETURN_STRING(s) return(s)
#define RETURN_CHAR(c) return(c)
#define RETURN_BOOL(b) return(b)
#define RETURN_PTR(p) return(p)
#define RETURN_LONG(l) return(l)
#define RETURN_CSTRING(s) return(s)
#define RETURN_CPTR(p) return(p)
#define RETURN_DWORD(dw) return(dw)

#define RETURN_COMMENT(args) return
#define RETURN_CONST_COMMENT(k, args) return(k)
#define RETURN_INT_COMMENT(i, args) return(i)
#define RETURN_STRING_COMMENT(s, args) return(s)
#define RETURN_CHAR_COMMENT(c, args) return(c)
#define RETURN_BOOL_COMMENT(b, args) return(b)
#define RETURN_PTR_COMMENT(p, args) return(p)
#define RETURN_LONG_COMMENT(l, args) return(l)
#define RETURN_CSTRING_COMMENT(s, args) return(s)
#define RETURN_CPTR_COMMENT(p, args) return(p)
#define RETURN_DWORD_COMMENT(dw, args) return(dw)

#define SET_DEBUG_PUT(pFunc) DEBUG_DO_NOTHING() /* Set the debug put routine */

#endif /* defined(_DEBUG) */

#define STRINGIZE(s) #s		   /* Convert a macro name to a string */
#define VALUEIZE(s) STRINGIZE(s)   /* Convert a macro value to a string */
#define MACRODEF(s) "#define " #s " " STRINGIZE(s)

/* Display a macro name and value. */
#define DEBUG_PRINT_MACRO(name) DEBUG_DO( \
  const char *pszName = #name; /* Don't use STRINGIZE because we're already inside a macro */ \
  const char *pszValue = "" STRINGIZE(name); /* Don't use VALUEIZE because we're already inside a macro */ \
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
      int nUtf8 = (int)lstrlenW(pwStr) * 2 + 1; \
      pUtf8 = malloc(nUtf8);			\
      DEBUG_WSTR2UTF8(pwStr, pUtf8, nUtf8);	\
    } while (0);				\
  ) /* DEBUG_FREE(pUtf8) MUST be used to free the UTF-8 string after use, else there will be a memory leak */

/* Clear the pointer, so that DEBUG_FREEUTF8() can safely be called again */
#define DEBUG_FREEUTF8(pUtf8) DEBUG_CODE(do {free(pUtf8); pUtf8 = NULL;} while (0))

#endif /* defined(_WIN32) */

#ifdef __cplusplus
}
#endif

#endif /* !defined(_DEBUGM_H) */

