/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** collision.c
*/

/*
$Author: stripes $
$Id: collision.c,v 2.9 1992/02/10 05:24:16 stripes Exp $

$Log: collision.c,v $
 * Revision 2.9  1992/02/10  05:24:16  stripes
 * Made the correct collision code faster.
 *
 * Revision 2.8  1992/02/06  03:16:43  stripes
 * Added code to attribute bullets to the proper sides
 *
 * Revision 2.7  1992/01/29  08:37:01  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.6  1991/12/15  22:36:31  aahz
 * removed i860 debug.
 *
 * Revision 2.5  1991/12/15  20:30:35  lidl
 * added some debugging info for the i860, trying to find the
 * cause of the mysteriously high initial velocies
 *
 * Revision 2.4  1991/09/19  05:28:24  lidl
 * a small bug-fix to prevent a problem with scrolling into walls
 *
 * Revision 2.3  1991/02/10  13:50:12  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:25  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:01  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:09  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:01  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "vstructs.h"
#include "screen.h"
#include "bullet.h"
#include "globals.h"


extern Weapon_stat weapon_stat[];
extern Map real_map;


/*
** Handles collisions between specified vehicle and all walls
*/
coll_vehicle_walls(v)
Vehicle *v;
{
	Loc *loc, *oloc;
	Picture *pic;
	int min_x, min_y, max_x, max_y, dx, dy, grid_x, grid_y, vx, vy, hx, hy;
	int check_vert, check_hor, crash_vert, crash_hor;
    WallSide hdir, vdir;

	loc = v->loc;
	oloc = v->old_loc;
	pic = &v->obj->pic[v->vector.rot];

	/* Compute box coordinates of bounding rectangle around vehicle.
	   Base from old location in case we switch boxes this frame. */
#ifdef SCROLL_INTO_WALL_BUG
	min_x = oloc->box_x + v->vector.xspeed - pic->offset_x;
	min_y = oloc->box_y + v->vector.yspeed - pic->offset_y;
#else
	min_x = oloc->box_x + (loc->x - oloc->x) - pic->offset_x;
	min_y = oloc->box_y + (loc->y - oloc->y) - pic->offset_y;
#endif
	max_x = min_x + pic->width;
	max_y = min_y + pic->height;

    /* Determine which sides of box, if any, intersect the bounding rectangle
       */
	if (min_x <= 0) {
		grid_x = oloc->grid_x;
		check_hor = -1;
	} else
	if (max_x >= BOX_WIDTH) {
		grid_x = oloc->grid_x + 1;
		check_hor = 1;
	} else {
		grid_x = oloc->grid_x;
		check_hor = 0;
	}

	if (min_y <= 0) {
		grid_y = oloc->grid_y;
		check_vert = -1;
	} else
	if (max_y >= BOX_HEIGHT) {
		grid_y = oloc->grid_y + 1;
		check_vert = 1;
	} else {
		grid_y = oloc->grid_y;
		check_vert = 0;
	}

	/* Check for crashes against the two walls closest to the vehicle */
	if (check_vert && (real_map[oloc->grid_x][grid_y].flags & NORTH_WALL)) {
		crash_vert = check_vert;
		vx = oloc->grid_x;
		vy = grid_y;
	} else {
		crash_vert = 0;
	}

	if (check_hor && (real_map[grid_x][oloc->grid_y].flags & WEST_WALL)) {
		crash_hor = check_hor;
		hx = grid_x;
		hy = oloc->grid_y;
	} else {
		crash_hor = 0;
	}

	/* * If we are overlapping a corner, and we didn't collide with * the
	   first two, check the two farther away. */
	if (check_vert && check_hor && !(crash_vert || crash_hor))
	{
		if (real_map[grid_x][oloc->grid_y + check_vert].flags & WEST_WALL) {
			crash_hor = check_hor;
			hx = grid_x;
			hy = oloc->grid_y + check_vert;
		}
		if (real_map[oloc->grid_x + check_hor][grid_y].flags & NORTH_WALL) {
			crash_vert = check_vert;
			vx = oloc->grid_y + check_hor;
			vy = grid_y;
		}
		/* We cannot crash into both of these walls, so pick the closest one */
		if (crash_hor && crash_vert) {
			dx = ((check_hor == 1) ? (max_y - BOX_HEIGHT) : -min_y);
			dy = ((check_vert == 1) ? (max_x - BOX_WIDTH) : -min_x);
			if (dx > dy) {
				crash_hor = 0;
			} else {
				crash_vert = 0;
			}
		}
	}
	/* If there was a crash, adjust the vehicle's location to against the
	   wall */
	if (crash_vert || crash_hor) {
		if (crash_vert == -1) {
			vdir = NORTH;
			dy = -min_y + 1;
		} else {
			if (crash_vert == 1) {
				vdir = SOUTH;
				dy = BOX_HEIGHT - max_y - 1;
			} else {
				dy = 0;
			}
		}

		if (crash_hor == -1) {
			hdir = WEST;
			dx = -min_x + 1;
		} else {
			if (crash_hor == 1) {
				hdir = EAST;
				dx = BOX_WIDTH - max_x - 1;
			} else {
				dx = 0;
			}
		}

		adjust_loc(loc, dx, dy);
		if (crash_vert) {
			vehicle_hit_wall(v, vx, vy, vdir);
		}
		if (crash_hor) {
			vehicle_hit_wall(v, hx, hy, hdir);
		}
	}
}

