/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef THREAD_POSIX
#include <pthread.h>
typedef pthread_t XtankThread;
#endif /* THREAD_POSIX */

#ifdef THREAD_SUNLWP
#include <lwp/lwp.h>
#include <lwp/stackdep.h>
typedef thread_t XtankThread;
#endif /* THREAD_SUNLWP */

#ifdef THREAD_SWAPCONTEXT
#include <stdlib.h>		/* needed for free() */
#include <ucontext.h>
typedef ucontext_t XtankThread;
#endif /* THREAD_SWAPCONTEXT */

#if !defined(THREAD_SUNLWP) && !defined(THREAD_SWAPCONTEXT) \
	&& !defined(THREAD_POSIX)
#define THREAD_EMPTY
typedef char XtankThread;
#endif

/* prototypes */

/* Call this once at start, returns pointer to main thread */
XtankThread *thread_setup(void);

/* Call this once for each new thread, returns pointer to the thread */
XtankThread *thread_init(char *buf, char *stack, int stacksize, void *(*func)());

/* Call this to switch to a new thread, returns pointer to previous thread */
XtankThread *thread_switch(XtankThread *newthd);

/* Call this to destroy a thread.  It does not deallocate the thread memory
   or any memory that was allocated for the thread's stack. */
XtankThread *thread_kill(XtankThread *thd);

#endif /* !_THREAD_H_ */
