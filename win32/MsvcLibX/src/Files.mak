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
    +snprintf.obj		\
    +spawn.obj			\
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
    +GetFileAttributes.obj	\
    +GetFileAttributesEx.obj	\
    +GetFullPathName.obj	\
    +GetLongPathName.obj	\

###############################################################################
#			Include files dependancies			      #
###############################################################################

I=..\include
CI=$(STINCLUDE)

$(I)\chdir.h: $(I)\unistd.h $(I)\iconv.h $(CI)\debugm.h

$(I)\config.h: $(I)\msvclibx.h $(I)\stdbool.h $(I)\unistd.h

$(I)\direct.h: $(I)\msvclibx.h 

$(I)\dirent.h: $(I)\inttypes.h $(I)\sys\stat.h

$(I)\error.h: $(I)\msvclibx.h 

# $(I)\fadvise.h:  

$(I)\fcntl.h: $(I)\msvclibx.h 

$(I)\fnmatch.h: $(I)\msvclibx.h 

$(I)\getcwd.h: $(I)\unistd.h $(CI)\debugm.h

# $(I)\getopt.h: 

$(I)\grp.h: $(I)\msvclibx.h 

# $(I)\inttypes.h: 

# $(I)\msvclibx.h: 

# $(I)\netdb.h: 

$(I)\process.h: $(I)\msvclibx.h 

$(I)\pwd.h: $(I)\msvclibx.h 

# $(I)\regex.h: 

$(I)\sys\stat.h: $(I)\msvclibx.h $(I)\sys\types.h

# $(I)\stdbool.h: 

# $(I)\stdint.h: 

$(I)\stdio.h: $(I)\msvclibx.h 

# $(I)\stdio--.h: 

$(I)\stdlib.h: $(I)\msvclibx.h 

# $(I)\system.h: 

$(I)\unistd.h: $(I)\msvclibx.h $(I)\dirent.h

# $(I)\utime.h:  

$(I)\windows.h: $(I)\msvclibx.h 

$(I)\xfreopen.h: $(I)\msvclibx.h 

$(I)\sys\types.h: $(I)\msvclibx.h 


###############################################################################
#			Source files dependancies			      #
###############################################################################

access.c: $(CI)\debugm.h $(I)\io.h $(I)\msvclibx.h $(I)\stdio.h $(I)\windows.h

asprintf.c: $(I)\msvclibx.h $(I)\stdio.h $(I)\stdlib.h $(I)\stdarg.h

aswprintf.c: $(I)\msvclibx.h $(I)\stdio.h $(I)\stdlib.h $(I)\stdarg.h

basename.c: $(I)\libgen.h $(I)\limits.h $(I)\msvclibx.h $(I)\stdlib.h $(I)\string.h

chdir.c: $(CI)\debugm.h $(I)\iconv.h $(I)\stdio.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

clock_gettime.c: $(I)\msvclibx.h $(I)\time.h $(I)\sys\stat.h $(I)\windows.h

dasprintf.c: $(I)\msvclibx.h $(I)\stdio.h $(I)\stdlib.h $(I)\stdarg.h

daswprintf.c: $(I)\msvclibx.h $(I)\stdio.h $(I)\stdlib.h $(I)\stdarg.h

debugv.c: $(CI)\debugm.h

dirent.c: $(CI)\debugm.h $(I)\dirent.h $(I)\io.h $(I)\reparsept.h $(I)\sys\stat.h $(I)\stdlib.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

dirname.c: $(I)\libgen.h $(I)\limits.h $(I)\msvclibx.h $(I)\stdlib.h $(I)\string.h

err2errno.c: $(CI)\debugm.h $(I)\msvclibx.h $(I)\stdio.h $(I)\windows.h

filetime.c: $(I)\sys\stat.h $(I)\time.h $(I)\windows.h

fnmatch.c: $(CI)\debugm.h $(I)\fnmatch.h $(I)\string.h

fopen.c: $(CI)\debugm.h $(I)\fcntl.h $(I)\io.h $(I)\msvclibx.h $(I)\stdio.h $(I)\windows.h

fstat64.c: fstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\stdint.h $(I)\stdio.h $(I)\string.h $(I)\windows.h

fstat64i32.c: fstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\stdint.h $(I)\stdio.h $(I)\string.h $(I)\windows.h

