/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

/* Window numbers for display procedures */
#define ANIM_WIN	0
#define GAME_WIN	1
#define CONS_WIN	2
#define MAP_WIN		3
#define HELP_WIN	4
#define MSG_WIN		5
#define STAT_WIN	6
#define WIN_3D1		7
#define MAX_STAT_WINDOWS 6

#define check_expose(w,status) (win_exposed(w) && \
				(status == REDISPLAY && (status = ON), \
				 expose_win(w,FALSE)))

#define print(str,x,y) \
  (display_mesg2(ANIM_WIN,str,x,y,S_FONT))
