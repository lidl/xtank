/************************************************\
* "RacerX" - an Xtank player by Robert D. Potter *
\************************************************/

/*
Hans' pair of robots for playing Race prompted me to bring "racerx" up
to date.  I hadn't touched it for about a year, but it didn't need
much tweaking to make it work with 1.2g.

Its noteworthy features are that it can work with or without Full
Map, it can deal with destructible walls (poorly), and it has a fairly
smart way of cutting corners.  Its main weakness is that it sometimes
gets "trapped" by certain scroll squares.  Also, it won't shoot at
enemies or outposts.

It beats "dum_maze" (almost) all of the time.  "roadrunner" always
segfaults on me so I can't tell how it would do.  (I wish you Xtank
gods wouldn't put broken robots on the robot list).  Anyway, let me
know how you like it.

Fixed a couple of problems.  Now much smarter about shooting at
destructible walls.
*/

/*
$Author: lidl $
$Id: RacerX.c,v 1.4 1991/09/25 07:18:40 lidl Exp $

$Rlog$
*/
#include <math.h>
#include <xtanklib.h>

static void RacerX_main();		/* forward reference */

Prog_desc RacerX_prog = {
    "RacerX",
    "Mach5",
    "Heads for the goal by the 'shortest' route.  Works with or without \
full-map, will shoot destructible walls (but doesn't realize they take \
longer).  Does not shoot at enemies or pay any attention to landmarks. \
Version 2.2.",
    "Robert Potter (rpotter@grip.cis.upenn.edu)",
    PLAYS_RACE | DOES_EXPLORE,
    5,
    RacerX_main
};

#define NODIR		(WEST+1)	/* (used as an array index, so Xtank's
					   NO_DIR is unsuitable) */
#define MAX_BOXES	(GRID_WIDTH * GRID_HEIGHT)	/* the maximum possible
							   number of boxes in
							   the map */

#define SHORT_CUT_DEPTH	5	/* how deep to search for short cuts in the
				   navigator's route */

#define give_up()	while (1) done();

#define map_wall_north(x,y) map_north_result(allp->my_map[x][y].flags)
#define map_wall_south(x,y) map_north_result(allp->my_map[x][y+1].flags)
#define map_wall_east(x,y) map_west_result(allp->my_map[x+1][y].flags)
#define map_wall_west(x,y) map_west_result(allp->my_map[x][y].flags)


/* note that Xtank's WallSide type is no good, since you can't use its NO_DIR
   as an array index */
typedef enum {
    North = NORTH,
    East = EAST,
    South = SOUTH,
    West = WEST,
    Nodir
} Direction;

/* information I may want to keep on a on a map box */
typedef struct {
    Boolean seen;		/* iff we have ever seen this box */
    Direction navdir;		/* direction (used by navigator) */
} BoxNotes;

/* everything I know, packaged into a structure so that it can be easily
   passed to subroutines (all this to avoid global data) */
typedef struct {
    int frame;			/* the current frame */
    Vehicle_info me;		/* information on my vehicle */
    Location prev_loc;		/* previous location */
    Boolean have_route;		/* iff the navigator found a route */
    Boolean saw_goal;		/* iff we have ever seen our goal */
    Box (*my_map)[GRID_HEIGHT];	/* my personal map of the world (provided by
				   Xtank itself) */
    BoxNotes boxnotes[GRID_WIDTH][GRID_HEIGHT];	/* my notes on each map box */
    Coord dest_wall;		/* box coordinates of the first destructible
				  wall in our path */
} All;

/********************************************************************\
* The following variables are essentially constants, thus can be     *
* shared between vehicles, thus can be global.  They are static so   *
* that there are no namespace conflicts with the rest of Xtank.      *
\********************************************************************/

/* convert a direction into a heading angle */
static Angle dir2angle[] = {
    3*PI/2,	/* North */
    0,		/* East */
    PI/2,	/* South */
    PI,		/* West */
    0,		/* NODIR */
};

