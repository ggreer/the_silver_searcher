/*
 * Module: sched.h
 *
 * Purpose:
 *      Provides an implementation of POSIX realtime extensions
 *      as defined in
 *
 *              POSIX 1003.1b-1993      (POSIX.1b)
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2012 Pthreads-win32 contributors
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
 */
#if !defined(_SCHED_H)
#define _SCHED_H
#define __SCHED_H_SOURCED__

#include <_ptw32.h>

/* We need a typedef for pid_t, (and POSIX requires <sched.h> to
 * define it, as it is defined in <sys/types.h>, but it does NOT
 * sanction exposure of everything from <sys/types.h>); there is
 * no pid_t in Windows anyway, (except that MinGW does define it
 * in their <sys/types.h>), so just provide a suitable typedef,
 * but note that we must do so cautiously, to avoid a typedef
 * conflict if MinGW's <sys/types.h> is also #included:
 */
#if ! defined __MINGW32__ || ! defined __have_typedef_pid_t

# if defined __MINGW64__
    typedef __int64 pid_t;
# else
    typedef int pid_t;
#endif

#if __GNUC__ < 4
/* GCC v4.0 and later, (as used by MinGW), allows us to repeat a
 * typedef, provided every duplicate is consistent; only set this
 * multiple definition guard when we cannot be certain that it is
 * permissable to repeat typedefs.
 */
#define __have_typedef_pid_t  1
#endif
#endif

/* POSIX.1-1993 says that <sched.h> WILL expose all of <time.h>
 */
#undef __SCHED_H_SOURCED__
#if _POSIX_C_SOURCE >= 200112L
/* POSIX.1-2001 and later revises this to say only that it MAY do so;
 * only struct timespec, and associated time_t are actually required,
 * so prefer to be selective; (MinGW.org's <time.h> offers an option
 * for selective #inclusion, when __SCHED_H_SOURCED__ is defined):
 */
#define __SCHED_H_SOURCED__
#define __need_struct_timespec
#define __need_time_t
#endif
#include <time.h>

#if defined __MINGW64__ || _MSC_VER >= 1900
/* These are known to define struct timespec, when <time.h> has been
 * #included, but may not, (probably don't), follow the convention of
 * defining __struct_timespec_defined, as adopted by MinGW.org; for
 * these cases, we unconditionally assume that struct timespec has
 * been defined, otherwise, if MinGW.org's criterion has not been
 * satisfied...
 */
#elif ! defined __struct_timespec_defined
#  ifndef _TIMESPEC_DEFINED
#  define _TIMESPEC_DEFINED
struct timespec
{ /* ...we fall back on this explicit definition.
   */
  time_t	tv_sec;
  int		tv_nsec;
};
#  endif
#endif

/*
 * Microsoft VC++6.0 lacks these *_PTR types
 */
#if defined(_MSC_VER) && _MSC_VER < 1300 && !defined(PTW32_HAVE_DWORD_PTR)
typedef unsigned long ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;
#endif

/* Thread scheduling policies */

enum
{ SCHED_OTHER = 0,
  SCHED_FIFO,
  SCHED_RR,
  SCHED_MIN   = SCHED_OTHER,
  SCHED_MAX   = SCHED_RR
};

struct sched_param
{ int  sched_priority;
};

/*
 * CPU affinity
 *
 * cpu_set_t:
 * Considered opaque but cannot be an opaque pointer due to the need for
 * compatibility with GNU systems and sched_setaffinity() et.al., which
 * include the cpusetsize parameter "normally set to sizeof(cpu_set_t)".
 *
 * FIXME: These are GNU, and NOT specified by POSIX; maybe consider
 * occluding them within a _GNU_SOURCE (or similar) feature test.
 */

#define CPU_SETSIZE (sizeof(size_t)*8)

#define CPU_COUNT(setptr) (_sched_affinitycpucount(setptr))

#define CPU_ZERO(setptr) (_sched_affinitycpuzero(setptr))

#define CPU_SET(cpu, setptr) (_sched_affinitycpuset((cpu),(setptr)))

#define CPU_CLR(cpu, setptr) (_sched_affinitycpuclr((cpu),(setptr)))

#define CPU_ISSET(cpu, setptr) (_sched_affinitycpuisset((cpu),(setptr)))

#define CPU_AND(destsetptr, srcset1ptr, srcset2ptr) (_sched_affinitycpuand((destsetptr),(srcset1ptr),(srcset2ptr)))

