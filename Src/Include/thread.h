/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
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
#if defined(apollo)
#ifndef SETJMP_H
#define SETJMP_H
#ifndef _JBLEN
#define _JBLEN 18
#endif
typedef int jmp_buf[_JBLEN];

#define setjmp(jmp) apsetjmp(jmp)
#define longjmp(jmp,ret) aplongjmp(jmp,ret)
#endif /* SETJMP_H */
#else /* We must be a vax */
typedef int jmp_buf[17];

/* We need replacement setjmp and longjmp functions for vax */
int vax_setjmp(), vax_longjmp();

#define setjmp(jmp)       vax_setjmp(jmp)
#define longjmp(jmp,ret)  vax_longjmp(jmp,ret)

#endif /* end of vax stuff */

  typedef struct _Thd {
	  jmp_buf state;			/* Current state of thread */
	  int sigstate;				/* Signal mask when at thread creation */
	  struct _Thd *oldthd;		/* Thread which executed prior to this one */
	  struct _Thd *(*func) ();	/* Main function for thread */
	  int stackoverflow;		/* Stack overflow boolean */
	  /* Stack for thread lies here */
  }
Thread;

#else /* vax and apollo shared code*/

/* This is the non-vax and non-apollo code */

#include <setjmp.h>

#if defined(hp9000s800)
#define setjmp _setjmp
#define longjmp _longjmp
#endif

#if defined(mips) || defined(mmax) || defined(MOTOROLA) || defined(__hp9000s800) || defined(__386BSD__) || defined(linux) || defined(bsdi) || defined(NeXT) || defined(__alpha)
  typedef struct _Thd {
	  jmp_buf state;			/* Current state of thread */
	  int sigstate;				/* Signal mask when at thread creation */
	  struct _Thd *oldthd;		/* Thread which executed prior to this one */
	  struct _Thd *(*func) ();	/* Main function for thread */
	  int stackoverflow;		/* Stack overflow boolean */
	  /* Stack for thread lies here */
  }
Thread;

#endif
#endif
#endif /* THREAD_MP */

#ifdef THREAD_POSIX
#include <pthread.h>
typedef pthread_t Thread;
#endif /* THREAD_POSIX */

#ifdef THREAD_SUNLWP
#include <lwp/lwp.h>
#include <lwp/stackdep.h>
typedef thread_t Thread;
#endif /* THREAD_SUNLWP */

#ifdef THREAD_SVR4
#include <ucontext.h>
typedef ucontext_t Thread;
#endif /* THREAD_SVR4 */

#if !defined(THREAD_MP) && !defined(THREAD_SUNLWP) && !defined(THREAD_SVR4) \
	&& !defined(THREAD_POSIX)
typedef char Thread;

#endif

/* Call this once at start, returns pointer to main thread */
Thread *thread_setup();

/* Call this once for each new thread, returns pointer to the thread */
Thread *thread_init( /* char *buf, unsigned int bufsize, Thread *(*func)() */ );

/* Call this to switch to a new thread, returns pointer to previous thread */
Thread *thread_switch( /* Thread *newthd  */ );

/* Call this to destroy a thread.  It does not deallocate the thread memory */
Thread *thread_kill( /* Thread *thd  */ );
