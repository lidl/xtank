/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** gr.h
*/

/* Window numbers for display procedures */
#define ANIM_WIN	0
#define GAME_WIN	1
#define CONS_WIN	2
#define MAP_WIN		3
#define HELP_WIN	4
#define MSG_WIN		5
#define STAT_WIN	6
#define MAX_STAT_WINDOWS 6

/* Indices for all the objects in the random_obj array */
#define XTANK_OBJ	0
#define TEAM_OBJ    1
#define TERP_OBJ    2

#define check_expose(w,status) (win_exposed(w) && \
				(status == REDISPLAY && (status = ON), \
				 expose_win(w,FALSE)))

#define print(str,x,y) \
  (display_mesg2(ANIM_WIN,str,x,y,S_FONT))
