/****************************************************************************/
/* mpbug.c -- Mike Parker's HANDY "C" DEBUGGING UTILITIES (IMPLEMENTATION)  */
/* Created:  10/1/87		Release:  0.7		Version:  12/03/87  */
/****************************************************************************
(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)

All Rights Reserved unless specified in the following include files: */
#include "mptsk.cpy" /*

DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
****************************************************************************/
#include "mpbug.h"
/****************************************************************************/
/****************************************************************************/
int	mpbug_usrbrk()
{
#ifdef	__TURBOC__
	return(0);
#else
	exit(-1);
#endif
}
/****************************************************************************/
void	mpbug_level(bytesperspace)
	unsigned	bytesperspace;
{
	static unsigned 	bps=		~0;
	static unsigned 	*stktop;
	unsigned int		n;
	if (bytesperspace) {	    /* IF bytesperspace NON-ZERO VALUE */
		stktop=	 &n;        /* INITIALIZE DEPTH MARKER */
		bps=	bytesperspace; /* AND DEPTH MEASURE */
	} else {
		putc('\n',stderr);   /* ELSE INDENT ACCORDING TO CUR DEPTH */
		if (bps != ~0) {
			n=	(stktop - &n)/bps;
			while (n--)	putc('#',stderr);
		}
	}
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
