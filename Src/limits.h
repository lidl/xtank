/* limits.h - fixed sizes for arrays and the like */

#ifndef _LIMITS_H_
#define _LIMITS_H_


#include "config.h"


#define NUM_BOXES	4	/* a map area NUM_BOXES x NUM_BOXES is visible
				   to the player in the animation window */

/*
** Maximum values for arrays of landmarks, blips, vehicles, and bullets
** These should be used as the sizes of the landmark_info, blip_info,
** vehicle_info, and bullet_info arrays.
*/
#define MAX_BLIPS	22
#define MAX_VEHICLES	16
#define MAX_BULLETS	300
#define MAX_DISCS       16
#define MAX_LANDMARKS   50
#define MAX_TURRETS	3
#define MAX_TEAMS	7	/* must be < #of bits in an int for
				   remove_vehicle() to work... */
#define MAX_EXPS	100
#define MAX_SPECIALS	8
#define MAX_PROGRAMS    3
#define MAX_LINES       256	/* number of lines drawn in 3d mode */
#define MAX_SEGMENTS	6
#define MAX_ENTRIES	45
#define MAX_STRING	24
#define MAX_VIEWS	32
#define MAX_SPEED	25	/* fastest tanks are expected to go, though
				   going over is allowed */

#define MAX_WEAPONS     6	/* number of weapons allowed on any one tank */

#define MAX_MESSAGES    8
#define MAX_DATA_LEN    31	/* number of bytes that can fit in a message */


#define MAX_ENGINES     16
#define MAX_ARMORS       7
#define MAX_BODIES      11
#define MAX_SUSPENSIONS  4
#define MAX_TREADS       5
#define MAX_BUMPERS      4


#ifdef UNIX
#define MAX_TERMINALS	10
#endif

#ifdef AMIGA
#define MAX_TERMINALS	1
#endif



#endif ndef _LIMITS_H_
