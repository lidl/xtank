/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** thread.c
*/

/*
$Author: lidl $
$Id: thread.c,v 2.8 1991/09/19 05:37:24 lidl Exp $

$Log: thread.c,v $
 * Revision 2.8  1991/09/19  05:37:24  lidl
 * ifdef'ed out the inclusion of the copyright file
 *
 * Revision 2.7  1991/09/15  07:02:03  lidl
 * added mips support via patches contributed
 *
 * Revision 2.6  1991/05/01  18:30:41  lidl
 * added code for Motorola's 68k based UNIX (R3V6)
 *
 * Revision 2.5  1991/04/26  22:47:29  lidl
 * added Motorola 88K fragment to make it work on the 88K running R32V2 UNIX
 *
 * Revision 2.4  1991/03/25  00:42:11  stripes
 * RS6K Patches (IBM is a rock sucker)
 *
 * Revision 2.3  1991/02/10  13:51:49  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:08  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:11  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:13:02  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:11  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "thread.h"

/* The current thread that is executing */
Thread *curthd;

#ifdef THREAD_MP
/****************************************************************************/
/* thread.c - THREAD CLUSTER (IMPLEMENTATION)				    */
/* Created:  10/31/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#if 0
#include "thread.cpy"
#endif
/****************************************************************************/
/* DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.		    */
/****************************************************************************/


Thread *thread_setup()
{
	static Thread mainthd[10];
	extern main();

	/* Initialize thread for the main program */
	curthd = mainthd;
	return thread_init((char *)curthd, 10 * sizeof(Thread), main);
}

