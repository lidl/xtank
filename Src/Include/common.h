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
#if !defined(MIN)
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif
#if !defined(MAX)
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif
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
