/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** update.c
*/

/*
$Author: stripes $
$Id: update.c,v 2.8 1992/02/10 05:26:49 stripes Exp $

$Log: update.c,v $
 * Revision 2.8  1992/02/10  05:26:49  stripes
 * Exaust trails for various crap.
 *
 * Revision 2.7  1992/01/29  08:37:01  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.6  1991/12/10  03:41:44  lidl
 * changed float to FLOAT, for portability reasons
 *
 * Revision 2.5  1991/11/22  06:01:12  stripes
 * Changed hover's skid & safety
 *
 * Revision 2.4  1991/10/27  22:34:27  aahz
 * made unguided missles have an exhaust trail.
 *
 * Revision 2.3  1991/02/10  13:51:55  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:14  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:18  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:43  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:14  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "disc.h"
#include "loc.h"
#include "vstructs.h"
#include "sysdep.h"
#include "bullet.h"
#include "terminal.h"
#include "globals.h"
#ifndef NO_NEW_RADAR
#include "graphics.h"
#endif /* !NO_NEW_RADAR */

extern Boolean intersect_wall();

extern Map real_map;
extern int frame;
extern Settings settings;
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
    FLOAT xadj, yadj;
    int i;

    vector = &v->vector;

    /* Decrement all weapon reload counters */
    if (v->num_weapons > 0)
	for (i = 0; i < v->num_weapons; i++)
	    if (v->weapon[i].reload_counter > 0)
		v->weapon[i].reload_counter--;

    /* Decrement fuel if the settings allow it */
    if (v->fuel > 0) {
	if (!settings.si.no_wear) {
	    v->fuel -= FUEL_CONSUME * MAX_SPEED *
		    ((vector->drive * vector->drive) /
		     (v->vdesc->max_speed * v->vdesc->max_speed));
	}
    } else
	v->fuel = 0;

    /* Decrement heat by heat_sinks every five frames */
    if ((frame % 5) == 0) {
	v->heat -= v->vdesc->heat_sinks + 1;
	if (v->heat < 0)
	    v->heat = 0;
    }
    /* Stop vehicle from sliding every 16 frames */
    if (v->status & VS_sliding) {
	if (!v->slide_count--)
	    v->status &= ~VS_sliding;
    }
    /* Update vector */
    update_vector(v);

    /* Get pointer to box vehicle is in */
    b = &real_map[v->loc->grid_x][v->loc->grid_y];

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


#if 0				/* basically a very old version (but maybe
				   should be used for hover treads -- it's a
				   better model for that) */
update_vector(v)
    Vehicle *v;
{
    FLOAT xdrive, ydrive, xaccel, yaccel;
    FLOAT turning_rate, friction, max_diff, accel, ratio;
    Box *b;
    Vector *vector = &v->vector;

    /* don't let them turn or accelerate if they are slicked with oil */
    if (v->status & VS_sliding)
	return;

    /* Update heading */
    if (v->fuel > 0) {
	int spd;

	if (v->safety == TRUE) { /* speed limits turning? */
	    spd = ABS((int) vector->speed);
	    if (spd > MAX_SPEED) {
		spd = MAX_SPEED;
	    }
    } else {
	    spd = 0;
    }

	turning_rate = v->turn_rate[spd];

    switch (vector->heading_flag) {
	case CLOCKWISE:
	    if ((vector->heading += turning_rate) >= vector->desired_heading) {
		vector->heading = vector->desired_heading;
		vector->heading_flag = NO_SPIN;
	    }
	    break;
	case COUNTERCLOCKWISE:
	    if ((vector->heading -= turning_rate) <= vector->desired_heading) {
		vector->heading = vector->desired_heading;
		vector->heading_flag = NO_SPIN;
	    }
	    break;
    }
    } else {
	vector->drive = 0.0;
    }

    xdrive = cos(vector->heading) * vector->drive;
    ydrive = sin(vector->heading) * vector->drive;

    /* If the vehicle owns discs, decrease the drive appropriately */
    if(v->num_discs > 0) {
	xdrive *= settings.si.owner_slowdown;
	ydrive *= settings.si.owner_slowdown;
    }

    /* Compute acceleration needed to make speed match drive */
    xaccel = xdrive - vector->xspeed;
    yaccel = ydrive - vector->yspeed;

    /* Amount of friction depends on box type */
    friction = (v->vdesc->treads != HOVER_TREAD &&
		real_map[v->loc->grid_x][v->loc->grid_y].type == SLIP) ?
		    settings.si.slip_friction : settings.si.normal_friction;

    /* If acc isn't in same direction as motion, then you're braking */
    max_diff = ((vector->xspeed * xdrive + vector->yspeed * ydrive <= 0) ?
		v->vdesc->tread_acc : v->vdesc->engine_acc) * friction;

    /* Make certain you're not accelerating by more than allowed amount */
    accel = sqrt(xaccel*xaccel + yaccel*yaccel);
    if (accel > max_diff) {
	ratio = max_diff/accel;
	xaccel *= ratio;
	yaccel *= ratio;
    }

    /* Compute new angle and speed */
    vector->yspeed += yaccel;
    vector->xspeed += xaccel;
    vector->speed = hypot(vector->yspeed, vector->xspeed);
    vector->angle = ATAN2(vector->yspeed, vector->xspeed);
}

