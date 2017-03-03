/*****************************************************************************\
*                                                                             *
*   Filename:	    reparsept.h						      *
*                                                                             *
*   Description:    Definitions for WIN32 reparse points.		      *
*                                                                             *
*   Notes:	    							      *
*                                                                             *
*   History:								      *
*    2014-02-28 JFL Created this file.                                        *
*									      *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

/* Reparse point iocontrol data buffer */
/* See http://msdn.microsoft.com/en-us/library/cc232006.aspx */
/* and http://msdn.microsoft.com/en-us/library/cc232007.aspx */

#include <windows.h>

/* NTFS reparse point definitions */

/* Constants from http://msdn.microsoft.com/en-us/library/dd541667.aspx */
/* Some, but not all, of them also defined in recent versions of winnt.h. */
/* Since the list varies a lot, redefine them one by one as needed */

#ifndef IO_REPARSE_TAG_RESERVED_ZERO
#define IO_REPARSE_TAG_RESERVED_ZERO	0x00000000	/* Reserved reparse tag value */
#endif
#ifndef IO_REPARSE_TAG_RESERVED_ONE
#define IO_REPARSE_TAG_RESERVED_ONE	0x00000001	/* Reserved reparse tag value */
#endif
#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT	0xA0000003	/* Used for mount point support */
#endif
#ifndef IO_REPARSE_TAG_HSM
#define IO_REPARSE_TAG_HSM		0xC0000004	/* Obsolete. Used by legacy Hierarchical Storage Manager Product */
#endif
#ifndef IO_REPARSE_TAG_DRIVER_EXTENDER
#define IO_REPARSE_TAG_DRIVER_EXTENDER	0x80000005	/* Home server drive extender */
#endif
#ifndef IO_REPARSE_TAG_HSM2
#define IO_REPARSE_TAG_HSM2		0x80000006	/* Obsolete. Used by legacy Hierarchical Storage Manager Product */
#endif
#ifndef IO_REPARSE_TAG_SIS
#define IO_REPARSE_TAG_SIS		0x80000007	/* Used by single-instance storage (SIS) filter driver. Server-side interpretation only, not meaningful over the wire */
#endif
#ifndef IO_REPARSE_TAG_WIM
#define IO_REPARSE_TAG_WIM              0x80000008      /* Mounted Windows boot Image File? */
#endif
#ifndef IO_REPARSE_TAG_CSV
#define IO_REPARSE_TAG_CSV              0x80000009      /* Cluster Shared Volume? */
#endif
#ifndef IO_REPARSE_TAG_DFS
#define IO_REPARSE_TAG_DFS		0x8000000A	/* Used by the DFS filter. The DFS is described in the Distributed File System (DFS): Referral Protocol Specification [MS-DFSC]. Server-side interpretation only, not meaningful over the wire */
#endif
#ifndef IO_REPARSE_TAG_FILTER_MANAGER
#define IO_REPARSE_TAG_FILTER_MANAGER	0x8000000B	/* Used by filter manager test harness */
#endif
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK		0xA000000C	/* Used for symbolic link support */
#endif
#ifndef IO_REPARSE_TAG_DFSR
#define IO_REPARSE_TAG_DFSR		0x80000012	/* Used by the DFS filter. The DFS is described in [MS-DFSC]. Server-side interpretation only, not meaningful over the wire */
#endif
#ifndef IO_REPARSE_TAG_DEDUP
#define IO_REPARSE_TAG_DEDUP		0x80000013	/* Mounted deduplicated volume? */
#endif
#ifndef IO_REPARSE_TAG_NFS
#define IO_REPARSE_TAG_NFS		0x80000014	/* Mounted NFS share? */
#endif

#if 0
#define IO_REPARSE_TAG_DRIVER_EXTENDER	0x80000005	/* Home server drive extender */
#define IO_REPARSE_TAG_FILTER_MANAGER	0x8000000B	/* Used by filter manager test harness */
#endif

#pragma pack(1)
typedef struct _REPARSE_READ_BUFFER {
  DWORD  ReparseTag;
  WORD   ReparseDataLength;
  WORD   Reserved;
  UCHAR  DataBuffer[1];
} REPARSE_READ_BUFFER, *PREPARSE_READ_BUFFER;
#define REPARSE_READ_BUFFER_HEADER_SIZE (sizeof(REPARSE_READ_BUFFER) - sizeof(UCHAR))

typedef struct _REPARSE_SYMLINK_READ_BUFFER {
  DWORD  ReparseTag;
  WORD   ReparseDataLength;
  WORD   Reserved;
  WORD   SubstituteNameOffset;
  WORD   SubstituteNameLength;
  WORD   PrintNameOffset;
  WORD   PrintNameLength;
  ULONG  Flags;
  WCHAR  PathBuffer[1];
} SYMLINK_READ_BUFFER, *PSYMLINK_READ_BUFFER;
#define SYMLINK_READ_BUFFER_HEADER_SIZE (sizeof(SYMLINK_READ_BUFFER) - sizeof(WCHAR))

typedef struct _REPARSE_MOUNTPOINT_READ_BUFFER {
  DWORD  ReparseTag;
  WORD   ReparseDataLength;
  WORD   Reserved;
  WORD   SubstituteNameOffset;
  WORD   SubstituteNameLength;
  WORD   PrintNameOffset;
  WORD   PrintNameLength;
  WCHAR  PathBuffer[1];
} MOUNTPOINT_READ_BUFFER, *PMOUNTPOINT_READ_BUFFER;
#define MOUNTPOINT_READ_BUFFER_HEADER_SIZE (sizeof(MOUNTPOINT_READ_BUFFER) - sizeof(WCHAR))

typedef struct _REPARSE_MOUNTPOINT_WRITE_BUFFER {
  DWORD  ReparseTag;
  DWORD  ReparseDataLength;
  WORD   Reserved;
  WORD   ReparseTargetLength;
  WORD   ReparseTargetMaximumLength;
  WORD   Reserved1;
  WCHAR  ReparseTarget[1];
} MOUNTPOINT_WRITE_BUFFER, *PMOUNTPOINT_WRITE_BUFFER;
#define MOUNTPOINT_WRITE_BUFFER_HEADER_SIZE (sizeof(MOUNTPOINT_WRITE_BUFFER) - sizeof(WCHAR))
#pragma pack()


