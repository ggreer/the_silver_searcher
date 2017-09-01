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
*    2017-04-10 JFL Added the new IO_REPARSE_TAG_LXSS_SYMLINK tag.            *
*    2017-06-27 JFL Added several other reparse tags gathered from Internet.  *
*		    Renamed IO_REPARSE_TAG_LXSS_SYMLINK as ..._TAG_LX_SYMLINK.*
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
/* All seem to come from NT DDK's ntifs.h, for installable file system drivers. */
/* Since the list varies a lot, redefine them one by one as needed */
/* Bit 31 = Tag owned by Microsoft
   Bit 30 = Reserved for Microsoft. Must be 0 for non-MS tags.
   Bit 29 = Surrogate bit. Points to another file of directory.
   Bit 28 = Directory bit. Any directory with this reparse tag can have children.
   Bits 16-27: Invalid and must be 0. */
/* Reparse tags, with the exception of IO_REPARSE_TAG_SYMLINK,
   are processed on the server and are not processed by a client after transmission over the wire. */
/* See https://github.com/prsyahmi/GpuRamDrive/blob/master/GpuRamDrive/3rdparty/inc/imdisk/ntumapi.h
   for a list of non-Microsoft reparse tags */

#ifndef IO_REPARSE_TAG_RESERVED_ZERO
#define IO_REPARSE_TAG_RESERVED_ZERO	0x00000000	/* Reserved reparse tag value */
#endif

#ifndef IO_REPARSE_TAG_RESERVED_ONE
#define IO_REPARSE_TAG_RESERVED_ONE	0x00000001	/* Reserved reparse tag value */
#endif

#ifndef IO_REPARSE_TAG_RESERVED_TWO
#define IO_REPARSE_TAG_RESERVED_TWO	0x00000002	/* Reserved reparse tag value */
#endif

#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT	0xA0000003	/* Used for mount point support */
#endif

#ifndef IO_REPARSE_TAG_HSM
#define IO_REPARSE_TAG_HSM		0xC0000004	/* Obsolete. Used by legacy Hierarchical Storage Manager Product */
#endif

#ifndef IO_REPARSE_TAG_DRIVE_EXTENDER
#define IO_REPARSE_TAG_DRIVE_EXTENDER	0x80000005	/* Home server drive extender */
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

#ifndef IO_REPARSE_TAG_IIS_CACHE
#define IO_REPARSE_TAG_IIS_CACHE        0xA0000010      /* Internet Information Services cache */
#endif

#ifndef IO_REPARSE_TAG_DFSR
#define IO_REPARSE_TAG_DFSR		0x80000012	/* Used by the DFS filter. The DFS is described in [MS-DFSC]. Server-side interpretation only, not meaningful over the wire */
#endif

#ifndef IO_REPARSE_TAG_DEDUP
#define IO_REPARSE_TAG_DEDUP		0x80000013	/* Deduplicated file, with metadata and links that point to where the file data is located in the chunk-store */
#endif                                                  /* Usable as a normal file. See https://blogs.technet.microsoft.com/filecab/2012/05/20/introduction-to-data-deduplication-in-windows-server-2012/ */

#ifndef IO_REPARSE_TAG_NFS
#define IO_REPARSE_TAG_NFS		0x80000014	/* NFS symbolic link, Win 8/SMB3 and later */
#endif

#ifndef IO_REPARSE_TAG_APPXSTREAM
#define IO_REPARSE_TAG_APPXSTREAM	0xC0000014	/* ? */
#endif

#ifndef IO_REPARSE_TAG_FILE_PLACEHOLDER
#define IO_REPARSE_TAG_FILE_PLACEHOLDER 0x80000015	/* Placeholder files enable users to view and manage Microsoft OneDrive files regardless of connectivity. */
#endif                                                  /* See https://msdn.microsoft.com/en-us/windows/compatibility/placeholder-files */

#ifndef IO_REPARSE_TAG_DFM
#define IO_REPARSE_TAG_DFM		0x80000016	/* ? */
#endif

#ifndef IO_REPARSE_TAG_WOF
#define IO_REPARSE_TAG_WOF		0x80000017	/* WOF (Windows Overlay Filesystem) compressed file */
#endif							/* See http://ntfs-3g-devel.sf.narkive.com/CRy8v4Ja/experimental-support-for-windows-10-system-compressed-files */

#ifndef IO_REPARSE_TAG_WCI
#define IO_REPARSE_TAG_WCI		0x80000018	/* Windows Container Image? */
#endif

#ifndef IO_REPARSE_TAG_GLOBAL_REPARSE
#define IO_REPARSE_TAG_GLOBAL_REPARSE	0x80000019	/* ? */
#endif

#ifndef IO_REPARSE_TAG_CLOUD
#define IO_REPARSE_TAG_CLOUD		0x9000001A	/* ? */
#endif

#ifndef IO_REPARSE_TAG_APPEXECLINK
#define IO_REPARSE_TAG_APPEXECLINK	0x8000001B	/* ? */
#endif

#ifndef IO_REPARSE_TAG_GVFS
#define IO_REPARSE_TAG_GVFS		0x9000001C	/* ? */
#endif

#ifndef IO_REPARSE_TAG_LX_SYMLINK
#define IO_REPARSE_TAG_LX_SYMLINK	0xA000001D	/* Linux Sub-System Symbolic Link */
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


