/****************************************************************************/
/* mpmisc.h -- Mike Parker's MISCELLANEOUS HANDY ``C'' EXTENSIONS	    */
/* Created:  8/1/87		Release:  0.7		Version:  06/27/88  */
/****************************************************************************
(c) Copyright 1987, 1988 by Michael Benjamin Parker     (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#include "mptsk.cpy" /*

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/
/* OVERVIEW:

This file contains a miscellaneous bunch of extensions to the ``C'' language,
mostly macros and typedefs.

Most of them are declarations for portability of code and data structures.

Some, as cast and fld2str, provide important functional extensions to
the ``C'' language.

See below for more details... */
/****************************************************************************/
#ifndef MPMISC_H
#define MPMISC_H
/****************************************************************************/
/****************************************************************************/
/* FOR FORMING PROTOTYPES FOR EXTERNAL FUNCTIONS WITH/WITHOUT ANSCI C */
#ifdef	ANSI_C
#define	ARGS(p)	p
#define	VOIDPTR	void PTR
#else
#define	ARGS(p)	()
#define	VOIDPTR	char PTR
#endif
/****************************************************************************/
/* for(ever) {LOOPS}							    */
#define	EVER	;;
#define	ever	;;
/* unil(COND) {LOOPS} */
#define	until(COND)	while(COND)
/****************************************************************************/
/****************************************************************************/
/* CASTING OPERATOR THAT WORKS ON ANY TYPE (INCLUDING STRUCTURES)	    */
#define	cast(TYPE,var)	(*((TYPE *)(&var)))
	/* CASTS VARIABLE var TO TYPE TYPE, EVEN IF TYPE IS A STRUCTURE */
/****************************************************************************/
/****************************************************************************/
/* ASSIGNMENT OPERATOR WHICH RETURNS THE OLD VALUE, NOT THE NEW VALUE    */
#define	assign(VAR,newval)				\
	(VAR);						\
	(VAR)=	newval
/* ASSIGNS VAR TO newval.  "RETURNS" THE OLD VALUE OF VAR */
/****************************************************************************/
/****************************************************************************/
/* FIELD-POINTER / STRUCTURE-POINTER CONVERSION	OPERATORS		    */
#define	str2fld(strptr,FLDNAM)		\
	(&((strptr)->FLDNAM))
	/* RETURNS THE ADDRESS OF A FIELD IN A STRUCTURE GIVEN A POINTER TO
	THE STRUCTURE AND ITS FIELD NAME */

#define	fld2str(STRTYPEID,FLDNAM,fldptr)				\
	((STRTYPEID)(((char PTR)(fldptr))				\
			- ((char PTR)str2fld((STRTYPEID)0,FLDNAM))))
/*
#define	fld2str(STRTYPEID,FLDNAM,fldptr)				\
	((STRTYPEID)int_2ptr(	ptr_2int(fldptr) -			\
				ptr_2int(str2fld((STRTYPEID)0,FLDNAM))))
*/
	/* RETURNS THE ADDRESS OF A STRUCTURE STRTYPEID WITH FIELD FLDNAM
		AT ADDRESS fldptr. */
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/* STACK-STORAGE ALLOCATION "DECLARATION"				    */
/*	STACK CHARACTER ARRAYS WITH SIZE SPECIFIED AT RUN TIME		    */
#ifdef	__TURBOC__
#define	stackalloc(VARIABLE,size,PGMBLOCK)		\
	{						\
		unsigned	varsize=	(size); \
		_SP-=	varsize;			\
		{					\
			char	(VARIABLE)[0];		\
			{PGMBLOCK}			\
		}					\
		_SP+=	varsize;			\
	}
