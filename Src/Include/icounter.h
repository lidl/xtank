/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef UNIX
typedef int Clock;
extern int elapsed_time;

#define clear_clock() \
  { elapsed_time = 0; }

#define write_clock(cl,micro) *(cl) = micro;

#define read_clock(cl) \
  (*(cl))

#define get_clock(cl) \
  { *(cl) = elapsed_time; }

#define compare_clocks(cl1,cl2) \
  ( (*(cl1)) > (*(cl2)) ? -1 : (*(cl1)) < (*(cl2)) ? 1 : 0 )
#endif

#ifdef AMIGA

#include <exec/types.h>
#include <exec/interrupts.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

  typedef struct {
	  int ticks, vpos;
  }
Clock;

extern struct Custom *c;
extern int count;

#define clear_clock() \
  { count = 0; }

#define write_clock(cl,micro) \
  ((cl)->ticks = (micro)/16667, (cl)->vpos = ((micro)%16667)>>6)

#define read_clock(cl) \
  ( ((cl)->ticks*16667) + ((cl)->vpos<<6) )

#define get_clock(cl) \
  { (cl)->ticks = count; (cl)->vpos = ((c->vposr&1)<<8)+(c->vhposr>>8); \
    if ((cl)->vpos < 10) (cl)->vpos += 256; }

#define compare_clocks(cl1,cl2) \
  ( (cl1)->ticks > (cl2)->ticks ? -1 : \
    (cl1)->ticks < (cl2)->ticks ?  1 : \
     (cl1)->vpos > (cl2)->vpos  ? -1 : \
     (cl1)->vpos < (cl2)->vpos  ?  1 : 0)
#endif
