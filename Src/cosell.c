#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** cosell.c
**
** version basic simple lame non-complicated to the max (by Doug Church)
** adjusted to better handle multiple discs by Adam Bryant.
*/

/*
$Author: lidl $
$Id: cosell.c,v 2.4 1992/06/07 02:45:08 lidl Exp $

$Log: cosell.c,v $
 * Revision 2.4  1992/06/07  02:45:08  lidl
 * Post Adam Bryant patches and a manual merge of the rejects (ugh!)
 *
 * Revision 2.3  1991/02/10  13:50:18  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:31  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:09  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:15  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:13  aahz
 * Initial revision
 * 
*/

#include "xtank.h"
#include "cosell.h"
#include "bullet.h"
#include "message.h"
#include "globals.h"


extern Bset *bset;
extern char *teams_entries[];
extern int frame;
extern Map real_map;


/* How many slicks must be on the playfield before cosell takes notice */
#define TOO_MANY_SLICKS 6

/* Number of frames before we will comment on an occurrence happening again.
 * This helps to avoid flurries of messages from the commentator saying
 * the same thing over and over.
 */
#define SLICK_THRESH   100
#define CHANGE_THRESH    6
#define SMASH_THRESH    25
#define SLICKED_THRESH  20
#define BOBBLE_THRESH   50
#define WICKET_THRESH    4
#define CLEAR_THRESH    30

#define HOWARD_SAYS_A_LITTLE(data) \
    compose_message(SENDER_COM, RECIPIENT_ALL, OP_TEXT, \
		    (Byte *)(sprintf data,buf))

#define HOWARD_SAYS(data) \
  return (compose_message(SENDER_COM,RECIPIENT_ALL,OP_TEXT, \
		  (Byte*)(sprintf data,buf)), 1)

/*
 * Comment on a game of Ultimate or Capture.
 *
 * The opcodes and fields are:
 *   COS_INIT_MOUTH takes no arguments
 *
 *   COS_OWNER_CHANGE takes:
 *           a data field of COS_IGNORE or COS_WALL_HIT.
 *	     and vh1 which is the current disk owner.
 *
 *   COS_SLICK_DROPPED takes no arguments
 *
 *   COS_BIG_SMASH takes:
 *           a data field which includes the collision speed (0-50) or so
 *	     the two vehicles which collided
 *
 *   COS_GOAL_SCORED takes:
 *           the vehicle which scored
 *
 *   COS_BEEN_SLICKED:
 *           the vehicle which was slicked.
 */
