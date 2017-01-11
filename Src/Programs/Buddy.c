/*
** Xtank
**
** Copyright 1990 by Daniel Schmidt; All rights reserved.
**
** $Id$
*/

/*
** thatsBUDDYtoyoubuddy
**
** An ultimate playing robot for xtank.
**
** This file is Copyright 1990 by Daniel Schmidt; All rights reserved.
**
** You are free to modify this file as long as you make clear
** what code is yours and what is mine.
**
*/

#include "xtanklib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifdef __GNUC__
#ifndef __STDC__
#define Inline __inline
#else
#define Inline inline
#endif
#else
#define Inline
#endif

/* States: currently only two */
#define GOT_DISC      0			/* I have the disc */
#define CHAOS         1			/* I don't have the disc */

#define DISC_FRAMES 20			/* How many frames of lookahead when chasing
                                   a disc */

#define THROW_THRESH .2			/* A magic number which determines how near
                                   the disc has to be to the "perfect"
                                   angle at which we'd like to throw it */

#define VEHICLE_TOO_CLOSE 150	/* To know when to play keepaway */

#define MAX_DISC_SPEED 25.0		/* Maximum speed at which I can throw
				   a disc */

#define DISC_CLOSE_SQ 800		/* If disc is this close to me, it's too close
                                   to be in orbit */
#define DISC_ORBIT_SQ 1600.0

#define HALF_BOX_WIDTH (BOX_WIDTH/2)
#define HALF_BOX_HEIGHT (BOX_HEIGHT/2)

#define MAX_FILL_DEPTH 12		/* Default depth for shortest-path searches */

/* Functions/Macros */
#define sqr(x)        ((x)*(x))
#define sqrtint(x)    (int) (sqrt((double)(x)))
#define fix_angle(x)  (x -= (2*PI) * floor(x/(2*PI)))
#define asinint(x)    (Angle) asin((double)(x))
#define acosint(x)    (Angle) acos((double)(x))

#define Buddy_spin_discs(x) (spin_discs(x), bi->spin = (x))
#define Buddy_wall_east(bi,x,y) map_west_result(((bi)->map)[(x)+1][y].flags)
#define Buddy_wall_west(bi,x,y) map_west_result(((bi)->map)[x][y].flags)
#define Buddy_wall_north(bi,x,y) map_north_result(((bi)->map)[x][y].flags)
#define Buddy_wall_south(bi,x,y) map_north_result(((bi)->map)[x][(y)+1].flags)

/* Structs */

typedef struct {		/* Information about goals */
	Location loc;		/* Location of goal  */
	Team team;		/* Team the goal is on */
} Goal_info;

typedef struct {		/* Info for calculating shortest paths */
	char nextx, nexty;	/* The next box I should travel to */
	Boolean seen;		/* If I know how to get from here to my
				   destination */
	char dist;		/* Distance from here to the destination */
} Mazebox;

/* Binfo contains all data which would otherwise be global.  We need
   to keep it in one structure and pass it around from function to
   function so that multiple copies of Buddy don't use the same data. */

typedef struct {
	Angle desireddir;	/* what angle I'd like to go at */
	float maxspeed;		/* max possible speed */

	  /* Here I store everything I know about the world */
	  Vehicle_info vinfo[MAX_VEHICLES];
	  Blip_info blinfo[MAX_BLIPS];
	  Disc_info dinfo[MAX_DISCS];
	  Goal_info ginfo[MAX_LANDMARKS];
	  Landmark_info linfo[MAX_LANDMARKS];
	  Settings_info settings;

	  int numvehicles;
	  int numblips;
	  int numdiscs;
	  int numgoals;

	  Message msg;				/* Last received message */

	  /* Map containing information about walls and landmarks */
	  Box (*map)[GRID_HEIGHT];

	  /* Maps containing information about how to get from one place
      to another.  `maze' is for calculating paths to the enemy goal;
      `scrapmaze' is for calculating all other paths. */
	  Mazebox maze[GRID_WIDTH][GRID_HEIGHT];
	  Mazebox scrapmaze[GRID_WIDTH][GRID_HEIGHT];

	  /* Global space for arrays used in Buddy_compute_path */
	  Coord cutting_edge[100], cutting_edge2[100];

	  /* The last destination I tried to find a path to */
	  Coord lastfillorigin;

	  /* Array of locations I should throw the disc at when I
      want to throw it in a goal. */
	  int num_goal_locs;
	  Location goal_locs_1[4];
	  Location goal_locs_2[4];

	  Vehicle_info me;		/* What I am is what I am */

	  int frame;			/* Current frame of the game */
	  int next_frame;		/* Frame after this */

	  int discowned;		/* How many discs I own */
	  Spin spin;			/* What way I'm spinning discs */
	  int mode;			/* What state I'm in */
	  int men_on;			/* Enemy vehicles near me */
	  int closestgoal;		/* Closest goal to me */
	  int enemy_goal;		/* Number of enemy goal */
	Boolean throwing;		/* If I'm in the middle of throwing,
					   don't be distracted */
} Binfo;

/* Function prototypes */
static void Buddy_main(void);
static void Buddy_clear_the_disc(Binfo *);
static int Buddy_compute_path(Binfo *, int, int, int, int,
        Mazebox maze[GRID_WIDTH][GRID_HEIGHT], int);
static int Buddy_compute_path_to_square(Binfo *, int, int, int, int);
static void Buddy_deal_with_disc(Binfo *);
static void Buddy_deal_with_messages(Binfo *);
static void Buddy_find_goals(Binfo *);
static void Buddy_find_the_action(Binfo *);
static int Buddy_get_clear_range(Binfo *, Location *, Location *);
static void Buddy_go_get_the_disc(Binfo *);
static void Buddy_go_to_box(Binfo *, int, int);
static void Buddy_go_to_goal(Binfo *);
static void Buddy_maybe_pass(Binfo *, int);
static void Buddy_move_into_box(Binfo *, int, int);
static void Buddy_panic(Binfo *);
static void Buddy_play_keep_away(Binfo *);
static void Buddy_set_open_goal_locs(Binfo *, Goal_info *);
static void Buddy_throw_at_goal(Binfo *);
static void Buddy_throw_at_loc(Binfo *, Location *, float);
static void Buddy_throw_in_range(Binfo *, Location *, Location *, int, float);
static void Buddy_wander(Binfo *);
static void Buddy_sulk(void);

/* The data about the Buddy program */

Prog_desc Buddy_prog =
{
	"Buddy",
	"Ultimate7",
	"Buddy is for playing ultimate and being a general nuisance. \
Use him only in mazes where there is only one enemy goal. \
Make sure to set Full Map on in the Settings menu.\
Give him a tank with a mapper and radar.  I recommend Ultimate7.",
	"Dan Schmidt",
	PLAYS_ULTIMATE | USES_TEAMS | USES_MESSAGES,
	8,
	Buddy_main
};

/* Buddy_show_off(size)
**
** Tells everybody how studly I am.
*/
static void Inline
Buddy_show_off(int size)
{
	char text[MAX_DATA_LEN];

	sprintf(text, "%d bytes, beat that", size);
	send_msg(RECIPIENT_ALL, OP_TEXT, (Byte *) text);
}

