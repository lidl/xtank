/****************************************************************************/
/* mpsem.h  -- SEMAPHORE CLUSTER (INTERFACE)				    */
/* Created:  11/22/87		Release:  0.7		Version:  12/03/87  */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#include "mptsk.cpy" /*

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/
/* OVERVIEW:
The semaphore cluster implements semaphores needed for
multi-processor exection control (inter-proprocessor syncronization).
The semaphores can be used as gates so to only allow a certain number
of processors (usually one) to access data the semaphore protects, so
that changes to the data are autonomous.

To control access to a particular data structure, include MPSEM datatype
as one of its fields.  When the data structure is created, initialize
the semaphore to the number of simultaneous accessors you intend to allow.
Then, to access the data structure, always enter a critical section
(see below).

The semaphores implemented here are "machine-level".  When waiting for a
semaphore to be released, the processor just spins in a loop rather than
doing something else productive.  And if there are other processors waiting,
there is no implicit ordering of which gets the semaphore first.  Finally,
when the semaphore is claimed, all signals are blocked and if the processor
doesn't release the semaphore quickly, interrupts will be lost (possibly
preventing other tasks from being run and losing I/O to the computer).

Thus, these semaphores are designed for low-level system programming
to synchronize *processors*.  If *task* syncronization is desired,
higher-level constructs built on this system programming should be used.
****************************************************************************/
/****************************************************************************/
#ifndef	MPSEM_H
#define	MPSEM_H
/****************************************************************************/
/****************************************************************************/
/* DATA INTERFACE:							    */
#ifdef	ANSI_C
	typedef volatile int	MPSEM, *MPSEMID;
#else
	typedef 	int	MPSEM, *MPSEMID;
#endif
/****************************************************************************/
/****************************************************************************/
/* CODE INTERFACE:							    */
/****************************************************************************/
/* int	mpsem_val(MPSEMID mpsem)	int

(A MACRO)  RETURNS OR ASSIGNS THE CURRENT VALUE OF THE SEMAPHORE IN ONE
INDIVISABLE OPERATION.  (USUALLY ASSIGNED TO THE NUMBER OF CLAIMS WHICH
CAN BE ACTIVE AT ONCE) */
/****************************************************************************/
/* void	mpsem_inc(MPSEMID mpsem);

INCREMENTS THE SEMAPHORE IN ONE INDIVISABLE OPERATION. */
/****************************************************************************/
/* void	mpsem_dec(MPSEMID mpsem);

DECREMENTS THE SEMAPHORE IN ONE INDIVISABLE OPERATION. */
/****************************************************************************/
/* void	mpsem_critsect(MPSEMID mpsem, CODEBLOCK); 	*/
/*
A MACRO WHICH TEMPORARILY CLAIMS mpsem AND BLOCKS ALL SIGNALS FOR THE
EVALUATION OF CODEBLOCK.  SPECIFICALLY, IT
	WAITS UNTIL SEMAPHORE mpsem CAN BE CLAIMED, THEN...
	BLOCKS ALL SIGNALS,
	CLAIMS THE SEMAPHORE,
	EVALUATES CODEBLOCK,
	RELEASES THE SEMAPHORE, AND
	RESTORES THE SIGNALS TO THEIR ORIGINAL VALUE.	*/
/****************************************************************************/
/* void	mpsem_take(MPSEMID mpsem);	*/
/*
WAITS UNTIL THE SEMAPHORE IS POSITIVE, THEN DECREMENTS (CLAIMS) IT.  IF THERE
ARE SEVERAL WAITERS, THE OPERATION MAY TAKE AN UNDETERMINED AMOUT OF TIME.
HAS NO EFFECT ON SIGNALS.  mpsem_critsect SHOULD BE USED IF POSSIBLE. */
/****************************************************************************/
/* void	mpsem_give(MPSEMID mpsem);

INCREMENTS (RELEASES) THE SEMAPHORE IMMEDIATELY.
(ASSUMES THE SEMAPHORE HAD BEEN CLAIMED BEFORE)
HAS NO EFFECT ON SIGNALS.  mpsem_critsect SHOULD BE USED IF POSSIBLE. */
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/* STRUCTURE, EXTERNAL-VARIBLE, AND MACRO DEFINITION:			    */
/****************************************************************************/
#include "mpsig.h"
#define	mpsem_init(mpsem)	\
			(mpsem_val((MPSEMID)(mpsem))= 1, (MPSEMID)(mpsem))
#define	mpsem_dinit(mpsem)	((void PTR)(mpsem))
#define	mpsem_val(mpsem)	(*(mpsem))
#define	mpsem_inc(mpsem)	(mpsem_val(mpsem)++)
#define	mpsem_dec(mpsem)	(--mpsem_val(mpsem))	
#define mpsem_give(mpsem)	mpsem_inc(mpsem)
#define	mpsem_take(mpsem)	while (mpsem_dec(mpsem), mpsem_val(mpsem)<0) \
					mpsem_inc(mpsem)
#define	mpsem_critsect(mpsem,CODEBLOCK)					\
{									\
	BOOL	semclaimed=	BOOL_FALSE;				\
	do {								\
		mpsig_critsect({					\
			mpsem_dec(mpsem);				\
			semclaimed= bool_create(mpsem_val(mpsem) >= 0);	\
			if (semclaimed)	{CODEBLOCK};			\
			mpsem_inc(mpsem);				\
		});							\
	} while (!semclaimed);						\
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#endif /* MPSEM_H */

