/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** gr.h
*/

/*
$Author: rpotter $
$Id: gr.h,v 2.3 1991/02/10 13:50:36 rpotter Exp $

$Log: gr.h,v $
 * Revision 2.3  1991/02/10  13:50:36  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:50  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:30  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:12:47  rpotter
 * small changes
 * 
 * Revision 1.2  90/12/29  21:31:11  aahz
 * renamed a define.
 * 
 * Revision 1.1  90/12/29  21:02:23  aahz
 * Initial revision
 * 
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
