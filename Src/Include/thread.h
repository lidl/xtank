/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** thread.h
*/

/*
$Author: lidl $
$Id: thread.h,v 2.15 1992/09/07 18:50:50 lidl Exp $

$Log: thread.h,v $
 * Revision 2.15  1992/09/07  18:50:50  lidl
 * started support for 386bsd systems
 *
 * Revision 2.14  1992/08/19  05:00:16  lidl
 * fixed for HPs?? (I hope)
 *
 * Revision 2.13  1992/04/18  15:37:42  lidl
 * defining the thread structure for i860's also
 *
 * Revision 2.12  1992/04/09  05:10:58  lidl
 * re-structured to allow for easier modification.  Lets hope I didn't
 * screw things too badly
 *
 * Revision 2.11  1992/01/29  08:41:54  lidl
 * changed comments only
 *
 * Revision 2.10  1992/01/26  05:46:28  lidl
 * botched attempt at i860 style longjmp hack
 *
 * Revision 2.9  1991/12/16  02:56:27  lidl
 * added support for SVR4 style threading
 *
 * Revision 2.8  1991/10/07  03:16:12  lidl
 * added multimax support (sys 5 universe)
 *
 * Revision 2.7  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.6  1991/09/15  07:02:52  lidl
 * added mips support
 *
 * Revision 2.5  1991/05/01  20:12:02  lidl
 * added Motorola patches for their SYS5 based Unix
 *
 * Revision 2.4  1991/03/25  00:42:11  stripes
 * RS6K Patches (IBM is a rock sucker)
 *
 * Revision 2.3  1991/02/10  13:51:50  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:10  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:13  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:39  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:12  aahz
 * Initial revision
 * 
*/

#ifdef THREAD_MP

/* The original idea for the so-called "THREAD_MP" style of supporting */
/* was developed by Michael Benjamin Parker.  For conditions of use, */
/* please refer to the Doc/thread.cpy file. */

/****************************************************************************/
/* thread.h - THREAD CLUSTER (INTERFACE)				    */
/* Created:  10/31/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************/
/* (c) Copyright 1987 by Michael Benjamin Parker      (USA SS# 557-49-4130) */
/* All Rights Reserved unless specified in the thread.cpy file              */
/* DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.                   */
/****************************************************************************/

/* This threads package is known to run correctly on the following hardware:
 *      IBM RT
 *      DEC VAX
 *      Sun 3   (does not always work.  If not, use THREAD_SUNLWP below)
 *      HP9000 Series 800
 *      Apollo
 *	Motorola 88k and 68k boxes (hopefully)
 *	Encore Multimax (Sys V Universe)
 */

#include <signal.h>

#if defined(vax) || defined(apollo)
# if defined(apollo)
#  ifndef SETJMP_H
#    define SETJMP_H
#    ifndef _JBLEN
#      define _JBLEN 18
#    endif
     typedef int jmp_buf[_JBLEN];
#    define setjmp(jmp) apsetjmp(jmp)
#    define longjmp(jmp,ret) aplongjmp(jmp,ret)
#  endif /* SETJMP_H */
# else /* We must be a vax */
typedef int jmp_buf[17];

/* We need replacement setjmp and longjmp functions for vax */
   int   vax_setjmp(), vax_longjmp();
#  define setjmp(jmp)       vax_setjmp(jmp)
#  define longjmp(jmp,ret)  vax_longjmp(jmp,ret)

# endif /* end of vax stuff */

typedef struct _Thd
{
    jmp_buf state;	/* Current state of thread */
    int   sigstate;	/* Signal mask when at thread creation */
    struct _Thd *oldthd;	/* Thread which executed prior to this one */
    struct _Thd *(*func) ();	/* Main function for thread */
    int   stackoverflow;	/* Stack overflow boolean */
    /* Stack for thread lies here */
}     Thread;
#else /* vax and apollo shared code*/

/* This is the non-vax and non-apollo code */

# include <setjmp.h>

# if defined(hp9000s800)
#  define setjmp _setjmp
#  define longjmp _longjmp
# endif

# if defined(mips) || defined(mmax) || defined(MOTOROLA) || defined(__hp9000s800) || defined(__386BSD__)
typedef struct _Thd
{
	jmp_buf	state;		/* Current state of thread */
	int	sigstate;	/* Signal mask when at thread creation */
	struct	_Thd *oldthd;	/* Thread which executed prior to this one */
	struct	_Thd *(*func) ();	/* Main function for thread */
	int	stackoverflow;	/* Stack overflow boolean */
	/* Stack for thread lies here */
}	Thread;
# endif
#endif 
#endif /* THREAD_MP */

#ifdef THREAD_SUNLWP
#include <lwp/lwp.h>
#include <lwp/stackdep.h>
typedef thread_t Thread;
#endif /* THREAD_SUNLWP */

#ifdef THREAD_SVR4
#include <ucontext.h>
typedef ucontext_t Thread;
#endif /* THREAD_SVR4 */

#if !defined(THREAD_MP) && !defined(THREAD_SUNLWP) && !defined(THREAD_SVR4)
typedef char Thread;
#endif

/* Call this once at start, returns pointer to main thread */
Thread *thread_setup();

/* Call this once for each new thread, returns pointer to the thread */
Thread *thread_init(/* char *buf, unsigned int bufsize, Thread *(*func)() */);

/* Call this to switch to a new thread, returns pointer to previous thread */
Thread *thread_switch( /* Thread *newthd  */ );

/* Call this to destroy a thread.  It does not deallocate the thread memory */
Thread *thread_kill( /* Thread *thd  */ );