/* Buddy_init_bi(bi)
**
** Initializes the Binfo structure.
*/
static void Inline
Buddy_init_bi(Binfo *bi)
{
	get_settings(&bi->settings);
	if (bi->settings.full_map)
		bi->map = map_get();
	else
		Buddy_sulk();
	bi->throwing = FALSE;
	bi->frame = 0;
	bi->next_frame = -1;
	bi->maxspeed = max_speed();
	bi->numgoals = 0;
	bi->enemy_goal = -1;
	bi->men_on = 0;
	bi->num_goal_locs = 0;
	bi->lastfillorigin.x = -1;
	bi->spin = CLOCKWISE;
	spin_discs(CLOCKWISE);
}

/* Buddy_sulk(bi)
**
** Complains that I haven't been given the full map,
** and refuses to play any more.
*/
static void Inline
Buddy_sulk(void)
{
	char text[MAX_DATA_LEN];

	strcpy(text, "GIVE ME A FULL MAP NOW.");
	send_msg(RECIPIENT_ALL, OP_TEXT, (Byte *) text);

	set_abs_drive(0.0);
	while (TRUE)
		done();
}

/* Buddy_initialize_maze(bi)
**
** Initializes the internal maze representation.
*/
static void Inline
Buddy_initialize_maze(Binfo *bi)
{
	int i, j;

	for (i = 0; i < GRID_WIDTH; ++i) {
		for (j = 0; j < GRID_HEIGHT; ++j) {
			bi->maze[i][j].seen = FALSE;
		}
	}
}

/* Buddy_main()
**
** The main procedure.
** Initializes stuff and then goes through an infinite loop until the
** game is reset for whatever reason.
*/
static void
Buddy_main(void)
{
	Binfo bi;					/* Everything you ever wanted to know */

	Buddy_initialize_maze(&bi);	/* Get the maze set up */
	Buddy_init_bi(&bi);			/* Initialize the bi structure */
	set_safety(FALSE);			/* No safe turns here */
	set_rel_drive(9.0);			/* Let's cruise */
	Buddy_show_off(sizeof(Binfo));

	get_self(&bi.me);
	Buddy_find_goals(&bi);		/* Find where the goals are */
	while (TRUE) {
		bi.frame = frame_number();

		/* Am I on the next frame yet? */
		if (bi.frame >= bi.next_frame) {
			/* Get external information */
			bi.next_frame = bi.frame + 1;
			get_self(&bi.me);
			get_vehicles(&bi.numvehicles, bi.vinfo);
			get_discs(&bi.numdiscs, bi.dinfo);
			bi.discowned = num_discs();
			bi.mode = (bi.discowned) ? (GOT_DISC) : (CHAOS);

			Buddy_deal_with_messages(&bi);

			/* Decide how to react based on my environment */
			switch (bi.mode) {
			  case GOT_DISC:
				  Buddy_deal_with_disc(&bi);
				  break;
			  case CHAOS:
				  set_rel_drive(9.0);
				  bi.throwing = FALSE;
				  if (bi.numdiscs)
					  Buddy_go_get_the_disc(&bi);
				  else
					  Buddy_find_the_action(&bi);
				  break;
			  default:
				  Buddy_panic(&bi);
				  break;
			}
		} else
			done();				/* Wait until the world changes */
	}
}

/* Buddy_deal_with_messages(bi)
**
** Deals with any messages that haven't been read yet.
** Currently, the message passing is used solely to pass information
** about the disc:
**   If I don't know where the disc is, ask my team.
**   If I do know where the disc is, and someone asked, tell them.
**   If someone told me where the disc is, store this information.
*/
static void
Buddy_deal_with_messages(Binfo *bi)
{
	int i;						/* counter */
	int msgs;					/* number of messages in queue */
	char data[MAX_DATA_LEN];	/* message to send */

	msgs = messages();

	if (msgs) {
		for (i = 0; i < msgs; ++i) {
			receive_msg(&bi->msg);
			if ((!bi->numdiscs) && (bi->msg.opcode == OP_LOCATION)) {
				/* It's a message telling me where the disc is */
				bi->numdiscs = 1;
				bi->dinfo[0].owner = NO_OWNER;
				bi->dinfo[0].loc.grid_x = bi->msg.data[0];
				bi->dinfo[0].loc.grid_y = bi->msg.data[1];
				bi->dinfo[0].loc.x = bi->msg.data[0] * BOX_WIDTH + HALF_BOX_WIDTH;
				bi->dinfo[0].loc.y = bi->msg.data[1] * BOX_HEIGHT + HALF_BOX_HEIGHT;
				bi->dinfo[0].xspeed = 0;
				bi->dinfo[0].yspeed = 0;
			}
			if ((bi->numdiscs) && (bi->msg.opcode == OP_HELP)) {
				/* Respond to the call for help */
				data[0] = bi->dinfo[0].loc.grid_x;
				data[1] = bi->dinfo[0].loc.grid_y;
				send_msg(bi->msg.sender, OP_LOCATION, (Byte *) data);
			}
		}
	}
	if (!bi->numdiscs)
		/* Ask for help */
		send_msg(MAX_VEHICLES + bi->me.team, OP_HELP, (Byte *) data);
}

/* Buddy_find_goals(bi)
**
** This function is called only once, at the beginning of the game.
** It stores the location of all the goals, and then calculates the
** shortest path to the enemy goal for every square in the maze
** (within a reasonable distance).
**
** Assumptions:
**   We know everything about the current maze.
**   There is only one enemy_goal.
*/
static void
Buddy_find_goals(Binfo *bi)
{
	int i;						/* counter */
	Goal_info *gi;				/* current goal I'm looking at */
	Landmark_info *li;			/* current landmark */
	int numlandmarks;			/* how many landmarks there are */

	get_landmarks(&numlandmarks, bi->linfo);
	if (numlandmarks) {
		for (i = 0; i < numlandmarks; ++i) {
			if (bi->linfo[i].type == GOAL) {
				gi = &bi->ginfo[bi->numgoals];
				li = &bi->linfo[i];
				gi->team = li->team;
				gi->loc.grid_x = li->x;
				gi->loc.grid_y = li->y;
				gi->loc.x = gi->loc.grid_x * BOX_WIDTH + HALF_BOX_WIDTH;
				gi->loc.y = gi->loc.grid_y * BOX_HEIGHT + HALF_BOX_HEIGHT;
				if (gi->team != bi->me.team) {	/* Is it an enemy goal? */
					bi->enemy_goal = bi->numgoals;
					Buddy_set_open_goal_locs(bi, gi);
				}
				bi->numgoals++;
			}
		}
	}
	/* If there are no enemy goals, what's the point of playing? */
	if (bi->enemy_goal == -1) {
		printf("No enemy goals!\n");
		set_abs_drive(0.0);
		while (TRUE)
			done();
	} else
		/* Find the shortest path from the enemy goal to all squares which
       are 30 or few squares away from it. */
		Buddy_compute_path(bi,
						   -1, -1,	/* so it won't find a path to me and exit */
						   bi->ginfo[bi->enemy_goal].loc.grid_x,
						   bi->ginfo[bi->enemy_goal].loc.grid_y,
						   bi->maze,
						   30);	/* maximum depth of search */
}

