/***********************************************\
* "Guard" - an Xtank player by Robert Potter	*
\***********************************************/

#include "xtanklib.h"
#include <math.h>

static void Guard_main();

Prog_desc Guard_prog =
{
	"Guard",
	"Guard",
    "Basic strategy is to collide with the opponent.  Naturally, this only works if the opponent's vehicle is MUCH more expensive (and slower).  Avoids collisions with walls (*almost* all of the time) avoids collisions with others on the same team (most of the time) wanders aimlessly if it can't find an enemy; buys supplies if it stumbles across them and has nothing better to do; is not smart about going around walls or searching for supplies.",
	"Robert Potter (potter@cs.rochester.edu)",
	PLAYS_COMBAT | USES_TEAMS,
	4,
	Guard_main
};


#define BRAKING_ACC (MAX_ACCEL * settings.normal_friction * tread_acc())
#define SLIP_FACTOR (settings.normal_friction / settings.slip_friction)


/* information about a particular vehicle */
typedef struct
{
	Vehicle_info vi;			/* from most recent sighting */
	Vehicle_info old_vi;		/* from previous sighting */
	int frame;					/* frame number when last seen (-1 if never) */
	int old_frame;				/* frame number when previously seen */
} VehicleData;

/* everything I know, packaged into a structure so that it can be easily
   passed to subroutines (all this to avoid global data) */
typedef struct
{
	Vehicle_info me;			/* information on my vehicle */
	int frame;					/* current frame number (clock value) */
	VehicleData v[MAX_VEHICLES];/* information on all vehicles, indexed by
								   vehicle ID */
	VehicleData *target;		/* what I want to attack (points into v[]) */
} All;

static Settings_info settings;	/* can be global because it can be shared
								   with other players */


/* changes a location by the given x and y (in pixel, not grid units) */

static void delta_loc(locp, delta_x, delta_y)
Location *locp;					/* gets modified */
int delta_x, delta_y;			/* in fine units */
{
	locp->x += delta_x;
	locp->y += delta_y;
	locp->grid_x = locp->x / BOX_WIDTH;
	locp->box_x = locp->x % BOX_WIDTH;
	locp->grid_y = locp->y / BOX_HEIGHT;
	locp->box_y = locp->y % BOX_HEIGHT;
}


/* decides if I can make the specified move without hitting a wall, taking the
   vehicle size into account.  This is done by checking the paths of the
   vehicle's bounding-box's corners. */

static self_clear_path(start, delta_x, delta_y)
Location *start;				/* starting position of vehicle */
int delta_x, delta_y;			/* proposed move */
{
	Location s, f;				/* start and finish of a vehicle corner */
	int w, h;					/* width and height of this vehicle (changes
								   with orientation) */

	vehicle_size(&w, &h);
	/* convert to difference from center of vehicle (assume that vehicle
	   location is center of vehicle; it sucks that we can't get offset_x and
	   offset_y) */
	w = (w + 1) / 2;
	h = (h + 1) / 2;

	/* upper left */
	s = *start;
	delta_loc(&s, -w, -h);
	f = s;
	delta_loc(&f, delta_x, delta_y);
	if (!clear_path(&s, &f))
		return FALSE;

	/* upper right */
	s = *start;
	delta_loc(&s, w, -h);
	f = s;
	delta_loc(&f, delta_x, delta_y);
	if (!clear_path(&s, &f))
		return FALSE;

	/* lower right */
	s = *start;
	delta_loc(&s, w, h);
	f = s;
	delta_loc(&f, delta_x, delta_y);
	if (!clear_path(&s, &f))
		return FALSE;

	/* lower left */
	s = *start;
	delta_loc(&s, -w, h);
	f = s;
	delta_loc(&f, delta_x, delta_y);
	if (!clear_path(&s, &f))
		return FALSE;

	return TRUE;
}


/* return true if I must stop now to avoid hitting a wall (deals with slip
   squares but not scroll squares) */

static will_hit_wall(vip)
Vehicle_info *vip;				/* self */
{
	FLOAT tus;					/* time-until-stop under full braking at the
								   current speed (ignoring scroll squares) */

	tus = speed() / BRAKING_ACC;
	if (landmark(vip->loc.grid_x, vip->loc.grid_y) == SLIP)
		tus *= SLIP_FACTOR;
	/* find out if a path is clear to the minimum distance I will go if I
	   DON'T brake now */
	return !self_clear_path(&vip->loc,
							(int) (vip->xspeed * (tus / 2 + TICKSZ)),
							(int) (vip->yspeed * (tus / 2 + TICKSZ)));
}

