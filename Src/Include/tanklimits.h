/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** $Id$
*/

/*
** Comment: fixed sizes for arrays and the like
*/

#ifndef _TANKLIMITS_H_
#define _TANKLIMITS_H_

#define NUM_BOXES	4			/* a map area NUM_BOXES x NUM_BOXES is visible
				   to the player in the animation window */

/*
** Maximum values for arrays of landmarks, blips, vehicles, and bullets
** These should be used as the sizes of the landmark_info, blip_info,
** vehicle_info, and bullet_info arrays.
*/
#define MAX_VEHICLES	16
#define MAX_BLIPS	(MAX_VEHICLES + 6)
#define MAX_BULLETS	511
#define MAX_DISCS       (15+1)
#define MAX_LANDMARKS   (32+16+2)
#define MAX_EXPS	255
#define MAX_PROGRAMS    3		/* maximum number of programs allowed on any
				   one vehicle */
#define MAX_LINES       256		/* number of lines drawn in 3d mode */
#define MAX_SEGMENTS	6
#define MAX_ENTRIES	60			/* size of crec->item array in newconsole.c */
#define MAX_STRING	24
#define MAX_VIEWS	32
#define MAX_SPEED	25			/* fastest tanks are expected to go, though
				   going over is allowed */

#define MAX_WEAPONS     6		/* number of weapons allowed on any one tank */

#define MAX_MESSAGES    8


#define MAX_ENGINES     16
#define MAX_ARMORS       9
#define MAX_BODIES      14
#define MAX_SUSPENSIONS  4
#define MAX_BUMPERS      4

#define MAX_VEHICLE_OBJS	MAX_BODIES
#define MAX_TURRET_OBJS		1
#define MAX_EXP_OBJS		10 
#define MAX_LANDMARK_OBJS	3

#define MAXPNAME	12			/* length of a player's name */

#ifdef UNIX
#define MAX_TERMINALS	14   /*HAK*/
#endif

#ifdef AMIGA
#define MAX_TERMINALS	1
#endif


#endif /* ndef _LIMITS_H_ */