/* convert a direction into differences in x and y coordinates */
static Coord dir2delta[] = {
    { 0, -1},	/* North */
    { 1,  0},	/* East */
    { 0,  1},	/* South */
    {-1,  0},	/* West */
    { 0,  0},	/* NODIR */
};

/* convert a direction into the pixel coordinates of the side of the box that
   will be the exit */
static Coord dir2exit[] = {
    {BOX_WIDTH/2,            1},	/* North (top center) */
    {BOX_WIDTH-1, BOX_HEIGHT/2},	/* East (right center) */
    {BOX_WIDTH/2, BOX_HEIGHT-1},	/* South (bottom center) */
    {          1, BOX_HEIGHT/2},	/* West (left center) */
    {BOX_WIDTH/2, BOX_HEIGHT/2},	/* NODIR (center center) */
};

/* convert a direction to its opposite */
static unsigned char dir2opposite[] = {
    South,	/* [North] */
    West,	/* [East] */
    North,	/* [South] */
    East,	/* [West] */
    NODIR,	/* [NODIR] */
};

static Settings_info settings;

/* changes a location by the given x and y.  Returns locp. */

static Location *delta_loc(locp, delta_x, delta_y)
    Location *locp;		/* gets modified */
    int delta_x, delta_y;	/* in pixels */
{
    locp->x += delta_x;
    locp->y += delta_y;
    locp->grid_x = locp->x / BOX_WIDTH;
    locp->box_x = locp->x % BOX_WIDTH;
    locp->grid_y = locp->y / BOX_HEIGHT;
    locp->box_y = locp->y % BOX_HEIGHT;

    return locp;
}


/* decides if I can make the specified move without hitting a wall, taking the
   vehicle size into account.  This is done by checking the paths of the
   vehicle's bounding-box's corners. */

static self_clear_path(start, delta_x, delta_y)
    Location *start;		/* starting position of vehicle */
    int delta_x, delta_y;	/* proposed move */
{
    Location s, f;		/* start and finish of a vehicle corner */
    int w, h;			/* width and height of this vehicle (changes
				   with orientation) */

    vehicle_size(&w, &h);
    /* convert to difference from center of vehicle (assume that vehicle
       location is center of vehicle; it sucks that we can't get offset_x and
       offset_y) */
    w = (w+1)/2;
    h = (h+1)/2;

    /* upper left */
    s = *start;
    delta_loc(&s, -w, -h);
    f = s;
    delta_loc(&f, delta_x, delta_y);
    if (! clear_path(&s, &f)) return False;

    /* upper right */
    s = *start;
    delta_loc(&s, w, -h);
    f = s;
    delta_loc(&f, delta_x, delta_y);
    if (! clear_path(&s, &f)) return False;

    /* lower right */
    s = *start;
    delta_loc(&s, w, h);
    f = s;
    delta_loc(&f, delta_x, delta_y);
    if (! clear_path(&s, &f)) return False;

    /* lower left */
    s = *start;
    delta_loc(&s, -w, h);
    f = s;
    delta_loc(&f, delta_x, delta_y);
    if (! clear_path(&s, &f)) return False;

    return True;
}


/* decides if a box has been seen before.  Suitable to pass to navigate(). */

static unseen_box(allp, x, y)
    All *allp;
    int x, y;			/* grid coords of a box */
{
    return !(allp->boxnotes[x][y].seen);
}


/* decides if a box is a goal of my team.  Suitable to pass to navigate(). */

static goal(allp, x, y)
    All *allp;
    int x, y;			/* grid coords of a box */
{
    return map_landmark(allp->my_map, x, y) == GOAL &&
	(map_team(allp->my_map, x, y) == allp->me.team ||
	 map_team(allp->my_map, x, y) == NEUTRAL);
}


static void notice_landmark(allp, x, y)
    All *allp;
    int x, y;			/* box coordinates */
{
#if 0
	if (map_landmark(allp->my_map, x, y) != NORMAL) {
		printf("racerx: see landmark %d at %d,%d\n",
			map_landmark(allp->my_map, x, y), x, y);
	}
#endif

    if (goal(allp, x, y)) {
	allp->saw_goal = True;
#if 0
	printf("racerx: I see my goal.\n");
#endif
    }
}


