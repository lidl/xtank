/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "malloc.h"
#include "xtank.h"
#include "vstructs.h"
#include "screen.h"
#include "bullet.h"
#include "lowlib.h"
#include "globals.h"
#include "assert.h"
#include "proto.h"


extern Weapon_stat weapon_stat[];
extern Map real_map;


/*
** Handles collisions between specified vehicle and all walls
*/
void
coll_vehicle_walls(Vehicle *v)
{
	Loc *loc, *oloc;
	Picture *pic;
	int min_x, min_y, max_x, max_y, dx, dy, grid_x, grid_y, vx, vy, hx, hy;
	int check_vert, check_hor, crash_vert, crash_hor;
	WallSide hdir, vdir;

	if (v->just_ported) {
		v->just_ported = FALSE;
		return;
	}
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
	} else if (max_x >= BOX_WIDTH) {
		grid_x = oloc->grid_x + 1;
		check_hor = 1;
	} else {
		grid_x = oloc->grid_x;
		check_hor = 0;
	}

	if (min_y <= 0) {
		grid_y = oloc->grid_y;
		check_vert = -1;
	} else if (max_y >= BOX_HEIGHT) {
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
	if (check_vert && check_hor && !(crash_vert || crash_hor)) {
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
void
coll_bullets_maze(void)
{
	extern Bset *bset;
	Bullet *b;
	Box *bbox;
	Loc *loc, *old_loc;
	register int gx, gy, ogx, ogy;
	int i;

	for (i = 0; i < bset->number; i++) {
		b = bset->list[i];
		if (b->life < 0)
			continue;

		loc = b->loc;
		old_loc = b->old_loc;

		gx = b->loc->grid_x;
		gy = b->loc->grid_y;
		ogx = b->old_loc->grid_x;
		ogy = b->old_loc->grid_y;


		/*
	 * some things fly overhead, and don't hit walls (most of the time)
	 */

		/*
	 * If this bullet is not flying overhead...
	 *   perform normal bullet/wall checks
	 * If bullet is past outer boundry destroy bullet
	 */

		/*
	 * modifing my own code again...
	 *
	 * I think in general it would be safer to destroy ANYTHING
	 * that makes it off the grid.
	 *
	 * bullet get zapped next update cycle when update_bull
	 * notices that it's ran out of life..
     *
	 * define DESTROY_ONLY_FLYING for original behavior    -ane
	 */


		if ((int) b->loc->z <= 4) {
			/* A bullet with no owner can not hit an outpost in the first 5 frames
	   of its life.  This prevents outposts from shooting themselves. There
	   is no identifier in the bullet to say that the outpost fired it. */
			bbox = &real_map[gx][gy];
			if (bbox->type == OUTPOST &&
				(b->owner != (Vehicle *) NULL ||
				 b->life < weapon_stat[(int) b->type].frames - 5) &&
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
			if (gx == ogx) {
				if (gy == ogy) {
					/* has not moved into a new box */
					continue;
				} else if (gy > ogy) {
					/* moved south into new box */
					if (real_map[gx][gy].flags & NORTH_WALL)
						bul_hit_wall(b, gx, gy, SOUTH);
				} else {
					/* moved north into new box */
					if (real_map[ogx][ogy].flags & NORTH_WALL)
						bul_hit_wall(b, ogx, ogy, NORTH);
				}
			} else if (gx > ogx) {
				if (gy == ogy) {
					/* moved east into new box */
					if (real_map[gx][gy].flags & WEST_WALL)
						bul_hit_wall(b, gx, gy, EAST);
				} else if (gy > ogy) {
					/* moved southeast into new box */
					if (b->xspeed * loc->box_y > b->yspeed * loc->box_x) {
						/* hit south side */
						if (real_map[ogx][gy].flags & NORTH_WALL) {
							if (bul_hit_wall(b, ogx, gy, SOUTH))
								if (real_map[gx][ogy].flags & WEST_WALL)
									bul_hit_wall(b, gx, ogy, EAST);
						} else if (real_map[gx][gy].flags & WEST_WALL)
							bul_hit_wall(b, gx, gy, EAST);
					} else {
						/* hit east side */
						if (real_map[gx][ogy].flags & WEST_WALL) {
							if (bul_hit_wall(b, gx, ogy, EAST))
								if (real_map[ogx][gy].flags & NORTH_WALL)
									bul_hit_wall(b, ogx, gy, SOUTH);
						} else if (real_map[gx][gy].flags & NORTH_WALL)
							bul_hit_wall(b, gx, gy, SOUTH);
					}
				} else {
					/* moved northeast into new box */
					if (b->xspeed * (BOX_HEIGHT - loc->box_y) >
						b->yspeed * loc->box_x) {
						/* hit east side */
						if (real_map[gx][ogy].flags & WEST_WALL) {
							if (bul_hit_wall(b, gx, ogy, EAST))
								if (real_map[ogx][ogy].flags & NORTH_WALL)
									bul_hit_wall(b, ogx, ogy, NORTH);
						} else if (real_map[gx][ogy].flags & NORTH_WALL)
							bul_hit_wall(b, gx, ogy, NORTH);
					} else {
						/* hit north side */
						if (real_map[ogx][gy].flags & NORTH_WALL) {
							if (bul_hit_wall(b, ogx, gy, NORTH))
								if (real_map[gx][ogy].flags & WEST_WALL)
									bul_hit_wall(b, gx, ogy, EAST);
						} else if (real_map[gx][gy].flags & WEST_WALL)
							bul_hit_wall(b, gx, gy, EAST);
					}
				}
			} else {
				if (gy == ogy) {
					/* moved west into new box */
					if (real_map[ogx][ogy].flags & WEST_WALL)
						bul_hit_wall(b, ogx, ogy, WEST);
				} else if (gy > ogy) {
					/* moved southwest into new box */
					if (b->xspeed * loc->box_y > b->yspeed * (BOX_WIDTH - loc->box_x)) {
						/* hit west side */
						if (real_map[ogx][ogy].flags & WEST_WALL) {
							if (bul_hit_wall(b, ogx, ogy, WEST))
								if (real_map[ogx][gy].flags & NORTH_WALL)
									bul_hit_wall(b, ogx, gy, SOUTH);
						} else if (real_map[gx][gy].flags & NORTH_WALL)
							bul_hit_wall(b, gx, gy, SOUTH);
					} else {
						/* hit south side */
						if (real_map[ogx][gy].flags & NORTH_WALL) {
							if (bul_hit_wall(b, ogx, gy, SOUTH))
								if (real_map[ogx][ogy].flags & WEST_WALL)
									bul_hit_wall(b, ogx, ogy, WEST);
						} else if (real_map[ogx][gy].flags & WEST_WALL)
							bul_hit_wall(b, ogx, gy, WEST);
					}
				} else {
					/* moved northwest into new box */
					if (b->xspeed * (BOX_HEIGHT - loc->box_y) >
						b->yspeed * (BOX_WIDTH - loc->box_x)) {
						/* hit north side */
						if (real_map[ogx][ogy].flags & NORTH_WALL) {
							if (bul_hit_wall(b, ogx, ogy, NORTH))
								if (real_map[ogx][ogy].flags & WEST_WALL)
									bul_hit_wall(b, ogx, ogy, WEST);
						} else if (real_map[ogx][gy].flags & WEST_WALL)
							bul_hit_wall(b, ogx, gy, WEST);
					} else {
						/* hit west side */
						if (real_map[ogx][ogy].flags & WEST_WALL) {
							if (bul_hit_wall(b, ogx, ogy, WEST))
								if (real_map[ogx][ogy].flags & NORTH_WALL)
									bul_hit_wall(b, gx, ogy, NORTH);
						} else if (real_map[gx][ogy].flags & NORTH_WALL)
							bul_hit_wall(b, ogx, ogy, NORTH);
					}
				}
			}
		}
#ifdef DESTROY_ONLY_FLYING
		if ((int) b->loc->z > 4) {
#endif
			if ((b->loc->grid_x > MAZE_HEIGHT + 1) ||
				(b->loc->grid_y > MAZE_WIDTH + 1) ||
				(b->loc->grid_x < 2) ||
				(b->loc->grid_y < 2)) {
				if (b->life == weapon_stat[(int) b->type].frames - 1)
					b->life = -2;	/* undisplayed bullet */
				else
					b->life = -1;
			}
			continue;
#ifdef DESTROY_ONLY_FLYING
		}
#endif
	}
}

#define LINE_BULLETS
/*
** Handles all collisions between bullets and tanks.
*/
void
coll_bullets_vehicles(void)
{
	extern Bset *bset;
	Bullet *b;
	Vehicle *v;
	Loc *bloc, *vloc;
	Picture *pic;
	int i, j;
	int dx, dy, max_dx, max_dy;
	int vehicles_flag;
	int vx, vy, tx, ty;			/* Should these all be float? */

	for (i = 0; i < bset->number; i++) {
		b = bset->list[i];
		if (b->life < 0)
			continue;
		bloc = b->loc;


		/*
		 * only worry about bullets down here
		 */

		if ((int) bloc->z > 4)
			continue;


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
		if (vehicles_flag == 0 || (b->owner != (Vehicle *) NULL &&
					   vehicles_flag == b->owner->flag && !b->hurt_owner)) {
			continue;
		}
		for (j = 0; j < num_veh_alive; j++) {
			v = live_vehicles[j];
			if ((v->flag & vehicles_flag) == 0) {
				continue;
			}

			/* He could have been killed earlier in
			 * this frame, but he still would be on the
			 * live vehicle list... (HAK)
			 */
			if(!tstflag(v->status, VS_is_alive))
				continue;

			/* If the bullet shouldn't hurt this vehicle, ignore it */
			if (v == b->owner && !b->hurt_owner) {
				continue;
			}

			/* ignore low weapons if we have hovers */
			if(bloc->z < 0 && v->vdesc->treads == HOVER_TREAD
			    && !((b->hit_flgs & F_HOVER) && !rnd(4)))
				continue;

			if(b->hit_flgs & F_NOHIT)
				continue;

			vloc = v->loc;
			pic = &v->obj->pic[v->vector.rot];

			/* Check to see if attacker's bullet is near the vehicle */
#ifdef LINE_BULLETS
			{
				int x1, y1, x2, y2;

				x1 = b->old_loc->x - vloc->x + pic->offset_x;
				x2 = bloc->x - vloc->x + pic->offset_x;
				y1 = b->old_loc->y - vloc->y + pic->offset_y;
				y2 = bloc->y - vloc->y + pic->offset_y;

				/* line_in_rect will return the results
				 * of the clip calculation in x1,y1,x2,y2
				 * HAK 3/93
				 */
				if (!line_in_rect(&x1, &y1, &x2, &y2, 0, 0, pic->width, pic->height)) {
					continue;
				}
	/* The clipping algorithm that is used by line_in_rect should
	 * return the point at which the bullet originally collided in
	 * either x1,y1 or x2,y2.  Find out which using the bullet's
	 * x and y velocities.  (HAK)
	 */
				if (((b->xspeed < 0) && (x2 > x1))
				 || ((b->yspeed < 0) && (y2 > y1))
				 || ((b->xspeed > 0) && (x2 < x1))
				 || ((b->yspeed > 0) && (y2 < y1))) {
					x1 = x2; y1 = y2;
				}
				dx = x1 - pic->offset_x;
				dy = y1 - pic->offset_y;

			/* A little hack... change the bullet's loc so
			 * make_explosion() will put the explosion in the
			 * right place.
			 */
				{
					FLOAT temp;
					temp = b->loc->z;
					update_loc(vloc, b->loc, dx, dy);
					b->loc->z = temp;
				}
			}
#else  /*LINE_BULLETS*/
			max_dx = pic->width >> 1;
			max_dy = pic->height >> 1;

#ifdef SLOWER					/* About 40 times slower on a 68020, on a SPARC ??? */
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
#endif  /*LINE_BULLETS*/

			/* Bullet has hit the vehicle */
			bul_hit_vehicle(v, b, dx, dy);
		}
	}
}

/*
** Handles all collisions between vehicles and other vehicles.
*/
void
coll_vehicles_vehicles(void)
{
	Vehicle *v1, *v2;
	Loc *v1loc, *v2loc;
	Loc v1oloc, v2oloc;
	Picture *pic1, *pic2;
	unsigned int vehicles_flag;
	float x1off, y1off, x2off, y2off;
	int dx, dy, max_dx, max_dy, shx1, shy1, shx2, shy2;
	int i, j, k;

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
#define NUM_CHK 3
			v2loc = v2->loc;
			pic2 = &v2->obj->pic[v2->vector.rot];

			v1oloc = *(v1->old_loc);
			v2oloc = *(v2->old_loc);

			x1off = (v1loc->x - v1oloc.x) / NUM_CHK;
			y1off = (v1loc->y - v1oloc.y) / NUM_CHK;
			x2off = (v2loc->x - v2oloc.x) / NUM_CHK;
			y2off = (v2loc->y - v2oloc.y) / NUM_CHK;

			max_dx = (pic1->width + pic2->width) >> 1;
			max_dy = (pic1->height + pic2->height) >> 1;

			for(k=0; k < NUM_CHK+1; k++) {
				if(k) {
					v1oloc.x += x1off;
					v1oloc.y += y1off;
					v2oloc.x += x2off;
					v2oloc.y += y2off;
				}

			/* Check if bounding boxes overlap in x coordinates */
				dx = v2oloc.x - v1oloc.x;
				if (dx < -max_dx || dx > max_dx)
					continue;

			/* Check if bounding boxes overlap in y coordinates */
				dy = v2oloc.y - v1oloc.y;
				if (dy < -max_dy || dy > max_dy)
					continue;
				break;
			}
			if(k == NUM_CHK+1)
				continue;

			if(!k) {
				dx = v2loc->x - v1loc->x;
				if (dx < -max_dx || dx > max_dx)
					continue;
				dy = v2loc->y - v1loc->y;
				if (dy < -max_dy || dy > max_dy)
					continue;
			}

			shx2 = ((dx > 0) ? (max_dx - dx) : (-max_dx - dx)) >> 1;
			shy2 = ((dy > 0) ? (max_dy - dy) : (-max_dy - dy)) >> 1;
			if (ABS(shx2) < ABS(shy2))
				shy2 = 0;
			else
				shx2 = 0;

			shy1 = -shy2; shx1 = -shx2;

			if(k) {
				shx1 += (int)(v1oloc.x - v1loc->x);
				shy1 += (int)(v1oloc.y - v1loc->y);
				shx2 += (int)(v2oloc.x - v2loc->x);
				shy2 += (int)(v2oloc.y - v2loc->y);
			}

			adjust_loc(v1->loc, shx1, shy1);
			adjust_loc(v2->loc, shx2, shy2);

			vehicle_hit_vehicle(v1, v2, max_dx, max_dy, dx, dy);
		}
	}
}

