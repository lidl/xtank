/*
** Xtank
**
** Copyright 1991 by Hans Mejdahl Jeppesen
**
** Pzkw_I.c
*/

/*
$Author: lidl $
$Id: Pzkw_I.c,v 1.1.1.1 1995/02/01 00:25:46 lidl Exp $
*/

/***********************************************************************/
/*  			Pzkw_I.c,				       */
/*    by Hans Mejdahl Jeppesen, januar 1991 (c) Lucifer-data.          */
/* Can use every tank. No mine, and use turret and front weapons only  */
/***********************************************************************/

#include "xtanklib.h"
#include <math.h>
#include <stdio.h>
#define artful_square(x) ((x)*(x))

#define SE 0
#define S  1
#define SW 2
#define W  3
#define NW 4
#define N  5
#define NE 6
#define E  7
#define ATTACK 1
#define HUNTS 2
#define WAITS 3
#define NOTURN 37

#define abs(x) ((x)>0?(x):(-x))

static Vehicle_info my_vehicle;

void Pzkw_I_main();

Prog_desc Pzkw_I_prog = {
    "Pzkw_I",
    "TT_25",
    "Don't know yet",
    "Hans Mejdah Jeppesen",
    PLAYS_COMBAT | DOES_SHOOT | USES_TEAMS,
    1,
    Pzkw_I_main
};

static FLOAT angle_towards(x, y)/* return the approximated angle from robot
				   to x,y */
int x, y;
{
    Location myloc;

    get_location(&myloc);
    return ATAN2((double) (y - myloc.y), (double) (x - myloc.x));
}
static FLOAT angle_towards_grid(x, y)
int x, y;
{
    Location myloc;

    get_location(&myloc);
    return ATAN2((double) (y - myloc.grid_y), (double) (x - myloc.grid_x));
}

static FLOAT correct_for_walls(x, y, v)
int x, y;
FLOAT v;
{
    FLOAT w;

    w = ((v) > 0 ? (v) : (2 * PI + v));
    if (wall(NORTH, x, y))
	if (3.93 < w && w < 5.5)
	    w = ((w) > 4.71 ? (4.71 - w) : (4.71 + w));
    if (wall(WEST, x, y))
	if (2.26 < w && w < 3.93)
	    w = ((w) > PI ? (PI - w) : (PI + w));
    if (wall(SOUTH, x, y))
	if (0.79 < w && w < 2.26)
	    w = ((w) > 1.57 ? (1.57 - w) : (1.57 + w));;
    if (wall(EAST, x, y))
	if (w > 5.5 || w < 0.79)
	    w = -w;
    return w;
}

static int drive_towards_blip(x, y)	/* return -1 if no blip */
int *x, *y;
{
    int no_blip;
    Blip_info blip[MAX_BLIPS];

    get_blips(&no_blip, blip);
    while (no_blip > 0) {
	*x = blip[0].x;
	*y = blip[0].y;
	return 1;
    }
    return 0;
}


void Pzkw_I_main()
{
    Vehicle_info enemy;		/* -1 if freind spotted */
    FLOAT angle;
    int i = 0;
    int loc_x, loc_y;
    int state = WAITS;
    Location me;
    Message msg;
    char data[MAX_DATA_LEN];

    get_self(&my_vehicle);
    send_msg(MAX_VEHICLES + my_vehicle.team, OP_ACK, "");
    while (1) {
	get_location(&me);
	switch (state) {
	    case ATTACK:
		if (get_closest_enemy(&enemy)) {
		    set_rel_drive(9.0);
		    angle = angle_towards(enemy.loc.x, enemy.loc.y);
		    turn_vehicle(angle);
		    turn_all_turrets(angle);
		    fire_all_weapons();
		    while (messages()) {
			receive_msg(&msg);
			if ((msg.opcode == OP_ACK)) {
			    data[0] = enemy.loc.grid_x;
			    data[1] = enemy.loc.grid_y;
			    send_msg(MAX_VEHICLES + my_vehicle.team, OP_LOCATION, data);;
			    break;
			}
		    }
		} else
		    state = WAITS;
		break;
	    case HUNTS:
		set_rel_drive(9.0);
		if (get_closest_enemy(&enemy)) {
		    data[0] = me.grid_x;
		    data[1] = me.grid_y;
		    send_msg(MAX_VEHICLES + my_vehicle.team, OP_LOCATION, data);
		    state = ATTACK;
		} else {
		    angle = angle_towards_grid(loc_x, loc_y);
		    angle = correct_for_walls(me.grid_x, me.grid_y, angle);
		    turn_vehicle(angle);
		    if (me.grid_x == loc_x && me.grid_y == loc_y) {
			state = WAITS;
		    }
		}
		break;
	    case WAITS:
		set_rel_drive(0.0);
		if (get_closest_enemy(&enemy)) {
		    state = ATTACK;
		} else {
		    if (drive_towards_blip(&loc_x, &loc_y)) {

			state = HUNTS;
		    }
		    while (messages()) {
			receive_msg(&msg);
			if ((msg.opcode == OP_LOCATION)) {
			    loc_x = msg.data[0];
			    loc_y = msg.data[1];
			    state = HUNTS;
			}
		    }
		}
		break;
	}
	done();
    }
}
