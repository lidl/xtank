/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** clfkr.h
*/

/*
$Author: lidl $
$Id: clfkr.h,v 1.1.1.1 1995/02/01 00:25:40 lidl Exp $
*/

#ifndef _CLFKR_H_
#define _CLFKR_H_

/* game configuration that dictates how xtank will run */
  struct CLFkr {
	  Boolean AutoExit;			/* should xtank exit immediately after running? */
	  Boolean AutoStart;		/* should xtank skip the main menu and begin    */
	  /* execution automatically?                     */
	  Boolean UseSetting;		/* should xtank use a settings file specified   */
	  /* on the command line?                         */
	  Boolean PrintScores;		/* should xtank print scores to standard out    */
	  /* after running a game                         */
	  /* on the command line?                         */
	  char *Settings;			/* filename where settings should be loaded     */
	  /* from.                                        */
	  Boolean NoDelay;			/* Tells whether all pauses should be bypassed  */
	  Boolean NoIO;				/* Tells if all screen i/o should be skipped    */
  };

#endif /* ndef _CLFKR_H_ */
