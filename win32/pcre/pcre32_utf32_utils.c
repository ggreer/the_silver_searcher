/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
           Copyright (c) 1997-2012 University of Cambridge

-----------------------------------------------------------------------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of the University of Cambridge nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------
*/


/* This module contains a function for converting any UTF-32 character
strings to host byte order. */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Generate code with 32 bit character support. */
#define COMPILE_PCRE32

#include "pcre_internal.h"

#ifdef SUPPORT_UTF
static pcre_uint32
swap_uint32(pcre_uint32 value)
{
return ((value & 0x000000ff) << 24) |
       ((value & 0x0000ff00) <<  8) |
       ((value & 0x00ff0000) >>  8) |
       (value >> 24);
}
#endif


/*************************************************
*  Convert any UTF-32 string to host byte order  *
*************************************************/

/* This function takes an UTF-32 string and converts
it to host byte order. The length can be explicitly set,
or automatically detected for zero terminated strings.
BOMs can be kept or discarded during the conversion.
Conversion can be done in place (output == input).

Arguments:
  output     the output buffer, its size must be greater
             or equal than the input string
  input      any UTF-32 string
  length     the number of 32-bit units in the input string
             can be less than zero for zero terminated strings
  host_byte_order
             A non-zero value means the input is in host byte
             order, which can be dynamically changed by BOMs later.
             Initially it contains the starting byte order and returns
             with the last byte order so it can be used for stream
             processing. It can be NULL, which set the host byte
             order mode by default.
  keep_boms  for a non-zero value, the BOM (0xfeff) characters
             are copied as well

Returns:     the number of 32-bit units placed into the output buffer,
             including the zero-terminator
*/

int
pcre32_utf32_to_host_byte_order(PCRE_UCHAR32 *output, PCRE_SPTR32 input,
  int length, int *host_byte_order, int keep_boms)
{
#ifdef SUPPORT_UTF
/* This function converts any UTF-32 string to host byte order and optionally
removes any Byte Order Marks (BOMS). Returns with the remainig length. */
int host_bo = host_byte_order != NULL ? *host_byte_order : 1;
pcre_uchar *optr = (pcre_uchar *)output;
const pcre_uchar *iptr = (const pcre_uchar *)input;
const pcre_uchar *end;
/* The c variable must be unsigned. */
register pcre_uchar c;

if (length < 0)
  end = iptr + STRLEN_UC(iptr) + 1;
else
  end = iptr + length;

while (iptr < end)
  {
  c = *iptr++;
  if (c == 0x0000feffu || c == 0xfffe0000u)
    {
    /* Detecting the byte order of the machine is unnecessary, it is
    enough to know that the UTF-32 string has the same byte order or not. */
    host_bo = c == 0x0000feffu;
    if (keep_boms != 0)
      *optr++ = 0x0000feffu;
    }
  else
    *optr++ = host_bo ? c : swap_uint32(c);
  }
if (host_byte_order != NULL)
  *host_byte_order = host_bo;

#else /* SUPPORT_UTF */
(void)(output);  /* Keep picky compilers happy */
(void)(input);
(void)(keep_boms);
(void)(host_byte_order);
#endif /* SUPPORT_UTF */
return length;
}

/* End of pcre32_utf32_utils.c */