static void note_surroundings(allp)
    All *allp;
{
    int gx, gy;			/* current grid coords */
    int x, y;			/* coords of nearby box */

    if (!settings.full_map) {	/* %% won't update outposts */
	gx = allp->me.loc.grid_x;
	gy = allp->me.loc.grid_y;

	for (x = MAX(0, gx - 2); x <= MIN(GRID_WIDTH - 1, gx + 2); ++x) {
	    for (y = MAX(0, gy - 2); y <= MIN(GRID_WIDTH - 1, gy + 2); ++y) {
		allp->boxnotes[x][y].seen = True;
		notice_landmark(allp, x, y);
	    }
	}
    }
}


/* find a path from the current location to one that fits destfunc().  Leaves
   the result in the map and returns success in allp->have_route. */

static navigate(allp, destfunc, through_unseen)
    All *allp;
    int (*destfunc)();		/* gets called with allp and x and y grid
				   coordinates.  Returns true if that box is a
				   destination. */
    int through_unseen;		/* true if the search should proceed even
				   through boxes that have not been seen yet */
{
    Coord queue[MAX_BOXES];	/* used to implement breadth-first search
				   through map */
    int head, tail;		/* indexes into queue[] */
    Coord *cp;			/* data on box we're currently looking at */
    register int x, y;
    int startx, starty;

    /* mark all boxes as unsearched */
    for (x = 0; x < GRID_WIDTH; ++x) {
	for (y = 0; y < GRID_WIDTH; ++y) {
	    allp->boxnotes[x][y].navdir = NODIR;
	}
    }

    /* current location is start of search */
    startx = allp->me.loc.grid_x;
    starty = allp->me.loc.grid_y;
    allp->boxnotes[startx][starty].navdir = North;	/* anything but NODIR
							   */

    tail = 0;
    queue[0].x = startx;
    queue[0].y = starty;
    head = 1;

    while (tail < head) {	/* until queue is empty */
	cp = &queue[tail++];
	x = cp->x;
	y = cp->y;

	if (destfunc(allp, x, y)) {	/* found a destination? */
	    int dir;		/* direction of path from this box (i.e the
				   direction from which the search found this
				   box) */
	    int fromdir;	/* direction by which we came to this box on
				   the retrace */

	    /* now we are at the goal, with a path behind us leading back to
	       the start, so we have to retrace the path, reversing the arrows
	       as we go */
	    allp->boxnotes[startx][starty].navdir = NODIR;	/* terminates
								   */
	    fromdir = NODIR;
	    do {
		dir = allp->boxnotes[x][y].navdir;
		allp->boxnotes[x][y].navdir = fromdir;	/* reverse it */
		x += dir2delta[dir].x;
		y += dir2delta[dir].y;
		fromdir = dir2opposite[dir];	/* for next box */
	    } while (fromdir != NODIR);
	    allp->have_route = True;
	    return True;
	}

	/* quit now if we shouldn't search further from here */
	if (!through_unseen && !allp->boxnotes[cp->x][cp->y].seen) continue;

	/* put all accessible unexplored adjacent boxes into queue (navdir is
	   used to indicate where the search came _from_) */

	if (y > 0 && allp->boxnotes[x][y - 1].navdir == NODIR &&
	    map_wall_north(x, y) != MAP_WALL) {
	    allp->boxnotes[x][y - 1].navdir = South;
	    queue[head].x = x;
	    queue[head].y = y - 1;
	    ++head;
	}
	if (y < GRID_HEIGHT-1 && allp->boxnotes[x][y + 1].navdir == NODIR &&
	    map_wall_south(x, y) != MAP_WALL) {
	    allp->boxnotes[x][y + 1].navdir = North;
	    queue[head].x = x;
	    queue[head].y = y + 1;
	    ++head;
	}
	if (x > 0 && allp->boxnotes[x - 1][y].navdir == NODIR &&
	    map_wall_west(x, y) != MAP_WALL) {
	    allp->boxnotes[x - 1][y].navdir = East;
	    queue[head].x = x - 1;
	    queue[head].y = y;
	    ++head;
	}
	if (x < GRID_WIDTH-1 && allp->boxnotes[x + 1][y].navdir == NODIR &&
	    map_wall_east(x, y) != MAP_WALL) {
	    allp->boxnotes[x + 1][y].navdir = West;
	    queue[head].x = x + 1;
	    queue[head].y = y;
	    ++head;
	}
    }
    allp->have_route = False;
    return False;
}


