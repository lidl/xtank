/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** update.c
*/

#include "malloc.h"
#include "xtank.h"
#include "disc.h"
#include "loc.h"
#include "vstructs.h"
#include "sysdep.h"
#include "bullet.h"
#include "terminal.h"

extern Boolean intersect_wall();

extern Map box;
extern int frame;
extern Settings settings;
extern int num_vehicles;
extern Vehicle *vehicle[];
extern Eset *eset;
extern Bset *bset;
extern Weapon_stat weapon_stat[];
extern Tread_stat tread_stat[];


#ifdef AMIGA
#include "screen.h"

/*
** Makes loc = old_loc + (dx,dy)
*/
update_loc(old_loc, loc, dx, dy)
Loc *old_loc, *loc;
int dx, dy;
{
		(loc)->x = (old_loc)->x + dx;
		(loc)->y = (old_loc)->y + dy;

		(loc)->box_x = (old_loc)->box_x + dx;
		(loc)->box_y = (old_loc)->box_y + dy;

    if ((loc)->box_x >= BOX_WIDTH) {
			(loc)->box_x -= BOX_WIDTH;
			(loc)->grid_x = (old_loc)->grid_x + 1;
    } else if ((loc)->box_x < 0) {
			(loc)->box_x += BOX_WIDTH;
			(loc)->grid_x = (old_loc)->grid_x - 1;
    } else (loc)->grid_x = (old_loc)->grid_x;

    if ((loc)->box_y >= BOX_HEIGHT) {
			(loc)->box_y -= BOX_HEIGHT;
			(loc)->grid_y = (old_loc)->grid_y + 1;
    } else if ((loc)->box_y < 0) {
			(loc)->box_y += BOX_HEIGHT;
			(loc)->grid_y = (old_loc)->grid_y - 1;
    } else (loc)->grid_y = (old_loc)->grid_y;
		}

#endif							/* AMIGA */

/*
** Computes the state of the specified vehicle for one frame.
*/
update_vehicle(v)
Vehicle *v;
{
	Loc *loc, *old_loc;
	Vector *vector;
	Box *b;
	float xadj, yadj;
	int i;

	vector = &v->vector;

	/* Decrement all weapon reload counters */
	if (v->num_weapons > 0)
		for (i = 0; i < v->num_weapons; i++)
			if (v->weapon[i].reload_counter > 0)
				v->weapon[i].reload_counter--;

	/* Decrement fuel if the settings allow it */
	if (v->fuel > 0)
	{
		if (!settings.si.no_wear)
		{
			v->fuel -= FUEL_CONSUME * MAX_SPEED *
				((vector->drive * vector->drive) /
				 (v->vdesc->max_speed * v->vdesc->max_speed));
		}
	}
	else
		v->fuel = 0;

	/* Decrement heat by heat_sinks every five frames */
	if ((frame % 5) == 0)
	{
		v->heat -= v->vdesc->heat_sinks + 1;
		if (v->heat < 0)
			v->heat = 0;
	}
	/* Stop vehicle from sliding every 16 frames */
	if (v->status & VS_sliding)
	{
		if (!v->slide_count--)
			v->status &= ~VS_sliding;
	}

	/* Update vector */
	update_vector(v);

	/* Get pointer to box vehicle is in */
	b = &box[v->loc->grid_x][v->loc->grid_y];

	/* Handle interesting box types after xspeed and yspeed calculation */
	box_type_check(v, b, &xadj, &yadj);

	/* Update location */
	loc = v->old_loc;
	v->old_loc = old_loc = v->loc;
	v->loc = loc;

	update_loc(old_loc, loc, vector->xspeed + xadj, vector->yspeed + yadj);

	/* Update turrets */
	if (v->num_turrets > 0)
		for (i = 0; i < v->num_turrets; i++)
			update_turret(&v->turret[i]);
}

