/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** screen.h
*/

/*
$Author: aahz $
$Id: screen.h,v 2.5 1992/09/12 09:40:09 aahz Exp $

$Log: screen.h,v $
 * Revision 2.5  1992/09/12  09:40:09  aahz
 * reordered windows.  Made more important windows closer to
 * the top of the screen so reduced size displays only miss the garbage
 *
 * Revision 2.4  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.3  1991/02/10  13:51:34  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:52  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:50  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:25  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:01  aahz
 * Initial revision
 * 
*/

#ifdef S1024x864
/* Window geometry information */
#define ANIM_WIN_X        0
#define ANIM_WIN_Y        0
#define ANIM_WIN_WIDTH    768
#define ANIM_WIN_HEIGHT   768

#define HELP_WIN_X        0
#define HELP_WIN_Y        768
#define HELP_WIN_WIDTH    (1015-MAP_WIN_WIDTH)
#define HELP_WIN_HEIGHT   90

#define MAP_WIN_X         768
#define MAP_WIN_Y         0
#define MAP_WIN_WIDTH     250
#define MAP_WIN_HEIGHT    250

#define CONS_WIN_X        768
#define CONS_WIN_Y        (MAP_WIN_Y + MAP_WIN_HEIGHT + 6)
#define CONS_WIN_WIDTH    250
#define CONS_WIN_HEIGHT   306

#define MSG_WIN_X         768
#define MSG_WIN_Y         (CONS_WIN_Y + CONS_WIN_HEIGHT + 6)
#define MSG_WIN_WIDTH     250
#define MSG_WIN_HEIGHT    92

#define GAME_WIN_X        768
#define GAME_WIN_Y        (MSG_WIN_Y + MSG_WIN_HEIGHT + 6)
#define GAME_WIN_WIDTH    250
#define GAME_WIN_HEIGHT   192

/* Battle windows.  Not always mapped */
#define STAT_WIN_X	768
#define STAT_WIN_Y	0
#define STAT_WIN_WIDTH	252
#define STAT_WIN_HEIGHT	97

#define BORDER          3
#define STAT_BORDER	2
#endif

#ifdef S640x400
/* Window geometry information */
#define ANIM_WIN_X        0
#define ANIM_WIN_Y        0
#define ANIM_WIN_WIDTH    400
#define ANIM_WIN_HEIGHT   400

#define HELP_WIN_X        0
#define HELP_WIN_Y        310
#define HELP_WIN_WIDTH    638
#define HELP_WIN_HEIGHT   88

#define GAME_WIN_X        400
#define GAME_WIN_Y        0
#define GAME_WIN_WIDTH    238
#define GAME_WIN_HEIGHT   192

#define CONS_WIN_X        400
#define CONS_WIN_Y        0
#define CONS_WIN_WIDTH    238
#define CONS_WIN_HEIGHT   158

#define MAP_WIN_X         400
#define MAP_WIN_Y         160
#define MAP_WIN_WIDTH     238
#define MAP_WIN_HEIGHT    238

#define MSG_WIN_X         0
#define MSG_WIN_Y         0
#define MSG_WIN_WIDTH     238
#define MSG_WIN_HEIGHT    88

/* Battle windows.  Not always mapped */
#define STAT_WIN_X	400
#define STAT_WIN_Y	0
#define STAT_WIN_WIDTH	238
#define STAT_WIN_HEIGHT	60

#define BORDER          1
#define STAT_BORDER	1
#endif


#define SCREEN_WIDTH	ANIM_WIN_WIDTH	/* for robot interface */
#define SCREEN_HEIGHT	ANIM_WIN_HEIGHT

#define BOX_WIDTH	(ANIM_WIN_WIDTH/NUM_BOXES)
#define BOX_HEIGHT	(ANIM_WIN_HEIGHT/NUM_BOXES)
