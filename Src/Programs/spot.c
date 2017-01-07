/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "sysdep.h"
#include "xtanklib.h"
#include <math.h>

static void spot_main(void);
static void spot_get_unstuck(void);
static void spot_attack_closest(int *, FLOAT *);
static void spot_spin(FLOAT);

Prog_desc spot_prog = {
	"spot",
	"malice#1",
	"Spot is a tank which takes the simplistic, although effective tact \
of charging at the closest vehicle and firing continuously at it. It \
is quite effective for all it's simplicity; its weakness is its inability \
    to get around walls.",
	"Terry Donahue",
	PLAYS_COMBAT | DOES_SHOOT,
	2,
	spot_main
};

typedef struct
{
	int front;
	int back;
	int left;
	int right;
} Armor_on;

static void
spot_main(void)
{
	extern int num_veh_alive;
	int dist = 9999;
	FLOAT ang = 0.0;

	while (1)
	{
		if (speed() == 0 && dist > 70 * 70)
			spot_get_unstuck();
		else if (speed() == 0)
			spot_spin(ang);

		/* attack the closest vehicle */
		spot_attack_closest(&dist, &ang);
	}
}


/*
** Gets the vehicle unstuck from an obstacle.  Call it if you think you
** ran into something.
*/
static void
spot_get_unstuck(void)
{
	int framenum = frame_number();
	FLOAT curangle = heading();

	/* First backup away from the obstacle */
	set_rel_drive(-3.0);

	/* Wait until 3 frames have passed, so we back up enough */
	while (frame_number() < framenum + 3);

	/* Turn around and get moving */
	turn_vehicle(curangle + PI);
	set_rel_drive(5.0);
}


/*
** Moves all turrets at the closest vehicle.
** Shoots all weapons at that vehicle, regardless of whether
** or not it is in range, or the turrets are rotated yet.
*/
static void
spot_attack_closest(int *dist, FLOAT *ang)
{
    int veh_cnt;		/* the number of vehicles I can see */
    Vehicle_info vehicle[MAX_VEHICLES];	/* the array of vehicles I can see */
    Vehicle_info *v;
    Location my_loc;		/* my location */
    int i;			/* the vehicle number I'm looking at */
    int dx, dy;			/* the distance between me and my target */
    int min_dx, min_dy;
    int min_dist;		/* the minimum target distance found */

    get_vehicles(&veh_cnt, vehicle); /* look around */

    /* If there are no targets, give up */
    if (veh_cnt == 0)
	return;

    get_location(&my_loc);	/* find my location */
    min_dist = 9999999;		/* set minimum distance away to large number */

    /* Go through all the vehicles I can see and look for the one I want * to
       shoot at. */
    for (i = 0; i < veh_cnt; i++)
    {
	v = &vehicle[i];

	/* Figure out the distance between me and him (in pixels) * along the
	   x axis (dx) and the y axis (dy) */
	dx = BOX_WIDTH * (v->loc.grid_x - my_loc.grid_x)
	    + v->loc.box_x - my_loc.box_x;
	dy = BOX_HEIGHT * (v->loc.grid_y - my_loc.grid_y)
	    + v->loc.box_y - my_loc.box_y;

	*dist = dx * dx + dy * dy;
	if (*dist < min_dist)
	{
	    min_dx = dx;
	    min_dy = dy;
	    min_dist = *dist;
	}
    }

    aim_all_turrets(min_dx, min_dy); /* turn my turret(s) toward him */
    fire_all_weapons();		/* let him have it */

    /* move towards the vehicle too */
    *ang = ATAN2(dy, dx);
    if (*dist > 70 * 70)
	turn_vehicle(*ang);
    set_rel_drive(5.0);
}

static void
spot_spin(FLOAT ang)
{
	Armor_on arm;
	int highest;

	set_abs_drive(0.0);

	highest = armor(FRONT);
	arm.back = armor(BACK);
	arm.left = armor(LEFT);
	arm.right = armor(RIGHT);

	if (highest < arm.back)
		highest = arm.back;
	if (highest < arm.left)
		highest = arm.left;
	if (highest < arm.right)
		highest = arm.right;

	if (highest < arm.back)
		turn_vehicle(ang + PI);
	if (highest < arm.left)
		turn_vehicle(ang + .5 * PI);
	if (highest < arm.back)
		turn_vehicle(ang - .5 * PI);
}
