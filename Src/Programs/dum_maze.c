/*
** Xtank
**
** Copyright 1991 by Hans Mejdahl Jeppesen
**
** $Id$
*/

/**********************************************************************/
/*			 dum_maze.c				      */
/*    by Hans Mejdahl Jeppesen, januar 1991 (c) Lucifer-data.         */
/*	   must use a slow tank, and low scroll-speed    	      */
/**********************************************************************/

#include "xtanklib.h"
#include <math.h>
#define artful_square(x) ((x)*(x))

#define SE 0
#define S  1
#define SW 2
#define W  3
#define NW 4
#define N  5
#define NE 6
#define E  7

#define NOTURN 37

#define abs(x) ((x)>0?(x):(-x))

static Vehicle_info my_vehicle;

void dum_maze_main();

Prog_desc dum_maze_prog = {
    "dum_maze",
    "?",
    "Don't know yet",
    "Hans Mejdah Jeppesen",
    PLAYS_RACE,
    1,
    dum_maze_main
};

static float angle_towards(x, y, me)	/* return the from robot to x,y */
int x, y;
Location me;
{
    return ATAN2((double) (y - me.y), (double) (x - me.x));
}

static WallSide left(dir)
WallSide dir;
{
    switch (dir) {
	case NORTH:
	    return WEST;
	case EAST:
	    return NORTH;
	case SOUTH:
	    return EAST;
	case WEST:
	    return SOUTH;
    }
}
static WallSide right(dir)
WallSide dir;
{
    switch (dir) {
	case NORTH:
	    return EAST;
	case EAST:
	    return SOUTH;
	case SOUTH:
	    return WEST;
	case WEST:
	    return NORTH;
    }
}

static WallType wall_info(dir, me)
WallSide dir;
Location me;
{
    return wall(dir, me.grid_x, me.grid_y);
}

static WallSide new_grid(dir, me)
WallSide dir;
Location me;
{
    if (wall_info(right(dir), me) != MAP_WALL)
	dir = right(dir);
    else if (wall_info(dir, me) == MAP_WALL) {
	dir = left(dir);
	if (wall_info(dir, me) == MAP_WALL)
	    dir = left(dir);
    }
    return dir;
}
static Location go_towards(dir, me)
WallSide dir;
Location me;
{
    Location next;
    WallSide next_dir;

    next = me;
    next.x = me.grid_x * BOX_HEIGHT + BOX_HEIGHT / 2;
    next.y = me.grid_y * BOX_WIDTH + BOX_HEIGHT / 2;
    switch (dir) {
	case NORTH:
	    next.grid_y = me.grid_y - 1;
	    next.y = (me.grid_y - 1) * BOX_WIDTH + BOX_HEIGHT / 2;
	    break;
	case EAST:
	    next.grid_x = me.grid_x + 1;
	    next.x = (me.grid_x + 1) * BOX_HEIGHT + BOX_HEIGHT / 2;
	    break;
	case SOUTH:
	    next.grid_y = me.grid_y + 1;
	    next.y = (me.grid_y + 1) * BOX_WIDTH + BOX_HEIGHT / 2;
	    break;
	case WEST:
	    next.grid_x = me.grid_x - 1;
	    next.x = (me.grid_x - 1) * BOX_HEIGHT + BOX_HEIGHT / 2;
	    break;
    }

    return next;

}


static void fire(dir)
WallSide dir;
{

    switch (dir) {
	case NORTH:
	    turn_all_turrets(4.71);
	    break;
	case EAST:
	    turn_all_turrets(0);
	    break;
	case SOUTH:
	    turn_all_turrets(1.70);
	    break;
	case WEST:
	    turn_all_turrets(3.14);
    }
    fire_all_weapons();
}

void dum_maze_main()
{
    int last_x, last_y, state, again;
    WallSide dir;
    Location me, ps;

    get_self(&my_vehicle);
    last_x = -1;
    last_y = -1;
    dir = EAST;
    set_rel_drive(9.0);
    get_location(&ps);
    while (1) {
	get_location(&me);
	if (wall_info(dir, me) == MAP_DEST)
	    fire(dir);
	if (me.grid_x == ps.grid_x && me.grid_y == ps.grid_y) {
	    dir = new_grid(dir, ps);
	    ps = go_towards(dir, ps);
	}
	turn_vehicle(ATAN2((double) (ps.y - me.y), (double) (ps.x - me.x)));
    }
}