/*
** Updates the motion vector for the vehicle.
*/
update_vector(v)
Vehicle *v;
{
	Vector *vector = &v->vector;
	float turning_rate;			/* fastest they can turn at current speed */
	float friction;				/* friction with the ground */
	float heading_diff;			/* difference between direction it is moving
								   and direction it is pointing */
	float roll_speed;			/* how fast it is rolling forward */
	float slide_speed;			/* how fast is is sliding sideways */
	float accel_lim;			/* the limit to acceleration imposed by
								   ground and tread friction */
	float desired_acc;			/* how much more speed the driver wants */


	/* don't let them turn or accelerate if they are slicked with oil */
	if (v->status & VS_sliding)
		return;

	/* Update heading */

	if (v->fuel > 0)
	{
        int spd = 0;

		if (v->safety == TRUE)
		{						/* pay attention to speed? */
            spd = ABS((int) vector->speed);
            if (spd > MAX_SPEED)
			{
                spd = MAX_SPEED;
			}
		}
        turning_rate = v->turn_rate[spd];
	}
	else
	{
		turning_rate = 0;		/* can't turn with no fuel */
	}

	switch (vector->heading_flag)
	{
		case CLOCKWISE:
			if ((vector->heading += turning_rate) >= vector->desired_heading)
			{
				vector->heading = vector->desired_heading;
                vector->heading_flag = NO_SPIN;
			}
			break;
		case COUNTERCLOCKWISE:
			if ((vector->heading -= turning_rate) <= vector->desired_heading)
			{
				vector->heading = vector->desired_heading;
                vector->heading_flag = NO_SPIN;
			}
			break;
	}

    /* when out of fuel, pretend they want to stop (actually, we ought to let
       them coast to a stop, but that involves figuring out what the rolling
       friction should be, ugh) */
	if (v->fuel <= 0)
		vector->drive = 0.0;

	/* break velocity into components w.r.t the heading */
	heading_diff = vector->angle - vector->heading;
	roll_speed = cos(heading_diff) * vector->speed;
	slide_speed = sin(heading_diff) * vector->speed;

    /* determine the ground friction */
    friction = (v->vdesc->treads != 4 &&	/* not hover treads */
		box[v->loc->grid_x][v->loc->grid_y].type == SLIP) ?
			settings.si.slip_friction :
			settings.si.normal_friction;

    /* modify that by the tread friction to produce the total traction */
	friction *= tread_stat[v->vdesc->treads].friction;

    /* and convert that to an acceleration (note that the mass of the vehicle
       falls out of the equation, since the frictional force is proportional to
       the mass) */
	accel_lim = friction * MAX_ACCEL;

	/* reduce sliding speed by accel_lim */
    if (ABS(slide_speed) > accel_lim)
	{
		slide_speed -= accel_lim * SIGN(slide_speed);
	/* %%% vehicle is skidding, so perhaps we should reduce accel_lim
	   (dynamic friction < static friction).  Would provide a motivation
	   for using the "safe turning" option. */
	}
	else
	{
		slide_speed = 0;
	}

    /* figure out how much they want to accelerate */
	if (v->num_discs > 0)
	{
	/* if they have discs, pretend they want to go slower (more discs does
	   NOT mean more slowdown) */
        desired_acc = (vector->drive * settings.si.owner_slowdown) -
	    roll_speed;
	}
	else
	{
		desired_acc = vector->drive - roll_speed;
	}

    /* if they are trying to speed up, take engine power limit into account
       (otherwise they are braking, and that's only limited by traction) */
    if (ABS(roll_speed + desired_acc) > ABS(roll_speed))
	{
        accel_lim = MIN(accel_lim, v->vdesc->acc);
	}

    /* don't let them accelerate/decelerate more than accel_lim */
    if (ABS(desired_acc) > accel_lim)
	{
        desired_acc = accel_lim * SIGN(desired_acc);
	}

    /* do the accelaration (finally) */
	roll_speed += desired_acc;

	/* Compute new angle and speed */
	vector->xspeed = cos(vector->heading) * roll_speed +
		cos(vector->heading + PI / 2) * slide_speed;
	vector->yspeed = sin(vector->heading) * roll_speed +
		sin(vector->heading + PI / 2) * slide_speed;

	assign_speed(vector, vector->xspeed, vector->yspeed);
}

/*
** Computes the rotation (0 to 16) from the heading (-PI to PI).
*/
update_rotation(v)
Vehicle *v;
{
	int views;

	views = v->obj->num_pics;
	v->vector.old_rot = v->vector.rot;
    v->vector.rot = ((int)(v->vector.heading / (2*PI) * views + views + .5)) %
	views;
}

update_turret(t)
Turret *t;
{
	float delta_angle;
	int views;
	Boolean angle_changed = TRUE;

	t->old_rot = t->rot;
	delta_angle = t->turn_rate;
	switch (t->angle_flag)
	{
        case NO_SPIN:
			angle_changed = FALSE;
			break;
		case CLOCKWISE:
			if ((t->angle += delta_angle) >= t->desired_angle)
			{
				t->angle = t->desired_angle;
                t->angle_flag = NO_SPIN;
			}
			break;
		case COUNTERCLOCKWISE:
			if ((t->angle -= delta_angle) <= t->desired_angle)
			{
				t->angle = t->desired_angle;
                t->angle_flag = NO_SPIN;
			}
			break;
	}
	if (angle_changed == TRUE)
	{
		views = t->obj->num_pics;
		t->rot = ((int) ((t->angle) / (2 * PI) * views + views + .5)) % views;
	}
}

