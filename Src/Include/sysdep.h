/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** sysdep.h
*/

/*
$Author: lidl $
$Id: sysdep.h,v 2.7 1991/09/15 07:01:09 lidl Exp $

$Log: sysdep.h,v $
 * Revision 2.7  1991/09/15  07:01:09  lidl
 * added i860 to list of machines that complain about domain errors with
 * atan2.
 *
 * Revision 2.6  1991/08/23  16:25:53  aahz
 * checked in so others can play with it (KJL)
 *
 * Revision 2.5  1991/05/01  18:28:01  lidl
 * added a couple of minor define() checkings for Motorola Sys5 unix
 *
 * Revision 2.4  1991/03/06  06:58:35  stripes
 * Added mips to the list of systems that need a paranoid ATAN2
 *
 * Revision 2.3  1991/02/10  13:51:45  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:04  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:06  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:36  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:09  aahz
 * Initial revision
 * 
*/

#ifndef SYSDEP_H
#define SYSDEP_H


#if defined(sun) || defined(hpux) || defined(apollo) || defined(mips) || defined(MOTOROLA) || defined(i860)
/* Avoid domain errors when both x and y are 0 */
#define ATAN2(Y,X) ((X)==0 && (Y)==0 ? 0.0 : atan2((double)Y, (double)X))
#else
#define ATAN2(Y,X) atan2((double)Y, (double)X)
#endif


#if defined(AMIGA) || defined(hpux) || defined(MOTOROLA) || defined(i860)
/* drem() not available so replace with wrapper around fmod() */
static double temp_drem;
#define drem(a,b) ((temp_drem = fmod(a,b)) > (b)/2 ? temp_drem-(b) : temp_drem)
#endif


#if 0
 /* If you have no drem or fmod try this... */
#define drem(a,b) (((double)(a))-(int)((((double)(a))/((double)(b)))+((double) 0.5))*(b))
#endif


#if defined(mips) && defined(ultrix)
/* DEC bites again - broken math.h */
extern double fmod(), drem();
/* rumor that DEC's drem has a bug.  Consider using the macro above. */
#endif


#ifdef SYSV
#define random lrand48
#define srandom srand48
#if defined(__hpux) || defined(__STDC__) || defined(STDC_LIBRARIES)
#define bcopy(s,d,n) memmove(d,s,n)
#else
#define bcopy(s,d,n) memcpy(d,s,n)
#endif
#define bzero(d,n) memset(d,0,n)
#define bcmp(a,b,n) memcmp(a,b,n)
#define index(s,c) strchr(s,c)
#define rindex(s,c) strrchr(s,c)
#endif


#endif /* def SYSDEP_H */
