/*
 * sched_setaffinity.c
 *
 * Description:
 * POSIX scheduling functions that deal with CPU affinity.
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

#include "pthread.h"
#include "implement.h"
#include "sched.h"

int
sched_setaffinity (pid_t pid, size_t cpusetsize, cpu_set_t *set)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Sets the CPU affinity mask of the process whose ID is pid
      *	     to the value specified by mask.  If pid is zero, then the
      *	     calling process is used.  The argument cpusetsize is the
      *	     length (in bytes) of the data pointed to by mask.  Normally
      *	     this argument would be specified as sizeof(cpu_set_t).
      *
      *	     If the process specified by pid is not currently running on
      *	     one of the CPUs specified in mask, then that process is
      *	     migrated to one of the CPUs specified in mask.
      *
      * PARAMETERS
      *      pid
      *      			Process ID
      *
      *      cpusetsize
      *      			Currently ignored in pthreads4w.
      *      			Usually set to sizeof(cpu_set_t)
      *
      *      mask
      *      			Pointer to the CPU mask to set (cpu_set_t).
      *
      * DESCRIPTION
      *      Sets the CPU affinity mask of the process whose ID is pid
      *	     to the value specified by mask.  If pid is zero, then the
      *	     calling process is used.  The argument cpusetsize is the
      *	     length (in bytes) of the data pointed to by mask.  Normally
      *	     this argument would be specified as sizeof(cpu_set_t).
      *
      *	     If the process specified by pid is not currently running on
      *	     one of the CPUs specified in mask, then that process is
      *	     migrated to one of the CPUs specified in mask.
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              EFAULT          'mask' is a NULL pointer.
      *              EINVAL          '*mask' contains no CPUs in the set
      *                              of available CPUs.
      *              EAGAIN          The system available CPUs could not
      *                              be obtained.
      *              EPERM           The process referred to by 'pid' is
      *                              not modifiable by us.
      *              ESRCH           The process referred to by 'pid' was
      *                              not found.
      *              ENOSYS			 Function not supported.
      *
      * ------------------------------------------------------
      */
{
#if ! defined(NEED_PROCESS_AFFINITY_MASK)
  DWORD_PTR vProcessMask;
  DWORD_PTR vSystemMask;
  HANDLE h;
  int targetPid = (int)(size_t) pid;
  int result = 0;

  cpusetsize = sizeof(cpu_set_t);

  if (NULL == set)
	{
	  result = EFAULT;
	}
  else
	{
	  if (0 == targetPid)
		{
		  targetPid = (int) GetCurrentProcessId ();
		}

	  h = OpenProcess (PROCESS_QUERY_INFORMATION|PROCESS_SET_INFORMATION, PTW32_FALSE, (DWORD) targetPid);

	  if (NULL == h)
		{
		  result = (((0xFF & ERROR_ACCESS_DENIED) == GetLastError()) ? EPERM : ESRCH);
		}
	  else
		{
		  if (GetProcessAffinityMask (h, &vProcessMask, &vSystemMask))
			{
			  /*
			   * Result is the intersection of available CPUs and the mask.
			   */
			  DWORD_PTR newMask = vSystemMask & ((_sched_cpu_set_vector_*)set)->_cpuset;

			  if (newMask)
				{
				  if (SetProcessAffinityMask(h, newMask) == 0)
				    {
					  switch (GetLastError())
					    {
					  	  case (0xFF & ERROR_ACCESS_DENIED):
					  		  result = EPERM;
					  		  break;
					  	  case (0xFF & ERROR_INVALID_PARAMETER):
					  		  result = EINVAL;
					  		  break;
					  	  default:
					  		  result = EAGAIN;
					  		  break;
					    }
				    }
				}
			  else
				{
				  /*
				   * Mask does not contain any CPUs currently available on the system.
				   */
				  result = EINVAL;
				}
			}
		  else
			{
			  result = EAGAIN;
			}
          CloseHandle(h);
		}
	}

  if (result != 0)
    {
	  PTW32_SET_ERRNO(result);
	  return -1;
    }
  else
    {
	  return 0;
    }

#else

  PTW32_SET_ERRNO(ENOSYS);
  return -1;

#endif
}