#ifdef DONTNEED
static int nowall(angle)
FLOAT angle;
{
	Location loc;
	int section;
	FLOAT speed_vehicle;

	speed_vehicle = speed();
	printf("speed = %f\n", speed_vehicle);

	if (speed_vehicle > 2.0 * acc())
		return (1);
	get_location(&loc);

#define OLD

#ifdef OLD
	if (angle > 0 && angle < PI / 2)
	{
		section = EAST;
	}
	else if (angle < PI)
	{
		section = NORTH;
	}
	else if (angle < 3 * PI / 2)
	{
		section = SOUTH;
	}
	else
		section = WEST;
#else
	if (angle > 0 && angle < PI / 4)
	{
		section = EAST;
	}
	else if (angle < 3 / 4 * PI)
	{
		section = NORTH;
	}
	else if (angle < 5 / 4 * PI)
	{
		section = WEST;
	}
	else if (angle < 7 / 4 * PI)
		section = SOUTH;
	else
		section = EAST;
#endif

	return (1 != wall(section, loc.grid_x, loc.grid_y));
}

#endif

/* figure out which vehicle to attack, return false if none is found */

static get_target(allp)
All *allp;
{
	int dx, dy;
	unsigned long min_range, range;
	int id;
	VehicleData *vdp;

#define VERY_FAR 9999999		/* an unreasonably long distance away */

	min_range = VERY_FAR;
	for (id = 0; id < MAX_VEHICLES; ++id)
	{
		vdp = &(allp->v[id]);

		/* ignore vehicles that haven't been spotted this frame */
		if (vdp->frame != allp->frame)
			continue;

		/* Ignore vehicles on my team (unless I'm neutral) */
		if (vdp->vi.team == allp->me.team && allp->me.team != NEUTRAL)
			continue;

		dx = vdp->vi.loc.x - allp->me.loc.x;
		dy = vdp->vi.loc.y - allp->me.loc.y;
		range = dx * dx + dy * dy;
		if (range < min_range)
		{
			min_range = range;
			allp->target = vdp;
		}
	}

	return (min_range == VERY_FAR) ? FALSE : TRUE;
}

static get_friend(allp)
All *allp;
{
	int dx, dy;
	unsigned long min_range;
	int id;
	VehicleData *vdp;

#define VERY_FAR 9999999		/* an unreasonably long distance away */

	min_range = VERY_FAR;
	for (id = MAX_VEHICLES; id >= 0; --id)
	{
		vdp = &(allp->v[id]);

		/* ignore vehicles that haven't been spotted this frame */
		if (vdp->frame != allp->frame)
			continue;

		/* Get vehicles on my team (unless I'm neutral) */
		if (vdp->vi.team != allp->me.team || allp->me.team == NEUTRAL)
			continue;

		dx = vdp->vi.loc.x - allp->me.loc.x;
		dy = vdp->vi.loc.y - allp->me.loc.y;
		min_range = dx * dx + dy * dy;
		allp->target = vdp;
	}

	return (min_range == VERY_FAR) ? FALSE : TRUE;
}


/* return a random float in the range [0, max) */

static FLOAT frand(maxr)
FLOAT maxr;						/* maximum value */
{
	extern long random();

	return (random() & 0xffff) * maxr / 0x10000;
}


static void turn_off_all_weapons()
{
	WeaponNum wn;

	wn = num_weapons();
	while (wn--)
	{
		turn_off_weapon(wn);
	}
}


static void turn_on_all_weapons()
{
	WeaponNum wn;

	wn = num_weapons();
	while (wn--)
	{
		turn_on_weapon(wn);
	}
}


static need_fuel(fraction)
double fraction;				/* I need it if below this */
{
	return fuel() < max_fuel() * fraction;
}


static need_armor(fraction)
double fraction;				/* I need it if below this */
{
	int side;

	for (side = 0; side < MAX_SIDES; ++side)
	{
		if (armor(side) < max_armor(side) * fraction)
			return TRUE;
	}
	return FALSE;
}


