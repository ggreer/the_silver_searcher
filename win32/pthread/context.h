/*
 * context.h
 *
 * Description:
 * POSIX thread macros related to thread cancellation.
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

#ifndef  __PTW32_CONTEXT_H
#define  __PTW32_CONTEXT_H

#undef  __PTW32_PROGCTR

#if defined(_M_IX86) || (defined(_X86_) && !defined(__amd64__))
#define  __PTW32_PROGCTR(Context)  ((Context).Eip)
#endif

#if defined (_M_IA64) || defined(_IA64)
#define  __PTW32_PROGCTR(Context)  ((Context).StIIP)
#endif

#if defined(_MIPS_) || defined(MIPS)
#define  __PTW32_PROGCTR(Context)  ((Context).Fir)
#endif

#if defined(_ALPHA_)
#define  __PTW32_PROGCTR(Context)  ((Context).Fir)
#endif

#if defined(_PPC_)
#define  __PTW32_PROGCTR(Context)  ((Context).Iar)
#endif

#if defined(_AMD64_) || defined(__amd64__)
#define  __PTW32_PROGCTR(Context)  ((Context).Rip)
#endif

#if defined(_ARM_) || defined(ARM) || defined(_M_ARM) || defined(_M_ARM64)
#define PTW32_PROGCTR(Context)  ((Context).Pc)
#endif

#if !defined (__PTW32_PROGCTR)
#error Module contains CPU-specific code; modify and recompile.
#endif

#endif