int
sched_getaffinity (pid_t pid, size_t cpusetsize, cpu_set_t *set)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Gets the CPU affinity mask of the process whose ID is pid
      *	     to the value specified by mask.  If pid is zero, then the
      *	     calling process is used.  The argument cpusetsize is the
      *	     length (in bytes) of the data pointed to by mask.  Normally
      *	     this argument would be specified as sizeof(cpu_set_t).
      *
      * PARAMETERS
      *      pid
      *      			Process ID
      *
      *      cpusetsize
      *      			Currently ignored in pthreads4w.
      *      			Usually set to sizeof(cpu_set_t)
      *
      *      mask
      *      			Pointer to the CPU mask to set (cpu_set_t).
      *
      * DESCRIPTION
      *      Sets the CPU affinity mask of the process whose ID is pid
      *	     to the value specified by mask.  If pid is zero, then the
      *	     calling process is used.  The argument cpusetsize is the
      *	     length (in bytes) of the data pointed to by mask.  Normally
      *	     this argument would be specified as sizeof(cpu_set_t).
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              EFAULT          'mask' is a NULL pointer.
      *              EAGAIN          The system available CPUs could not
      *                              be obtained.
      *              EPERM           The process referred to by 'pid' is
      *                              not modifiable by us.
      *              ESRCH           The process referred to by 'pid' was
      *                              not found.
      *
      * ------------------------------------------------------
      */
{
  DWORD_PTR vProcessMask;
  DWORD_PTR vSystemMask;
  HANDLE h;
  int targetPid = (int)(size_t) pid;
  int result = 0;

  cpusetsize = sizeof(cpu_set_t);

  if (NULL == set)
    {
	  result = EFAULT;
    }
  else
    {

#if ! defined(NEED_PROCESS_AFFINITY_MASK)

	  if (0 == targetPid)
	    {
		  targetPid = (int) GetCurrentProcessId ();
	    }

	  h = OpenProcess (PROCESS_QUERY_INFORMATION, PTW32_FALSE, (DWORD) targetPid);

	  if (NULL == h)
	    {
		  result = (((0xFF & ERROR_ACCESS_DENIED) == GetLastError()) ? EPERM : ESRCH);
	    }
	  else
	    {
		  if (GetProcessAffinityMask (h, &vProcessMask, &vSystemMask))
		    {
			  ((_sched_cpu_set_vector_*)set)->_cpuset = vProcessMask;
		    }
		  else
		    {
			  result = EAGAIN;
		    }
          CloseHandle(h);
	    }

#else
	  ((_sched_cpu_set_vector_*)set)->_cpuset = (size_t)0x1;
#endif

    }

  if (result != 0)
    {
	  PTW32_SET_ERRNO(result);
	  return -1;
    }
  else
    {
	  return 0;
    }
}

/*
 * Support routines for cpu_set_t
 */
int _sched_affinitycpucount (const cpu_set_t *set)
{
  size_t tset;
  int count;

  /*
   * Relies on tset being unsigned, otherwise the right-shift will
   * be arithmetic rather than logical and the 'for' will loop forever.
   */
  for (count = 0, tset = ((_sched_cpu_set_vector_*)set)->_cpuset; tset; tset >>= 1)
    {
	  if (tset & (size_t)1)
	  	{
		  count++;
	  	}
    }
  return count;
}

void _sched_affinitycpuzero (cpu_set_t *pset)
{
	((_sched_cpu_set_vector_*)pset)->_cpuset = (size_t)0;
}

void _sched_affinitycpuset (int cpu, cpu_set_t *pset)
{
	((_sched_cpu_set_vector_*)pset)->_cpuset |= ((size_t)1 << cpu);
}

void _sched_affinitycpuclr (int cpu, cpu_set_t *pset)
{
	((_sched_cpu_set_vector_*)pset)->_cpuset &= ~((size_t)1 << cpu);
}

int _sched_affinitycpuisset (int cpu, const cpu_set_t *pset)
{
	return ((((_sched_cpu_set_vector_*)pset)->_cpuset &
			((size_t)1 << cpu)) != (size_t)0);
}

void _sched_affinitycpuand(cpu_set_t *pdestset, const cpu_set_t *psrcset1, const cpu_set_t *psrcset2)
{
	((_sched_cpu_set_vector_*)pdestset)->_cpuset =
			(((_sched_cpu_set_vector_*)psrcset1)->_cpuset &
					((_sched_cpu_set_vector_*)psrcset2)->_cpuset);
}

void _sched_affinitycpuor(cpu_set_t *pdestset, const cpu_set_t *psrcset1, const cpu_set_t *psrcset2)
{
	((_sched_cpu_set_vector_*)pdestset)->_cpuset =
			(((_sched_cpu_set_vector_*)psrcset1)->_cpuset |
					((_sched_cpu_set_vector_*)psrcset2)->_cpuset);
}

void _sched_affinitycpuxor(cpu_set_t *pdestset, const cpu_set_t *psrcset1, const cpu_set_t *psrcset2)
{
	((_sched_cpu_set_vector_*)pdestset)->_cpuset =
			(((_sched_cpu_set_vector_*)psrcset1)->_cpuset ^
					((_sched_cpu_set_vector_*)psrcset2)->_cpuset);
}

int _sched_affinitycpuequal (const cpu_set_t *pset1, const cpu_set_t *pset2)
{
  return (((_sched_cpu_set_vector_*)pset1)->_cpuset ==
		  ((_sched_cpu_set_vector_*)pset2)->_cpuset);
}