fullpath.c: $(I)\limits.h $(I)\stdlib.h $(I)\limits.h $(I)\stdlib.h $(I)\windows.h

fwrite.c: $(CI)\debugm.h $(I)\iconv.h $(I)\msvclibx.h $(I)\stdio.h $(I)\iconv.h $(I)\stdio.h $(I)\unistd.h $(I)\windows.h

getcwd.c: $(CI)\debugm.h $(I)\stdio.h $(I)\unistd.h $(I)\windows.h

GetFileAttributes.c: $(I)\limits.h $(I)\windows.h

GetFileAttributesEx.c: $(I)\limits.h $(I)\windows.h

GetFullPathName.c: $(CI)\debugm.h $(I)\limits.h $(I)\stdio.h $(I)\stdlib.h $(I)\unistd.h $(I)\windows.h

GetLongPathName.c: $(CI)\debugm.h $(I)\limits.h $(I)\stdio.h $(I)\windows.h

getopt.c: $(I)\getopt.h $(I)\stdio.h $(I)\stdlib.h $(I)\string.h

getpagesize.c: $(I)\msvclibx.h $(I)\unistd.h $(I)\windows.h

getppid.c: $(I)\unistd.h $(I)\windows.h

gettimeofday.c: $(I)\msvclibx.h $(I)\time.h $(I)\sys\time.h

grp.c: $(I)\grp.h 

iconv.c: $(CI)\debugm.h $(I)\fcntl.h $(I)\iconv.h $(I)\io.h $(I)\stdio.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

lstat32.c: lstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\stdint.h $(I)\stdio.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

lstat32i64.c: lstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\stdint.h $(I)\stdio.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

lstat64.c: lstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\stdint.h $(I)\stdio.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

lstat64i32.c: lstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\stdint.h $(I)\stdio.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

main.c: $(I)\iconv.h $(I)\msvclibx.h $(I)\stdio.h $(I)\stdlib.h $(I)\windows.h

mb2wpath.c: $(CI)\debugm.h $(I)\direct.h $(I)\msvclibx.h $(I)\windows.h

mkdir.c: $(I)\direct.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\stdio.h $(I)\windows.h

mkdtemp.c: $(I)\fcntl.h $(I)\stdlib.h $(I)\time.h $(I)\unistd.h

mkstemp.c: $(I)\fcntl.h $(I)\io.h $(I)\stdlib.h $(I)\time.h $(I)\unistd.h

open.c: $(CI)\debugm.h $(I)\fcntl.h $(I)\io.h $(I)\msvclibx.h $(I)\fcntl.h $(I)\windows.h

pwd.c: $(I)\pwd.h 

readlink.c: $(CI)\debugm.h $(I)\unistd.h $(I)\reparsept.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

realpath.c: $(CI)\debugm.h $(I)\direct.h $(I)\stdlib.h $(I)\string.h $(I)\unistd.h $(I)\windows.h

rmdir.c: $(CI)\debugm.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\stdio.h $(I)\string.h $(I)\windows.h

snprintf.c: $(I)\msvclibx.h $(I)\stdio.h $(I)\stdlib.h $(I)\stdarg.h

spawn.c: $(CI)\debugm.h $(I)\msvclibx.h $(I)\process.h $(I)\stdio.h $(I)\windows.h

strerror.c: $(I)\msvclibx.h $(I)\stdlib.h $(I)\string.h

strndup.c: $(I)\stdlib.h $(I)\string.h

strptime.c: $(I)\string.h $(I)\time.h

symlink.c: $(CI)\debugm.h $(I)\reparsept.h $(I)\unistd.h $(I)\windows.h

uname.c: $(I)\msvclibx.h $(I)\stdlib.h $(I)\sys\utsname.h $(I)\windows.h

unlink.c: $(CI)\debugm.h $(I)\msvclibx.h $(I)\sys\stat.h $(I)\string.h $(I)\windows.h

utime.c: $(CI)\debugm.h $(I)\io.h $(I)\string.h $(I)\unistd.h $(I)\utime.h $(I)\sys\time.h $(I)\windows.h

utimes.c: $(CI)\debugm.h $(I)\io.h $(I)\string.h $(I)\unistd.h $(I)\sys\time.h $(I)\windows.h

xfreopen.c: $(I)\fcntl.h $(I)\io.h $(I)\string.h $(I)\xfreopen.h

