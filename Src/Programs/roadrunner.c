/*
$Author: lidl $
$Id: roadrunner.c,v 1.7 1992/08/19 05:19:46 lidl Exp $

$Log: roadrunner.c,v $
 * Revision 1.7  1992/08/19  05:19:46  lidl
 * changed to use SQRT instead of sqrt
 *
 * Revision 1.6  1991/12/15  20:56:27  lidl
 * changed all "float" occurances to "FLOAT"
 *
 * Revision 1.5  1991/09/29  15:40:22  lidl
 * changed all occurances of atan2 to ATAN2, so it uses the correct macro
 *
 * Revision 1.4  1991/09/25  05:35:38  lidl
 * added some diagnostic messages if the game isn't RACE and Full_Map isn't on
 *
 * Revision 1.3  1991/09/19  05:31:37  lidl
 * added RCS headers, run through indent, cleaned up compile-time bugs
 *
*/

/**********************************************************************/
/* 			<< roadrunner.c >>                            */
/*        by Hans Mejdahl Jeppesen, based on runner.c                 */
/*              (c) januar 1991, Lucifer data.                        */
/*  Can use nearly every tank, but a fast tank is preferred           */
/**********************************************************************/

#include "sysdep.h"
#include "xtanklib.h"
#include <math.h>
#include <stdio.h>

#define abs(x) ((x)>0?(x):(-x))
#define is(a,b) ((a)==(b)?(1):(0))
#define gridx(x) (int)(x/BOX_WIDTH)
#define gridy(y) (int)(y/BOX_HEIGHT)

void roadrunner_main();

Prog_desc roadrunner_prog = {
    "roadrunner",
    "TT_25",
    "Try to find and use the shortest way throug a race-maze.",
    "Hans Mejdah Jeppesen",
    PLAYS_RACE,
    2,
    roadrunner_main
};

static Settings_info settings;

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
FLOAT angle;
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

/* return the length of the road, if 0 no goal is found */
static int calc_road(road, me)
Road_Maze road[];
Location me;
{
    Raze_Maze maze[30][30];
    WallSide dir;
    int x, y, px, py, ox, oy, nx, ny, h, length, found, side;

    for (x = 0; x < 30; x++)
	for (y = 0; y < 30; y++) {
	    maze[x][y].sx = -1;
	    maze[x][y].px = -1;
	}
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
	return length;
    } else
	return 0;
}

static int is_clear_path(x, y, go_x, go_y)
int x, y, go_x, go_y;
{
    int dx, dy, gx, gy, depth;
    FLOAT A, Ba, Bs;

    dx = go_x - x;
    dy = go_y - y;
    if (gridx(x) == gridx(go_x) && gridy(y) == gridy(go_y))
	return 2;
    if (dx != 0) {
	A = (FLOAT) dy / dx;
	Bs = (FLOAT) (y - A * x - 50 * SQRT(1 + A * A));
	Ba = (FLOAT) (y - A * x + 50 * SQRT(1 + A * A));
	depth = 0;
	if (dx < 0)
	    for (gx = gridx(x); gx > gridx(go_x); gx--) {
		if (depth++ > 4)
		    return 0;
		gy = gridy((FLOAT) (A * gx * BOX_WIDTH + Bs));
		if (wall(WEST, gx, gy) == MAP_WALL)
		    return 0;
		gy = gridy((FLOAT) (A * gx * BOX_WIDTH + Ba));
		if (wall(WEST, gx, gy) == MAP_WALL)
		    return 0;
	    }
	else
	    for (gx = gridx(x); gx < gridx(go_x); gx++) {
		if (depth++ > 4)
		    return 0;
		gy = gridy((FLOAT) (A * BOX_WIDTH * (gx + 1) + Bs));
		if (wall(EAST, gx, gy) == MAP_WALL)
		    return 0;
		gy = gridy((FLOAT) (A * BOX_WIDTH * (gx + 1) + Ba));
		if (wall(EAST, gx, gy) == MAP_WALL)
		    return 0;
	    }
	if (dy != 0)
	    depth = 0;
	if (dy < 0)
	    for (gy = gridy(y); gy > gridy(go_y); gy--) {
		if (depth++ > 4)
		    return 0;
		gx = gridx((gy * BOX_HEIGHT - Ba) / A);
		if (wall(NORTH, gx, gy) == MAP_WALL)
		    return 0;
		gx = gridx((gy * BOX_HEIGHT - Bs) / A);
		if (wall(NORTH, gx, gy) == MAP_WALL)
		    return 0;
	    }
	else
	    for (gy = gridy(y); gy < gridy(go_y); gy++) {
		if (depth++ > 4)
		    return 0;
		gx = gridx((FLOAT) (((gy + 1) * BOX_HEIGHT - Ba) / A));
		if (wall(SOUTH, gx, gy) == MAP_WALL)
		    return 0;
		gx = gridx((FLOAT) (((gy + 1) * BOX_HEIGHT - Bs) / A));
		if (wall(SOUTH, gx, gy) == MAP_WALL)
		    return 0;
	    }
    }
    return 1;
}