/*
** Computes new locations for all the bullets, and removes dead ones.
*/
update_bullets()
{
	Loc *loc, *old_loc;
	Bullet *b;
	int i;

	for (i = 0; i < bset->number; i++)
	{
		b = bset->list[i];

		/* decrement life and see if it's dead */
		while (--b->life < 0)
		{
			/* if not last on list fill up hole, otherwise, do next bullet */
			if (i != --bset->number)
			{
				bset->list[i] = bset->list[bset->number];
				bset->list[bset->number] = b;
				b = bset->list[i];
			}
			else
				break;
		}

		/* If it is a mine, seeker, slick, or disc run the special code for
		   it */
		switch (b->type)
		{
			case MINE:
			case SLICK:
				update_mine(b);
				break;
			case SEEKER:
				update_seeker(b);
				break;
			case DISC:
				update_disc(b);
				break;
		}

		/* Update the bullet location */
		loc = b->old_loc;
		old_loc = b->old_loc = b->loc;
		b->loc = loc;

		update_loc(old_loc, loc, b->xspeed, b->yspeed);
	}
}

/*
** Stops the bullet after 5 frames of movement and lets it hurt its owner.
*/
update_mine(b)
Bullet *b;
{
	int same_loc = FALSE;
	extern Terminal *term;

    if (b->life == weapon_stat[(int)b->type].frames - 5)
	{
		/* if a mine already exists here, continue mine for 2 frames. this
		   will avoid the mines cancelling each other out do to XOR. */

#ifdef NOTINUSE
		for (i = 0; i < bset->number; i++)
		{
			bptr = bset->list[i];
			if (bptr != b)
			{
				if (bptr->type == b->type)
				{
					if (term)
						j = term->num;
					else
						j = 0;

					if (bptr->loc->screen_x[j] == b->loc->screen_x[j] &&
							bptr->loc->screen_y[j] == b->loc->screen_y[j])
					{
						if (bptr->xspeed == 0.0 && bptr->yspeed == 0.0)
						{
							same_loc = TRUE;
							break;
						}
					}
				}
			}
		}
#endif

		if (same_loc)
		{
			b->life -= 2;
		}
		else
		{
			b->xspeed = b->yspeed = 0.0;
			b->hurt_owner = TRUE;
		}
	}
}

#define SEEKER_ACC 2			/* For grins, change back to 2 soon - JMO */

/*
** Looks for heat sources and moves towards them, leaving a trail of exhaust.
*/
update_seeker(b)
Bullet *b;
{
	Loc *loc;
    float accel, sp, sp2, axs, ays, xdir, ydir;
	int dx, dy, seek, best_dx, best_dy, best_seek, best_heat, i;

	/* Make a trail of exhaust */
	make_explosion(b->loc, EXP_EXHAUST);

	/* Find all vehicles that would affect heat seeking */
	best_seek = 0;
	for (i = 0; i < num_vehicles; i++)
	{
		/* * Is vehicle within 3 boxes, in line of sight, in front of the
		   seeker and * hotter/closer than the previous targets? */
		loc = vehicle[i]->loc;
		dx = (int) (loc->x - b->loc->x);
		dy = (int) (loc->y - b->loc->y);

        seek = (50 + vehicle[i]->heat) * (BOX_WIDTH * BOX_WIDTH * 9 -
					  (dx * dx + dy * dy));
		if (seek > best_seek &&
				(dx * b->xspeed + dy * b->yspeed > 0) &&
				!intersect_wall(b->loc, loc))
		{
			best_seek = seek;
			best_heat = vehicle[i]->heat;
			best_dx = dx;
			best_dy = dy;
		}
	}

	/* If we found something to seek, adjust speed components to follow it */
	if (best_seek > 0 && (rnd(30) < best_heat))
	{
        sp = weapon_stat[(int)b->type].ammo_speed;
		sp2 = sp * sp;
		xdir = ((b->xspeed > 0) ? 1 : -1);
		ydir = ((b->yspeed > 0) ? 1 : -1);
        axs = ABS(b->xspeed);
        ays = ABS(b->yspeed);
		if (b->xspeed * best_dy < b->yspeed * best_dx)
            accel = SEEKER_ACC;
		else
            accel = -SEEKER_ACC;

		if (axs > ays)
		{
            b->yspeed -= xdir * accel;
            if (ABS(b->yspeed) >= sp)
			{
				b->xspeed = 0;
				b->yspeed = ydir * sp;
			}
			else
				b->xspeed = xdir * sqrt(sp2 - b->yspeed * b->yspeed);
		}
		else
		{
            b->xspeed += ydir * accel;
            if (ABS(b->xspeed) >= sp)
			{
				b->yspeed = 0;
				b->xspeed = xdir * sp;
			}
			else
				b->yspeed = ydir * sqrt(sp2 - b->xspeed * b->xspeed);
		}
	}
}