static need_ammo(fraction)
double fraction;				/* I need it if below this */
{
	int weapon;
	Weapon_info winfo;

	for (weapon = num_weapons(); weapon-- > 0;)
	{
		get_weapon(weapon, &winfo);
		if (weapon_ammo(weapon) < winfo.max_ammo * fraction)
			return TRUE;
	}
	return FALSE;
}


static at_box_center(selfp)
Vehicle_info *selfp;			/* self */
{
	return (ABS(selfp->loc.box_x - BOX_WIDTH / 2) < LANDMARK_WIDTH / 2 &&
			ABS(selfp->loc.box_y - BOX_HEIGHT / 2) < LANDMARK_HEIGHT / 2);
}

/* moves to the center of the current box (presumably a store).  Returns true
   if I am already there. */

static goto_box_center(selfp)
Vehicle_info *selfp;			/* self */
{
	if (!at_box_center(selfp))
	{
		turn_vehicle(ATAN2(BOX_HEIGHT / 2 - selfp->loc.box_y,
						   BOX_WIDTH / 2 - selfp->loc.box_x));
		set_abs_drive(acc());	/* be able to stop quickly */
		return FALSE;
	}
	set_abs_drive(0.0);
	return TRUE;
}


/* do the appropriate thing with any landmark I might be on.  Returns true if
   something useful is being done. */

static handle_local_landmark(selfp)
Vehicle_info *selfp;			/* self */
{
	switch (landmark(selfp->loc.grid_x, selfp->loc.grid_y))
	{
		case FUEL:
			if (need_fuel(0.99) && at_box_center(selfp))
			{
				set_abs_drive(0.0);
				return TRUE;
			}
			else if (need_fuel(0.9))
			{
				goto_box_center(selfp);
			}
			break;
		case AMMO:
			if (need_ammo(1.0) && at_box_center(selfp))
			{
				set_abs_drive(0.0);
				turn_off_all_weapons();
				return TRUE;
			}
			else if (need_ammo(0.9))
			{
				goto_box_center(selfp);
			}
			break;
		case ARMOR:
			if (need_armor(1.0) && at_box_center(selfp))
			{
				set_abs_drive(0.0);
				return TRUE;
			}
			else if (need_armor(0.9))
			{
				goto_box_center(selfp);
			}
			break;
		case OUTPOST:
			if (settings.outpost_strength)
			{
				set_rel_drive(9.0);		/* Run away!  Run away! */
				return TRUE;
			}
			break;
	}
	return FALSE;				/* didn't find anything to do */
}


/* updates information on all visible vehicles */
static void look_vehicles(allp)
All *allp;
{
	Vehicle_info vi[MAX_VEHICLES];
	register VehicleData *vdp;
	Location loc;
	int i;

	get_location(&loc);

	get_vehicles(&i, vi);
	while (--i >= 0)
	{
		vdp = &(allp->v[vi[i].id]);		/* our data on this vehicle */

		/* copy old info to older */
		vdp->old_vi = vdp->vi;
		vdp->old_frame = vdp->frame;

		/* if blocked by a wall, don't mark this guy */
		if (clear_path(&loc, &vi[i].loc))
		{
			/* fill in new info */
			vdp->vi = vi[i];
			vdp->frame = allp->frame;
		}
	}
}


static void chase_target(allp)
All *allp;
{
	double angle;
	int tx, ty, dx, tsp;				/* anticipated location of target */

	/* current target location */
	tx = allp->target->vi.loc.x;
	ty = allp->target->vi.loc.y;

#if 0							/* something more sophisticated is needed */
	/* simple prediction: its current speed times my time to get to its
	   current location */
	tx += allp->target->vi.xspeed * ABS(tx - allp->me.loc.x) / max_speed();
	ty += allp->target->vi.yspeed * ABS(ty - allp->me.loc.y) / max_speed();
#endif

	/* figure direction of anticipated target location */
	angle = ATAN2(ty - allp->me.loc.y, tx - allp->me.loc.x);

	/* chase 'em */
        if (allp->target->vi.team != allp->me.team || allp->me.team == NEUTRAL)
           set_rel_drive(9.0);
        else {
           dx = (tx-allp->me.loc.x) * (tx-allp->me.loc.x);
           dx += (ty-allp->me.loc.y) * (ty-allp->me.loc.y);
           if (dx > 40000)
              set_rel_drive(9.0);
           else {
              tsp = (allp->target->vi.xspeed) * (allp->target->vi.xspeed);
              tsp += (allp->target->vi.yspeed) * (allp->target->vi.yspeed);
              tsp = (int) SQRT((FLOAT) tsp);
              if (dx < 10000) angle += 2.00;
              else if (dx < 20000) tsp >>= 1;
              else if (dx > 24000) tsp += 1;
              set_abs_drive((FLOAT) tsp);
           }
        }
        turn_vehicle(angle);

	/* blast 'em (if I have any weapons...) */
	turn_all_turrets(angle);
	turn_on_all_weapons();
        if (allp->target->vi.team != allp->me.team || allp->me.team == NEUTRAL)
       	   fire_all_weapons();
}


