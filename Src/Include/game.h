/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** game.h
*/

/*
$Author: lidl $
$Id: game.h,v 1.1.1.1 1995/02/01 00:25:40 lidl Exp $
*/

#ifndef _GAME_H_
#define _GAME_H_

/* the different games that can be played */
  typedef enum {
	  COMBAT_GAME,
	  WAR_GAME,
	  ULTIMATE_GAME,
	  CAPTURE_GAME,
	  RACE_GAME,
	  STQ_GAME
  } Game;

#endif /* _GAME_H_ */
