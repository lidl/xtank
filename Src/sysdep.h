/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** sysdep.h
*/

#if defined(sun) || defined(hpux) || defined(apollo)
/* Avoid domain errors when both x and y are 0.
 */
#define ATAN2(_Y,_X) (((_X)==0 && (_Y)==0) ? 0.0 : \
		                             atan2((double)_Y,(double)_X))
#else
#define ATAN2(_Y,_X) (atan2((double)_Y,(double)_X))
#endif

#if defined(AMIGA) || defined(hpux)
/* drem() not available so replace with wrapper around fmod().
 */
static double temp_drem;

#define drem(a,b) ((temp_drem = fmod(a,b)) > (b)/2 ? temp_drem-(b) : temp_drem)
#endif

#if 0
 /* If you have no drem of fmod try this... */
#define drem(a,b) (((double)(a)) - (int) ((((double)(a))/((double)(b)))
+((double) 0.5)) * (b))
#endif

/* DEC bites again - broken math.h */

#if defined(mips) && defined(ultrix)
extern double fmod();
extern double drem();
#endif
