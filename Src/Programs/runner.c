/*
$Author: lidl $
$Id: runner.c,v 1.5 1991/09/29 15:40:22 lidl Exp $

$Log: runner.c,v $
 * Revision 1.5  1991/09/29  15:40:22  lidl
 * changed all occurances of atan2 to ATAN2, so it uses the correct macro
 *
 * Revision 1.4  1991/09/19  05:23:59  lidl
 * cleaned up a few little compiling errors, made it use the right flags
 * in the program description.
 * Run through indent. Added RCS header information.
 *
*/

/**********************************************************************/
/*		 runner.c 					      */
/* by Hans Mejdahl Jeppesen, januar 1991 (c) Lucifer-data. 	      */
/*      can use a fast-tank, but the tank needs good handling.        */
/**********************************************************************/

#include "xtanklib.h"
#include <math.h>
#include <stdio.h>

#define abs(x) ((x)>0?(x):(-x))
#define is(a,b) ((a)==(b)?(1):(0))

void runner_main();

Prog_desc runner_prog = {
    "runner",
    "TT_25",
    "Tries to find and use the shortest way through a race-maze.",
    "Hans Mejdah Jeppesen",
    PLAYS_RACE,
    1,
    runner_main
};

typedef struct {
    char px, py, sx, sy;
} Raze_Maze;

typedef struct {
    char x, y;
} Road_Maze;

static WallSide rotate(dir)
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

static int fire(angle, me)
float angle;
Location me;
{
    turn_all_turrets(angle);
    angle = ((angle) < 0 ? (6.28 + angle) : (((angle)) > 6.28 ? (angle - 6.28) : (angle)));
    if (angle < 5.5 && angle > 3.9)
	return wall(NORTH, me.grid_x, me.grid_y);
    if (angle >= 5.5 || angle < 0.8)
	return wall(EAST, me.grid_x, me.grid_y);
    if (angle < 2.4 && angle >= 0.8)
	return wall(SOUTH, me.grid_x, me.grid_y);
    else
	return wall(WEST, me.grid_x, me.grid_y);

}

void runner_main()
{
    Road_Maze road[901];
    Raze_Maze maze[30][30];
    int x, y, px, py, ox, oy, nx, ny, h, length, found, side, state;
    WallSide dir;
    Location me;
    float angle;

    done();
    for (x = 0; x < 30; x++)
	for (y = 0; y < 30; y++) {
	    maze[x][y].sx = -1;
	    maze[x][y].px = -1;
	}
    get_location(&me);
    px = me.grid_x;
    py = me.grid_y;
    ox = px;
    oy = py;
    maze[px][py].px = px;
    maze[px][py].py = py;
    length = 0;
    found = 0;
    while (found == 0) {
	dir = NORTH;
	for (side = 0; (side < 4 && found == 0); side++) {
	    if (wall(dir, px, py) != MAP_WALL) {
		nx = px + is(EAST, dir) - is(WEST, dir);
		ny = py + is(SOUTH, dir) - is(NORTH, dir);
		if (maze[nx][ny].px == -1) {
		    if (landmark(nx, ny) == GOAL)
			found = 1;
		    maze[ox][oy].sx = nx;
		    maze[ox][oy].sy = ny;
		    maze[nx][ny].px = px;
		    maze[nx][ny].py = py;
		    ox = nx;
		    oy = ny;
		}
	    }
	    dir = rotate(dir);
	}
	h = maze[px][py].sx;
	py = maze[px][py].sy;
	px = h;
	if (px == -1)
	    found = -1;
    }
    x = nx;
    y = ny;
    road[0].x = nx;
    road[0].y = ny;
    length++;
    if (found == 1) {
	while (maze[x][y].px != me.grid_x || maze[x][y].py != me.grid_y) {
	    road[length].x = maze[x][y].px;
	    road[length].y = maze[x][y].py;
	    h = maze[x][y].px;
	    y = maze[x][y].py;
	    x = h;
	    length++;
	}
	road[length].x = me.grid_x;
	road[length].y = me.grid_y;
	send_msg(RECIPIENT_ALL, OP_TEXT, "DA DA, DA DAAA!!");
	state = length;
	nx = road[state].x;
	ny = road[state].y;
	while (1) {
	    set_rel_drive(9.0);
	    get_location(&me);
	    if (me.grid_x == nx && me.grid_y == ny) {
		nx = road[state].x;
		ny = road[state].y;
		x = BOX_WIDTH * nx + BOX_WIDTH / 2;
		y = BOX_HEIGHT * ny + BOX_HEIGHT / 2;
		state--;
		if (state == 0)
		    send_msg(RECIPIENT_ALL, OP_TEXT, "I'LL WIN !.");
	    }
	    angle = ATAN2((double) (y - me.y), (double) (x - me.x));
	    turn_vehicle(angle);
	    if (fire(angle, me) == MAP_DEST)
		fire_all_weapons();
	    done();
	}
    } else
	send_msg(RECIPIENT_ALL, OP_TEXT, "I refuse to run!.");
    done;
}
