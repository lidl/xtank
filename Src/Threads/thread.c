/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "sysdep.h"
#include "thread.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "proto.h"

#ifdef SVR4
#include <stdio.h>
#endif

/* The current thread that is executing */
static Thread *curthd;

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
	return thread_init((char *) curthd, 10 * sizeof(Thread), main);
}

Thread *thread_init(char *buf, unsigned int bufsize, Thread (*(func(void))))
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
#ifdef SVR4
	sigprocmask(NULL, NULL, thd->sigstate);
#else
	thd->sigstate = sigsetmask(~0);
#endif
	if (setjmp(thd->state)) {
#ifdef SVR4
		sigprocmask(SIG_SETMASK, curthd->sigstate, NULL);
#else
		sigsetmask(curthd->sigstate);
#endif
		for (;;)
			curthd->oldthd = thread_switch((*curthd->func) (curthd->oldthd));
	}
#ifdef SVR4
	sigprocmask(SIG_SETMASK, thd->sigstate, NULL);
#else
	sigsetmask(thd->sigstate);
#endif

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

#if	(defined(ibm032) || defined(ibm370))	/* IBM RT */
	thd->state[0] = ((unsigned) (bufend)) & ~3;
#endif

#ifdef sun
#ifdef SUNOS4_0					/* Suns running 4.0 or higher */
	thd->state[2] = ((unsigned) (bufend)) & ~3;
#endif /* SUNOS4_0 */

#ifdef SUNOS3_0					/* Suns running less than 4.0 */
	thd->state[14] = ((unsigned) (bufend)) & ~3;
#endif /* SUNOS3_0 */
#endif

#ifdef mmax
	/* Multimax stack grows downwards, offset is 7, align to 32 bit
	   boundary. */
	thd->state[7] = ((unsigned) (bufend)) & ~3;
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
	{
		unsigned *foo = (unsigned *) thd->state;

		foo[1] = ((unsigned) (thd + 2) + 100) & ~7;
	}
#endif
#endif

#if defined(hp9000s300) && defined(__hpux)
	thd->state[12] = ((unsigned) bufend) & ~3;
#endif
#if defined(hp9000) && defined(hp300)
	/* HP Bobcat running 4.3BSD */
	/* Stack grows downwards, SP in state[2]. align to 32-bit boundary */
	thd->state[2] = ((unsigned) (bufend)) & ~3;
#endif

#ifdef apollo
	thd->state[13] = bufend - (thd->state[14] - thd->state[13]);
	thd->state[14] = bufend;
#endif

#ifdef NeXT
	/* Stack grows downwards, SP in state[2]. align to 32-bit boundary */
	thd->state[2] = ((unsigned) (bufend)) & ~3;
#endif

#if defined(__386BSD__) || defined (__bsdi__)
	/* Stack grows downwards, SP in state[2], FP in state[3]. */
	thd->state[2] = bufend & ~3;
	thd->state[3] = bufend & ~3;
#endif

#ifdef linux
	/* Stack grows downwards.  Align to 32-bit boundary */
	/*	thd->state->__jmpbuf->__sp = bufend & ~3; */
	thd->state->__jmpbuf[JB_SP] = bufend & ~3;
#endif

#if defined(__alpha) && defined(__osf__)
	thd->state[34] = ((unsigned) (bufend)) & ~7;
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
#ifdef SVR4
	unsigned long sigstate[4];
#endif

	/* Check for stack overflow */
	if (newthd->stackoverflow) {
		return 0;
	}

	/* If not switching to current thread, longjmp to new thread */
	if (newthd != curthd) {
#ifdef SVR4
		sigprocmask(NULL, NULL, sigstate);
#else
		int sigstate = sigsetmask(~0);
#endif
		if (!setjmp(curthd->state)) {
			newthd->oldthd = curthd;
			curthd = newthd;
#if defined(_IBMR2)
			mylongjmp(curthd->state, 1);
#else
			longjmp(curthd->state, 1);
#endif
		}
		newthd = curthd->oldthd;
#ifdef SVR4
		sigprocmask(NULL, sigstate, NULL);
#else
		sigsetmask(sigstate);
#endif
	}
	/* Return the previous current thread */
	return newthd;
}

Thread *thread_kill(thd)
Thread *thd;
{
	return thd;
}

#endif /* THREAD_MP */

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

