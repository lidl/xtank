/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** icounter.h
*/

/*
$Author: lidl $
$Id: icounter.h,v 2.4 1991/09/15 09:24:51 lidl Exp $

$Log: icounter.h,v $
 * Revision 2.4  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.3  1991/02/10  13:50:46  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:01  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:46  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:38  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:31  aahz
 * Initial revision
 * 
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

typedef struct
{
    int   ticks, vpos;
} Clock;

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