static vectors_cross(x1, y1, xs1, ys1, x2, y2, xs2, ys2)
int x1, y1;						/* start of first vector */
FLOAT xs1, ys1;					/* speeds of first vector */
int x2, y2;						/* start of second vector */
FLOAT xs2, ys2;					/* speeds of second vector */
{
	FLOAT tx, ty;				/* times until "collision" in x and y */

	if (!xs1 && !ys1 && !xs2 && !ys2)
		return FALSE;

	tx = (x2 - x1) / (xs1 - xs2);
	ty = (y2 - y1) / (ys1 - ys2);

	return (tx >= 0 && tx <= 1 && ty >= 0 && ty <= 1 &&
			ABS(tx - ty) < 0.2);
}


static will_hit_vehicle(me, it)
Vehicle_info *me;				/* assumed to be self */
Vehicle_info *it;
{
	int tus;

	tus = speed() / BRAKING_ACC + (TICKSZ << 1);

	if (vectors_cross(me->loc.x, me->loc.y,
					  me->xspeed * tus, me->yspeed * tus,
					  it->loc.x, it->loc.y,
					  it->xspeed * tus, it->yspeed * tus))
	{
		return TRUE;
	}

	return FALSE;
}


/* not perfect, but not overly conservative either */

static void vehicle_collide_check(allp)
All *allp;
{
	int id;
	VehicleData *vdp;

	for (id = 0; id < MAX_VEHICLES; ++id)
	{
		vdp = &(allp->v[id]);

		/* only do vehicles I can see */
		if (vdp->frame != allp->frame)
			continue;

		/* only check vechicles on my team */
		if (vdp->vi.team != allp->me.team || allp->me.team == NEUTRAL)
			continue;

		if (will_hit_vehicle(&allp->me, &vdp->vi))
		{
			/* go parallel to them */
			turn_vehicle(ATAN2(vdp->vi.yspeed, vdp->vi.xspeed));
			break;
		}
	}
}


static void main_loop(allp)
All *allp;
{
	while (1)
	{
		/* get fresh data */
		get_self(&allp->me);
		allp->frame = frame_number();
		look_vehicles(allp);

		if (get_target(allp))
		{
			chase_target(allp);
		}
                else if (get_friend(allp))
                {
                        chase_target(allp);
                }
		else if (!handle_local_landmark(&allp->me))
		{
			/* nothing to do, so maybe go somewhere and hope to find an enemy
			   (this is slightly wasteful of fuel though) */
			set_rel_drive(3.0);
			if (speed() == 0 || !(random() & 127))
			{
				turn_vehicle((Angle) frand(2 * PI));
			}
		}

		/* check for collision with other vehicles */
		vehicle_collide_check(allp);

		/* last but not least, do wall avoidance */
		if (will_hit_wall(&allp->me))
		{
			double angle;

			/* stop unless I am pointing away from the direction of motion */
			angle = heading() - ATAN2(allp->me.yspeed, allp->me.xspeed);
			angle = fixed_angle(angle);
			if (angle < PI / 2 || angle > PI * 3 / 2)
			{
				set_abs_drive(0.0);
			}
		}

		done();					/* wait for start of next frame */
	}
}


static void Guard_main()
{
	All all;
	int i;

	get_settings(&settings);

	/* initialize vehicle information to unknown */
	for (i = 0; i < MAX_VEHICLES; ++i)
	{
		all.v[i].frame = all.v[i].old_frame = -1;
	}

	main_loop(&all);
}