Thread *thread_init(char *buf, unsigned int bufsize, Thread (*(*func(void))))
{
	/* remember to give pointer to _top_ of stack */
	lwp_create((thread_t *) buf, func, MINPRIO, LWPNOLASTRITES,
		(stkalign_t *) (buf + bufsize), 0);
	return (Thread *) buf;
}

Thread *thread_switch(Thread *newthd)
{
	volatile thread_t * last_thread;

	if (!SAMETHREAD(*newthd, *curthd)) {
		last_thread = curthd;
		curthd = newthd;
		lwp_yield(*newthd);
		return last_thread;
	}
	return curthd;
}

Thread *thread_kill(Thread *thd)
{
	lwp_destroy(*thd);
	return thd;
}

#endif

#ifdef THREAD_SVR4
/*
** System V Release 4 thread implementation by Kurt Lidl, December 1991
*/

Thread *thread_setup()
{
	curthd = (ucontext_t *) malloc(sizeof(ucontext_t));
	if (getcontext((ucontext_t *) curthd) != NULL) {
		printf("Error returned from getcontext(): %d\n", errno);
		assert(0);
	}
	return (Thread *) curthd;
}

/* Walt Webber (walt%okimicro@uu.psi.com) says this:
   (from a conversion on 20-Dec-1991)

   it *is* possible to do a setcontext() inside of a signal
   handler and still survive the event, in a robust fashion
   also -- the signal handler is only installed for the
   "distinquished context" ie -- the scheduling thread in our case
   -- it is responsible for all the contexts signal handling
*/

/* The following code is derived from a piece of */
/* sample code from Peter Chubb, peterc@softway.oz.au */
Thread *thread_init(char *buf, unsigned int bufsize, Thread (*(*func(void))))
{
	stack_t st;

	/* malloc a stack space for the context */
	if ((st.ss_sp = (char *) malloc(bufsize)) == (char *) NULL) {
		return (ucontext_t *) NULL;
	}
	st.ss_size = bufsize;
	st.ss_flags = 0;

	if (getcontext((ucontext_t *) buf) != NULL) {
		printf("Error returned from getcontext(): %d\n", errno);
		assert(0);
	}
	/* Modify the context to have a new stack */
	STRUCT_ASSIGN((((ucontext_t *)buf)->uc_stack), st, stack_t);
	((ucontext_t *)buf)->uc_link = curthd;  /* depends on a global */
	/* Make the modified context */
	makecontext((ucontext_t *) buf, (void (*)()) func, 0);
	return (Thread *) buf;
}

Thread *thread_switch(Thread *newthd)
{
	ucontext_t * last_thread;

	if (newthd != curthd) {
		last_thread = curthd;
		curthd = newthd;
		swapcontext(last_thread, newthd);	/* switch threads */
		return last_thread;
	}
	return curthd;
}

Thread *thread_kill(Thread *thd)
{
	free(thd->uc_stack);	/* free the stack space buffer */
	free(thd);	/* free the context save buffer */
	return thd;
}

#endif /* THREAD_SVR4 */

#ifdef THREAD_POSIX

/*
** POSIX threading code -- at least a rough cut at a first pass,
** provided by bird@sevior.triumf.ca (Tony Ambardar)
**
** Hacked into submission by Kurt Lidl (lidl@pix.net)
*/

Thread *thread_setup()
{
	int status;
	int policy;
	pthread_attr_t attr;
	struct sched_param params;

#if defined(__bsdi__)
	fprintf(stderr,"thread_setup() starting\n");
#endif

	curthd = (pthread_t *) malloc(sizeof(pthread_t));
	*curthd = pthread_self();

	status=pthread_attr_getschedpolicy(&attr, &policy);
	if (status != 0) {
		perror("pthread_attr_getschedpolicy");
		exit(17);
	}
	status=pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (status != 0) {
		perror("pthread_attr_setschedpolicy");
		exit(17);
	}
#if 0
	params.sched_priority = PTHREAD_SCHED_MAX_PRIORITY;
#endif
	status = pthread_attr_getschedparam(&attr, &params);
	if (status != 0) {
		perror("pthread_attr_getschedparam");
		exit(17);
	}
#if defined(__bsdi__)
	fprintf(stderr,"thread_setup() finishing\n");
#endif

	return curthd;
}