/* Buddy_set_open_goal_locs(bi,gi)
**
** Computes locations to aim for when I want to throw a disc
** into the goal.  One location is at the left of the opening
** of the goal, and the other is at the right.  Later,
** Buddy_throw_at_goal will attempt to throw between the two
** points.
*/
static void
Buddy_set_open_goal_locs(Binfo *bi, Goal_info *gi)
{
	Location *g1, *g2;

	if (!Buddy_wall_north(bi, gi->loc.grid_x, gi->loc.grid_y)) {
		g1 = &bi->goal_locs_1[bi->num_goal_locs];
		g2 = &bi->goal_locs_2[bi->num_goal_locs++];
		g1->grid_x = gi->loc.grid_x;
		g1->grid_y = gi->loc.grid_y;
		g2->grid_x = gi->loc.grid_x;
		g2->grid_y = gi->loc.grid_y;
		g1->x = gi->loc.x - HALF_BOX_WIDTH + 1;
		g2->x = gi->loc.x + HALF_BOX_WIDTH - 1;
		g1->y = gi->loc.y - HALF_BOX_WIDTH + 1;
		g2->y = gi->loc.y - HALF_BOX_WIDTH + 1;
	}
	if (!Buddy_wall_south(bi, gi->loc.grid_x, gi->loc.grid_y)) {
		g1 = &bi->goal_locs_1[bi->num_goal_locs];
		g2 = &bi->goal_locs_2[bi->num_goal_locs++];
		g1->grid_x = gi->loc.grid_x;
		g1->grid_y = gi->loc.grid_y;
		g2->grid_x = gi->loc.grid_x;
		g2->grid_y = gi->loc.grid_y;
		g1->x = gi->loc.x - HALF_BOX_WIDTH + 1;
		g2->x = gi->loc.x + HALF_BOX_WIDTH - 1;
		g1->y = gi->loc.y + HALF_BOX_WIDTH - 1;
		g2->y = gi->loc.y + HALF_BOX_WIDTH - 1;
	}
	if (!Buddy_wall_west(bi, gi->loc.grid_x, gi->loc.grid_y)) {
		g1 = &bi->goal_locs_1[bi->num_goal_locs];
		g2 = &bi->goal_locs_2[bi->num_goal_locs++];
		g1->grid_x = gi->loc.grid_x;
		g1->grid_y = gi->loc.grid_y;
		g2->grid_x = gi->loc.grid_x;
		g2->grid_y = gi->loc.grid_y;
		g1->x = gi->loc.x - HALF_BOX_WIDTH + 1;
		g2->x = gi->loc.x - HALF_BOX_WIDTH + 1;
		g1->y = gi->loc.y - HALF_BOX_WIDTH + 1;
		g2->y = gi->loc.y + HALF_BOX_WIDTH - 1;
	}
	if (!Buddy_wall_east(bi, gi->loc.grid_x, gi->loc.grid_y)) {
		g1 = &bi->goal_locs_1[bi->num_goal_locs];
		g2 = &bi->goal_locs_2[bi->num_goal_locs++];
		g1->grid_x = gi->loc.grid_x;
		g1->grid_y = gi->loc.grid_y;
		g2->grid_x = gi->loc.grid_x;
		g2->grid_y = gi->loc.grid_y;
		g1->x = gi->loc.x + HALF_BOX_WIDTH - 1;
		g2->x = gi->loc.x + HALF_BOX_WIDTH - 1;
		g1->y = gi->loc.y - HALF_BOX_WIDTH + 1;
		g2->y = gi->loc.y + HALF_BOX_WIDTH - 1;
	}
}

/* Buddy_panic(bi)
**
** This routine is only called in case of some internal error.
** And that never happens, right?
*/
static void
Buddy_panic(Binfo *bi)
{
	char buf[MAX_DATA_LEN];

	strcpy(buf, "Hey, who is this anyway");
	send_msg(RECIPIENT_ALL, OP_TEXT, (Byte *) buf);
	bi->mode = CHAOS;
}

/* Buddy_deal_with_disc(bi)
**
** Performs the appropriate actions when I have the disc.
** Currently this consists of:
**   Heading for the enemy goal.
**   Throwing the disc into the enemy goal if possible.
**   Passing to a teammate if it seems like a good idea.
**   Preventing enemy vehicles from stealing the disc.
**   Clearing the disc if I'm in trouble.
*/
static void Inline
Buddy_deal_with_disc(Binfo *bi)
{
	Buddy_go_to_goal(bi);
	if (bi->discowned)
		Buddy_throw_at_goal(bi);
	Buddy_maybe_pass(bi, BOX_WIDTH * BOX_WIDTH * 2);
	Buddy_play_keep_away(bi);
	if (bi->men_on > 1)
		Buddy_clear_the_disc(bi);
}

/* Buddy_throw_at_goal(bi)
**
** Throws the disc into the enemy goal.  It looks for open sides of
** goals (as stored by Buddy_set_open_goal_locs) and attempts to pass
** into them.
*/
static void Inline
Buddy_throw_at_goal(Binfo *bi)
{
	int i;						/* counter */
	Boolean cp1, cp2;			/* clear path to loc 1?  loc 2? */

	for (i = 0; i < bi->num_goal_locs; ++i) {
		/* See if I have a chance of throwing it in */
		cp1 = clear_path(&bi->dinfo[0].loc, &bi->goal_locs_1[0]);
		cp2 = clear_path(&bi->dinfo[0].loc, &bi->goal_locs_2[0]);
		if (cp1 || cp2) {
			bi->throwing = TRUE;
			/* If I have a clear path to one end but not the other, the
	 throw's going to be tricky (thus the !(cp1 && cp2) argument) */
			Buddy_throw_in_range(bi, &bi->goal_locs_1[0], &bi->goal_locs_2[0],
								 !(cp1 && cp2), MAX_DISC_SPEED);
			return;
		}
	}
}

/* Buddy_go_to_goal(bi)
**
** Goes to the enemy goal, taking the shortest path.
*/
static void Inline
Buddy_go_to_goal(Binfo *bi)
{
	int mx, my;					/* My location on the grid */

	mx = bi->me.loc.grid_x;
	my = bi->me.loc.grid_y;

	/* Do I know how to get to the goal from here? */
	if (bi->maze[mx][my].seen)
		Buddy_move_into_box(bi, bi->maze[mx][my].nextx, bi->maze[mx][my].nexty);
	else
		Buddy_wander(bi);
}

/* Buddy_maybe_pass(bi,mindistsqr)
**
** Passes the disc to a teammate if it seems like a good idea.
**
** Current criteria are:
**   There must be a clear path to the teammate.
**   The teammate's distance squared must be > mindistsqr.
**   The teammate must be closer to the enemy goal than I am.
**
** There should probably be rules about
**   Am I double-teamed?
**   Is my teammate open? (no enemies near him)
*/
static void
Buddy_maybe_pass(Binfo *bi, int mindistsqr)
{
	int i;						/* counter; current vehicle */
	int dx, dy;					/* deltas to current vehicle */
	int dist;					/* distance to current vehicles */
	int distsqr;				/* square of distance to current vehicle */

	/* Don't throw to a teammate if I'm in the middle of throwing at a goal. */
	if (bi->numvehicles && !bi->throwing) {
		for (i = 0; i < bi->numvehicles; ++i) {
			/* on my team? */
			if (bi->vinfo[i].team == bi->me.team) {
				/* only pass if he's closer to the enemy goal than I am */
				if (bi->maze[bi->vinfo[i].loc.grid_x][bi->vinfo[i].loc.grid_y].seen &&
					bi->maze[bi->vinfo[i].loc.grid_x][bi->vinfo[i].loc.grid_y].dist <
					bi->maze[bi->me.loc.grid_x][bi->me.loc.grid_y].dist) {
					/* is there a clear path to him? */
					if (clear_path(&bi->dinfo[0].loc, &bi->vinfo[i].loc)) {
						dx = bi->vinfo[i].loc.x - bi->me.loc.x;
						dy = bi->vinfo[i].loc.y - bi->me.loc.y;
						distsqr = sqr(dx) + sqr(dy);
						/* Pass to him if he's not too close to me. */
						if (distsqr > mindistsqr) {
							dist = sqrtint(distsqr);
							/* Throw a lead pass.  This could probably be a lot better. */
							bi->vinfo[i].loc.x += ((int) bi->vinfo[i].xspeed) * dist / 20;
							bi->vinfo[i].loc.y += ((int) bi->vinfo[i].yspeed) * dist / 20;
							Buddy_throw_at_loc(bi, &bi->vinfo[i].loc, MAX_DISC_SPEED);
						}
					}
				}
			}
		}
	}
}