/*
** Handles all collisions between bullets and maze.
*/
coll_bullets_maze()
{
    extern Bset *bset;
    Bullet *b;
    Box *bbox;
    Loc *loc, *old_loc;
    register int gx, gy, ogx, ogy;
    int i;

    for (i = 0; i < bset->number; i++)
    {
	b = bset->list[i];
	if (b->life < 0)
	    continue;

	loc = b->loc;
	old_loc = b->old_loc;

	gx = loc->grid_x;
	gy = loc->grid_y;
	ogx = old_loc->grid_x;
	ogy = old_loc->grid_y;

#ifndef NO_NEW_RADAR

	/*
	 * some things fly overhead, and don't hit walls (most of the time)
	 */

	/*
	 * If this bullet is flying overhead...
	 *   if bullet is past outer boundry destroy bullet
	 * else
	 *   perform normal bullet/wall checks
	 */

	if ((int) b->loc->z > 4) {
	    if ( (gx > MAZE_HEIGHT + 1) | (gy > MAZE_WIDTH + 1) | (gx < 2) | (gy < 2)) {
		if (b->life == weapon_stat[(int)b->type].frames - 1)
		    b->life = -2;           /* undisplayed bullet */
		else
		    b->life = -1;
	    }
	} else {

#endif /* !NO_NEW_RADAR */

	/* A bullet with no owner can not hit an outpost in the first 5 frames
	   of its life.  This prevents outposts from shooting themselves. There
	   is no identifier in the bullet to say that the outpost fired it. */
	bbox = &real_map[gx][gy];
	if (bbox->type == OUTPOST &&
	    (b->owner != (Vehicle *) NULL ||
	     b->life < weapon_stat[(int)b->type].frames - 5) &&
	    coll_outpost(bbox, loc))
	    bul_hit_outpost(b, bbox, gx, gy);

	/*  Check for walls
        **
        **  The following massive special casing is primarily done for speed.
        **  There are 9 cases to consider, one for each of the nine boxes in
        **  the area of the bullet.
        **
        **  For the 4 diagonal cases, there is further special casing to
        **  determine which side of the box the bullet passed through
        **
        **  If the bullet does hit a wall, then the distances to move the
        **  bullet back are computed (dx and dy).  These are sent to
        **  bullet_hit_wall() which determines the actual results of the
        **  collision.
        **
        ****
        **  Aug 13, 90, changed bul_hit_wall to boolean (see hit.c) to stop
        **  bullets from bouncing through corners.  Basically, it returns
        **  whether a ricochet occured (ie the "bullet" is a disc or ricochet
        **  bullet).  TRUE means that a bounce occured and this code should
        **  keep tracking the bullet...  FALSE means the bullet is dead and we
        **  dont need to bother.  This also meant the initial if of the
        **  diagonal cases had to be changed to call the second check if the
        **  bullet still existed.  And also at a different location since the
        **  bounce changes the coordinates of the second box hit.
        */
	if (gx == ogx)
	{
	    if (gy == ogy)
	    {
		/* has not moved into a new box */
		continue;
	    }
	    else if (gy > ogy)
	    {
		/* moved south into new box */
		if (real_map[gx][gy].flags & NORTH_WALL)
		    bul_hit_wall(b, gx, gy, SOUTH);
	    }
	    else
	    {
		/* moved north into new box */
		if (real_map[ogx][ogy].flags & NORTH_WALL)
		    bul_hit_wall(b, ogx, ogy, NORTH);
	    }
	}
	else if (gx > ogx)
	{
	    if (gy == ogy)
	    {
		/* moved east into new box */
		if (real_map[gx][gy].flags & WEST_WALL)
		    bul_hit_wall(b, gx, gy, EAST);
	    }
	    else if (gy > ogy)
	    {
		/* moved southeast into new box */
		if (b->xspeed * loc->box_y > b->yspeed * loc->box_x)
		{
		    /* hit south side */
		    if (real_map[ogx][gy].flags & NORTH_WALL)
		    {
			if (bul_hit_wall(b, ogx, gy, SOUTH))
			    if (real_map[gx][ogy].flags & WEST_WALL)
				bul_hit_wall(b, gx, ogy, EAST);
		    }
		    else if (real_map[gx][gy].flags & WEST_WALL)
			bul_hit_wall(b, gx, gy, EAST);
		}
		else
		{
		    /* hit east side */
		    if (real_map[gx][ogy].flags & WEST_WALL)
		    {
			if (bul_hit_wall(b, gx, ogy, EAST))
			    if (real_map[ogx][gy].flags & NORTH_WALL)
				bul_hit_wall(b, ogx, gy, SOUTH);
		    }
		    else if (real_map[gx][gy].flags & NORTH_WALL)
			bul_hit_wall(b, gx, gy, SOUTH);
		}
	    }
	    else
	    {
		/* moved northeast into new box */
		if (b->xspeed * (BOX_HEIGHT - loc->box_y) >
		    b->yspeed * loc->box_x)
		{
		    /* hit east side */
		    if (real_map[gx][ogy].flags & WEST_WALL)
		    {
			if (bul_hit_wall(b, gx, ogy, EAST))
			    if (real_map[ogx][ogy].flags & NORTH_WALL)
				bul_hit_wall(b, ogx, ogy, NORTH);
		    }
		    else if (real_map[gx][ogy].flags & NORTH_WALL)
			bul_hit_wall(b, gx, ogy, NORTH);
		}
		else
		{
		    /* hit north side */
		    if (real_map[ogx][gy].flags & NORTH_WALL)
		    {
			if (bul_hit_wall(b, ogx, gy, NORTH))
			    if (real_map[gx][ogy].flags & WEST_WALL)
				bul_hit_wall(b, gx, ogy, EAST);
		    }
		    else if (real_map[gx][gy].flags & WEST_WALL)
			bul_hit_wall(b, gx, gy, EAST);
		}
	    }
	}
	else
	{
	    if (gy == ogy)
	    {
		/* moved west into new box */
		if (real_map[ogx][ogy].flags & WEST_WALL)
		    bul_hit_wall(b, ogx, ogy, WEST);
	    }
	    else if (gy > ogy)
	    {
		/* moved southwest into new box */
		if (b->xspeed * loc->box_y > b->yspeed * (BOX_WIDTH - loc->box_x))
		{
		    /* hit west side */
		    if (real_map[ogx][ogy].flags & WEST_WALL)
		    {
			if (bul_hit_wall(b, ogx, ogy, WEST))
			    if (real_map[ogx][gy].flags & NORTH_WALL)
				bul_hit_wall(b, ogx, gy, SOUTH);
		    }
		    else if (real_map[gx][gy].flags & NORTH_WALL)
			bul_hit_wall(b, gx, gy, SOUTH);
		}
		else
		{
		    /* hit south side */
		    if (real_map[ogx][gy].flags & NORTH_WALL)
		    {
			if (bul_hit_wall(b, ogx, gy, SOUTH))
			    if (real_map[ogx][ogy].flags & WEST_WALL)
				bul_hit_wall(b, ogx, ogy, WEST);
		    }
		    else if (real_map[ogx][gy].flags & WEST_WALL)
			bul_hit_wall(b, ogx, gy, WEST);
		}
	    }
	    else
	    {
		/* moved northwest into new box */
		if (b->xspeed * (BOX_HEIGHT - loc->box_y) >
		    b->yspeed * (BOX_WIDTH - loc->box_x))
		{
		    /* hit north side */
		    if (real_map[ogx][ogy].flags & NORTH_WALL)
		    {
			if (bul_hit_wall(b, ogx, ogy, NORTH))
			    if (real_map[ogx][ogy].flags & WEST_WALL)
				bul_hit_wall(b, ogx, ogy, WEST);
		    }
		    else if (real_map[ogx][gy].flags & WEST_WALL)
			bul_hit_wall(b, ogx, gy, WEST);
		}
		else
		{
		    /* hit west side */
		    if (real_map[ogx][ogy].flags & WEST_WALL)
		    {
			if (bul_hit_wall(b, ogx, ogy, WEST))
			    if (real_map[ogx][ogy].flags & NORTH_WALL)
				bul_hit_wall(b, gx, ogy, NORTH);
		    }
		    else if (real_map[gx][ogy].flags & NORTH_WALL)
			bul_hit_wall(b, ogx, ogy, NORTH);
		}
	    }
	}
#ifndef NO_NEW_RADAR
	}
#endif /* !NO_NEW_RADAR */
    }
}