#endif				/* old stuff */


/* Updates the velocity and orientation of the given vehicle.  Rotation done is
   unrealistically.  So is acceleration, but a somewhat more realistic
   algorthim I tried was less fun.  Hovertreads should be treated separately,
   but are not.  */

update_vector(v)
    Vehicle *v;
{
    Vector *vector = &v->vector;
    FLOAT roll_speed;		/* how fast vehicle is rolling forward */
    FLOAT slide_speed;		/* how fast vehicle is sliding sideways */
    FLOAT roll_acc, slide_acc;
    FLOAT traction;		/* the limit to acceleration imposed by ground
				   and tread friction */
    FLOAT desired_speed;	/* how fast driver wants to go */
    FLOAT desired_acc;		/* how much the driver wants to speed up */
    FLOAT drive_acc;		/* acceleration from engine power */
    FLOAT ground_friction;

    /* don't let them turn or accelerate if they are slicked with oil
       (actually, air friction should be taken into account, but I'm lazy) */
    if (v->status & VS_sliding)
        return;

    /* determine the ground friction here */
    if (v->vdesc->treads == HOVER_TREAD) {
        ground_friction = settings.si.normal_friction;
    } else {
    /* on slip square? */
        ground_friction =
            real_map[v->loc->grid_x][v->loc->grid_y].type == SLIP ?
            settings.si.slip_friction :
            settings.si.normal_friction;
    }

    /* determine the traction between treads and ground */
    traction = ground_friction * v->vdesc->tread_acc;

	if (v->vdesc->treads == HOVER_TREAD)
	{
		traction = 1.0;
	}

    if (v->fuel > 0) {
        FLOAT turning_rate;

        if (v->safety == TRUE && traction < vector->speed) {
            turning_rate = asin(traction / vector->speed);
            if (turning_rate > v->max_turn_rate)
                turning_rate = v->max_turn_rate;
        } else {
            turning_rate = v->max_turn_rate;
        }

        switch (vector->heading_flag) {
              case CLOCKWISE:
            if ((vector->heading += turning_rate) >= vector->desired_heading) {
                vector->heading = vector->desired_heading;
                vector->heading_flag = NO_SPIN;
            }
            break;
          case COUNTERCLOCKWISE:
            if ((vector->heading -= turning_rate) <= vector->desired_heading) {
                vector->heading = vector->desired_heading;
                vector->heading_flag = NO_SPIN;
            }
            break;
        }
    } else {			/* no fuel? */
	/* pretend they want to stop (actually, we ought to let them coast to a
	   stop, but that involves figuring out what the rolling friction
	   should be, ugh) */
        vector->drive = 0.0;
    }

    /* break up the motion into components W.R.T. the orientation of the
       vehicle */
    {
        FLOAT heading_diff;	/* difference between direction it is moving
                   and direction it is facing */

        heading_diff = vector->angle - vector->heading;
        roll_speed = cos(heading_diff) * vector->speed;
        slide_speed = sin(heading_diff) * vector->speed;
    }

    if (slide_speed != 0.0 &&
        v->vdesc->treads != HOVER_TREAD &&
        v->safety == FALSE) {
        traction *= 0.7;	/* dynamic friction < stiction */
    }

    /* figure out what they WANT to do */
    desired_speed = vector->drive;
    if (v->num_discs > 0) {
        /* if they have discs, pretend they want to go slower (more discs
           does NOT mean more slowdown) */
        desired_speed *= settings.si.owner_slowdown;
    }
    desired_acc = desired_speed - roll_speed;

    /* take engine power limit into account */
    if (ABS(desired_acc) > v->vdesc->engine_acc) {
        drive_acc = v->vdesc->engine_acc * SIGN(desired_acc);
    } else {
        drive_acc = desired_acc;
    }

    if (drive_acc * roll_speed >= 0) { /* speed up? */
    /* deal with sideways skidding */
    if (ABS(slide_speed) <= traction) { /* enough to stop? */
        slide_acc = -slide_speed;
    } else {
        slide_acc = traction * SIGN(-slide_speed);
    }
    roll_acc = drive_acc;
    } else {			/* braking */
	FLOAT scale = vector->speed / traction;

	/* the acceleration due to skidding acts in a direction opposite the
	   direction of motion */
	if (scale < 1.0) {	/* traction sufficient to stop? */
        roll_acc = -roll_speed;
        slide_acc = -slide_speed;
    } else {
        /* acceleration is limited by traction */
        roll_acc = -roll_speed / scale;
        slide_acc = -slide_speed / scale;
    }

    /* engine contributes to the braking effort (fun, not realistic) */
	roll_acc += drive_acc;

	if (ABS(desired_acc) < ABS(roll_acc)) {
        roll_acc = desired_acc; /* don't over-compensate */
    }
    }

    /* make sure that the magnitude of the acceleration does not exceed the
       traction limit */
    {
    FLOAT total_acc = hypot(roll_acc, slide_acc);

    if (total_acc > traction) {
        FLOAT scale = traction / total_acc;

        roll_acc *= scale;
        slide_acc *= scale;
    }
    }

    /* add the roll_acc and slide_acc to the motion */
    vector->xspeed += cos(vector->heading) * roll_acc +
	cos(vector->heading + PI/2) * slide_acc;
    vector->yspeed += sin(vector->heading) * roll_acc +
	sin(vector->heading + PI/2) * slide_acc;
    vector->speed = hypot(vector->yspeed, vector->xspeed);
    vector->angle = ATAN2(vector->yspeed, vector->xspeed);
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
    FLOAT delta_angle;
    int views;
    Boolean angle_changed = TRUE;

    t->old_rot = t->rot;
#ifdef TEST_TURRETS
    t->old_end.y = t->end.y;
    t->old_end.x = t->end.x;
#endif /* TEST_TURREST */
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
#ifdef TEST_TURRETS
	t->end.x = cos(t->angle) * TURRET_LENGTH;
	t->end.y = sin(t->angle) * TURRET_LENGTH;
#endif /* TEST_TURRETS */
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
			case PROCKET:
			case UMISSLE:
                make_explosion(b->loc, EXP_EXHAUST);
				break;
			case DISC:
				update_disc(b);
				break;
#ifndef NO_NEW_RADAR
			case HARM:
				update_harm(b);
				break;
#endif /* !NO_NEW_RADAR */
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
    FLOAT accel, sp, sp2, axs, ays, xdir, ydir;
    int dx, dy, seek, best_dx, best_dy, best_seek, best_heat, i;

    /* Make a trail of exhaust */
    make_explosion(b->loc, EXP_EXHAUST);

    /* Find all vehicles that would affect heat seeking */
    best_seek = 0;
    for (i = 0; i < num_veh_alive; i++) 
    {
	/* Is vehicle within 3 boxes, in line of sight, in front of the seeker
	   and hotter/closer than the previous targets? */
	loc = live_vehicles[i]->loc;
	dx = (int) (loc->x - b->loc->x);
	dy = (int) (loc->y - b->loc->y);

        seek = (50 + live_vehicles[i]->heat) * (BOX_WIDTH * BOX_WIDTH * 9 -
					  (dx * dx + dy * dy));
	if (seek > best_seek &&
	    (dx * b->xspeed + dy * b->yspeed > 0) &&
	    !intersect_wall(b->loc, loc))
	{
	    best_seek = seek;
	    best_heat = live_vehicles[i]->heat;
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

#ifndef NO_NEW_RADAR

/*
 * Adapted from "Graphics Gems"
 * gives approximate distance from loc1 to loc2
 * with only overestimations, and then never by more
 * than (9/8) + one bit uncertainty.
 */

/* !!! Wrong place for this, move! !!! */

long idist(loc1, loc2)
lCoord loc1, loc2;
{
    if ((loc2.x -= loc1.x) < 0) loc2.x = -loc2.x;
    if ((loc2.y -= loc1.y) < 0) loc2.y = -loc2.y;
    return (loc2.x + loc2.y - (((loc2.x>loc2.y) ? loc2.y : loc2.x) >> 1) );
}

/*
 * 
 */

#define HARM_ACC 4

update_harm(b)
Bullet *b;
{
    Loc *loc, best_loc;
    lCoord bCoord, vCoord, best_vCoord;
    float accel, sp, sp2, axs, ays, xdir, ydir;
    int i;
    
    int dx, dy, best_dx, best_dy;
    Vehicle *best_v;
    long best_dist, dist;
    float angle;
    int projected_x, projected_y, time_to_target;
    int my_speed;

   make_explosion(b->loc, EXP_EXHAUST);
   /*
    * If age > BOOST_PHASE
    *   perform updates
    * else if age = BOOST_PHASE
    *   set elevation to flying
    */

    b->state = RED;

    if (weapon_stat[(int)b->type].frames - b->life > BOOST_PHASE)  {


       /* 
        * Look at each vehicle, at determine it's brightness relative to
        * the last target coordinate.
        */

        best_dist = 1<<30;

       /*
        * don't look for radar when we are within 1 boxes (they see us by now)
        */

	bCoord.x = b->loc->x;
	bCoord.y = b->loc->y;


        if (idist(bCoord, b->target) > BOX_WIDTH) 
            for (i = 0; i < num_veh_alive; i++) {


	   /*
            * Rule out team members (and ourselves), 
            * those with radar off,  
            * those who are IFF friends
	    *
	    * Find the vehicle and coordinates that are emitting closest to
	    * the stored coordinates.  It would be cheating to save the actual
	    * vehicle pointer we found last update!
            */

	    if ( (b->owner->team == live_vehicles[i]->team)
                 || (live_vehicles[i]->special[(SpecialType)NEW_RADAR].status != SP_on)
                 || (live_vehicles[i]->special[(SpecialType)RADAR].status != SP_on)
                 || (live_vehicles[i]->have_IFF_key[b->owner->number]) 
		)
                continue;


	    vCoord.x = live_vehicles[i]->loc->x;
	    vCoord.y = live_vehicles[i]->loc->y;

	    dist = idist(b->target, vCoord);

	    if (dist < best_dist) {
	        best_v = live_vehicles[i];
	        best_dist = dist;
	        best_vCoord = vCoord;
		b->state = GREEN;
            }

        } 


       /* 
        *  If nothing was emitting 2 boxes from our target, (or we are
	*  within two boxes of our target, as best_dist will still be a
	*  big num) look for something on radar near our target 
	*  coordinates.
        */

#define MAGIC_RCS 10

        if (best_dist > BOX_WIDTH) for (i = 0; i < num_veh_alive; i++) {


	    if ( (b->owner->team == live_vehicles[i]->team)
               || (live_vehicles[i]->have_IFF_key[b->owner->number])
	       || (live_vehicles[i]->rcs < MAGIC_RCS)
		 )
                continue;
		
	    vCoord.x = live_vehicles[i]->loc->x;
	    vCoord.y = live_vehicles[i]->loc->y;

	    dist = idist(b->target, vCoord);

	    if (dist < best_dist) {
	        best_v = live_vehicles[i];
	        best_dist = dist;
	        best_vCoord = vCoord;
		b->state = YELLOW;
            }
	}

	/*
         * make sure we locked on to something, then use code stolen from seeker
         */

	if (best_dist != 1<<30) {

	    b->target = best_vCoord;
	    best_dx = (int) (best_v->loc->x - b->loc->x);
	    best_dy = (int) (best_v->loc->y - b->loc->y);

	    sp = weapon_stat[(int)b->type].ammo_speed;
	    sp2 = sp * sp;

	    xdir = ((b->xspeed > 0) ? 1 : -1);
	    ydir = ((b->yspeed > 0) ? 1 : -1);

	    axs = ABS(b->xspeed);
	    ays = ABS(b->yspeed);

	    if (b->xspeed * best_dy < b->yspeed * best_dx)
		accel = HARM_ACC;
	    else
		accel = -HARM_ACC;

	    if (axs > ays) {
		b->yspeed -= xdir * accel;
		if (ABS(b->yspeed) >= sp) {
		    b->xspeed = 0;
		    b->yspeed = ydir * sp;
		}
		else
		    b->xspeed = xdir * sqrt(sp2 - b->yspeed * b->yspeed);
	    } else {
		b->xspeed += ydir * accel;
		if (ABS(b->xspeed) >= sp) {
		    b->yspeed = 0;
		    b->xspeed = xdir * sp;
		} else
		    b->yspeed = ydir * sqrt(sp2 - b->xspeed * b->xspeed);
	    }

        /*
         * Dive on target if close
         */

         if ( (best_dx < 10) && (best_dy < 10) ) b->loc->z = 1;
    }
	} else if (weapon_stat[(int)b->type].frames - b->life == BOOST_PHASE)
		b->loc->z = 9;

}

#endif /* !NO_NEW_RADAR */


/*
** Applies friction to disc in free flight, and computes its orbit when owned.
*/
update_disc(b)
Bullet *b;
{
    FLOAT dx, dy;
    FLOAT dist;
    FLOAT angle, delta;

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

    for (i = 0; i < num_veh_alive; i++) {
	v = live_vehicles[i];

	/* don't do anything if the vehicle isn't alive */
	if (!tstflag(v->status, VS_is_alive))
	    continue;

	loc = v->loc;
	old_loc = v->old_loc;
	if ((loc->grid_x != old_loc->grid_x) ||
	    (loc->grid_y != old_loc->grid_y)) {
	    real_map[old_loc->grid_x][old_loc->grid_y].flags &= ~v->flag;
	    real_map[loc->grid_x][loc->grid_y].flags |= v->flag;
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

	for (i = 0; i < num_veh_alive; i++) {
		v = live_vehicles[i];
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
    for (i = 0; i < num_veh_alive; i++) {
	v = live_vehicles[i];
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