/* searches down the path the navigator found for short-cuts: clear lines from
   my vehicle's current position to boxes later on in the route.  Returns the
   angle to go in (occaisonally BAD_ANGLE). */

static Angle recursive_short_cut(allp, gx, gy, depth)
    All *allp;
    int gx, gy;			/* box to check out (initially the box my
				   vehicle is in) */
    int depth;			/* how many boxes down the path to search */
{
    Angle a;
    Direction dir;
    int dx, dy;

    dir = allp->boxnotes[gx][gy].navdir;	/* direction navigator says to
						   go from this box */
    if (dir == NODIR) {
	return BAD_ANGLE;
    }

    /* remember the first destructible wall along our path */
    /* so we can shoot at it */
    if (allp->dest_wall.x == -1) {
	switch (dir) {
		case North:
			if (map_wall_north(gx,gy) == MAP_DEST) {
				allp->dest_wall.x = gx;
				allp->dest_wall.y = gy;
			}
			break;
		case South:
			if (map_wall_south(gx,gy) == MAP_DEST) {
				allp->dest_wall.x = gx;
				allp->dest_wall.y = gy;
			}
			break;
		case East:
			if (map_wall_east(gx,gy) == MAP_DEST) {
				allp->dest_wall.x = gx;
				allp->dest_wall.y = gy;
			}
			break;
		case West:
			if (map_wall_west(gx,gy) == MAP_DEST) {
				allp->dest_wall.x = gx;
				allp->dest_wall.y = gy;
			}
			break;
		default:
			printf("%s: direction bug!\n", RacerX_prog.name);
	}
    }


    /* if not at bottom of recursion, first check for shortcuts to the rest of
       the route (which will be better shortcuts than any shortcut from here)
       */
    if (depth > 0) {
	a = recursive_short_cut(allp,
				gx + dir2delta[dir].x,
				gy + dir2delta[dir].y,
				depth - 1);
	if (a != BAD_ANGLE) {
	    return a;
	}
    }

    /* check to see if there is a clear linear path between my vehicle and the
       part of the destination box where we want to be */

    /* %% */
    dx = ((gx * BOX_WIDTH) + dir2exit[dir].x) - allp->me.loc.x;
    dy = ((gy * BOX_HEIGHT) + dir2exit[dir].y) - allp->me.loc.y;
    if (self_clear_path(&(allp->me.loc), dx, dy)) {
	return ATAN2(dy, dx);	/* found shortcut, so return its angle */
    }
    return BAD_ANGLE;		/* didn't find the shortcut */
}



/* follow the route that navigate() produced */

static void follow_route(allp)
    All *allp;
{
    Angle angle;

    if (! allp->have_route) {
	printf("%s: I don't have a route!\n", RacerX_prog.name);
	return;
    }

    allp->dest_wall.x = -1;   /* no destructible wall yet */
    angle = recursive_short_cut(allp, allp->me.loc.grid_x,
		allp->me.loc.grid_y, SHORT_CUT_DEPTH);
    if (angle == BAD_ANGLE) {	/* assume destructible wall */
	int gx = allp->me.loc.grid_x;
	int gy = allp->me.loc.grid_y;
	Direction navdir = allp->boxnotes[gx][gy].navdir;
	int dx = dir2exit[navdir].x - allp->me.loc.box_x;
	int dy = dir2exit[navdir].y - allp->me.loc.box_y;

	angle = ATAN2(dy, dx);
	turn_vehicle(angle);
	set_rel_drive(9.0);
    } else {
	turn_vehicle(angle);
	turn_all_turrets(angle);	/* handle_weapons() may override */
	set_rel_drive(9.0);	/* %% */
    }
}