comment(op, dat, vh1, vh2, db)
char op;
int dat;
Vehicle *vh1, *vh2;
Bullet *db;
{
    extern Vehicle *disc_old_owner(), *disc_last_owner(), *disc_cur_owner();
    static int slick_frame = 0;	/* frame when last slick was dropped */
    static int smash_frame;	/* frame when last big collision occurred */
    static int bobble_frame;	/* frame when owned disc last hit wall */
    static int change_frame;	/* frame when disc ownership last changed */
    static int clear_frame;	/* frame when disc clear message was given */
    static int slicked_frame[MAX_VEHICLES];
    Bullet *bul;
    Box *b;
    char buf[80];		/* Hold cosell messages */
    int i, j;

    if (slick_frame == -1031)
    {
	HOWARD_SAYS_A_LITTLE((buf, "And the tension runs high"));
	HOWARD_SAYS_A_LITTLE((buf, "as the players enter the"));
	HOWARD_SAYS_A_LITTLE((buf, "arena.  The fans are on"));
        HOWARD_SAYS_A_LITTLE((buf, "their many little feets."));
	HOWARD_SAYS_A_LITTLE((buf, "Remember, on any given day"));
	HOWARD_SAYS_A_LITTLE((buf, "any one tank can beat any"));
        HOWARD_SAYS_A_LITTLE((buf, "other tank."));
	slick_frame = -500;
        return 0;
    }
    else if (slick_frame == -1032)
    {
	HOWARD_SAYS_A_LITTLE((buf, "Well folks, after that"));
	HOWARD_SAYS_A_LITTLE((buf, "goal I think everyone"));
	HOWARD_SAYS_A_LITTLE((buf, "will be working a little"));
        HOWARD_SAYS_A_LITTLE((buf, "harder."));
	slick_frame = -500;
        return 0;
    }

    switch (op)
    {
      case COS_INIT_MOUTH:
	for (smash_frame = 0; smash_frame < MAX_VEHICLES; smash_frame++)
	    slicked_frame[smash_frame] = -1000;
	slick_frame = smash_frame = change_frame =
	  bobble_frame = clear_frame = -1032 + dat;
	break;
      case COS_OWNER_CHANGE:
	if (change_frame + CHANGE_THRESH > frame)
	{
	    if (bobble_frame + BOBBLE_THRESH < frame)
	    {
		bobble_frame = frame;
		HOWARD_SAYS_A_LITTLE((buf, "It's up in the air"));
	    }
	}
	change_frame = frame;
	if (vh1 == NULL) {
	    if (disc_last_owner(db) != NULL) {
		if (dat == COS_WALL_HIT) {
		    HOWARD_SAYS((buf, "%s screws up",
				 disc_last_owner(db)->disp));
		} else {
		    HOWARD_SAYS((buf, "It's a pass from %s",
				 disc_last_owner(db)->disp));
		}
	    }
	} else {
	    if (disc_last_owner(db) == NULL) {		/* was in the air */
		if (disc_old_owner(db) != NULL) {
		    if (disc_old_owner(db)->team == vh1->team)
			HOWARD_SAYS((buf, "Pass complete to %s", vh1->disp));
		    else
		      HOWARD_SAYS((buf, "Intercepted by %s", vh1->disp));
		} else {
		    HOWARD_SAYS((buf, "%s controls the disc", vh1->disp));
		}
	    } else {
		if (disc_last_owner(db)->team == vh1->team)
		    HOWARD_SAYS((buf, "Handoff to %s", vh1->disp));
		else
		    HOWARD_SAYS((buf, "Stolen by %s", vh1->disp));
	    }
	}
	break;
      case COS_SLICK_DROPPED:
	if (slick_frame + SLICK_THRESH < frame)
	    if (bset->number > TOO_MANY_SLICKS * num_veh_alive)
		if (rnd(6))
		{
		    slick_frame = frame;
		    HOWARD_SAYS((buf, "Wessonality prevails"));
		}
	break;
      case COS_BIG_SMASH:
	if (rnd(3)) {
	    if (dat>40)
		HOWARD_SAYS((buf,"%s + %s go BOOM",vh1->disp,vh2->disp));
	    else if (dat>20)
		HOWARD_SAYS((buf,"%s + %s bump heads", vh1->disp, vh2->disp));
	    else if (dat>5 && !rnd(3))
		HOWARD_SAYS((buf,"%s + %s hit fenders", vh1->disp,
			     vh2->disp));
	}
	break;
      case COS_GOAL_SCORED:
	HOWARD_SAYS((buf, "Goal scored by %s", vh1->disp));
      case COS_BEEN_SLICKED:
	if (slicked_frame[vh1->number] + SLICKED_THRESH < frame)
	{
	    slicked_frame[vh1->number] = frame;
	    HOWARD_SAYS((buf, "%s's been slicked", vh1->disp));
	}
	break;
      default:
	HOWARD_SAYS((buf, "I'm sooooo confused"));
    }

    /* If we decided not to comment on the particular there is a chance we'll
       say something else, pretty clever of us if I say so ourselves. */
    if (!rnd(3))
    {
	if ((vh1 = disc_cur_owner(db)) == NULL)
	{			/* This will tell if discs are free in goal */
	    for (i = 0; i < bset->number; i++)
	    {
		bul = bset->list[i];
		if (bul->type == DISC)
		{
		    b = &real_map[bul->loc->grid_x][bul->loc->grid_y];
		    if (b->type == GOAL)
			HOWARD_SAYS((buf, "It's loose in a %s goal!",
				     teams_entries[b->team]));
		}
	    }
	} else if (clear_frame + CLEAR_THRESH > frame) {
	    /* Do they have it in a goal */
	    b = &real_map[bul->loc->grid_x][bul->loc->grid_y];
	    if ((b->type == GOAL) &&
		(vh1->team != b->team)) {
		clear_frame = frame;
		HOWARD_SAYS((buf, "%s better clear the disc",
			     vh1->disp));
	    }
	}
    }

    return 0;
}
