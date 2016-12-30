/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
** Copyright 2016 by Kurt Lidl.
**
*/

#ifndef _CLFKR_H_
#define _CLFKR_H_

/* game configuration that dictates how xtank will run */
typedef struct {
	Boolean AutoExit;	/* Exit immediately after running?	*/
	Boolean AutoStart;	/* Skip the main menu and begin
				   execution automatically?		*/
	Boolean UseSetting;	/* Use a settings file specified
				   on the command line?			*/
	Boolean PrintScores;	/* Print scores to standard out after
				   running a game?			*/
	char *Settings;		/* File from which to load settings.	*/
	Boolean NoDelay;	/* Skip all pauses during gameplay?	*/
	Boolean NoIO;		/* Skip all screen I/O?			*/
} CLFkr;

#endif /* _CLFKR_H_ */
