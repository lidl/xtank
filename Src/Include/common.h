/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** common.h
*/

/*
$Author: lidl $
$Id: common.h,v 2.7 1991/10/07 03:16:12 lidl Exp $

$Log: common.h,v $
 * Revision 2.7  1991/10/07  03:16:12  lidl
 * fixed a botch in sgi support on mips platforms (hopefully)
 *
 * Revision 2.6  1991/09/19  05:29:07  lidl
 * mips header problem cleaned up
 *
 * Revision 2.5  1991/08/22  03:08:15  aahz
 * avoided warnings on the i860 machine.
 *
 * Revision 2.4  1991/03/25  00:40:13  stripes
 * RS6K patches
 *
 * Revision 2.3  1991/02/10  13:50:14  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:27  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:03  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:11  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:08  aahz
 * Initial revision
 * 
*/

#ifndef _COMMON_H_
#define _COMMON_H_


/* Useful constants */

#ifndef NULL
#define NULL 0
#endif

#ifndef PI
#define PI 3.14159265358979323846  /* Not PI, but an incredible simulation */
#endif

#define BAD_VALUE       (-1)


/* Useful macros */

#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define ABS(x) (((x) > 0) ? (x) : -(x))
#define SIGN(x)	((x) < 0 ? -1 : (x) == 0 ? 0 : 1)
#define SQR(x) ((x)*(x))


/* macros for handling sets of bits.  Variables are assumed to be integers,
   flags are assumed to be something like (1 << 7) */

/* set "flag" in variable "var */
#define setflag(var,flag) ((var) |= (flag))
/* clear "flag" in variable "var */
#define clrflag(var,flag) ((var) &= ~(flag))
/* test "flag" in variable "var */
#define tstflag(var,flag) ((var) & (flag))


/* takes two Vehicle pointers, returns true if they are on the same team */
#define SAME_TEAM(v1,v2) ((v1)==(v2) || ((v1)->team == (v2)->team && \
					 (v1->team != NEUTRAL)))


#define rorre(dat) \
  {printf("error: %s\n", (char *)dat); \
  exit(17);}


/* avoid some lint warnings */
#if (!defined(hpux) && !defined(sgi))
# if (!defined(_IBMR2))
#  ifndef i860
#   ifndef mips
extern char *sprintf(), *memset(), *memcpy();
#   else
extern char *sprintf();
#   endif
#  endif
extern char *malloc(), *calloc(), *realloc(), *strcpy();
# endif
extern long random();
extern void exit();
extern double atan2(), hypot(), sin(), cos(), sqrt(), floor(), aint();
extern double pow(), asin();
#endif


#endif ndef _COMMON_H_