#endif	
/****************************************************************************/
/****************************************************************************/
/* EXACT BIT WIDTH TYPES FOR DATA MANIPULATED ON MORE THAN ONE MACHINE	    */
typedef	unsigned char	UB08;
typedef unsigned short	UB16;
typedef	unsigned long	UB32;
#ifdef	ANSI_C
typedef	signed char	SB08;
typedef signed short	SB16;
typedef signed long	SB32;
#else
typedef		char	SB08;
typedef 	short	SB16;
typedef 	long	SB32;
#endif;
/****************************************************************************/
/****************************************************************************/
/* BOOLEAN DATA TYPE							    */
#ifndef	xwindows
typedef	enum {
	BOOL_FALSE,
	BOOL_TRUE
} BOOL;
#else
#define	BOOL	unsigned char
/* typedef	unsigned char	BOOL; */
#define	BOOL_FALSE	('\0')
#define	BOOL_TRUE	('\1')
#endif
/****************************************************************************/
#define	bool_create(value)	((value)?(BOOL_TRUE):(BOOL_FALSE))
/****************************************************************************/
/****************************************************************************/
/* INTEGER TYPE -- FOR PORTABILITY & CONSISTENT POINTER/INTEGER CONVERSIONS */
#define	INT	long	/* INTEGER WHICH CAN HOLD AT LEAST AS MANY
			   BYTES AS MACHINE CAN ADDRESS */
typedef	SB32	SINT;
typedef UB32	UINT;	
#define	SINT_d	"ld"	/* printf TYPE */
#define	UINT_d	"lu"	/* dECIMAL */
#define	UINT_o	"lo"	/* oCTAL */
#define	UINT_x	"lx"	/* HExIDECIMAL */
#define	UINT_X	"lX"
/****************************************************************************/
/* INTEGER OPERATIONS							    */
#define	int_min(a,b)	(((a)<(b))?(a):(b))
#define	int_sign(no)	((no)?(((no)>0)?(1):(-1)):(0))

/* RETURNS a WRAPED WITHIN b (A DECENT % (mod) FUNCTION) */
#define	int_wrap(a,b)				\
(                                              	\
	( ( (a) && (!((a)>=0) != !((b)>=0)) )	\
		?(b)				\
		:(0)				\
	)					\
	+					\
	((a) % abs(b))				\
)
/* WRAPS a WITHIN b (FOR POSITIVE b ONLY!) */
#define	int_wraper(a,b)				\
	while ((a) < 0)		(a)+=	(b);	\
	while ((a) > (b))	(a)-=	(b)	
/****************************************************************************/
/****************************************************************************/
/* POINTER TYPE -- FOR PORTABILITY & CONSISTENT POINTER/INTEGER CONVERSIONS */
#if (defined(__TURBOC__) || defined(M_I86))
#define	PTR	far *

/* ANY PTR TO UNIQUE INTEGER */
#define	ptr_2int(ptr)							\
((UB32)(	 (((((UB32)((void far *)(ptr))) & 0xFFFF0000) >> 12) +	\
		 ((UB16)((void far *)(ptr))))				\
	& 0x000FFFFF) )

/* ANY INT TO UNIQUE NORMAL PTR */
#define	int_2ptr(Int)							\
((VOIDPTR)	( ((((UB32)(Int)) & 0x000F0000) << 12) | ((UB16)(Int)) ) )

/* ANY INT TO UNIQUE MIN. OFFSET PTR
	 (SAME AS NORMALIZED huge POINTER IN AT LEAST TURBO C) */
#define	int_2ptrl(Int)							\
((VOIDPTR)	( ((((UB32)(Int)) & 0x000FFFF0) << 12) 			\
			| (((UB16)(Int)) & 0x000F) ) )

/* ANY INT TO UNIQUE MAX. OFFSET PTR */
#define	int_2ptrh(Int)							\
((VOIDPTR)	((((((UB32)(Int)) & 0x000FFFF0) << 12) - 0x0FFF0000)	\
			| ((((UB16)(Int)) & 0x000F) | 0xFFF0) ) )
/* If this looks slow & messy,...
		thank Intel for their braindamaged segmented architecture! */
#define	PTR_	"Fp"	/* printf TYPE */
#else
#define	PTR	*
#define	ptr_2int(ptr)	((UINT)		(ptr))
#define	int_2ptr(Int)	((VOIDPTR)	(Int))
#define	int_2ptrl(Int)	((VOIDPTR)	(Int))
#define	int_2ptrh(Int)	((VOIDPTR)	(Int))
#define	PTR_	"ld"
#endif
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#endif	/* MPMISC_H */
