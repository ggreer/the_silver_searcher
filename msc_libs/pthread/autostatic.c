/*
 * autostatic.c
 *
 * Description:
 * This translation unit implements static auto-init and auto-exit logic.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 *
 *      Contact Email: rpj@callisto.canberra.edu.au
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
 */

#include "pthread.h"
#include "implement.h"

#if defined(PTW32_STATIC_LIB)

#if defined(PTW32_CONFIG_MINGW) || defined(_MSC_VER)

/* For an explanation of this code (at least the MSVC parts), refer to
 *
 * http://www.codeguru.com/cpp/misc/misc/threadsprocesses/article.php/c6945/
 * ("Running Code Before and After Main")
 *
 * Compatibility with MSVC8 was cribbed from Boost:
 *
 * http://svn.boost.org/svn/boost/trunk/libs/thread/src/win32/tss_pe.cpp
 *
 * In addition to that, because we are in a static library, and the linker
 * can't tell that the constructor/destructor functions are actually
 * needed, we need a way to prevent the linker from optimizing away this
 * module. The pthread_win32_autostatic_anchor() hack below (and in
 * implement.h) does the job in a portable manner.
 */

static int on_process_init(void)
{
    pthread_win32_process_attach_np ();
    return 0;
}

static int on_process_exit(void)
{
    pthread_win32_thread_detach_np  ();
    pthread_win32_process_detach_np ();
    return 0;
}

#if defined(PTW32_CONFIG_MINGW)
__attribute__((section(".ctors"), used)) static int (*gcc_ctor)(void) = on_process_init;
__attribute__((section(".dtors"), used)) static int (*gcc_dtor)(void) = on_process_exit;
#elif defined(_MSC_VER)
#  if _MSC_VER >= 1400 /* MSVC8 */
#    pragma section(".CRT$XCU", long, read)
#    pragma section(".CRT$XPU", long, read)
__declspec(allocate(".CRT$XCU")) static int (*msc_ctor)(void) = on_process_init;
__declspec(allocate(".CRT$XPU")) static int (*msc_dtor)(void) = on_process_exit;
#  else
#    pragma data_seg(".CRT$XCU")
static int (*msc_ctor)(void) = on_process_init;
#    pragma data_seg(".CRT$XPU")
static int (*msc_dtor)(void) = on_process_exit;
#    pragma data_seg() /* reset data segment */
#  endif
#endif

#endif /* defined(PTW32_CONFIG_MINGW) || defined(_MSC_VER) */

/* This dummy function exists solely to be referenced by other modules
 * (specifically, in implement.h), so that the linker can't optimize away
 * this module. Don't call it.
 */
void ptw32_autostatic_anchor(void) { abort(); }

#endif /* PTW32_STATIC_LIB */

#if ! defined(PTW32_BUILD_INLINED)
/*
 * Avoid "translation unit is empty" warnings
 */
typedef int foo;
#endif