static void check_clock(allp)
    All *allp;
{
    int previous_frame;

    previous_frame = allp->frame;
    allp->frame = frame_number();
    if (allp->frame > previous_frame + TICKSZ) {
	printf("%s: lost %d turns\n", RacerX_prog.name,
	       (allp->frame - previous_frame) / TICKSZ - 1);
    }
}


/* deals with the weapons and turrets */

static void handle_weapons(allp)
    All *allp;
{
    Location target;
    BoxNotes *np;
    WeaponNum wn;
    int range_sqr;	/* square of distance to target */

    target.x = -1;	/* start with no target */

    if (allp->dest_wall.x != -1) {	/* destructible wall in our path? */
	/* target is middle of wall */
	np = &allp->boxnotes[allp->dest_wall.x][allp->dest_wall.y];
	target.box_x = dir2exit[np->navdir].x;
	target.box_y = dir2exit[np->navdir].y;
	target.grid_x = allp->dest_wall.x;
	target.grid_y = allp->dest_wall.y;
	target.x = target.box_x + BOX_WIDTH * target.grid_x;
	target.y = target.box_y + BOX_HEIGHT * target.grid_y;

	aim_all_turrets(target.x - allp->me.loc.x,
		target.y - allp->me.loc.y);

	/* can we shoot it? */
	if (clear_path(&allp->me.loc, &target)) {
		range_sqr = (SQR(target.x - allp->me.loc.x) +
			SQR(target.y - allp->me.loc.y));

		for (wn = 0; wn < MAX_WEAPONS; ++wn) {
			Weapon_info winfo;

			if (get_weapon(wn, &winfo) != BAD_VALUE) {
				if (SQR(winfo.range) >= range_sqr) {
					turn_on_weapon(wn);
				} else {
					turn_off_weapon(wn);
				}
			}
		}
		fire_all_weapons();
	}
    }
}

static void RacerX_main()
{
    All *allp;
    register int x, y;

    done();

    allp = (All *) calloc(1, sizeof(*allp));
    if (allp == NULL) {
	printf("%s:  calloc() failed!\n", RacerX_prog.name);
	give_up();
    }

    if (!has_special(MAPPER)) {
	printf("%s: Gack!  How can I race without a mapper?!?\n",
		RacerX_prog.name);
	give_up();
    }

    allp->frame = frame_number();

    get_settings(&settings);

    if (settings.game != RACE_GAME) {
	printf("%s: Hmph!  Can't we play Race instead?\n", RacerX_prog.name);
	send_msg(RECIPIENT_ALL, OP_TEXT, "Can't we play Race instead??");
    }
    if (settings.rel_shoot) {
	send_msg(RECIPIENT_ALL, OP_TEXT, "Relative shooting sucks!");
    }
    if (!has_special(MAPPER)) {
	send_msg(RECIPIENT_ALL, OP_TEXT, "No mapper?!?  I quit!");
	give_up();
    }

    /* initialize map */
    allp->my_map = map_get();
    if (settings.full_map) {
	for (x = 0; x < GRID_WIDTH; ++x) {
	    for (y = 0; y < GRID_WIDTH; ++y) {
		allp->boxnotes[x][y].seen = True;
		notice_landmark(allp, x, y);
	    }
	}
    }

    allp->prev_loc.grid_x = allp->prev_loc.grid_y = -1;

    while (1) {
	check_clock(allp);
	allp->prev_loc = allp->me.loc;
	get_self(&allp->me);

	if (allp->prev_loc.grid_x != allp->me.loc.grid_x ||
	    allp->prev_loc.grid_y != allp->me.loc.grid_y) {	/* new box? */
	    note_surroundings(allp);
	    /* either head for goal or explore */
	    if (! (allp->saw_goal && navigate(allp, goal, True))) {
		if (! navigate(allp, unseen_box, False)) {
		    send_msg(RECIPIENT_ALL, OP_TEXT, "What, no goal?!?");
		    give_up();
		}
	    }
	}
	follow_route(allp);
	handle_weapons(allp);

	done();
    }
}

