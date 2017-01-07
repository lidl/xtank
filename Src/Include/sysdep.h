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
