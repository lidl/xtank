/****************************************************************************/
/* mpthd.c - THREAD CLUSTER (IMPLEMENTATION)				    */
/* Created:  10/31/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#include "mptsk.cpy" /*

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/
#include "mpthd.h"
#define	NDEBUG
#include "mpbug.h"
/****************************************************************************/
MPTHDID	mpthd_me_reg;	/* A DEDICATED "REGISTER" OF THE PROCESSOR POINTING
			TO THE THREAD IT CURRENTLY EXECUTES.
			ON SINGLE-PROCESSOR IMPLEMENTATIONS (AS THIS ONE),
  			CAN BE A GLOBAL VARIABLE */
extern	main();		/* THE PROGRAM'S main FUNCTION */
/****************************************************************************/
/****************************************************************************/
void	_mpthd_setup(mpthd)
	MPTHDID	mpthd;
{
	mpthd_me()=	mpthd;
	mpsem_take(str2fld(mpthd_me(),mpsem));
}
/****************************************************************************/
#define	mpthd_mainmpthds	10

void	mpthd_setup()
{
	static	MPTHD	mpthd[mpthd_mainmpthds];
	mpthd_me()=	mpthd;
	mpdef_mysize()=	0;
	mpthd_init(mpthd,mpthd_mainmpthds*sizeof(MPTHD),(MPTHDFN)main);
	mpsem_take(str2fld(mpthd_me(),mpsem));
}
/****************************************************************************/
void	mpthd_execute()		/* INTERNAL USE ONLY! */
	/* EXECUTES THE THREAD IN INFINITE mpthdfn / switch LOOP */
{
	MPTHDID	oldmpthd=	mpthd_me()->oldmpthd;	/* SAVE oldmpthd */

        /* RELEASE oldmpthd'S SEMAPHORE AND RESTORE INTERRUPTS */
	mpsem_give(&(oldmpthd->mpsem));
	mpsig_state(mpthd_me()->mpsigstate);

	/* NEW THREAD INFINITE EVALUATE-SWITCH LOOP */
	for (EVER)
		mpthd_me()->oldmpthd=
			mpthd_switch(
				(mpthd_me()->mpthdfn)(mpthd_me()->oldmpthd));
}
/****************************************************************************/
MPTHDID	mpthd_init(buff, size, mpthdfn)
	VOIDPTR	buff;
	UINT	size;
	MPTHDFN	mpthdfn;
{
	MPTHDID	mpthd=	(MPTHDID)buff;
	if ((size < (3*sizeof(MPTHD)+mpdef_mysize())))
		return((MPTHDID)0);
	{			/* COPY ALL DEFAULT VALUES OF PARENT THREAD */
		MPDEFID	defpos=	mpdef_size(mpthd)= mpdef_mysize();
		while (defpos--)
			mpthd->mpdefs[defpos]=	mpthd_me()->mpdefs[defpos];
	}
	mpbug_lev();mpbug_valp(mpthd);

	/* INITIALIZE "LOCAL" VARIABLES FOR NEW STATE */
	mpthd->mpthdfn=			mpthdfn;
	mpsem_val(&(mpthd->mpsem))=	1;	/* ALLOW AT MOST 1 ACCESSOR */
	mpthd->stackoverflow=		BOOL_FALSE;

	/* COPY CURRENT STATE */
	mpthd->mpsigstate=		mpsig_state(MPSIG_ALLBLOCKED);
	if (setjmp(mpthd->state))	mpthd_execute();  /* EXECUTE NEW
						THREAD WHEN SWITCHED TO */
					mpsig_state(mpthd->mpsigstate);

	/* MODIFY COPY OF CURRENT STATE -- SET UP NEW STACK POINTER */
	{
		UINT	uniqptr=	ptr_2int(buff)
					+ (size-sizeof(mpthd->state));
		char PTR buffend=	int_2ptrh(uniqptr);
		mpbug_vali(size);mpbug_valp(buffend);
		mpbug_bp();
#ifdef	__TURBOC__
		mpbug_lev();mpbug_cmt("Setting stack state");
		mpthd->state[0].j_ss=	FP_SEG(buffend);
		mpthd->state[0].j_sp=	FP_OFF(buffend);
#endif
#ifdef	M_I86
		mpthd->state[3]=	FP_OFF(buffend);
	/*	mpthd->state[?]=	FP_SEG(buffend); */
/* WARNING:	MICROSOFT "C"'s jmp_buf MAY NOT BE POWERFUL ENOUGH TO IMPLEMENT
	CO-ROUTINES (WITH DIFFERENT STACKS) */
#endif
#ifdef	vax
		buffend-=	sizeof(MPTHD);
		mpthd->state[0]=		ptr_2int(buffend);
#endif
#if	(defined(ibm032) || defined(ibm370))
		mpthd->state[0]=		ptr_2int(buffend);
#endif	
	}
	return(mpthd);
}
/****************************************************************************/
VOIDPTR	mpthd_dinit(mpthd)
	MPTHDID	mpthd;
{

	if (mpthd==mpthd_me())	return(0);	/* CAN'T DESTROY YOURSELF! */
	mpsem_critsect(str2fld(mpthd,mpsem),{;});/* WAIT UNTIL mpthd IS FREE*/
	return((VOIDPTR)mpthd);
}
/****************************************************************************/
/****************************************************************************/
/* THE mpthd_switch() FUNCTION	- FOR SWITCHING BETWEEN THREADS OF EXECUTION*/
/****************************************************************************/
MPTHDID	mpthd_switch(newmpthd)
	MPTHDID	newmpthd;
{
	mpbug_valp(newmpthd);
	if (newmpthd->stackoverflow)	return((MPTHDID)0);
	if (newmpthd!=mpthd_me()) {
		mpsem_critsect(&(newmpthd->mpsem),{
			if (!setjmp(mpthd_me()->state)) {
				newmpthd->oldmpthd=	mpthd_me();
				mpthd_me()=		newmpthd;
				longjmp(mpthd_me()->state,1);
			}
			newmpthd=	mpthd_me()->oldmpthd;
		});
	}
	return(newmpthd);	/* RETURN OLD THREAD SWITCHED FROM */
}
/****************************************************************************/
/****************************************************************************/

