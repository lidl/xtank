/*
** Xtank
**
** Copyright 1992 by Kurt Lidl
** Copyright 1993 by Pix Technologies Corp.
**
** threadglue.c
*/

/*
$Author: lidl $
$Id: threadglue.c,v 1.1.1.1 1995/02/01 00:25:39 lidl Exp $
*/

#include <assert.h>
#include "xtanklib.h"
#include "xtank.h"
#include "thread.h"
#include "icounter.h"
#include "proto.h"

/* Pointer to the program scheduler's thread */
/* this is a global located in program.c */
extern Thread *scheduler_thread;

/*
** Initializes the threader and signal handler at the beginning of a battle.
*/
init_threader()
{

	/* Initialize threading system */
	scheduler_thread = thread_setup();

	/* how 'bout some error checking here? */

#ifdef THREADING_DEFINED
	syntax error - if this define is used, choose another
#endif

#ifdef THREAD_MP
#define THREADING_DEFINED
#endif

#ifdef THREAD_SUNLWP
#define THREADING_DEFINED
#endif

#ifdef THREAD_SVR4
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