/* Buddy_play_keep_away(bi)
**
** Makes sure no one steals the disc.
** Also sets bi->men_on, the number of enemy vehicles near me.
*/
static void
Buddy_play_keep_away(Binfo *bi)
{
	int i;						/* counter; current vehicle */
	int dx, dy;					/* deltas of current vehicle */
	int distsqr;				/* square of distance to current vehicle */
	int cdx, cdy;				/* deltas of closest vehicle */
	int closestdistsqr = VEHICLE_TOO_CLOSE * VEHICLE_TOO_CLOSE;

	/* how close is too close */
	Angle delta;				/* Angle between enemy vehicle and the disc */

	bi->men_on = 0;
	if (bi->numvehicles)
		for (i = 0; i < bi->numvehicles; ++i) {
			/* Is it an enemy? */
			if (bi->vinfo[i].team != bi->me.team) {
				/* Look ahead one frame on his position */
				dx = bi->vinfo[i].loc.x + (int) bi->vinfo[i].xspeed - bi->me.loc.x;
				dy = bi->vinfo[i].loc.y + (int) bi->vinfo[i].yspeed - bi->me.loc.y;
				distsqr = (sqr(dx) + sqr(dy));
				if (distsqr < VEHICLE_TOO_CLOSE * VEHICLE_TOO_CLOSE)
					++bi->men_on;
				if (distsqr < closestdistsqr) {
					closestdistsqr = distsqr;
					cdx = dx;
					cdy = dy;
				}
			}
		}
	if (bi->men_on) {
		delta = bi->dinfo[0].angle - ATAN2(cdy, cdx);
		fix_angle(delta);
		/* Spin the discs so as to maximize delta */
		if (delta < PI)
			Buddy_spin_discs(CLOCKWISE);
		else
			Buddy_spin_discs(COUNTERCLOCKWISE);
	}
}

/* Buddy_clear_the_disc(bi)
**
** Clears the disc in a generally good direction.
**
** Unless I'm real close to a goal, I find out a good area to throw
** the disc in, and then throw it in that direction.
*/
static void
Buddy_clear_the_disc(Binfo *bi)
{
	Location loc1, loc2;		/* endpoints of range */

	if (!(bi->maze[bi->me.loc.grid_x][bi->me.loc.grid_y].seen &&
		  bi->maze[bi->me.loc.grid_x][bi->me.loc.grid_y].dist < 3))
		if (Buddy_get_clear_range(bi, &loc1, &loc2))
			Buddy_throw_in_range(bi, &loc1, &loc2, FALSE, MAX_DISC_SPEED);
}

/* Buddy_get_clear_range(bi,loc1,loc2)
**
** Calculates two locations that a clearing throw should pass between.
** These locations are put in loc1 and loc2.
**
** I look ahead two squares towards the goal, and try to throw in
** that general direction.
*/
static int
Buddy_get_clear_range(Binfo *bi, Location *loc1, Location *loc2)
{
	int mx, my;					/* my grid coordinate */
	int gx, gy;					/* next square in path to goal */
	int gx2, gy2;				/* next square after that */

	mx = bi->me.loc.grid_x;
	my = bi->me.loc.grid_y;

	if (!bi->maze[mx][my].seen)
		return (FALSE);

	gx = bi->maze[mx][my].nextx;
	gy = bi->maze[mx][my].nexty;

	gx2 = bi->maze[gx][gy].nextx;
	gy2 = bi->maze[gx][gy].nexty;

#define set_locs(x1,y1,x2,y2) \
  { loc1->grid_x = (x1); loc1->grid_y = (y1); \
    loc2->grid_x = (x2); loc2->grid_y = (y2); }

	/* Now comes the massive special casing.  Basically, I want to
     throw a disc so that it will go through (gx2,gy2).  Walls
     between me and that location may either be good (by making bounce
     passes possible) or bad (by blocking the path to the location. */

	if (gy2 == gy)
		if (gx2 == mx + 2)		/* east */
			if (Buddy_wall_north(bi, gx, gy))
				if (Buddy_wall_south(bi, gx, gy))
					set_locs(gx, gy, gx, gy + 1)
					  else
					set_locs(gx, gy, gx + 1, gy + 1)
					  else
					if (Buddy_wall_south(bi, gx, gy))
						set_locs(gx + 1, gy, gx, gy + 1)
						  else
						set_locs(gx + 1, gy, gx + 1, gy + 1)
						  else	/* west */
						if (Buddy_wall_north(bi, gx, gy))
							if (Buddy_wall_south(bi, gx, gy))
								set_locs(gx + 1, gy, gx + 1, gy + 1)
								  else
								set_locs(gx + 1, gy, gx, gy + 1)
								  else
								if (Buddy_wall_south(bi, gx, gy))
									set_locs(gx, gy, gx + 1, gy + 1)
									  else
									set_locs(gx, gy, gx, gy + 1)
									  else
									if (gx2 == mx + 1)
										if (gy2 == my + 1)	/* southeast */
											if (Buddy_wall_north(bi, gx2, gy2))
												set_locs(gx2, gy2, gx2, gy2 + 1)
												  else
												if (Buddy_wall_west(bi, gx2, gy2))
													set_locs(gx2, gy2, gx2 + 1, gy2)
													  else
													set_locs(gx2, gy2 + 1, gx2 + 1, gy2)
													  else	/* northeast */
													if (Buddy_wall_south(bi, gx2, gy2))
														set_locs(gx2, gy2 + 1, gx2, gy2)
														  else
														if (Buddy_wall_west(bi, gx2, gy2))
															set_locs(gx2 + 1, gy2 + 1, gx2, gy2 + 1)
															  else
															set_locs(gx2, gy2, gx2 + 1, gy2 + 1)
															  else
															if (gx2 == mx)
																if (gy2 == my + 2)	/* south */
																	if (Buddy_wall_west(bi, gx, gy))
																		if (Buddy_wall_east(bi, gx, gy))
																			set_locs(gx, gy, gx + 1, gy)
																			  else
																			set_locs(gx, gy, gx + 1, gy + 1)
																			  else
																			if (Buddy_wall_east(bi, gx, gy))
																				set_locs(gx, gy + 1, gx + 1, gy)
																				  else
																				set_locs(gx, gy + 1, gx + 1, gy + 1)
																				  else	/* north */
																				if (Buddy_wall_west(bi, gx, gy))
																					if (Buddy_wall_east(bi, gx, gy))
																						set_locs(gx, gy + 1, gx + 1, gy + 1)
																						  else
																						set_locs(gx, gy + 1, gx + 1, gy)
																						  else
																						if (Buddy_wall_east(bi, gx, gy))
																							set_locs(gx, gy, gx + 1, gy + 1)
																							  else
																							set_locs(gx, gy, gx + 1, gy)
																							  else
																							if (gx2 == mx - 1)
																								if (gy2 == my + 1)	/* southwest */
																									if (Buddy_wall_north(bi, gx2, gy2))
																										set_locs(gx2 + 1, gy2, gx2 + 1, gy2 + 1)
																										  else
																										if (Buddy_wall_east(bi, gx2, gy2))
																											set_locs(gx2, gy2, gx2 + 1, gy2)
																											  else
																											set_locs(gx2, gy2, gx2 + 1, gy2 + 1)
																											  else	/* northwest */
																											if (Buddy_wall_south(bi, gx2, gy2))
																												set_locs(gx2 + 1, gy2, gx2 + 1, gy2 + 1)
																												  else
																												if (Buddy_wall_east(bi, gx2, gy2))
																													set_locs(gx2, gy2 + 1, gx2 + 1, gy2 + 1)
																													  else
																													set_locs(gx2, gy2 + 1, gx2 + 1, gy2)
																													  else {
																													printf("I give up: <%d %d>  <%d %d>  <%d %d>\n",
																														   mx, my, gx, gy, gx2, gy2);
																													return (FALSE);
																													}
	/* Fill up the rest of the location structures */
	loc1->x = loc1->grid_x * BOX_WIDTH;
	loc1->y = loc1->grid_y * BOX_HEIGHT;
	loc2->x = loc2->grid_x * BOX_WIDTH;
	loc2->y = loc2->grid_y * BOX_HEIGHT;

#if 0
	printf("<%d %d> - <%d %d> - <%d %d>\n",
		   mx, my, gx, gy, gx2, gy2);
	printf("throw at: <%d %d> - <%d %d>\n",
		   loc1->grid_x, loc1->grid_y, loc2->grid_x, loc2->grid_y);
#endif /* 0 */

	return (TRUE);
}