/*
** Handles all collisions between bullets and tanks.
*/
coll_bullets_vehicles()
{
    extern Bset *bset;
    Bullet *b;
    Vehicle *v;
    Loc *bloc, *vloc;
    Picture *pic;
    int i, j;
    int dx, dy, max_dx, max_dy;
    int vehicles_flag;
	int vx, vy, tx, ty;		/* Should these all be float? */

    for (i = 0; i < bset->number; i++)
    {
	b = bset->list[i];
	if (b->life < 0)
	    continue;
	bloc = b->loc;

#ifndef NO_NEW_RADAR

	/*
 	 * only worry about bullets down here
 	 */

	if ((int) bloc->z > 4) continue;

#endif /* NO_NEW_RADAR */

	/* first see if there are any vehicles around */


	/*
	 **     get_flags(bloc,vehicles_flag);
	 ** THIS ISN'T A MACRO BECAUSE LATTICE C's 512 BYTE MACRO LIMIT
	 **
	 ** Sets flag to the logical OR of the vehicle flags of the 4
	 ** boxes closest to the specified location.
	 */

	{
	    register int grid_x;
	    register int grid_y;

	    grid_x = bloc->grid_x;
	    grid_y = bloc->grid_y;

	    if (bloc->box_x > BOX_WIDTH / 2)
		if (bloc->box_y > BOX_HEIGHT / 2)
		    vehicles_flag = real_map[grid_x][grid_y].flags |
			real_map[grid_x + 1][grid_y].flags |
			    real_map[grid_x][grid_y + 1].flags |
				real_map[grid_x + 1][grid_y + 1].flags;
		else
		    vehicles_flag = real_map[grid_x][grid_y].flags |
			real_map[grid_x + 1][grid_y].flags |
			    real_map[grid_x][grid_y - 1].flags |
				real_map[grid_x + 1][grid_y - 1].flags;
	    else if (bloc->box_y > BOX_HEIGHT / 2)
		vehicles_flag = real_map[grid_x][grid_y].flags |
		    real_map[grid_x - 1][grid_y].flags |
			real_map[grid_x][grid_y + 1].flags |
			    real_map[grid_x - 1][grid_y + 1].flags;
	    else
		vehicles_flag = real_map[grid_x][grid_y].flags |
		    real_map[grid_x - 1][grid_y].flags |
			real_map[grid_x][grid_y - 1].flags |
			    real_map[grid_x - 1][grid_y - 1].flags;
	    vehicles_flag &= ANY_VEHICLE;
	}

	/*
	 ** END OF WOULD-BE MACRO
	 */

	/* If there are no vehicles to collide with, exit quickly */
	if (vehicles_flag == 0 ||
	    (b->owner != (Vehicle *) NULL &&
	     vehicles_flag == b->owner->flag &&
	     !b->hurt_owner))
	    continue;

	for (j = 0; j < num_veh_alive; j++) {
	    v = live_vehicles[j];
	    if ((v->flag & vehicles_flag) == 0)
		continue;

	    /* If the bullet shouldn't hurt this vehicle, ignore it */
	    if (v == b->owner && !b->hurt_owner)
		continue;
	    vloc = v->loc;
	    pic = &v->obj->pic[v->vector.rot];
	    max_dx = pic->width >> 1;
	    max_dy = pic->height >> 1;

	    /* Check to see if attacker's bullet is near the vehicle */
#ifdef SLOWER /* About 40 times slower on a 68020, on a SPARC ??? */
	    dx = bloc->x - vloc->x;
#else /* SLOWER */
	    dx = (int) bloc->x - (int) vloc->x;
#endif /* SLOWER */
	    if (dx < -max_dx || dx > max_dx)
		continue;
#ifdef SLOWER
	    dy = bloc->y - vloc->y;
#else /* SLOWER */
	    dy = (int) bloc->y - (int) vloc->y;
#endif /* SLOWER */
	    if (dy < -max_dy || dy > max_dy)
		continue;

		/* now find the REAL location of the collision */

		/* find relative velocity */
		vx = b->xspeed - v->vector.xspeed;
		vy = b->yspeed - v->vector.yspeed;

		/* check possible adjust along both */
		if ((vx != 0) && (vy != 0)) {
			/* find 100 * time to reach x edge */
			if (vx > 0) {
				tx = (100 * (max_dx + dx)) / vx;
			} else {
				tx = (100 * -(max_dx - dx)) / vx;
			}

			/* find 100 * time to reach y edge */
			if (vy > 0) {
				ty = (100 * (max_dy + dy)) / vy;
			} else {
				ty = (100 * -(max_dy - dy)) / vy;
			}

			/* which hit first? */
			if (tx > ty) {
				/* y did; put it on y edge */
				dy = SIGN(-vy) * max_dy;
				dx -= (ty * vx) / 100;
			} else {
				/* x did; put it on x edge */
				dx = SIGN(-vx) * max_dx;
				dy -= (tx * vy) / 100;
			}

		} else {
			if (vx != 0) {
				dx = SIGN(-vx) * max_dx;
			} else {
				if (vy != 0) {
					dy = SIGN(-vy) * max_dy;
				}
			}
		}

	    /* Bullet has hit the vehicle */
	    bul_hit_vehicle(v, b, dx, dy);
	}
    }
}

