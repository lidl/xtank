#****************************************************************************/
#* mpthd.mak -- MAKEFILE FOR MPTHD CLUSTER				    */
#* Created:  11/29/87		Release:  0.7		Version:  06/27/88  */
#****************************************************************************
#(c) Copyright 1987, 1988 by Michael Benjamin Parker    (USA SS# 557-49-4130)
#
#All Rights Reserved unless specified in the following include files: */
##include "mptsk.cpy" /*
#
#DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
#****************************************************************************/
#****************************************************************************/
#* OVERVIEW:
#
#****************************************************************************/
#****************************************************************************/
# TURBO_COMP - Large Mem Model, define ANSI_C, No Stack-overflow Checking
# 	Map File, Line Numbers
TURBO_COMP=	tcc -c -ml -DANSI_C -N- -M -y
UNIX_COMP=	cc -c -O $*.c

## FOR MS-DOS		ON C COMPILER...
turbo:			mpthd.obj mpbug.obj mpdos.obj
	del mpthd.lib
	tlib mpthd.lib	+mpthd.obj +mpbug.obj +mpdos.obj

ms:			mpthd.obj mpbug.obj mpdos.obj
	del mpthd.lib
	lib mpthd.lib	+mpthd.obj +mpbug.obj +mpdos.obj;


## FOR BSD 3.4 UNIX 	ON MACHINE...
vax:			mpthd.o mpbug.o mpvaxsj.o
	-rm mpthd.a
	ar q mpthd.a	$?
	ranlib mpthd.a

rt:			mpthd.o mpbug.o
	-rm mpthd.a
	ar q mpthd.a	$?
	ranlib mpthd.a

##############################################################################
## OBJECT FILES...
mpthd.o:	mpthd.c mpthd.h mpsem.h mpsig.h mpbug.h mpmisc.h mpvaxsj.h
		$(UNIX_COMP)
mpthd.obj:	mpthd.c mpthd.h mpsem.h mpsig.h mpbug.h mpmisc.h
		$(TURBO_COMP) $*.c
mpbug.o:	mpbug.c mpbug.h
		$(UNIX_COMP)
mpbug.obj:	mpbug.c mpbug.h
		$(TURBO_COMP) $*.c
mpdos.obj:	mpdos.c mpdos.h
		$(TURBO_COMP) $*.c
mpvaxsj.o:	mpvaxsj.s
		cc -c $*.s
##############################################################################
clean:
		-del mp*.obj
		-rm mp*.o
##############################################################################
##############################################################################
##############################################################################