/* Buddy_throw_at_loc(bi,loc,speed)
**
** Throws the disc at the location loc with speed speed.
*/
static void
Buddy_throw_at_loc(Binfo *bi, Location *loc, float spd)
{
	Vehicle_info *me;			/* me */
	Disc_info *disc;			/* the disc */
	Angle angle_to_loc;			/* angle from disc to location */
	Angle angle_to_me;			/* angle from disc to me */
	Angle disc_angle;			/* angle of disc's current velocity */
	Angle diff;					/* difference between disc's direction
				   and direction to the location */
	Angle new_disc_angle;		/* disc's angle next frame */
	Angle delta;				/* for updating disc, ripped from xtank */
	Angle diff2;				/* diff for next frame */
	int dx, dy;					/* deltas from disc to me */
	float dist;					/* distance squared from disc to me */

	/* Get some information */
	me = &(bi->me);
	disc = &(bi->dinfo[0]);
	angle_to_me = ATAN2(me->loc.y - disc->loc.y, me->loc.x - disc->loc.x);
	angle_to_loc = ATAN2(loc->y - disc->loc.y, loc->x - disc->loc.x);
	disc_angle = ATAN2(disc->yspeed, disc->xspeed);
	diff = angle_to_loc - disc_angle;
	if (diff < 0)
		diff += 2 * PI;

#if 0
	printf("to me: %.2f   disc: %.2f   to loc: %.2f   diff: %.2f\n",
		   angle_to_me, disc_angle, angle_to_loc, diff);
#endif

	/* Throw the disc if disc_angle is close enough to angle_to_loc */
	if (diff < THROW_THRESH || diff > 2 * PI - THROW_THRESH) {
		throw_discs(spd, FALSE);
		bi->throwing = FALSE;
		return;
	}
	/* Predict the status of the disc next frame.  This code is stolen
     directly from update_disc in xtank.  So sue me; I figure I should
     be allowed to know the physics of the world. */
	dx = me->loc.x - disc->loc.x;
	dy = me->loc.y - disc->loc.y;
	dist = dx * dx + dy * dy;
	delta = (dist <= DISC_ORBIT_SQ) ? PI / 2 * (2 - (dist / DISC_ORBIT_SQ))
	  : PI / 2 * (DISC_ORBIT_SQ / dist);
	new_disc_angle = (bi->spin == COUNTERCLOCKWISE) ? angle_to_me + delta
	  : angle_to_me - delta;
	diff2 = new_disc_angle - angle_to_loc;
	fix_angle(diff2);

#if 0
	printf("new disc: %.2f    new diff: %.2f\n", new_disc_angle, diff2);
#endif

	/* If things will be good next frame, throw it then */
	if (diff2 < THROW_THRESH || diff2 > 2 * PI - THROW_THRESH) {
		throw_discs(spd, TRUE);
		bi->throwing = FALSE;
		return;
	}
	/* Spin the disc towards a better location.  To understand why this
     code works, draw a couple of sample situations and try it out. */
	if (diff < PI / 2)
		Buddy_spin_discs(CLOCKWISE);
	else if (diff < PI)
		Buddy_spin_discs(COUNTERCLOCKWISE);
	else if (diff < 3 * PI / 2)
		Buddy_spin_discs(CLOCKWISE);
	else
		Buddy_spin_discs(COUNTERCLOCKWISE);
}