#define CPU_OR(destsetptr, srcset1ptr, srcset2ptr) (_sched_affinitycpuor((destsetptr),(srcset1ptr),(srcset2ptr)))

#define CPU_XOR(destsetptr, srcset1ptr, srcset2ptr) \
 (_sched_affinitycpuxor((destsetptr),(srcset1ptr),(srcset2ptr)))

#define CPU_EQUAL(set1ptr, set2ptr) (_sched_affinitycpuequal((set1ptr),(set2ptr)))

typedef union
{ char     cpuset[CPU_SETSIZE/8];
  size_t  _align;
} cpu_set_t;

__PTW32_BEGIN_C_DECLS

PTW32_DLLPORT int PTW32_CDECL sched_yield (void);

PTW32_DLLPORT int PTW32_CDECL sched_get_priority_min (int policy);

PTW32_DLLPORT int PTW32_CDECL sched_get_priority_max (int policy);

/* FIXME: this declaration of sched_setscheduler() is NOT as prescribed
 * by POSIX; it lacks const struct sched_param * as third argument.
 */
PTW32_DLLPORT int PTW32_CDECL sched_setscheduler (pid_t pid, int policy);

/* FIXME: In addition to the above five functions, POSIX also requires:
 *
 *   int sched_getparam (pid_t, struct sched_param *);
 *   int sched_setparam (pid_t, const struct sched_param *);
 *
 * both of which are conspicuous by their absence here!
 */

/* Compatibility with Linux - not standard in POSIX
 * FIXME: consider occluding within a _GNU_SOURCE (or similar) feature test.
 */
PTW32_DLLPORT int PTW32_CDECL sched_setaffinity (pid_t pid, size_t cpusetsize, cpu_set_t *mask);

PTW32_DLLPORT int PTW32_CDECL sched_getaffinity (pid_t pid, size_t cpusetsize, cpu_set_t *mask);

/*
 * Support routines and macros for cpu_set_t
 */
PTW32_DLLPORT int PTW32_CDECL _sched_affinitycpucount (const cpu_set_t *set);

PTW32_DLLPORT void PTW32_CDECL _sched_affinitycpuzero (cpu_set_t *pset);

PTW32_DLLPORT void PTW32_CDECL _sched_affinitycpuset (int cpu, cpu_set_t *pset);

PTW32_DLLPORT void PTW32_CDECL _sched_affinitycpuclr (int cpu, cpu_set_t *pset);

PTW32_DLLPORT int PTW32_CDECL _sched_affinitycpuisset (int cpu, const cpu_set_t *pset);

PTW32_DLLPORT void PTW32_CDECL _sched_affinitycpuand(cpu_set_t *pdestset, const cpu_set_t *psrcset1, const cpu_set_t *psrcset2);

PTW32_DLLPORT void PTW32_CDECL _sched_affinitycpuor(cpu_set_t *pdestset, const cpu_set_t *psrcset1, const cpu_set_t *psrcset2);

PTW32_DLLPORT void PTW32_CDECL _sched_affinitycpuxor(cpu_set_t *pdestset, const cpu_set_t *psrcset1, const cpu_set_t *psrcset2);

PTW32_DLLPORT int PTW32_CDECL _sched_affinitycpuequal (const cpu_set_t *pset1, const cpu_set_t *pset2);

/* Note that this macro returns ENOTSUP rather than ENOSYS, as
 * might be expected. However, returning ENOSYS should mean that
 * sched_get_priority_{min,max} are not implemented as well as
 * sched_rr_get_interval.  This is not the case, since we just
 * don't support round-robin scheduling. Therefore I have chosen
 * to return the same value as sched_setscheduler when SCHED_RR
 * is passed to it.
 *
 * FIXME: POSIX requires this to be defined as a function; this
 * macro implementation is permitted IN ADDITION to the function,
 * but the macro alone is not POSIX compliant!  Worse still, it
 * imposes a requirement on the caller, to ensure that both the
 * declaration of errno, and the definition of ENOTSUP, are in
 * scope at point of call, (which it may wish to do anyway, but
 * POSIX imposes no such constraint)!
 */
#define sched_rr_get_interval(_pid, _interval) \
  ( errno = ENOTSUP, (int) -1 )

__PTW32_END_C_DECLS

#undef __SCHED_H_SOURCED__
#endif	/* !_SCHED_H */