void siouxeyecide(message)
char *message;
{
	if (message != NULL) {
		send_msg(RECIPIENT_ALL, OP_TEXT, message);
		for (;;) {
			done();
		}
	}
}

void roadrunner_main()
{
    Road_Maze road[900];
    int x, y, go_x, go_y, px, py, length, state, stop, frame, i;
    WallSide dir;
    Location me;
    FLOAT angle;
    char sendthis[80];

    get_settings(&settings);

    if (settings.game != RACE_GAME) {
	sprintf(sendthis, "%s: I'd rather play Race instead!", roadrunner_prog.name);
	siouxeyecide(sendthis);
    }
    if (!settings.full_map) {
	sprintf(sendthis, "%s: I don't know what to do without a Full_Map!\n",
		roadrunner_prog.name);
	siouxeyecide(sendthis);
    }

    done();

    get_location(&me);
    if ((length = calc_road(road, me)) > 0) {
	send_msg(RECIPIENT_ALL, OP_TEXT, "Meep! Meep!");
	state = length;
	px = 0;
	py = 0;
	while (1) {
	    get_location(&me);
	    if (fire(angle, me) == MAP_DEST) {
		fire_all_weapons();
		set_rel_drive(5.0);
	    } else
		set_rel_drive(9.0);
	    if (is_clear_path(me.x, me.y,
			      (BOX_WIDTH * road[state].x + BOX_WIDTH / 2),
			   (BOX_HEIGHT * road[state].y + BOX_HEIGHT / 2))) {
		while (is_clear_path(me.x, me.y,
			    (BOX_WIDTH * road[state - 1].x + BOX_WIDTH / 2),
			 (BOX_HEIGHT * road[state - 1].y + BOX_HEIGHT / 2)))
		    if (state > 0)
			state--;
		go_x = (BOX_WIDTH * road[state].x + BOX_WIDTH / 2);
		go_y = BOX_HEIGHT * road[state].y + BOX_HEIGHT / 2;
	    } else {
		i = 0;
		while (i < 5 && !is_clear_path(me.x, me.y,
			    (BOX_WIDTH * road[state + i].x + BOX_WIDTH / 2),
			 (BOX_HEIGHT * road[state + i].y + BOX_HEIGHT / 2)))
		    if (state + i < length && i < 5)
			i++;
		go_x = (BOX_WIDTH * road[state + i].x + BOX_WIDTH / 2);
		go_y = BOX_HEIGHT * road[state + i].y + BOX_HEIGHT / 2;
	    }
	    angle = ATAN2((double) (go_y - me.y), (double) (go_x - me.x));
	    turn_vehicle(angle);
	    done();
	}
    } else {
	while (1) {
	    done();
        }
    }
}