/* Buddy_throw_in_range(bi,loc1,loc2,tricky,spd)
**
** Throws the disc at the specified speed so that it will pass
** between loc1 and loc2.
**
** If tricky is true, it means that not all locations between loc1
** and loc2 are reachable from me (i.e. there are walls in the way),
** so I have to be careful that the disc can actually get where I'm
** throwing it.
*/
static void
Buddy_throw_in_range(Binfo *bi, Location *loc1, Location *loc2, int tricky, float spd)
{
	Vehicle_info *me;			/* me */
	Disc_info *disc;			/* the disc */
	Angle angle_to_loc1;		/* angle from disc to loc1 */
	Angle angle_to_loc2;		/* angle from disc to loc2 */
	Angle angle_to_me;			/* angle from disc to me */
	Angle disc_angle;			/* angle of disc's current velocity */
	Angle tmp_angle;			/* for switching angle_to_loc1&2 */
	Angle width;				/* angular width of viable throws */
	Angle new_disc_angle;		/* disc's angle next frame */
	Angle good_angle;			/* an excellent throw */
	Angle diff;					/* difference between disc's angle and
				   ideal angle */
	Angle delta;				/* for updating disc, ripped from xtank */
	Location dest;				/* where the disc will go after I releas it */
	float frac;					/* 0.0 if dest is loc1, 1.0 if dest is loc2,
				   .5 if it's halfway between, etc. */
	int dx, dy;					/* deltas from disc to me */
	float dist;					/* distance squared from disc to me */

	/* Get some information */
	me = &(bi->me);
	disc = &(bi->dinfo[0]);
	angle_to_me = ATAN2(me->loc.y - disc->loc.y, me->loc.x - disc->loc.x);
	angle_to_loc1 = ATAN2(loc1->y - disc->loc.y, loc1->x - disc->loc.x);
	angle_to_loc2 = ATAN2(loc2->y - disc->loc.y, loc2->x - disc->loc.x);
	disc_angle = ATAN2(disc->yspeed, disc->xspeed);
	if (disc_angle < 0)
		disc_angle += 2 * PI;

#if 0
	if (tricky)
		printf("A tricky situation!\n");
	printf("disc: <%d %d>  loc1: <%d %d>  loc2: <%d %d>\n",
		   disc->loc.x, disc->loc.y,
		   loc1->x, loc1->y, loc2->x, loc2->y);
#endif

	/* Find out how big the range of good throws is.  I assume that
     the angle to loc1 is less than the angle to loc2; if it isn't,
     I switch them. */
	width = angle_to_loc2 - angle_to_loc1;
	if (width < 0)
		width += 2 * PI;
	if (width > PI) {
		tmp_angle = angle_to_loc1;
		angle_to_loc1 = angle_to_loc2;
		angle_to_loc2 = tmp_angle;
		width = 2 * PI - width;
	}
	if (angle_to_loc1 < 0)
		angle_to_loc1 += 2 * PI;
	if (angle_to_loc2 < 0)
		angle_to_loc2 += 2 * PI;

	/* Force the angle to loc2 to be greater than the angle to loc1,
     even if it means driving it above 2*PI. */
	if (angle_to_loc2 < angle_to_loc1)
		angle_to_loc2 += 2 * PI;

#if 0
	printf("angles: disc %.2f    locs %.2f  %.2f     width %.2f\n",
		   disc_angle, angle_to_loc1, angle_to_loc2, width);
#endif

	/* Throw the disc if releasing it right now would send it between
     loc1 and loc2 */
	if (angle_to_loc1 < disc_angle && disc_angle < angle_to_loc2) {
		if (!tricky) {
			/* I don't have to worry about walls */
			throw_discs(spd, FALSE);
			bi->throwing = FALSE;
			return;
		}
		frac = (disc_angle - angle_to_loc1) / width;
		/* Compute dest, the location between loc1 and loc2 that the
       disc will go to.  This computation is not exactly correct,
       because it assumes something is linear that isn't, but it's
       a lot faster than the correct computation. */
		dest.x = (1 - frac) * loc1->x + frac * loc2->x;
		dest.y = (1 - frac) * loc1->y + frac * loc2->y;
		dest.grid_x = dest.x / BOX_WIDTH;
		dest.grid_y = dest.y / BOX_HEIGHT;

#if 0
		printf("frac: %.2f  dest: <%d %d>\n", frac, dest.x, dest.y);
#endif

		/* If the disc can make it to dest, heave it */
		if (clear_path(&disc->loc, &dest)) {
			throw_discs(spd, FALSE);
			bi->throwing = FALSE;
			return;
		}
	}
	/* Predict where the disc is going.  See Buddy_throw_to_loc. */
	dx = me->loc.x - disc->loc.x;
	dy = me->loc.y - disc->loc.y;
	dist = dx * dx + dy * dy;
	delta = (dist <= DISC_ORBIT_SQ) ? PI / 2 * (2 - (dist / DISC_ORBIT_SQ))
	  : PI / 2 * (DISC_ORBIT_SQ / dist);
	new_disc_angle = (bi->spin == COUNTERCLOCKWISE) ? angle_to_me + delta
	  : angle_to_me - delta;
	fix_angle(new_disc_angle);

#if 0
	printf("new disc angle: %.2f  spin:%d \n", new_disc_angle, disc->spin);
#endif

	/* If throwing next frame will win, do it */
	if (angle_to_loc1 < new_disc_angle && new_disc_angle < angle_to_loc2) {
		if (!tricky) {
			throw_discs(spd, TRUE);
			bi->throwing = FALSE;
			return;
		}
		frac = (new_disc_angle - angle_to_loc1) / width;
		dest.x = (1 - frac) * loc1->x + frac * loc2->x;
		dest.y = (1 - frac) * loc1->y + frac * loc2->y;
		dest.grid_x = dest.x / BOX_WIDTH;
		dest.grid_y = dest.y / BOX_HEIGHT;

#if 0
		printf("frac: %.2f  dest: <%d %d>\n", frac, dest.x, dest.y);
#endif

		if (clear_path(&disc->loc, &dest)) {
			throw_discs(spd, TRUE);
			bi->throwing = FALSE;
			return;
		}
	}
	/* No throw in the near future works, so try to maneuver the disc into
     a position where we can throw to the center of the range. */
	good_angle = (angle_to_loc1 + angle_to_loc2) / 2;
	diff = good_angle - disc_angle;
	fix_angle(diff);

#if 0
	printf("good angle: %.2f   diff: %.2f\n", good_angle, diff);
#endif

	if (diff < PI / 2)
		Buddy_spin_discs(CLOCKWISE);
	else if (diff < PI)
		Buddy_spin_discs(COUNTERCLOCKWISE);
	else if (diff < 3 * PI / 2)
		Buddy_spin_discs(CLOCKWISE);
	else
		Buddy_spin_discs(COUNTERCLOCKWISE);
}

/* Buddy_find_the_action(bi)
**
** This procedure is called if I don't have the disc and I don't
** know where it is.  It causes me to move towards the center of
** mass of all other vehicles.
*/
static void
Buddy_find_the_action(Binfo *bi)
{
	int i;						/* counter; current blip */
	int blips;					/* number of blips seen */
	int x, y;					/* accumulator of blip locations, so that
				   dividing by 'blips' will find the
				   center of mass */

	get_blips(&bi->numblips, bi->blinfo);
	if (bi->numblips) {
		x = 0;
		y = 0;
		blips = 0;

		for (i = 0; i < bi->numblips; ++i) {
			/* only look at ones out of view; the vehicles we can see
	 probably don't have a clue of where the action is either. */
			if (bi->blinfo[i].x < bi->me.loc.grid_x - 2 ||
				bi->blinfo[i].x > bi->me.loc.grid_x + 2 ||
				bi->blinfo[i].y < bi->me.loc.grid_y - 2 ||
				bi->blinfo[i].y > bi->me.loc.grid_y + 2) {
				x += bi->blinfo[i].x;
				y += bi->blinfo[i].y;
				blips++;
			}
		}

		/* Head for the action */
		if (blips)
			Buddy_go_to_box(bi, x / blips, y / blips);
		else
			Buddy_wander(bi);
	} else
		Buddy_wander(bi);
}

/* Buddy_move_into_box(bi,gx,gy)
**
** Moves me into the box specified by gx and gy, by moving to the
** the closest edge.  Moving to the edge of the box instead of the
** center enables me to cut corners.
**
** Assumptions:
**   I'm in a box adjacent to the box I want to move into.
**   There isn't a wall blocking my way in.
**
** This function is generally used to traverse the shortest path to
** a destination.  It is called repeatedly with successive squares
** in the path.
*/
static void
Buddy_move_into_box(Binfo *bi, int gx, int gy)
{
	int dx, dy;					/* deltas from me to location */

	/* Initially, assume I'm heading for the center of the box */
	dx = gx * BOX_WIDTH - bi->me.loc.x + HALF_BOX_WIDTH;
	dy = gy * BOX_HEIGHT - bi->me.loc.y + HALF_BOX_HEIGHT;

	/* If I'm west of the box, head for its west edge, etc.
     The offset is to keep me safely away from corners. */
	if (gx > bi->me.loc.grid_x)
		dx -= HALF_BOX_WIDTH - 20;
	else if (gx < bi->me.loc.grid_x)
		dx += HALF_BOX_WIDTH - 20;

	if (gy > bi->me.loc.grid_y)
		dy -= HALF_BOX_HEIGHT - 20;
	else if (gy < bi->me.loc.grid_y)
		dy += HALF_BOX_HEIGHT - 20;

	set_rel_drive(9.0);
	turn_vehicle((Angle) ATAN2(dy, dx));
}

