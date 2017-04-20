/*-
 * Copyright (c) 1988 Terry Donahue
 * Portions Copyright (c) 1992,1993,1999,2017 Kurt Lidl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include "sysdep.h"
#include "thread.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "proto.h"

/* The current thread that is executing */
static XtankThread *curthd;

#ifdef THREAD_SUNLWP
/* Sun thread implementation using LWP package by Bill Bolosky
** (bolosky@cs.rochester.edu) and Robert Potter(potter@cs.rochester.edu),
** November 1989. */

/*
** Remember to link with -llwp
*/

XtankThread *
thread_setup(void)
{
	curthd = (thread_t *) malloc(sizeof(thread_t));
	lwp_self(curthd);
	return curthd;
}

XtankThread *
thread_init(char *buf, char *stack_buf, unsigned int bufsize, void (*(*func(void))))
{
	/* remember to give pointer to _top_ of stack */
	lwp_create((thread_t *) buf, func, MINPRIO, LWPNOLASTRITES,
		(stkalign_t *) (buf + bufsize), 0);
	return (XtankThread *) buf;
}

XtankThread *
thread_switch(XtankThread *newthd)
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

XtankThread *
thread_kill(XtankThread *thd)
{
	lwp_destroy(*thd);
	return thd;
}

#endif /* THREAD_SUNLWP */

#ifdef THREAD_SWAPCONTEXT
/*
** getcontext/setcontext thread implementation by Kurt Lidl, December 1991
*/

XtankThread *
thread_setup(void)
{
	/* allocate a context for the main thread of control */
	curthd = (ucontext_t *) malloc(sizeof(ucontext_t));

	/* initialize the context */
	if (getcontext((ucontext_t *) curthd) != 0) {
		printf("Error returned from getcontext(): %d\n", errno);
		assert(0);
	}
	return (XtankThread *) curthd;
}

/* Walt Webber (walt%okimicro@uu.psi.com) says this:
   (from a conversion on 20-Dec-1991)

   it *is* possible to do a setcontext() inside of a signal
   handler and still survive the event, in a robust fashion
   also -- the signal handler is only installed for the
   "distinquished context" ie -- the scheduling thread in our case
   -- it is responsible for all the contexts signal handling
*/

/*
 * The following code was originally derived from a piece of sample
 * code from Peter Chubb, peterc@softway.oz.au, but has been
 * substantially re-written over the years.
 */
XtankThread *
thread_init(char *buf, char *stack, int stacksize, void *(*func)(void))
{
	/* initialize the buffer with a context */
	if (getcontext((ucontext_t *) buf) != 0) {
		printf("Error returned from getcontext(): %d\n", errno);
		assert(0);
	}

	/* Modify the context to have a new stack */
	((ucontext_t *)buf)->uc_stack.ss_sp = stack;
	((ucontext_t *)buf)->uc_stack.ss_size = stacksize;
	((ucontext_t *)buf)->uc_link = curthd;  /* depends on a global */

	/* Make the modified context */
	makecontext((ucontext_t *) buf, (void (*)()) func, 0);

	return (XtankThread *) buf;
}

XtankThread *
thread_switch(XtankThread *newthd)
{
	ucontext_t *last_thread;

	if (newthd != curthd) {
		last_thread = curthd;
		curthd = newthd;
		swapcontext(last_thread, newthd);	/* switch threads */
		return last_thread;
	}
	return curthd;
}

XtankThread *
thread_kill(XtankThread *thd)
{
	/* nothing to do here for swapcontext() based threading */

	return thd;
}

#endif /* THREAD_SWAPCONTEXT */

#ifdef THREAD_POSIX
/*
** POSIX threading code -- at least a rough cut at a first pass,
** provided by bird@sevior.triumf.ca (Tony Ambardar)
**
** Hacked into submission by Kurt Lidl (lidl@pix.net)
*/

XtankThread *
thread_setup(void)
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
	status = pthread_attr_getschedparam(&attr, &params);
	if (status != 0) {
		perror("pthread_attr_getschedparam");
		exit(17);
	}
#if 0
	params.sched_priority = PTHREAD_SCHED_MAX_PRIORITY;
	pthread_attr_setschedparam(&attr, &params);
#endif
#if defined(__bsdi__)
	fprintf(stderr,"thread_setup() finishing\n");
#endif

	return curthd;
}

/* NOTE: By default, the stack for each thread is allocated internally */
/* in pthreads, so bufsize is really ignored, at least for now. */
XtankThread *
thread_init(char *buf, unsigned int bufsize, void(*(func(void))))
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
	pthread_attr_setschedpolicy(&prog_attr, SCHED_FIFO);

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

	status=pthread_create((XtankThread *) buf, &prog_attr,
		(void *) func, (void *) 0);
	if (status != 0) {
		perror("pthread_create");
		printf("Couldn't create a thread!\n");
		exit(17);
	}
	return (XtankThread *) buf;
}

XtankThread *
thread_switch(XtankThread *newthd)
{
	XtankThread *oldthd;

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

XtankThread *
thread_kill(XtankThread *thd)
{
	int status;

	status = pthread_cancel(*thd);
	if (status != 0) {
		perror("pthread_cancel");
		printf("Couldn't kill a thread\n");
		exit(17);
	}
	pthread_detach((pthread_t) thd);
	return thd;
}
#endif /* THREAD_POSIX */

#if !defined(THREAD_SUNLWP) && !defined(THREAD_SWAPCONTEXT) \
	&& !defined(THREAD_POSIX)
/*
** Empty implementation used when no threads package is available.
*/

XtankThread *
thread_setup(void)
{
	return (XtankThread *)0;
}

XtankThread *
thread_init(char *buf, unsigned int bufsize, void (*)func(void))
{
	return (XtankThread *) 0;
}

XtankThread *
thread_switch(XtankThread *newthd)
{
	return newthd;
}

XtankThread *
thread_kill(XtankThread *thd, char *prog_buf, char *stack_buf)
{
	return thd;
}

#endif