Thread *thread_init(buf, bufsize, func)
char *buf;
unsigned int bufsize;
Thread *(*func) ();
{
	int bufend;
	Thread *thd;

	/* If buffer size is too small, return 0 */
	if (bufsize < 3 * sizeof(Thread))
		return (Thread *) 0;

	/* Initialize local variables for new state */
	thd = (Thread *) buf;
	thd->func = func;
	thd->stackoverflow = 0;

	/* Copy current signal state and setjmp to the thread */
	thd->sigstate = sigsetmask(~0);
	if (setjmp(thd->state))
	{
		sigsetmask(curthd->sigstate);
		for (;;)
			curthd->oldthd = thread_switch((*curthd->func) (curthd->oldthd));
	}
	sigsetmask(thd->sigstate);

	/* Modify current state's stack pointer in jmp_buf state */
	bufend = (unsigned int) buf + bufsize - 1 - sizeof(thd->state);

	/* Set stack pointer in jmp_buf to bufend. This is extremely machine and
	   OS dependent. Often you can find out what register of the jmp_buf has
	   the stack pointer by looking in /usr/include/setjmp.h on your machine.
	   It is a good idea to make this number 32-bit (or sometimes 64-bit)
	   aligned.  You also need to know whether your stack grows upwards
	   (increasing address values) or downwards, to determine whether the
	   stack pointer should start at the beginning or end of the empty space
	   after the thread structure. */

#if (defined(_IBMR2))
	bufend = (bufend - 7) & ~7;
	thd->state[3] = bufend;
#endif

#if defined(mips) && defined(ultrix)
	thd->state[32] = ((unsigned) (bufend)) & ~3;
#endif

#ifdef vax						/* Vaxen */
	bufend -= sizeof(Thread);
	thd->state[0] = ((unsigned) (bufend)) & ~3;
#endif

#if	(defined(ibm032) || defined(ibm370))		/* IBM RT */
	thd->state[0] = ((unsigned) (bufend)) & ~3;
#endif

#ifdef sun

#ifdef SUNOS4_0					/* Suns running 4.0 or higher */
	thd->state[2] = ((unsigned) (bufend)) & ~3;
#endif

#ifdef SUNOS3_0					/* Suns running less than 4.0 */
	thd->state[14] = ((unsigned) (bufend)) & ~3;
#endif

#endif

#ifdef sgi						/* Iris4d running 3.3 */
	thd->state[2] = bufend & ~7;
#endif

#ifdef hp9000s800
#ifdef OLDCODE
	/* Stack grows upwards, SP in state[1]. 48 is stack frame size, align to
	   doubleword boundary */
	thd->state[1] = ((unsigned) (thd + 1) + 48 + 7) & ~7;
#else
	/* Stack grows upwards, SP in state[1]. Align to doubleword boundary */
	/* This is rather ad-hoc... but seems to work. */
	thd->state[1] = ((unsigned) (thd + 2) + 100) & ~7;
#endif
#endif

#ifdef hp9000s300
	thd->state[12] = ((unsigned) bufend) & ~3;
#endif

#ifdef apollo
	thd->state[13] = bufend - (thd->state[14] - thd->state[13]);
	thd->state[14] = bufend;
#endif

#ifdef NeXT
	/* Stack grows downwards, SP in state[2]. align to 32-bit boundary */
	thd->state[2] = ((unsigned) (bufend)) & ~3;
#endif

#ifdef hp300
	/* Stack grows downwards, SP in state[2]. align to 32-bit boundary */
	thd->state[2] = ((unsigned) (bufend)) & ~3;
#endif

#if defined(sequent) && defined(i386)
	/* Sequent Symmetry. Stack grows downwards, SP in state[4]. align to
	   32-bit boundary */
	thd->state[4] = ((unsigned (bufend)) &~3;
#endif

#if defined(MOTOROLA) && defined(m88k)
	/* Motorola 88k system running their R32V2 UNIX */
	/* The stack grows downwards.
	   R31 is used as the stack pointer, it is the 2nd word
	   of the jmp_buf (state). Doubleword aligned. PEK'90 */
	thd->state[1] = ((unsigned) (bufend)) & ~7;
#endif

#if defined(MOTOROLA) && defined(m68k)
	/* Motorola's 68k based UNIX (R3V6) */
	/* The stack grows downwards.
	   The stack pointer is in the 13th word of the jmp_buf.
	   Aligned to longword boundary. PEK '90 */
	thd->state[12] = ((unsigned) (bufend)) & ~3;
#endif

	return thd;
}

Thread *thread_switch(newthd)
Thread *newthd;
{
	/* Check for stack overflow */
	if (newthd->stackoverflow)
		return 0;

	/* If not switching to current thread, longjmp to new thread */
	if (newthd != curthd)
	{
		int sigstate = sigsetmask(~0);

		if (!setjmp(curthd->state))
		{
			newthd->oldthd = curthd;
			curthd = newthd;
#if defined(_IBMR2)
			mylongjmp(curthd->state, 1);
#else
			longjmp(curthd->state, 1);
#endif
		}
		newthd = curthd->oldthd;
		sigsetmask(sigstate);
	}
	/* Return the previous current thread */
	return newthd;
}

Thread *thread_kill(thd)
Thread *thd;
{
	return thd;
}

#endif

#ifdef THREAD_SUNLWP
/* Sun thread implementation using LWP package by Bill Bolosky
** (bolosky@cs.rochester.edu) and Robert Potter(potter@cs.rochester.edu),
** November 1989. */
/**
** Remember to link with -llwp
*/

Thread *thread_setup()
{
	curthd = (thread_t *) malloc(sizeof(thread_t));
	lwp_self(curthd);
	return curthd;
}

Thread *thread_init(buf, bufsize, func)
char *buf;
unsigned int bufsize;
Thread *(*func)();
{
	/* remember to give pointer to _top_ of stack */
	lwp_create((thread_t *) buf, func, MINPRIO, LWPNOLASTRITES,
			   (stkalign_t *) (buf + bufsize), 0);
	return (Thread *) buf;
}

Thread *thread_switch(newthd)
Thread *newthd;
{
	thread_t *last_thread;

	if (!SAMETHREAD(*newthd, *curthd))
	{
		last_thread = curthd;
		curthd = newthd;
		lwp_yield(*newthd);
		return last_thread;
	}
	return curthd;
}

Thread *thread_kill(thd)
Thread *thd;
{
	lwp_destroy(*thd);
	return thd;
}

#endif

#if !defined(THREAD_MP) && !defined(THREAD_SUNLWP)
/* Empty implementation used when no threads package is available.
*/

Thread *thread_setup()
{
	return (Thread *) 0;
}

Thread *thread_init(buf, bufsize, func)
char *buf;
unsigned int bufsize;
int (*func) ();

{
	return (Thread *) 0;
}

Thread *thread_switch(newthd)
Thread *newthd;
{
	return newthd;
}

Thread *thread_kill(thd)
Thread *thd;
{
	return thd;
}

#endif
