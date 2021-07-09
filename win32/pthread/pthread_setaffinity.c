/*
 * pthread_setaffinity.c
 *
 * Description:
 * This translation unit implements thread cpu affinity setting.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads4w - POSIX Threads for Windows
 *      Copyright 1998 John E. Bossom
 *      Copyright 1999-2018, Pthreads4w contributors
 *
 *      Homepage: https://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *
 *      https://sourceforge.net/p/pthreads4w/wiki/Contributors/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"

int
pthread_setaffinity_np (pthread_t thread, size_t cpusetsize,
                                  const cpu_set_t *cpuset)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *   The pthread_setaffinity_np() function sets the CPU affinity mask
      *   of the thread thread to the CPU set pointed to by cpuset.  If the
      *   call is successful, and the thread is not currently running on one
      *   of the CPUs in cpuset, then it is migrated to one of those CPUs.
      *
      * PARAMETERS
      *		thread
      *					The target thread
      *
      *		cpusetsize
      *					Ignored in pthreads4w.
      *					Usually set to sizeof(cpu_set_t)
      *
      *		cpuset
      *					The new cpu set mask.
      *
      *   				The set of CPUs on which the thread will actually run
      *   				is the intersection of the set specified in the cpuset
      *   				argument and the set of CPUs actually present for
      *   				the process.
      *
      * DESCRIPTION
      *   The pthread_setaffinity_np() function sets the CPU affinity mask
      *   of the thread thread to the CPU set pointed to by cpuset.  If the
      *   call is successful, and the thread is not currently running on one
      *   of the CPUs in cpuset, then it is migrated to one of those CPUs.
      *
      * RESULTS
      * 				0		Success
      * 				ESRCH	Thread does not exist
      * 				EFAULT	pcuset is NULL
      * 				EAGAIN	The thread affinity could not be set
      * 				ENOSYS  The platform does not support this function
      *
      * ------------------------------------------------------
      */
{
#if ! defined(HAVE_CPU_AFFINITY)

  return ENOSYS;

#else

  int result = 0;
  __ptw32_thread_t * tp;
  __ptw32_mcs_local_node_t node;
  cpu_set_t processCpuset;

  __ptw32_mcs_lock_acquire (&__ptw32_thread_reuse_lock, &node);

  tp = (__ptw32_thread_t *) thread.p;

  if (NULL == tp || thread.x != tp->ptHandle.x || NULL == tp->threadH)
    {
	  result = ESRCH;
    }
  else
	{
	  if (cpuset)
		{
		  if (sched_getaffinity(0, sizeof(cpu_set_t), &processCpuset))
		    {
			  result =  __PTW32_GET_ERRNO();
		    }
		  else
			{
			  /*
			   * Result is the intersection of available CPUs and the mask.
			   */
			  cpu_set_t newMask;

			  CPU_AND(&newMask, &processCpuset, cpuset);

			  if (((_sched_cpu_set_vector_*)&newMask)->_cpuset)
				{
				  if (SetThreadAffinityMask (tp->threadH, ((_sched_cpu_set_vector_*)&newMask)->_cpuset))
					{
					  /*
					   * We record the intersection of the process affinity
					   * and the thread affinity cpusets so that
					   * pthread_getaffinity_np() returns the actual thread
					   * CPU set.
					   */
					  tp->cpuset = ((_sched_cpu_set_vector_*)&newMask)->_cpuset;
					}
				  else
					{
					  result = EAGAIN;
					}
				}
			  else
				{
				  result = EINVAL;
				}
			}
		}
	  else
		{
		  result = EFAULT;
		}
	}

  __ptw32_mcs_lock_release (&node);

  return result;

#endif
}

int
pthread_getaffinity_np (pthread_t thread, size_t cpusetsize, cpu_set_t *cpuset)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *   The pthread_getaffinity_np() function returns the CPU affinity mask
      *   of the thread thread in the CPU set pointed to by cpuset.
      *
      * PARAMETERS
      *		thread
      *					The target thread
      *
      *		cpusetsize
      *					Ignored in pthreads4w.
      *					Usually set to sizeof(cpu_set_t)
      *
      *		cpuset
      *					The location where the current cpu set
      *					will be returned.
      *
      *
      * DESCRIPTION
      *   The pthread_getaffinity_np() function returns the CPU affinity mask
      *   of the thread thread in the CPU set pointed to by cpuset.
      *
      * RESULTS
      * 				0		Success
      * 				ESRCH	thread does not exist
      * 				EFAULT	cpuset is NULL
      *                                 ENOSYS  The platform does not support this function
      *
      * ------------------------------------------------------
      */
{
#if ! defined(HAVE_CPU_AFFINITY)

  return ENOSYS;

#else

  int result = 0;
  __ptw32_thread_t * tp;
  __ptw32_mcs_local_node_t node;

  __ptw32_mcs_lock_acquire(&__ptw32_thread_reuse_lock, &node);

  tp = (__ptw32_thread_t *) thread.p;

  if (NULL == tp || thread.x != tp->ptHandle.x || NULL == tp->threadH)
    {
	  result = ESRCH;
    }
  else
    {
	  if (cpuset)
	    {
		  if (tp->cpuset)
		    {
			  /*
			   * The application may have set thread affinity independently
			   * via SetThreadAffinityMask(). If so, we adjust our record of the threads
			   * affinity and try to do so in a reasonable way.
			   */
			  DWORD_PTR vThreadMask = SetThreadAffinityMask(tp->threadH, tp->cpuset);
			  if (vThreadMask && vThreadMask != tp->cpuset)
			    {
				  (void) SetThreadAffinityMask(tp->threadH, vThreadMask);
				  tp->cpuset = vThreadMask;
			    }
		    }
		  ((_sched_cpu_set_vector_*)cpuset)->_cpuset = tp->cpuset;
		}
	  else
	    {
		  result = EFAULT;
	    }
    }

  __ptw32_mcs_lock_release(&node);

  return result;

#endif
}
