/****************************************************************************/
/* mpthd.h  -- THREAD CLUSTER (INTERFACE)				    */
/* Created:  10/31/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#include "mptsk.cpy" /*

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/
/* OVERVIEW:

The thread cluster implements basic context switching in the
``C'' environment for single or multiprocessor machines.  It provides
primitives useful for implementing multitasking.

The cluster allows its user to create independent threads of execution
(contexts), each with its own processor state, stack frame (local
variables), and heap.  All threads share the same address space, however,
and so can share global variables and the global heap for simple
communication.

To help manage the somewhat complex notion of parallel processing, a
thread is treated as an ordinary data object.  Though a thread is
executed, it is created and destroyed, examined and modified, just like
any other data object.

The number of bizzar flow control constructs has also been kept to a
minimum.  Namely, the thread cluster has a single switch function which
uquickly switches between contexts, and operates quite like an ordinary
function call.

As well as being fast, the implementation is highly portable.  Its design
is independent of the operating system, number of processors, memory
management hardware and software, and C library routines (except setjmp and
longjmp).  And the powerful interface can be used to quickly build
a full multiprocessing/multitasking enviornment in ``C'' that runs on
many machines.

SUPPORTS:
	MS-DOS, in Turbo C 1.0+
	UNIX BSD 4.3 on	IBM RT
TO BE SUPPORTED:
	MS-DOS in Microsoft C 5.0+
	UNIX BSD 4.3 on	VAX (file mpvaxsj.s STILL HAS BUGS IN IT)
	UNIX BSD 4.3 on SUN

NOTE:	for multiprocessor implementations, the design assumes a shared
address space between processors.

WARNING: Must be compiled with all stack-overflow checking features
turned off!
*****************************************************************************/
/****************************************************************************/
#ifndef	MPTHD_H
#define MPTHD_H
#include "mpmisc.h"
/****************************************************************************/
#if (!((defined(unix) && (defined(vax) || defined(ibm032)||defined(ibm370))) \
	|| defined(__TURBOC__) || defined(M_I86)))
	Error!  Currently, this code only runs:
		in UNIX C (on the IBM RT or VAX)
		in MS-DOS (in  Turbo C or Microsoft C)
#endif
/****************************************************************************/
/****************************************************************************/
/* DATA INTERFACE:							    */
typedef	UINT	MPDEFID;
typedef	struct _MPTHD	MPTHD, PTR MPTHDID;	/* THREAD (CONTEXT) STRUCT  */
#ifdef	ANSI_C
typedef	MPTHDID	(*MPTHDFN)(MPTHDID oldmpthd);	/* THREAD FUNCTION TYPE	    */
#else
typedef	MPTHDID	(*MPTHDFN)();
#endif
/****************************************************************************/
/****************************************************************************/
/* CODE INTERFACE:							    */
/****************************************************************************/
MPTHDID	mpthd_switch	ARGS((MPTHDID newmpthd)); /*

Switches between thread contexts.  Specifically, switches from the
caller's (current) context to the newmpthd context.  Returns the context
(oldmpthd) which restored the calling thread's control.  An indivisable
(atomic) operation.	*/
/****************************************************************************/
/*MPTHDID	mpthd_me(void)		MPTHDID

Returns the unique id of the calling thread. */
/****************************************************************************/
MPTHDID	mpthd_init	ARGS((VOIDPTR buff, UINT size, MPTHDFN mpthdfn)); /*

Initializes a new thread structure in buff of size bytes, and returns the
id of the new thread structure.  This structure stores the new thread of
execution's context, including its processor state, stack, and possibly
heap.

When the new thread is switched to, it will call tskfn with the id of the
previous thread which first gave it control.  When the mpthdfn returns,
the new thread will switch to the thread id mpthdfn returns.  When the
switch returns, the new thread will call mpthdfn with the thread id
switch returns...  And and this way the new thread loops infinitely,
alternately calling mpthdfn and switch with each other's return values:

	for (EVER) {
		oldmpthd=	mpthd_switch(mpthdfn(oldmpthd));
	}

On the first call of mpthdfn, oldmpthd is the id of the first thread to
switch to the new thread.  Also, on the first call of mpthdfn, the signal
state (signals blocked/unblocked) is the same as the signal state when
the new thread was initialized.

WARNING: To prevent unrealistically small stack/heap sizes, the size
of buff should be at least 3 times the sizeof of the MPTHD structure. */
/****************************************************************************/
VOIDPTR		mpthd_dinit	ARGS((MPTHDID mpthd)); /*

Deinitializes the mpthd structure, returning a pointer to the buffer original
buffer space so that it can be reclaimed.

WARNING:  You cannot denitialize a currently executing thread.  Thus, iff you
try to deinitalize yourself (mpthd==mpthd_me()), nil will be returned.  And,
iff you try to deinitalize a thread executing on any other processor, the
operation will wait until the processor has switched to another thread. */
/****************************************************************************/
/* STRTYPE PTR	mpthd_heap(MPTHDID mpthd,STRTYPE);	*/
/* STRTYPE PTR	mpthd_myheap(STRTYPE);			*/

