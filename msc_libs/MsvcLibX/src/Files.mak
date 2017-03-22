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
#                   							      #
#         © Copyright 2016 Hewlett Packard Enterprise Development LP          #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

# List of object files to build and include in the MsvcLibX library
# IMPORTANT NOTE: Every time you add an object file in the list here, also
#                 store its specific source file dependancies below.
OBJECTS = \
    +access.obj			\
    +basename.obj		\
    +chdir.obj			\
    +clock_gettime.obj		\
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

$(I)\windowsU.h: $(I)\msvclibx.h 

$(I)\xfreopen.h: $(I)\msvclibx.h 

$(I)\sys\types.h: $(I)\msvclibx.h 


###############################################################################
#			Source files dependancies			      #
###############################################################################

access.c: $(I)\MsvcLibX.h $(CI)\debugm.h

basename.c: $(I)\libgen.h

chdir.c: $(CI)\debugm.h $(I)\iconv.h $(I)\unistd.h

clock_gettime.c: $(I)\MsvcLibX.h $(I)\time.h $(I)\sys\stat.h

debugv.c: $(CI)\debugm.h

dirent.c: $(CI)\debugm.h $(I)\dirent.h $(I)\sys\stat.h $(I)\unistd.h

dirname.c: $(I)\libgen.h

err2errno.c: $(I)\MsvcLibX.h $(CI)\debugm.h

filetime.c: $(I)\sys\stat.h

fnmatch.c: $(CI)\debugm.h $(I)\fnmatch.h

fopen.c: $(I)\MsvcLibX.h

fstat64.c: fstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\MsvcLibX.h $(I)\sys\stat.h $(I)\stdint.h

fstat64i32.c: fstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\MsvcLibX.h $(I)\sys\stat.h $(I)\stdint.h

fullpath.c: $(I)\stdlib.h $(I)\limits.h

fwrite.c: $(I)\MsvcLibX.h  $(I)\stdio.h $(I)\iconv.h $(I)\unistd.h

getcwd.c: $(CI)\debugm.h $(I)\unistd.h

GetFileAttributesU.c: $(I)\windowsU.h $(I)\limits.h

GetFileAttributesExU.c: $(I)\windowsU.h $(I)\limits.h

GetFullPathNameU.c: $(I)\windowsU.h $(I)\limits.h $(CI)\debugm.h

GetLongPathNameU.c: $(I)\windowsU.h $(I)\limits.h $(CI)\debugm.h

getopt.c: $(I)\getopt.h

getpagesize.c: $(I)\MsvcLibX.h $(I)\unistd.h

# getppid.c:

gettimeofday.c: $(I)\MsvcLibX.h $(I)\time.h $(I)\sys\time.h

grp.c: $(I)\grp.h 

iconv.c: $(I)\iconv.h

lstat32.c: lstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\MsvcLibX.h $(I)\sys\stat.h $(I)\stdint.h $(I)\unistd.h

lstat32i64.c: lstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\MsvcLibX.h $(I)\sys\stat.h $(I)\stdint.h $(I)\unistd.h

lstat64.c: lstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\MsvcLibX.h $(I)\sys\stat.h $(I)\stdint.h $(I)\unistd.h

lstat64i32.c: lstat.c $(CI)\debugm.h $(I)\dirent.h $(I)\MsvcLibX.h $(I)\sys\stat.h $(I)\stdint.h $(I)\unistd.h

main.c: $(I)\MsvcLibX.h

mb2wpath.c: $(I)\MsvcLibX.h $(CI)\debugm.h

mkdir.c: $(I)\MsvcLibX.h $(I)\sys\stat.h

mkdtemp.c: $(I)\unistd.h

mkstemp.c: $(I)\unistd.h

open.c: $(I)\MsvcLibX.h $(I)\fcntl.h $(CI)\debugm.h

pwd.c: $(I)\pwd.h 

readlink.c: $(CI)\debugm.h $(I)\unistd.h $(I)\reparsept.h

realpath.c: $(CI)\debugm.h $(I)\unistd.h

rmdir.c: $(CI)\debugm.h $(I)\MsvcLibX.h $(I)\sys\stat.h

spawm.c: $(CI)\debugm.h $(I)\MsvcLibX.h $(I)\process.h

strerror.c: $(I)\MsvcLibX.h

# strndup.c: 

# strptime.c:

symlink.c: $(CI)\debugm.h $(I)\reparsept.h $(I)\unistd.h

uname.c: $(I)\MsvcLibX.h $(I)\sys\utsname.h

unlink.c: $(CI)\debugm.h $(I)\MsvcLibX.h $(I)\sys\stat.h

utime.c: $(CI)\debugm.h $(I)\unistd.h $(I)\utime.h $(I)\sys\time.h

xfreopen.c: $(I)\xfreopen.h