/*
** Applies friction to disc in free flight, and computes its orbit when owned.
*/
update_disc(b)
Bullet *b;
{
	float dx, dy;
	float dist;
	float angle, delta;

	/* If the disc is owned by someone, change its velocity to orbit him */
	if (b->owner != (Vehicle *) NULL)
	{
		/* compute the angle to the vehicle */
		dx = b->owner->loc->x - b->loc->x;
		dy = b->owner->loc->y - b->loc->y;
        angle = ATAN2(dy, dx);
		dist = dx * dx + dy * dy;

		/* Compute a delta which will bring us into orbit around our owner */
		if (dist <= DISC_ORBIT_SQ)
			delta = PI / 2 * (2 - (dist / DISC_ORBIT_SQ));
		else
			delta = PI / 2 * (DISC_ORBIT_SQ / dist);

		/* If disc_spin is set, orbit counterclockwise, otherwise, clockwise */
		if (b->owner->status & VS_disc_spin)
			angle += delta;
		else
			angle -= delta;

		/* Compute new xspeed and yspeed */
		b->xspeed = DISC_MED_SPEED * cos(angle);
		b->yspeed = DISC_MED_SPEED * sin(angle);
	}
	/* otherwise slow the disc down a bit */
	else
	{
		b->xspeed *= settings.si.disc_friction;
		b->yspeed *= settings.si.disc_friction;
	}
}

/*
** Decrements explosion lives and removes dead explosions from the set.
*/
update_explosions()
{
	Exp *e;
	int i;

	for (i = 0; i < eset->number; i++)
	{
		e = eset->list[i];
		/* Decrement life to see if it's dead */
		while (!(e->life--))
		{
			/* If not last one, move it */
			if (i != --eset->number)
			{
				eset->list[i] = eset->list[eset->number];
				eset->list[eset->number] = e;
				e = eset->list[i];
			}
		}
	}
}

/*
** Moves vehicle maze flags around when vehicles move from box to box.
*/
update_maze_flags()
{
	Vehicle *v;
	Loc *loc, *old_loc;
	int i;

	for (i = 0; i < num_vehicles; i++)
	{
		v = vehicle[i];

		/* don't do anything if the vehicle isn't alive */
		if (!(v->status & VS_is_alive))
			continue;

		loc = v->loc;
		old_loc = v->old_loc;
		if ((loc->grid_x != old_loc->grid_x) || (loc->grid_y != old_loc->grid_y))
		{
			box[old_loc->grid_x][old_loc->grid_y].flags &= ~v->flag;
			box[loc->grid_x][loc->grid_y].flags |= v->flag;
		}
	}
}

/*
** Updates all specials for all vehicles.
*/
update_specials()
{
	Vehicle *v;
	int i, num;

	for (i = 0; i < num_vehicles; i++)
	{
		v = vehicle[i];
		for (num = 0; num < MAX_SPECIALS; num++)
            do_special(v, (SpecialType)num, SP_update);
	}
}

/*
** Updates the screen locations of the terminal, vehicles, bullets,
** and explosions for the current terminal.
*/
update_screen_locs()
{
	extern Terminal *term;
	Vehicle *v;
	Exp *e;
	Loc *bloc;
	int sx, sy;
	int i;

    /* If terminal is tracking a vehicle, compute screen loc from vehicle loc
       */
	v = term->vehicle;
	if (v != (Vehicle *) NULL)
	{
		/* Compute the loc of the upper left corner of the animation window */
		term->loc.grid_x = v->loc->grid_x - NUM_BOXES / 2;
		term->loc.grid_y = v->loc->grid_y - NUM_BOXES / 2;
		term->loc.x = v->loc->x - NUM_BOXES * BOX_WIDTH / 2;
		term->loc.y = v->loc->y - NUM_BOXES * BOX_HEIGHT / 2;
	}
	/* Terminal screen offset */
	sx = term->loc.x;
	sy = term->loc.y;

	/* Compute screen coordinates for the vehicles */
	for (i = 0; i < num_vehicles; i++)
	{
		v = vehicle[i];
		v->loc->screen_x[term->num] = v->loc->x - sx;
		v->loc->screen_y[term->num] = v->loc->y - sy;
	}

	/* Compute the screen coordinates for the bullets */
	for (i = 0; i < bset->number; i++)
	{
		bloc = bset->list[i]->loc;
		bloc->screen_x[term->num] = bloc->x - sx;
		bloc->screen_y[term->num] = bloc->y - sy;
	}

	/* Compute the screen coordinates for the explosions */
	for (i = 0; i < eset->number; i++)
	{
		e = eset->list[i];
		e->old_screen_x[term->num] = e->screen_x[term->num];
		e->old_screen_y[term->num] = e->screen_y[term->num];
		e->screen_x[term->num] = e->x - sx;
		e->screen_y[term->num] = e->y - sy;
	}
}