/*
** Handles all collisions between vehicles and other vehicles.
*/
coll_vehicles_vehicles()
{
    Vehicle *v1, *v2;
    Loc *v1loc, *v2loc;
    Picture *pic1, *pic2;
    unsigned int vehicles_flag;
    int dx, dy, max_dx, max_dy, shiftx, shifty;
    int i, j;

    for (i = 0; i < num_veh_alive - 1; i++) {
	v1 = live_vehicles[i];
	v1loc = v1->loc;

	/* First see if there is anyone around */

	/*
	 **     get_flags(v1loc,vehicles_flag);
	 ** THIS ISN'T A MACRO BECAUSE LATTICE C's 512 BYTE MACRO LIMIT
	 **
	 ** Sets flag to the logical OR of the vehicle flags of the 4
	 ** boxes closest to the specified location.
	 */
	{
	    register int grid_x;
	    register int grid_y;

	    grid_x = v1loc->grid_x;
	    grid_y = v1loc->grid_y;

	    if (v1loc->box_x > BOX_WIDTH / 2)
		if (v1loc->box_y > BOX_HEIGHT / 2)
		    vehicles_flag = real_map[grid_x][grid_y].flags |
			real_map[grid_x + 1][grid_y].flags |
			    real_map[grid_x][grid_y + 1].flags |
				real_map[grid_x + 1][grid_y + 1].flags;
		else
		    vehicles_flag = real_map[grid_x][grid_y].flags |
			real_map[grid_x + 1][grid_y].flags |
			    real_map[grid_x][grid_y - 1].flags |
				real_map[grid_x + 1][grid_y - 1].flags;
	    else if (v1loc->box_y > BOX_HEIGHT / 2)
		vehicles_flag = real_map[grid_x][grid_y].flags |
		    real_map[grid_x - 1][grid_y].flags |
			real_map[grid_x][grid_y + 1].flags |
			    real_map[grid_x - 1][grid_y + 1].flags;
	    else
		vehicles_flag = real_map[grid_x][grid_y].flags |
		    real_map[grid_x - 1][grid_y].flags |
			real_map[grid_x][grid_y - 1].flags |
			    real_map[grid_x - 1][grid_y - 1].flags;
	    vehicles_flag &= ANY_VEHICLE;
	}
	/*
	 ** END OF WOULD-BE MACRO
	 */

	if ((vehicles_flag & ~v1->flag) == 0)
	    continue;

	pic1 = &v1->obj->pic[v1->vector.rot];
	for (j = i + 1; j < num_veh_alive; j++) {
	    v2 = live_vehicles[j];

	    /* See if this vehicle is nearby */
	    if ((vehicles_flag & v2->flag) == 0)
		continue;

	    v2loc = v2->loc;
	    pic2 = &v2->obj->pic[v2->vector.rot];

	    /* Check if bounding boxes overlap in x coordinates */
	    dx = v2loc->x - v1loc->x;
	    max_dx = (pic1->width + pic2->width) >> 1;
	    if (dx < -max_dx || dx > max_dx)
		continue;

	    /* Check if bounding boxes overlap in y coordinates */
	    dy = v2loc->y - v1loc->y;
	    max_dy = (pic1->height + pic2->height) >> 1;
	    if (dy < -max_dy || dy > max_dy)
		continue;

	    /* * Vehicles are pretty close to each other, so let's say they
	       hit. * Shift vehicles so their rectangles touch, instead of
	       overlapping. */
	    shiftx = ((dx > 0) ? (max_dx - dx + 3) : (-max_dx - dx - 3)) >> 1;
	    shifty = ((dy > 0) ? (max_dy - dy + 3) : (-max_dy - dy - 3)) >> 1;
            if (ABS(shiftx) < ABS(shifty))
		shifty = 0;
	    else
		shiftx = 0;

	    adjust_loc(v1loc, -shiftx, -shifty);
	    adjust_loc(v2loc, shiftx, shifty);
	    vehicle_hit_vehicle(v1, v2);
	}
    }
}
