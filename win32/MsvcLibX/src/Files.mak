###############################################################################
#									      #
#   File name	    Files.mak						      #
#									      #
#   Description     MsvcLibX Specific file dependancies			      #
#									      #
#   Notes	    							      #
#									      #
#   History								      #
#    2012-10-21 JFL Initial version                                           #
#    2013-03-27 JFL Added debugv.obj and getppid.obj.                         #
#    2014-02-03 JFL Added readlink.obj.                                       #
#    2014-02-05 JFL Added symlink.obj.                                        #
#    2014-02-06 JFL Added lstat*.obj.                                         #
#    2014-02-10 JFL Added realpath.obj.                                       #
#    2014-02-17 JFL Added err2errno.obj.                                      #
#    2014-02-26 JFL Added filetime.obj.                                       #
#    2014-02-27 JFL Added iconv.obj.                                          #
#    2014-02-28 JFL Added chdir.obj and getcwd.obj.                           #
#    2014-03-04 JFL Added fopen.obj.                                          #
#    2014-03-06 JFL Added strerror.obj.                                       #
#    2014-03-24 JFL Added access.obj.                                         #
#    2014-03-27 JFL Added spawn.obj.                                          #
#    2014-05-30 JFL Moved here the OBJECTS macro definition from NMakeFile.   #
#		    Added uname.obj and utimes.obj.                           #
#    2014-06-04 JFL Added clock_gettime.obj and gettimeofday.obj.             #
#    2014-06-24 JFL Added fstat64.obj and fstat64i32.obj.                     #
#    2014-07-01 JFL Added mb2wpath.obj.			                      #
#    2016-09-08 JFL Added basename.obj and dirname.obj.		 	      #
#    2016-09-12 JFL Added WIN32_OBJECTS, and several WIN32 UTF-8 routines.    #
#    2016-10-11 JFL moved debugm.h to SysToolsLib global C include dir.       #
#    2017-02-16 JFL Added open.obj.    			                      #
#    2017-02-27 JFL Added getpagesize.obj. 		                      #
#    2017-03-02 JFL Removed references to files removed earlier.              #
#    2017-03-03 JFL Added fwrite.obj.   		                      #
#    2017-03-22 JFL Added missing dependencies.			              #
#    2017-05-31 JFL Added dependencies on stdio.h, stdlib.h, string.h, etc.   #
#    2024-01-07 JFL Define both NMINCLUDE and STINCLUDE.		      #
#    2024-10-14 JFL Renamed variable I as XI to avoid conflicts with the I    #
#                   variable in DOS.mak and WIN32.mak.			      #
#    2017-03-03 JFL Added getenv.obj.   		                      #
#                   							      #
#         © Copyright 2016 Hewlett Packard Enterprise Development LP          #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

# List of object files to build and include in the MsvcLibX library
# IMPORTANT NOTE: Every time you add an object file in the list here, also
#                 store its specific source file dependancies below.
OBJECTS = \
    +access.obj			\
    +asprintf.obj		\
    +basename.obj		\
    +chdir.obj			\
    +clock_gettime.obj		\
    +CompactPath.obj		\
    +dasprintf.obj		\
    +debugv.obj			\
    +dirent.obj			\
    +dirname.obj		\
    +err2errno.obj		\
    +filetime.obj		\
    +fnmatch.obj		\
    +fopen.obj			\
    +fstat64i32.obj		\
    +fstat64.obj		\
    +fullpath.obj		\
    +fwrite.obj			\
    +getcwd.obj			\
    +getline.obj		\
    +getopt.obj			\
    +getpagesize.obj		\
    +getppid.obj		\
    +gettimeofday.obj		\
    +iconv.obj			\
    +lstat64i32.obj		\
    +lstat64.obj		\
    +main.obj			\
    +mb2wpath.obj		\
    +mkdir.obj			\
    +mkdtemp.obj		\
    +mkstemp.obj		\
    +open.obj			\
    +readlink.obj		\
    +realpath.obj		\
    +rmdir.obj			\
    +setenv.obj			\
    +snprintf.obj		\
    +spawn.obj			\
    +strcasestr.obj		\
    +strerror.obj		\
    +strndup.obj		\
    +strptime.obj		\
    +symlink.obj		\
    +uname.obj			\
    +unlink.obj			\
    +utime.obj			\
    +utimes.obj			\
    +xfreopen.obj		\
#    +lstat32.obj		\
#    +lstat32i64.obj		\

# WIN32 UTF-8 extension routines, used for implementing UTF-8 support for WIN32 libc.  
WIN32_OBJECTS = \
    +aswprintf.obj		\
    +daswprintf.obj		\
    +fileid.obj			\
    +GetEncoding.obj		\
    +getenv.obj			\
    +GetFileAttributes.obj	\
    +GetFileAttributesEx.obj	\
    +GetFullPathName.obj	\
    +GetLongPathName.obj	\
    +GetShareBasePath.obj	\

