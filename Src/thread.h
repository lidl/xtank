/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** thread.h
*/

#ifdef UNIX
/****************************************************************************/
/* mptsk.cpy  -- COPYRIGHT NOTICE FOR MAILBOX MULTITASKER & ASSOCIATED FILES*/
/* Created:  06/01/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************
(c) Copyright 1987, 1988 by Michael Benjamin Parker     (USA SS# 557-49-4130)

Welcome to Mike Parker's Mailbox Multitasker Package,
			the Portable "C" Multitasking Environment!

The purpose of this package is to 1) encourage multitasking exploration and
use and 2) to provide a portable standard for multitasking on personal
computers.

There are three layers to the Mailbox Multitasker.  Release 0.7 (06/27/88) is
a beta release of the lowest (MPTHD) layer (works) and a alpha release of the
middle (MPTSK) layer.  The highest (MPRES) layer is included but untested.  A
complete 1.0 release forthcoming.


Permission is granted to put the lowest (MPTHD) and middle (MPTSK) layer into applications and to experiment with the third layer (MPRES).

Permission is also granted to BYTE Magazine to distribute this package and
associated files on the BIX bulletin board system (see the article "First
Come, First Served" in the July 1988 BYTE for more details).

In general, this package is distributed as "shareware": permission is granted
to freely distribute Mailbox and associated files and documentation at no
cost, provided:

		It is not DISTRIBUTED or PUBLISHED for commercial gain
		(except for BYTE MAGAZINE)

		It is not USED (in programs) for commercial gain WITHOUT
		paying the license fee (see below).

		All copyright notices and provisions are preserved.

	All other rights reserved.

IF YOU FIND MAILBOX USEFUL, $35 IS ASKED TO HELP RECOVER DEVELOPMENT COSTS.
This licensing fee is a requirement for all commercial applications of the
package.


I would greatly appreciate any comments you have about the thread cluster and
associated context switching article.  Useful enhancements should be
submitted to me directly for inclusion in future releases.

I may be reached at the following addresses:

COLLEGE:
	Massachusetts Institute of Technology, '89
	East Campus - Munroe 303
	3 Ames Street.
	Cambridge, MA 02139
	(617) 225-6303 / mbparker@athena.mit.edu

HOME:
	721 East Walnut Ave.
	Orange, CA 92667-6833
	(714) 639-9497

SUMMER WORK:
	Bell Communications Research
	MRE 2E375 / (201) 829-4420
	
	University Relations
	444 Hoes Lane, RRC 4C-128
	Piscataway, NJ 08854
	(201) 699-2846

SUMMER HOME:
	Bell Communications Summer Internship
	Rutger's Unv, Busch Campus, Nichols Apt. 17
	P.O. Box 459
	Piscataway, NJ 08854
	(201) 932-0774

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/

#define	ARGS(p)	()
#define	VOIDPTR	char PTR
typedef	enum {
	BOOL_FALSE,
	BOOL_TRUE
} BOOL;
#define	INT	long
typedef unsigned long UINT;
#define	PTR	*
#define	PTR_	"ld"

#ifdef vax
typedef	int	MPVAXSJ[17];	/* HOLDS VAX REGISTERS psl AND r1..r15
				   RESPECTIVELY (r14 IS STACK POINTER) */
int	mpvaxsj_setjmp();
int	mpvaxsj_longjmp();

#define	setjmp(jmp)		mpvaxsj_setjmp(jmp)
#define	longjmp(jmp,ret)	mpvaxsj_longjmp(jmp,ret)
#define	jmp_buf			MPVAXSJ
#else
#include <setjmp.h>
#endif

typedef 	int	MPSEM, *MPSEMID;

typedef	struct _MPTHD	MPTHD, PTR MPTHDID;	/* THREAD (CONTEXT) STRUCT  */
typedef	MPTHDID	(*MPTHDFN)();
typedef UINT    MPDEFID;
typedef int     MPSIGSTATE;

struct _MPTHD {
      jmp_buf state;          /* THE CURRENT STATE OF THE THREAD */
      MPSIGSTATE mpsigstate;  /* THE SIGNAL STATE WHEN THREAD CREATED */
      MPSEM   mpsem;          /* SEMAPHORE TO ALLOW ONLY ONE PROCESSOR */
      MPTHDID oldmpthd;       /* THE THREAD EXECUTING PRIOR TO THIS ONE */
      MPTHDFN mpthdfn;        /* THE FUNCTION THIS THREAD EXECUTES */
      BOOL    stackoverflow;  /* STACK OVERFLOW DETECTOR VARIABLE */
      MPDEFID mpdefsize;      /* SIZE OF MPDEF'AULTS (IN CHARS) */
      char    mpdefs[1];      /* THE MPDEF'AULT STORAGE */
};

MPTHDID	mpthd_switch	ARGS((MPTHDID newmpthd));
MPTHDID	mpthd_init	ARGS((VOIDPTR buff, UINT size, MPTHDFN mpthdfn));
VOIDPTR	mpthd_dinit	ARGS((MPTHDID mpthd));
void	mpthd_setup	ARGS((void));

extern	MPTHDID	mpthd_me_reg;
#define	mpthd_me()	(mpthd_me_reg)

typedef MPTHD Thread;
#define thread_switch mpthd_switch
#define thread_init   mpthd_init
#define thread_setup  mpthd_setup
#endif
