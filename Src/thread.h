/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** thread.h
*/

#include "config.h"

#ifdef THREAD_MP
/****************************************************************************/
/* thread.h - THREAD CLUSTER (INTERFACE)				    */
/* Created:  10/31/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files:
#include "thread.cpy"

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/

/* This threads package is known to run correctly on the following hardware:
 *      IBM RT
 *      DEC VAX
 *      Sun 3   (doesn't always work.  If not, use THREAD_SUNLWP below)
 *      HP9000 Series 800
 *      Apollo
 */

#include <signal.h>

#if !defined(vax) && !defined(apollo)
#include <setjmp.h>

#ifdef hp9000s800
#define setjmp _setjmp
#define longjmp _longjmp
#endif
#else /* !defined(vax) && !defined(apollo) */
#ifdef apollo
#  ifndef SETJMP_H
#    define SETJMP_H
#    ifndef _JBLEN
#      define _JBLEN 18
#    endif
     typedef int jmp_buf[_JBLEN];
#    define setjmp(jmp) apsetjmp(jmp)
#    define longjmp(jmp,ret) aplongjmp(jmp,ret)
#  endif /* SETJMP_H */
#else /* apollo */
/* Replacement setjmp and longjmp functions for vax */
typedef int jmp_buf[17];
int   vax_setjmp(), vax_longjmp();

#define setjmp(jmp)       vax_setjmp(jmp)
#define longjmp(jmp,ret)  vax_longjmp(jmp,ret)
#endif

typedef struct _Thd
{
    jmp_buf state;	/* Current state of thread */
    int   sigstate;	/* Signal mask when at thread creation */
    struct _Thd *oldthd;	/* Thread which executed prior to this one */
    struct _Thd *(*func) ();	/* Main function for thread */
    int   stackoverflow;	/* Stack overflow boolean */
    /* Stack for thread lies here */
}     Thread;

#endif /* vax */

#endif /* THREAD_MP */

#ifdef THREAD_SUNLWP
#include <lwp/lwp.h>
#include <lwp/stackdep.h>
typedef thread_t Thread;
#endif

#if !defined(THREAD_MP) && !defined(THREAD_SUNLWP)
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
