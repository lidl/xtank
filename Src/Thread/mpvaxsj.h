/****************************************************************************/
/* mpvaxsj.h -- REPLACEMENT setjmp & longjmp FUNCTIONS FOR VAX (INTERFACE)  */
/* Created:  12/2/87		Release:  0.7		Version:  12/3/87   */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#include "mptsk.cpy" /*

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/
/* OVERVIEW:
These are the C functions setjmp and longjmp reimplemented in vax assembly
for the thread cluster.  The BSD 4.3 equivalent of this file, setjmp.s,
does its longjmp's by restoring each stack frame individually.
This complex method of storing the environment will not work to change
contexts.  Therefore, a more traditional setjmp and longjmp are implemented
here that simply save/restore the entire processor state except for
one register, r0, used for the return value.
WARNING:  THIS CODE WON'T WORK NOW.  STILL HAS BUGS
*/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#ifndef	SETJMP_H
#define	SETJMP_H
/****************************************************************************/
/****************************************************************************/
typedef	int	MPVAXSJ[17];	/* HOLDS VAX REGISTERS psl AND r1..r15
				   RESPECTIVELY (r14 IS STACK POINTER) */
/****************************************************************************/
/****************************************************************************/
int	mpvaxsj_setjmp();
/*	MPVAXSJ	env; */
int	mpvaxsj_longjmp();
/*	MPVAXSJ	env;
	int	ret; */
/****************************************************************************/
#endif	/* SETJMP_H */

