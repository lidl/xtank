/****************************************************************************/
/* mpbug.h -- Mike Parker's HANDY "C" DEBUGGING UTILITIES (INTERFACE)	    */
/* Created:  10/1/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************
(c) Copyright 1987, 1988 by Michael Benjamin Parker     (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#include "mptsk.cpy" /*

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/
/* OVERVIEW:
This file includes a set of macros handy for debugging "C" code.
The file may be included in a program to be tested.  When the program is
debugged, defining NDEBUG will nullify the effect of all mpbug calls.
However, if the code ever gets buggy again, undefining NDEBUG quickly
includes all the debugging statements.

See macros below for more details.
*/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/* 	#ifndef	MPBUG_H
	#define MPBUG_H	*/
/****************************************************************************/
#ifndef	NDEBUG
/****************************************************************************/
#ifdef	__TURBOC__
#ifndef	DOS_H
#define	DOS_H
#include <dos.h>
#endif	/* DOS_H */
#else
#include <signal.h>
#endif
#include <stdio.h>
/****************************************************************************/
/****************************************************************************/
#ifdef	ANSI_C
int	mpbug_usrbrk(void);
#else
int	mpbug_usrbrk();
#endif
/****************************************************************************/
#ifdef	__TURBOC__
#define mpbug_init(bps)		ctrlbrk(mpbug_usrbrk);	mpbug_level(bps)
#else
#define	mpbug_init(bps)		signal(SIGINT,mpbug_usrbrk);	mpbug_level(bps)
#endif
/* Initialize:  Initializes the debugging routines:
	Control-Break will halt program,
	mpbug_lev() will indent bps bytes per space for every byte on stack.
*/
/****************************************************************************/
#ifdef	ANSI_C
#define	mpbug_val(var,fmt)	fprintf(stderr,#var "= %" #fmt "\t",var)
#define	mpbug_vali(var)		mpbug_val(var,ld)
#define	mpbug_valp(var)		fprintf(stderr,#var "= %lX (%p)\t",ptr_2int(var),var)
#define	mpbug_vals(var)		mpbug_val(var,s)
#else
#define	mpbug_val(var,fmt)	fprintf(stderr,"?var= %ld\t",var)
#define	mpbug_vali(var)		fprintf(stderr,"?intvar= %ld\t",var)
#define	mpbug_valp(var)		fprintf(stderr,"?ptrvar = %lX\t",var)
#define	mpbug_vals(var)		fprintf(stderr,"?strvar %s\t",var)
	/* WARNING: ASSUMES fmt IS SIGNED LONG */
#endif
/* Value:  Prints the name & value of a variable var of "printf" format fmt */
/****************************************************************************/
#define	mpbug_pos()	fprintf(stderr,"PC= %s %d\t",__FILE__,__LINE__)
/* Position: Prints the current position (file & line) in the source code.  */
/****************************************************************************/
#define	mpbug_lev()		mpbug_level(0)
/* Level:  Starts a new line indented according to the current stack depth. */
/****************************************************************************/
#define	mpbug_cmt(string)	fprintf(stderr,string)
/* Comment:  An ordinary printf.					    */
/****************************************************************************/
#define	mpbug_bp()		putc('\007',stderr);getc(stdin);
/* Breakpoint:  Beeps stdout.  Pressing return continues.		    */
/****************************************************************************/
#ifdef	ANSI_C
	void	mpbug_level(unsigned bytesperspace);
#else
	void	mpbug_level();
#endif
/****************************************************************************/
/****************************************************************************/
#else /* NDEBUG DEFINED -- NO DEBUGGING */
#define	mpbug_init(bps)		(bps)
#define	mpbug_val(var,fmt)	(var)
#define	mpbug_valp(var)		(var)
#define	mpbug_vali(var)		(var)
#define	mpbug_vals(var)		(var)
#define	mpbug_pos()
#define	mpbug_lev()
#define	mpbug_cmt(string)	(string)
#define	mpbug_bp()
#endif
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/*	#endif	MPBUG_H */
