/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#ifndef SYSDEP_H
#define SYSDEP_H

#ifdef i860
#define STRUCT_ASSIGN(a,b,c) { memcpy(&(a), &(b), sizeof(c)); }
#else
#define STRUCT_ASSIGN(a,b,c) { a = b; }
#endif


#if defined(sun) || defined(hpux) || defined(apollo) || defined(mips) || defined(MOTOROLA) || defined(i860) || defined(mmax) || defined(__alpha) || defined(__bsdi__)
/* Avoid domain errors when both x and y are 0 */
#define ATAN2(_Y,_X) ((_X)==0 && (_Y)==0 ? 0.0 : atan2((double)_Y, (double)_X))
#else
#define ATAN2(Y,X) atan2((double)Y, (double)X)
#endif

#if 0
/* drem() not available so add a wrapper around fmod() */
#if !defined(drem)

#if defined(fmod)
static double temp_drem;

#define drem(a,b) \
	((temp_drem = fmod(a,b)) > (b)/2 ? temp_drem-(b) : temp_drem)

#else /* !fmod */

#define drem(a,b) \
	(((double)(a))-(int)((((double)(a))/((double)(b)))+((double) 0.5))*(b))
#endif /* !fmod */
#endif /* !drem */

#endif /* 0 */

/* If you have no cbrt in your library, try this */
/* also included linux -- not needed any more (it breaks too) */
#if defined(sequent) || defined(__hpux)
#define cbrt(n) pow(n, 1.0/3.0)
#endif

#if defined(mips) && defined(ultrix)
/* DEC bites again - broken math.h */
extern double fmod(), drem();

/* There's a rumor that DEC's drem has a bug.  Consider using the macro above. */
#endif

/*
** Ultrix doesn't have a prototype for this, so we must.  Typical.
*/
#if defined(ultrix) || defined(sequent)
#if defined(__STDC__) || defined(__cplusplus)
#define P_(s) s
#else
#define P_(s) ()
#endif
double rint P_((double x));
#undef P_
#endif

/*
** Some ANSI compilers bitch and moan about float != double, and then
** also have different arguement promotion rules.
*/

#if 1 || defined(i860) || defined(sparc)
#define FLOAT double
#else
#define FLOAT float
#endif

#if defined(SYSV) || defined(SVR4)
#ifndef sgi
#define random lrand48
#endif
#if !defined(sun) || !defined(SVR4)
#define srandom srand48
#endif
#if defined(__hpux) || defined(__STDC__) || defined(STDC_LIBRARIES)
#define bcopy(s,d,n) memmove(d,s,n)
#else
#define bcopy(s,d,n) memcpy(d,s,n)
#endif
#define bzero(d,n) memset(d,0,n)
#define bcmp(a,b,n) memcmp(a,b,n)
#define index(s,c) strchr(s,c)
#ifndef sgi
#define rindex(s,c) strrchr(s,c)
#endif
#endif


#endif /* SYSDEP_H */