###############################################################################
#			Include files dependancies			      #
###############################################################################

XI=..\include
CI=$(STINCLUDE)	# SysToolsLib global C include directory
MI=$(NMINCLUDE)	# NMaker include directory

$(XI)\chdir.h: $(XI)\unistd.h $(XI)\iconv.h $(MI)\debugm.h

$(XI)\config.h: $(XI)\msvclibx.h $(XI)\stdbool.h $(XI)\unistd.h

$(XI)\direct.h: $(XI)\msvclibx.h 

$(XI)\dirent.h: $(XI)\inttypes.h $(XI)\sys\stat.h

$(XI)\error.h: $(XI)\msvclibx.h 

# $(XI)\fadvise.h:  

$(XI)\fcntl.h: $(XI)\msvclibx.h 

$(XI)\fnmatch.h: $(XI)\msvclibx.h 

$(XI)\getcwd.h: $(XI)\unistd.h $(MI)\debugm.h

# $(XI)\getopt.h: 

$(XI)\grp.h: $(XI)\msvclibx.h 

# $(XI)\inttypes.h: 

# $(XI)\msvclibx.h: 

# $(XI)\netdb.h: 

$(XI)\process.h: $(XI)\msvclibx.h 

$(XI)\pwd.h: $(XI)\msvclibx.h 

# $(XI)\regex.h: 

$(XI)\sys\stat.h: $(XI)\msvclibx.h $(XI)\sys\types.h

# $(XI)\stdbool.h: 

$(XI)\stdint.h: $(XI)\wchar.h

$(XI)\stdio.h: $(XI)\msvclibx.h 

# $(XI)\stdio--.h: 

$(XI)\stdlib.h: $(XI)\msvclibx.h 

# $(XI)\system.h: 

$(XI)\unistd.h: $(XI)\msvclibx.h $(XI)\dirent.h

# $(XI)\utime.h:  

$(XI)\windows.h: $(XI)\msvclibx.h 

$(XI)\xfreopen.h: $(XI)\msvclibx.h 

$(XI)\sys\types.h: $(XI)\msvclibx.h 


###############################################################################
#			Source files dependencies			      #
###############################################################################

access.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\io.h $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\windows.h

asprintf.c: $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\stdlib.h $(XI)\stdarg.h

aswprintf.c: $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\stdlib.h $(XI)\stdarg.h

basename.c: $(XI)\libgen.h $(XI)\limits.h $(XI)\msvclibx.h $(XI)\stdlib.h $(XI)\string.h

chdir.c: $(MI)\debugm.h $(XI)\dos.h $(XI)\errno.h $(XI)\iconv.h $(XI)\stdio.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

clock_gettime.c: $(XI)\errno.h $(XI)\msvclibx.h $(XI)\time.h $(XI)\sys\stat.h $(XI)\windows.h

CompactPath.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\unistd.h $(XI)\reparsept.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

dasprintf.c: $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\stdlib.h $(XI)\stdarg.h

daswprintf.c: $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\stdlib.h $(XI)\stdarg.h

debugv.c: $(MI)\debugm.h

dirent.c: $(MI)\debugm.h $(XI)\dirent.h $(XI)\errno.h $(XI)\io.h $(XI)\reparsept.h $(XI)\sys\stat.h $(XI)\stdlib.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

dirname.c: $(XI)\libgen.h $(XI)\limits.h $(XI)\msvclibx.h $(XI)\stdlib.h $(XI)\string.h

encoding.c: $(MI)\debugm.h $(XI)\iconv.h $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\string.h

err2errno.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\windows.h

fileid.c: $(MI)\debugm.h $(XI)\msvclibx.h $(XI)\errno.h $(XI)\stdio.h $(XI)\sys\stat.h

filetime.c: $(XI)\sys\stat.h $(XI)\time.h $(XI)\windows.h

fnmatch.c: $(MI)\debugm.h $(XI)\fnmatch.h $(XI)\string.h

fopen.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\fcntl.h $(XI)\io.h $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\windows.h

fstat64.c: fstat.c $(MI)\debugm.h $(XI)\errno.h $(XI)\dirent.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\stdint.h $(XI)\stdio.h $(XI)\string.h $(XI)\windows.h

fstat64i32.c: fstat.c $(MI)\debugm.h $(XI)\dirent.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\stdint.h $(XI)\stdio.h $(XI)\string.h $(XI)\windows.h

fullpath.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\limits.h $(XI)\stdlib.h $(XI)\limits.h $(XI)\stdlib.h $(XI)\windows.h

fwrite.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\iconv.h $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\unistd.h $(XI)\windows.h