/* Buddy_go_get_the_disc(bi)
**
** Causes me to chase the disc, only if it is unowned or
** enemy controlled.  Otherwise, I head towards the enemy goal.
*/
static void
Buddy_go_get_the_disc(Binfo *bi)
{
	int dx, dy;					/* deltas to disc location */
	int distsq;					/* square of distance to disc */
	int i = 0;					/* counter; frame of lookahead */
	Location *discloc;			/* location of disc */
	Disc_info *discinfo;		/* info about disc */
	ID owner;					/* who owns the disc */

	owner = bi->dinfo[0].owner;

	if (owner == NO_OWNER || team(owner) != bi->me.team) {

		discinfo = &bi->dinfo[0];
		discloc = &discinfo->loc;

		/* Predict where the disc is going to go, and cut it off as soon
       as possible.  For each frame in the future, I update the disc's
       position, checking for bounces off walls.  If I think I can
       reach it by then, I head for that location to cut it off.
       If I can't reach it in the next DISC_FRAMES frames, I head
       towards where it will be then, hoping to be able to cut it off
       later. */

		while (i++ < DISC_FRAMES) {
			int oldx, oldy;		/* old grid location of disc */

			oldx = discloc->x / BOX_WIDTH;
			oldy = discloc->y / BOX_HEIGHT;

#ifdef B_DEBUG
			printf("%d: <%d %d> [%d %d] with speed %.2f %.2f\n",
				   i, oldx, oldy,
				   discloc->x - oldx * BOX_WIDTH,
				   discloc->y - oldy * BOX_HEIGHT,
				   discinfo->xspeed, discinfo->yspeed);
#endif /* B_DEBUG */

			/* Update disc's position for this frame */
			discloc->x += (int) discinfo->xspeed;
			discloc->y += (int) discinfo->yspeed;
			discloc->grid_x = discloc->x / BOX_WIDTH;
			discloc->grid_y = discloc->y / BOX_HEIGHT;

			/* Check to see if it tried to go through a wall, and bounce it
	 appropriately if it did.  This code is not perfect.  Yet. */
			if (oldx == discloc->grid_x - 1) {
				if (Buddy_wall_east(bi, oldx, oldy)) {
					/* Bring disc back to this side of the wall */
					discloc->x -= (discloc->x % BOX_WIDTH) * 2;
					/* Don't let the disc be right on a wall, or it will
	     seem to bounce twice. */
					if (discloc->x % BOX_WIDTH == 0)
						discloc->x -= 1;
					discinfo->xspeed = -discinfo->xspeed;
				}
			} else if (oldx == discloc->grid_x + 1) {
				if (Buddy_wall_west(bi, oldx, oldy)) {
					discloc->x += (BOX_WIDTH - (discloc->x % BOX_WIDTH)) * 2;
					if (discloc->x % BOX_WIDTH == 0)
						discloc->x += 1;
					discinfo->xspeed = -discinfo->xspeed;
				}
			}
			if (oldy == discloc->grid_y - 1) {
				if (Buddy_wall_south(bi, oldx, oldy)) {
					discloc->y -= (discloc->y % BOX_HEIGHT) * 2;
					if (discloc->y % BOX_HEIGHT == 0)
						discloc->y -= 1;
					discinfo->yspeed = -discinfo->yspeed;
				}
			} else if (oldy == discloc->grid_y + 1) {
				if (Buddy_wall_north(bi, oldx, oldy)) {
					discloc->y += (BOX_HEIGHT - (discloc->y % BOX_HEIGHT)) * 2;
					if (discloc->y % BOX_HEIGHT == 0)
						discloc->y += 1;
					discinfo->yspeed = -discinfo->yspeed;
				}
			}
			discinfo->xspeed *= bi->settings.disc_friction;
			discinfo->yspeed *= bi->settings.disc_friction;

			dx = discloc->x - bi->me.loc.x;
			dy = discloc->y - bi->me.loc.y;
			distsq = sqr(dx) + sqr(dy);

			/* If I can catch it, fine-tune my speed so I don't overrun it */
			if (distsq < sqr(((int) bi->maxspeed) * i)) {
				set_abs_drive((float) (sqrtint(distsq) / i));
				break;
			}
		}

		discloc->grid_x = discloc->x / BOX_WIDTH;
		discloc->grid_y = discloc->y / BOX_HEIGHT;
		discloc->box_x = discloc->x % BOX_WIDTH;
		discloc->box_y = discloc->y % BOX_HEIGHT;

		bi->desireddir = ATAN2(dy, dx);
		fix_angle(bi->desireddir);
		if (i == DISC_FRAMES)
			set_rel_drive(9.0);

		/* If there's a clear path to it, head for it.  Otherwise, avoid
       any intervening obstacles. */
		if (clear_path(&bi->me.loc, discloc))
			turn_vehicle(bi->desireddir);
		else
			Buddy_go_to_box(bi, discloc->grid_x, discloc->grid_y);
	} else
		Buddy_go_to_goal(bi);	/* my teammate has it, go out for a pass */
}

/* Buddy_go_to_box(bi,gx,gy)
**
** Causes me to go to the box specified by gx and gy, taking the shortest
** possible path.  This function is generally called when I want to go
** to that box, but there's something in the way.
*/
static void Inline
Buddy_go_to_box(Binfo *bi, int gx, int gy)
{
	int mx, my;					/* My location */

	mx = bi->me.loc.grid_x;
	my = bi->me.loc.grid_y;

	/* Try to find a path to that square.  If there is one, take that path,
     otherwise complain. */
	if (Buddy_compute_path_to_square(bi, mx, my, gx, gy))
		Buddy_move_into_box(bi,
							bi->scrapmaze[mx][my].nextx,
							bi->scrapmaze[mx][my].nexty);
	else
		printf("No way from <%d %d> to <%d %d>\n", mx, my, gx, gy);
}

/* Buddy_compute_path_to_square(bi,x1,y1,x2,y2)
**
** Finds the shortest path from the square <x1,y1> to the square <x2,y2>.
** Puts the information about that path (and all others of that distance
** or less) in the scrap maze.
*/
static int
Buddy_compute_path_to_square(Binfo *bi, int x1, int y1, int x2, int y2)
{
	int i, j;					/* counters */

	/* If the last path I computed was to the same destination, and
     I figured out what the path from this starting location is, use
     that information */
	if ((x2 == bi->lastfillorigin.x)
		&& (y2 == bi->lastfillorigin.y)
		&& bi->scrapmaze[x1][y1].seen)
		return (TRUE);
	else {
		/* Clear the scrap maze */
		for (i = 0; i < GRID_WIDTH; ++i) {
			for (j = 0; j < GRID_HEIGHT; ++j) {
				bi->scrapmaze[i][j].seen = FALSE;
			}
		}
		/* Store the destination so that the next call won't have to
       recompute everything if it's looking for paths to the same place. */
		bi->lastfillorigin.x = x2;
		bi->lastfillorigin.y = y2;
		return (Buddy_compute_path(bi, x1, y1, x2, y2, bi->scrapmaze, MAX_FILL_DEPTH));
	}
}

