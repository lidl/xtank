/****************************************************************************/
/* mpsig.h  -- SIGNAL/INTERRUPT CLUSTER (INTERFACE)			    */
/* Created:  11/22/87		Release:  0.7		Version:  06/27/87  */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files:
--no include files--

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/
/* OVERVIEW:
The signal/interrupt cluster implements signal/interrupt control needed
for processor - independent_hardware syncronization.  This cluster allows
hardware interrupts to be temporarily blocked so a fragment of code can
be executed without interruption (helping achieve automicity).

When the processor has blocked interrupts, if an interrupt occurs it
"waits" (is buffered) until the processor unblocks interrupts, and then
interrupts the processor.  However, each interrupt source (or type) is
``buffered'' only once, so if the processor is blocks too long and a
source interrupts more than once, only one interrupt event is remembered.
In other words, the processor is interrupted at most once for each
interrupt source when it unblocks.

Therefore, if the processor blocks too long, interrupts can be lost --
preventing the processor from doing other timely events (like I/O and
task switching) when it needs to.  "Too long" means longer than the
shortest period between interrupts of the fastest interrupt source.

In general, interrupts syncronization is for low-level system
programming.  For task-to-task and task-to-I/O syncronization,
higher-level constructs should be used.
****************************************************************************/
/****************************************************************************/
#ifndef	MPSIG_H
#define	MPSIG_H
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
/****************************************************************************/
typedef	int	MPSIGSTATE;	/* SIGNAL MASK (SIGNALS BLOCKED FROM
				DELIVERY - BIT IS ON IFF BLOCKED) */
/****************************************************************************/
/* CODE INTERFACE:							    */
/****************************************************************************/
/* MPSIGSTATE	mpsig_state(MPSIGSTATE newstate);

SETS THE SIGNAL BLOCKING STATE.  ALL SIGNALS ARE BLOCKED WITH
MPSIG_ALLBLOCKED (~0).  NO SIGNALS ARE BLOCKED WITH MPSIG_NONEBLOCKED (0).
RETURNS THE PREVIOUS SIGNAL STATE. */
/****************************************************************************/
/* void	mpsig_critsect(CODEBLOCK);

A MACRO WHICH EVALUATES CODEBLOCK WITHOUT ANY POSSIBLE INTERRUPTION, IE, IT:
	BLOCKS ALL SIGNALS,
	EVALUATES CODEBLOCK, AND
	THEN RESTORES SIGNALS TO THEIR PREVIOUS STATE. */
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/* STRUCTURE, EXTERNAL-VARIBLE, AND MACRO DEFINITION:			    */
/****************************************************************************/
#include "mpmisc.h"
#ifdef	unix
#include <signal.h>
#else
#include "mpdos.h"
#endif
/****************************************************************************/
#define	MPSIG_ALLBLOCKED	((MPSIGSTATE)~0)
#define	MPSIG_NONEBLOCKED	((MPSIGSTATE)0)
#ifndef	unix
#define	mpsig_state(new)			\
	(_FLAG & _FLAG_IF);			\
	(new)?(disable()):(enable())
#else
#define	mpsig_state(new)			\
	sigsetmask(new)
#endif
/****************************************************************************/
#define	mpsig_critsect(CODEBLOCK)					\
	{								\
		MPSIGSTATE mpsigstate=	mpsig_state(MPSIG_ALLBLOCKED);	\
		{CODEBLOCK}						\
					mpsig_state(mpsigstate);	\
	}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#endif /* MPSIG_H */