/* Returns STRTYPE pointer to a thread's local heap space.

The heap is just a buffer opposite the stack in the heap/stack field of the
thread structure.  The heap and stack share the leftover space in the
thread structure after the processor state has been stored.

Although no heap management functions are provided, a thread could
potentially use its heap to store its own set of dynamically allocated
variables.  More importantly, the heap can be used to pass arguments
between threads (global variables may also be used for this purpose).
One thread could store and retrieve variables into another thread's heap
(hopefully when it's not running), say, to pass arguments to a newly
initialized thread, and read arguments from an executed thread before
deinitalizing it.

WARNING:  Since there are no heap managment functions, there is no check
for heap/stack collision, only stack/processor state collision (and, by that
point, the heap would have been destroyed).

NOTE:	mpthd_myheap() is essentially mpthd_heap(mpthd_me())		*/
/****************************************************************************/
/****************************************************************************/
void	mpthd_setup	ARGS((void));
/*
###
MPTHDID	mpthd_setup	ARGS((VOIDPTR buf, UINT size));
Sets up the calling processor to use the thread cluster.

Your program's *main* function should look something like this:

void	main() {
	...
	MPTHD	mpthd_main[3];
	...
	mpthd_setup(mpthd_main,3*sizeof(MPTHD));
	...
	...
}

Returns the initialized thread, or NIL iff the initialization failed
(see mpthd_init()).

WARNING:   mpthd_setup should be called before any other mpthd function!    */
/****************************************************************************/
void	_mpthd_setup	ARGS((MPTHDID mpthd));
/*
Used by internally mpthd_setup, but provided here for higher-level
multitasking setup functions.  Makes an initialized thread the current
running thread.  */
/****************************************************************************/
/****************************************************************************/
/* STRUCTURE, EXTERNAL-VARIBLE, AND MACRO DEFINITION:			    */
/****************************************************************************/
/****************************************************************************/
#ifdef	vax
#include "mpvaxsj.h"
#define	setjmp(jmp)		mpvaxsj_setjmp(jmp)
#define	longjmp(jmp,ret)	mpvaxsj_longjmp(jmp,ret)
#define	jmp_buf			MPVAXSJ
#else
#include <setjmp.h>
#endif
#include "mpsem.h"
#include "mpsig.h"

struct _MPTHD {
	jmp_buf	state;		/* THE CURRENT STATE OF THE THREAD */
	MPSIGSTATE mpsigstate;	/* THE SIGNAL STATE WHEN THREAD CREATED */
	MPSEM	mpsem;		/* SEMAPHORE TO ALLOW ONLY ONE PROCESSOR */
	MPTHDID	oldmpthd;       /* THE THREAD EXECUTING PRIOR TO THIS ONE */
	MPTHDFN	mpthdfn;	/* THE FUNCTION THIS THREAD EXECUTES */
	BOOL	stackoverflow;	/* STACK OVERFLOW DETECTOR VARIABLE */
	MPDEFID mpdefsize; 	/* SIZE OF MPDEF'AULTS (IN CHARS) */
	char	mpdefs[1]; 	/* THE MPDEF'AULT STORAGE */
/*		|
		V

		^
		|
		stack		   THREAD'S STACK (ASSUME GROWS DOWN) */
};
/****************************************************************************/
/****************************************************************************/
extern	MPTHDID	mpthd_me_reg;
/****************************************************************************/
/****************************************************************************/
#define	mpthd_me()	(mpthd_me_reg)

#define	mpdef_size(mpthd)		((mpthd)->mpdefsize)
#define	mpdef_mysize()			mpdef_size(mpthd_me())
#define	mpdef_add(mpthd,TYPE)		\
	mpdef_size(mpthd);		\
	mpdef_size(mpthd)+=	(sizeof(TYPE))
#define	mpdef_myadd(TYPE)		mpdef_add(mpthd_me(),TYPE)
#define	mpdef_myval(mpdef,TYPE)		mpdef_val(mpthd_me(),mpdef,TYPE)
#define	mpdef_val(mpthd,mpdef,TYPE)	(*mpdef_idval(mpthd,mpdef,TYPE *))
#define	mpdef_myidval(mpdef,TYPEID)	mpdef_idval(mpthd_me(),mpdef,TYPEID)
#define	mpdef_idval(mpthd,mpdef,TYPEID)	((TYPEID)str2fld(mpthd,mpdefs[mpdef]))
#define	mpdef_idval2mpthd(mpdef,id)	fld2str(MPTHDID,mpdefs[mpdef],id)
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#endif	/* MPTHD_H */