Thread *thread_init(char *buf, unsigned int bufsize, int(*(func(void))))
/* NOTE: By default, the stack for each thread is allocated internally */
/* in pthreads, so bufsize is really ignored, at least for now. */
{
	pthread_attr_t prog_attr;
	struct sched_param params;
	int status;

	status = pthread_attr_init(&prog_attr);
	if (status != 0) {
		perror("pthread_attr_init");
		exit(17);
	}
	status = pthread_attr_getschedparam(&prog_attr, &params);
	if (status != 0) {
		perror("pthread_attr_getschedparam");
		exit(17);
	}

	/* first, setup everything for round-robin scheduling */
	pthread_attr_setschedpolicy(&prog_attr,SCHED_FIFO);

	/* then, give the about-to be created thread a really low priority */
	params.sched_priority = PTHREAD_SCHED_MIN_PRIORITY;
	pthread_attr_setschedparam(&prog_attr, &params);

#if 0
	pthread_attr_setstacksize(&prog_attr, (long) STACK_SIZE);
#endif
	/* finally, tell the system to use what we just cobbled up */
	pthread_attr_setinheritsched(&prog_attr,PTHREAD_EXPLICIT_SCHED);

#if defined(__bsdi__)
	/* non-portable, but might be worth it... */
	status = pthread_attr_setsuspendstate_np(&prog_attr,PTHREAD_CREATE_SUSPENDED);
	if (status != 0) {
		perror("pthread_attr_setsuspendstate_np PTHREAD_CREATE_SUSPENDED");
		exit(17);
	}
#endif

	status=pthread_create((Thread *) buf, &prog_attr,
	(void *) func, (void *) 0);
	if (status != 0) {
		perror("pthread_create");
		printf("Couldn't create a thread!\n");
		exit(17);
	}
	return (Thread *) buf;
}

Thread *thread_switch(Thread *newthd)
{
	Thread *oldthd;

	pthread_testcancel();
	if (pthread_equal((pthread_t)curthd, (pthread_t) newthd) == 0) {
		int status;
		pthread_attr_t attr;
		struct sched_param params;

		fprintf(stderr,"thread_switch() -> switching\n");
		oldthd = curthd;
		curthd = newthd;
#if 0
		/* now, lower the old thread's priority, so it runs */
		status = pthread_attr_getschedparam(&attr, &params);
		if (status != 0) {
			perror("pthread_attr_getschedparam");
		}
		params.sched_priority = PTHREAD_SCHED_MIN_PRIORITY;
		pthread_attr_setschedparam(&attr, &params);
#endif

		/* give over control */
#if defined(__bsdi__)
		status = pthread_resume_np(*newthd);
		if (status) {
			perror("pthread_resume_np");
		}
		fprintf(stderr,"pthread_resume_np() called (%x)\n", *newthd);
		pthread_yield();
		sleep(1);
		fprintf(stderr,"about to pthread_suspend_np(%x)\n", *oldthd);
		status = pthread_suspend_np(*oldthd);
		if (status) {
			perror("pthread_suspend_np");
		}
#else
		fprintf(stderr,"calling pthread_yield()\n");
		pthread_yield();
		pthread_testcancel();
#endif

#if 0
		/* now, raise the new thread's priority, so it runs */
		status = pthread_attr_getschedparam(&attr, &params);
		if (status != 0) {
			perror("pthread_attr_getschedparam");
		}
		params.sched_priority = PTHREAD_SCHED_MAX_PRIORITY;
		pthread_attr_setschedparam(&attr, &params);
#endif
		return oldthd;
	}
	return curthd;
}

Thread *thread_kill(Thread *thd)
{
	int status;

	status = pthread_cancel(*thd);
	if (status != 0) {
		perror("pthread_cancel");
		printf("Couldn't kill a thread\n");
		exit(17);
	}
	pthread_detach((pthread_) thd);
	return thd;
}

#endif /* THREAD_POSIX */

#if !defined(THREAD_MP) && !defined(THREAD_SUNLWP) && !defined(THREAD_SVR4) \
	&& !defined(THREAD_POSIX)
/*
** Empty implementation used when no threads package is available.
*/

Thread *thread_setup(void)
{
	return (Thread *)0;
}

Thread *thread_init(char *buf, unsigned int bufsize, int (*)func(void))
{
	return (Thread *) 0;
}

Thread *thread_switch(Thread *newthd)
{
	return newthd;
}

Thread *thread_kill(Thread *thd)
{
	return thd;
}

#endif
