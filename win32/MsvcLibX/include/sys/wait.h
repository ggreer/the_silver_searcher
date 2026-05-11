/*****************************************************************************\
*                                                                             *
*   Filename	    sys/wait.h						      *
*                                                                             *
*   Description     DOS/WIN32 port of standard C library's sys/wait.h.	      *
*                                                                             *
*   Notes	                                                              *
*                                                                             *
*   History								      *
*    2025-12-16 JFL Created this file.                                        *
*		    							      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <signal.h>	/* For SIGINT and SIGTERM definitions */


#ifdef _MSDOS

/* Analysis of the exit code of child tasks */
/* int 21H function 4DH termination type in AH:
   0x00 = Normal exit; 0x01 = Ctrl-C; 0x02 = Critical error; 0x03 = TSR */
/* But command.com sometimes exits with AX = 0x0300, for example after an echo.
   => Detect signals by checking if AH is 0x01 or 0x02 */
#define WIFEXITED(err) (!(((err) == 0x100) || ((err) == 0x200)))
#define WEXITSTATUS(err) ((err) & 0xFF)

#define WIFSIGNALED(err) (((err) == 0x100) || ((err) == 0x200))
#define WTERMSIG(err) (((err) == 0x100) ? SIGINT : SIGTERM)

#endif


#ifdef _WIN32

#include <winnt.h>	/* For STATUS_CONTROL_C_EXIT in <ntstatus.h> */

/* Analysis of the exit code of child tasks */
#define WIFEXITED(err) (((err) >= 0) && ((err) <= 0xFF))
#define WEXITSTATUS(err) ((err) & 0xFF)

#define WIFSIGNALED(err) (((unsigned)(err) >> 16) == 0xC000)
#define WTERMSIG(err) (((err) == STATUS_CONTROL_C_EXIT) ? SIGINT : SIGTERM)

#endif /* defined(_WIN32) */

#endif /* !defined(_SYS_WAIT_H) */