getcwd.c: $(MI)\debugm.h $(XI)\dos.h $(XI)\errno.h $(XI)\stdio.h $(XI)\unistd.h $(XI)\windows.h

getenv.c: $(MI)\debugm.h $(CI)\dict.h $(XI)\errno.h $(XI)\stdlib.h $(XI)\string.h $(XI)\iconv.h

getline.c: $(MI)\debugm.h $(XI)\limits.h $(XI)\errno.h $(XI)\stdio.h

GetEncoding.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\iconv.h $(XI)\stdio.h $(XI)\string.h

GetFileAttributes.c: $(XI)\limits.h $(XI)\windows.h

GetFileAttributesEx.c: $(XI)\limits.h $(XI)\windows.h

GetFullPathName.c: $(XI)\errno.h $(XI)\limits.h $(XI)\stdio.h $(XI)\stdlib.h $(XI)\unistd.h $(XI)\windows.h

GetLongPathName.c: $(MI)\debugm.h $(XI)\limits.h $(XI)\stdio.h $(XI)\windows.h

getopt.c: $(XI)\errno.h $(XI)\getopt.h $(XI)\stdio.h $(XI)\stdlib.h $(XI)\string.h

getpagesize.c: $(XI)\msvclibx.h $(XI)\unistd.h $(XI)\windows.h

getppid.c: $(XI)\unistd.h $(XI)\windows.h

GetShareBasePath.c: $(MI)\debugm.h $(XI)\iconv.h $(XI)\unistd.h $(XI)\windows.h

gettimeofday.c: $(XI)\msvclibx.h $(XI)\time.h $(XI)\sys\time.h

grp.c: $(XI)\grp.h 

iconv.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\fcntl.h $(XI)\iconv.h $(XI)\io.h $(XI)\stdio.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

lstat32.c: lstat.c $(MI)\debugm.h $(XI)\dirent.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\stdint.h $(XI)\stdio.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

lstat32i64.c: lstat.c $(MI)\debugm.h $(XI)\dirent.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\stdint.h $(XI)\stdio.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

lstat64.c: lstat.c $(MI)\debugm.h $(XI)\dirent.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\stdint.h $(XI)\stdio.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

lstat64i32.c: lstat.c $(MI)\debugm.h $(XI)\dirent.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\stdint.h $(XI)\stdio.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

main.c: $(XI)\iconv.h $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\stdlib.h $(XI)\windows.h

mb2wpath.c: $(MI)\debugm.h $(XI)\direct.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\windows.h

mkdir.c: $(XI)\direct.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\stdio.h $(XI)\windows.h

mkdtemp.c: $(XI)\errno.h $(XI)\fcntl.h $(XI)\stdlib.h $(XI)\time.h $(XI)\unistd.h

mkstemp.c: $(XI)\errno.h $(XI)\fcntl.h $(XI)\io.h $(XI)\stdlib.h $(XI)\time.h $(XI)\unistd.h

open.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\fcntl.h $(XI)\io.h $(XI)\msvclibx.h $(XI)\fcntl.h $(XI)\windows.h

pwd.c: $(XI)\pwd.h 

readlink.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\unistd.h $(XI)\reparsept.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

realpath.c: $(MI)\debugm.h $(XI)\direct.h $(XI)\errno.h $(XI)\stdlib.h $(XI)\string.h $(XI)\unistd.h $(XI)\windows.h

rmdir.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\stdio.h $(XI)\string.h $(XI)\windows.h

setenv.c: $(XI)\stdlib.h $(XI)\string.h

snprintf.c: $(XI)\msvclibx.h $(XI)\stdio.h $(XI)\stdlib.h $(XI)\stdarg.h

spawn.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\process.h $(XI)\stdio.h $(XI)\windows.h

strcasestr.c: $(XI)\string.h

strerror.c: $(XI)\errno.h $(XI)\msvclibx.h $(XI)\stdlib.h $(XI)\string.h

strndup.c: $(XI)\stdlib.h $(XI)\string.h

strptime.c: $(XI)\string.h $(XI)\time.h

symlink.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\reparsept.h $(XI)\unistd.h $(XI)\windows.h

uname.c: $(XI)\msvclibx.h $(XI)\dos.h $(XI)\stdlib.h $(XI)\sys\utsname.h $(XI)\windows.h

unlink.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\msvclibx.h $(XI)\sys\stat.h $(XI)\string.h $(XI)\windows.h

utime.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\io.h $(XI)\string.h $(XI)\unistd.h $(XI)\utime.h $(XI)\sys\time.h $(XI)\windows.h

utimes.c: $(MI)\debugm.h $(XI)\errno.h $(XI)\io.h $(XI)\string.h $(XI)\unistd.h $(XI)\sys\time.h $(XI)\windows.h

xfreopen.c: $(XI)\fcntl.h $(XI)\io.h $(XI)\string.h $(XI)\xfreopen.h

