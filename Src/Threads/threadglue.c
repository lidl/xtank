/*-
 * Copyright (c) 1992,1993,1999 Kurt Lidl
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

#include <assert.h>
#include "xtanklib.h"
#include "xtank.h"
#include "thread.h"
#include "icounter.h"
#include "proto.h"

/* Pointer to the program scheduler's thread */
/* this is a global located in program.c */
extern XtankThread *scheduler_thread;

/*
** Initializes the threader and signal handler at the beginning of a battle.
*/
void
init_threader(void)
{

	/* Initialize threading system */
	scheduler_thread = thread_setup();

	/* how 'bout some error checking here? */

#ifdef THREADING_DEFINED
	syntax error - if this define is used, choose another
#endif

#ifdef THREAD_SUNLWP
#define THREADING_DEFINED
#endif

#ifdef THREAD_SWAPCONTEXT
#define THREADING_DEFINED
#endif

#ifdef THREAD_POSIX
#define THREADING_DEFINED
#endif

#ifdef THREADING_DEFINED
	if (!scheduler_thread) {
		  rorre("malloc failed in thread_setup");
	}

#endif
	/* Initialize the itimer signal handler */
	setup_counter();
}
