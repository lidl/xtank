
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** icounter.h
*/

/*
$Author: lidl $
$Id: icounter.h,v 1.1.1.1 1995/02/01 00:25:40 lidl Exp $
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
