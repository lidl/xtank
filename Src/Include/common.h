/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** common.h
*/

/*
$Author: lidl $
$Id: common.h,v 1.1.1.1 1995/02/01 00:25:40 lidl Exp $
*/

#ifndef _COMMON_H_
#define _COMMON_H_

/* Useful constants */

#ifndef NULL
#define NULL 0
#endif

#ifndef PI
#define PI 3.14159265358979323846	/* Not PI, but an incredible simulation */
#endif

#ifndef SQRT_2
#define SQRT_2 1.4142136		/* close enough for goverment work */
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
#ifndef __GNUC__
#if (!defined(hpux) && !defined(sgi)) && !defined(__alpha)
#if (!defined(_IBMR2))
#if (!defined(i860) && !defined(sequent))
#if !defined(mips)
extern char *sprintf(), *memset(), *memcpy();

#else
extern char *sprintf();

#endif
#endif
#if defined(i860)
extern char *strcpy();

#else
extern char *malloc(), *calloc(), *realloc(), *strcpy();

#endif
#endif
#endif
extern long random();
extern void exit();
extern double atan2(), hypot(), sin(), cos(), sqrt(), floor(), aint();
extern double pow(), asin();

#endif


#endif /* ndef _COMMON_H_ */
