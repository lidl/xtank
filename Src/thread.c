#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** thread.c
*/

#include "thread.h"

extern char *malloc();


/* The current thread that is executing */
Thread *curthd;

#ifdef THREAD_MP
/****************************************************************************/
/* thread.c - THREAD CLUSTER (IMPLEMENTATION)				    */
/* Created:  10/31/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#include "thread.cpy"			/* DO NOT REMOVE OR ALTER THIS NOTICE AND ITS
								   PROVISIONS. ************************************************************************** */


Thread *thread_setup()
{
	static Thread mainthd[10];
	extern main();

	/* Initialize thread for the main program */
	curthd = mainthd;
	return thread_init(curthd, 10 * sizeof(Thread), main);
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
	/* Stack grows upwards, SP in state[1]. 48 is stack frame size, align to
	   doubleword boundary */
	thd->state[1] = ((unsigned) (thd + 1) + 48 + 7) & ~7;
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
			longjmp(curthd->state, 1);
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
			   (stkalign_t *) (((char *) buf) + bufsize), 0);
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
