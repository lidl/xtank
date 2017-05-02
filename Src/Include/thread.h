/*-
 * Copyright (c) 1988 Terry Donahue
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

void init_threader(void);

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