/* Buddy_compute_path(bi,x1,y1,x2,y2,maze,maxdepth)
**
** Computes the shortest path from the square <x1,y1> to the square <x2,y2>.
**
** `maze' is the representation of the maze which this function will store
**   results in.  Currently, Buddy uses two mazes; one is used solely for
**   paths to the goal and the other one is used for everything else.
** `maxdepth' is the maximum depth to search before giving up.
**
** A flood fill algorithm is used, going backwards from the goal to the
** starting location.  This means that the information is valid for any
** starting location, not just this one.
**
** Initially, the goal square is marked `seen'.  Then all squares adjacent
** to it are marked seen as well, and all squares adjacent to those, etc.
** Each `seen' square contains the location of the square which `saw' it.
** By tracing these locations, one will eventually end up at the original
** square.
*/
static int
Buddy_compute_path(Binfo *bi, int x1, int y1, int x2, int y2,
	Mazebox maze[GRID_WIDTH][GRID_HEIGHT], int maxdepth)
{
	int num_on_edge = 1;		/* number of squares on the edge of the fill */
	int next_num_on_edge;		/* number of squares for next iteration */
	int depth;					/* current depth of fill */
	Coord *curr_cut;			/* array of squares on edge of fill */
	Coord *next_cut;			/* array for next iteration */
	int cx, cy;					/* current square on edge */
	int i;						/* counter; number of current square
                                   in curr_cut */

	maze[x2][y2].seen = TRUE;	/* we've seen the destination square */
	maze[x2][y2].dist = 0;

	/* Set the cut arrays to space reserved in bi */
	curr_cut = bi->cutting_edge;
	next_cut = bi->cutting_edge2;

	/* The `cutting edge' of the fill is originally simply the first square */
	curr_cut[0].x = x2;
	curr_cut[0].y = y2;

	/* Keep flooding until I've reached maxdepth, or there's nowhere
     to flood */
	for (depth = 0; depth < maxdepth && num_on_edge > 0; ++depth) {
		next_num_on_edge = 0;
		for (i = 0; i < num_on_edge; ++i) {
			cx = curr_cut[i].x;
			cy = curr_cut[i].y;
			/* If I've accidentally flooded out of the maze, abort */
			if (cx < 0 || cy < 0 || cx >= GRID_WIDTH || cy >= GRID_HEIGHT)
				return (FALSE);
			/* If there's open space to the north, and I haven't seen that
	 square yet, flood into it */
			if (!Buddy_wall_north(bi, cx, cy) && !maze[cx][cy - 1].seen) {
				maze[cx][cy - 1].nextx = cx;
				maze[cx][cy - 1].nexty = cy;
				maze[cx][cy - 1].seen = TRUE;
				maze[cx][cy - 1].dist = depth + 1;
				/* Return if I've made it to the start square */
				if ((cx == x1) && (cy - 1 == y1))
					return (TRUE);
				/* Update the cut array for the next iteration */
				next_cut[next_num_on_edge].x = cx;
				next_cut[next_num_on_edge++].y = cy - 1;
			}
			if (!Buddy_wall_south(bi, cx, cy) && !maze[cx][cy + 1].seen) {
				maze[cx][cy + 1].nextx = cx;
				maze[cx][cy + 1].nexty = cy;
				maze[cx][cy + 1].seen = TRUE;
				maze[cx][cy + 1].dist = depth + 1;
				if ((cx == x1) && (cy + 1 == y1))
					return (TRUE);
				next_cut[next_num_on_edge].x = cx;
				next_cut[next_num_on_edge++].y = cy + 1;
			}
			if (!Buddy_wall_west(bi, cx, cy) && !maze[cx - 1][cy].seen) {
				maze[cx - 1][cy].nextx = cx;
				maze[cx - 1][cy].nexty = cy;
				maze[cx - 1][cy].seen = TRUE;
				maze[cx - 1][cy].dist = depth + 1;
				if ((cx - 1 == x1) && (cy == y1))
					return (TRUE);
				next_cut[next_num_on_edge].x = cx - 1;
				next_cut[next_num_on_edge++].y = cy;
			}
			if (!Buddy_wall_east(bi, cx, cy) && !maze[cx + 1][cy].seen) {
				maze[cx + 1][cy].nextx = cx;
				maze[cx + 1][cy].nexty = cy;
				maze[cx + 1][cy].seen = TRUE;
				maze[cx + 1][cy].dist = depth + 1;
				if ((cx + 1 == x1) && (cy == y1))
					return (TRUE);
				next_cut[next_num_on_edge].x = cx + 1;
				next_cut[next_num_on_edge++].y = cy;
			}
		}
		/* Switch the cut arrays so that the one that I filled up last
       time is now the primary one, and vice versa. */
		if (curr_cut == bi->cutting_edge) {
			curr_cut = bi->cutting_edge2;
			next_cut = bi->cutting_edge;
		} else {
			curr_cut = bi->cutting_edge;
			next_cut = bi->cutting_edge2;
		}
		num_on_edge = next_num_on_edge;
	}
	return (FALSE);				/* Couldn't find a path */
}

/* Buddy_wander(bi)
**
** This function is called when I haven't a clue where I should go.
** It causes me to continue going in my current direction but to
** avoid walls.
*/
static void
Buddy_wander(Binfo *bi)
{
	int mx, my;					/* my location */
	Angle dir;					/* my heading */

	mx = bi->me.loc.grid_x;
	my = bi->me.loc.grid_y;
	dir = bi->me.heading;

	if (dir > PI / 4 && dir < 3 * PI / 4) {	/* south */
		if (Buddy_wall_south(bi, mx, my)) {
			if (Buddy_wall_west(bi, mx, my))
				turn_vehicle(0.0);
			else if (Buddy_wall_east(bi, mx, my))
				turn_vehicle(PI);
			else if (dir < PI / 2)
				turn_vehicle(0.0);
			else
				turn_vehicle(PI);
		}
	} else if (dir >= 3 * PI / 4 && dir < 5 * PI / 4) {	/* west */
		if (Buddy_wall_west(bi, mx, my)) {
			if (Buddy_wall_north(bi, mx, my))
				turn_vehicle(PI / 2);
			else if (Buddy_wall_south(bi, mx, my))
				turn_vehicle(3 * PI / 2);
			else if (dir < PI)
				turn_vehicle(PI / 2);
			else
				turn_vehicle(3 * PI / 2);
		}
	} else if (dir >= 5 * PI / 4 && dir < 7 * PI / 4) {	/* north */
		if (Buddy_wall_north(bi, mx, my)) {
			if (Buddy_wall_east(bi, mx, my))
				turn_vehicle(PI);
			else if (Buddy_wall_west(bi, mx, my))
				turn_vehicle(0.0);
			else if (dir < 3 * PI / 2)
				turn_vehicle(PI / 2);
			else
				turn_vehicle(0.0);
		}
	} else if (dir >= 3 * PI / 4 && dir < 5 * PI / 4) {	/* east */
		if (Buddy_wall_east(bi, mx, my)) {
			if (Buddy_wall_south(bi, mx, my))
				turn_vehicle(3 * PI / 2);
			else if (Buddy_wall_north(bi, mx, my))
				turn_vehicle(PI / 2);
			else if (dir > PI)
				turn_vehicle(3 * PI / 2);
			else
				turn_vehicle(PI / 2);
		}
	}
}
