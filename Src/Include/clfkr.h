
/* clfkr.h - auto starting options et al */

#ifndef _CLFKR_H_
#define _CLFKR_H_



/* game configuration that dictates how xtank will run */
struct CLFkr
{
    Boolean AutoExit;    /* should xtank exit immediately after running? */
	Boolean AutoStart;   /* should xtank skip the main menu and begin    */
						 /* execution automatically?                     */
    Boolean UseSetting;  /* should xtank use a settings file specified   */
						 /* on the command line?                         */
    Boolean PrintScores; /* should xtank print scores to standard out    */
						 /* after running a game                         */
						 /* on the command line?                         */
    char   *Settings;    /* filename where settings should be loaded     */
						 /* from.                                        */
    Boolean NoDelay;     /* Tells whether all pauses should be bypassed  */
    Boolean NoIO;        /* Tells if all screen i/o should be skipped    */
}; 

#endif ndef _CLFKR_H_
