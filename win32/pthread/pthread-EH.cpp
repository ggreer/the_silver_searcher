/*
 * pthread-EH.cpp
 *
 * Description:
 * Work-around for https://gitlab.kitware.com/cmake/cmake/-/issues/21476 and
 * other rigs where 'compile C source file as C++' enforcement is a problem.
 * 
 * Quoting the CMake issue (remark by Kevin Puetz, 2020):
 *
 *     One answer that just came to mind: create a pthread.cxx, that just 
 *     consists of #include "pthread.c", and let the preprocessor paste the 
 *     code in. Then CMake just sees two files and it's all normal. [...]
 *
 * See also https://discourse.cmake.org/t/compiling-c-as-c-in-cmake-3-18/2172.
 *
 * --------------------------------------------------------------------------
 *
 *      pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999-2021 pthreads-win32 / pthreads4w contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * --------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define __PTHREAD_JUMBO_BUILD__ 1
#include "pthread.c"
