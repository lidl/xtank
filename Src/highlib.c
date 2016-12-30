/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "xtank.h"
#include "xtanklib.h"
#include "screen.h"
#include "bullet.h"
#include "graphics.h"
#include "terminal.h"
#include "vehicle.h"
#include "proto.h"

/* KLUDGE ALERT !!!! XXX */

#if defined(__STDC__) || defined(__cplusplus)
#define P_(s) s
#define Pi_(s)
#else
#define P_(s) ()
#define Pi_(s) s
#endif

extern Vehicle *cv;

/*
** Turns all turrets to the specified angle (in radians)
*/
void
turn_all_turrets( P_(Angle) angle)
Pi_(Angle angle;)
{
	int i;

	for (i = 0; i < cv->num_turrets; i++)
		turn_turret((TurretNum) i, angle);
}

/*
** Aims all turrets at a location dx away horizontally
** and dy away vertically from the vehicle
*/
void
aim_all_turrets(int dx, int dy)
{
	int i;

	for (i = 0; i < cv->num_turrets; i++)
		aim_turret((TurretNum) i, dx, dy);
}

/*
** Attempts to fire all weapons in numerical order.
*/
int
fire_all_weapons(void)
{
	int i;

	/* Check if the vehicle has any weapons */
	if (cv->num_weapons == 0) {
		check_time();
		return BAD_VALUE;
	}
	for (i = 0; i < cv->num_weapons; i++)
		anim_fire_weapon(i);

	return 0;
}
  
/*
 ** Attempts to fire all weapons in numerical order, that generate >= the
 **  amount of heat specified by temperature.
 */
int 
fire_hot_weapons(int temperature)
{
   int i;
   
   /* Check if the vehicle has any weapons */
   if (cv->num_weapons == 0)
     {
       check_time();
       return BAD_VALUE;
     }
   
   for (i = 0; i < cv->num_weapons; i++)
     {
       if (weapon_heat(i) >= temperature)
	 anim_fire_weapon(i);
     }
   
   return 0;
 }

/*
 ** Attempts to fire all weapons in numerical order, that generate < the
 ** amount of heat specified by temperature.
 */
int 
fire_cool_weapons(int temperature)
{
  int i;
  
  /* Check if the vehicle has any weapons */
  if (cv->num_weapons == 0)
    {
      check_time();
      return BAD_VALUE;
    }
  
  for (i = 0; i < cv->num_weapons; i++)
    {
      if (weapon_heat(i) < temperature)
 	anim_fire_weapon(i);
    }
  return 0;
}

/*
** Returns TRUE if there are no walls blocking the path from start to finish,
** otherwise returns FALSE.
*/
Boolean
clear_path(Location *start, Location *finish)
{
	int start_x, start_y, finish_x, finish_y;
	int dx, dy, lattice_dx, lattice_dy;
	int tgrid_x, tgrid_y, fgrid_x, fgrid_y;

	/* This does not do the "obvious" thing.  It does the fast thing.  It
       compares the slopes of lines (from the destination to gridpoints along
       the path) to the line of the path.  From these comparisons, it
       determines which gridpoint to walk to next.  It does NOT walk from box
       to box, but from gridpoint to gridpoint. */

	/* Set up temporary and final box coordinates */
	start_x = start->x;
	start_y = start->y;
	finish_x = finish->x;
	finish_y = finish->y;
	tgrid_x = start->grid_x;
	tgrid_y = start->grid_y;
	fgrid_x = finish->grid_x;
	fgrid_y = finish->grid_y;

	/* Computed x and y deltas from start to finish */
	dx = finish_x - start_x;
	dy = finish_y - start_y;

	/* Figure out the general direction that the line is travelling in so that
       we can write specific code for each case.

       In the NE, SE, NW, and SW cases, lattice_dx and lattice_dy are the
       deltas from the starting location to the lattice point that the path is
       heading towards.  The slope of the line is compared to the slope to the
       lattice point This determines which wall the path intersects.  Instead
       of comparing dx/dy with lattice_dx/lattice_dy, I multiply both sides by
       dy * lattice_dy, which lets me do 2 multiplies instead of 2 divides. */
	if (fgrid_x > tgrid_x)
		if (fgrid_y > tgrid_y) {/* Southeast */
			lattice_dx = (tgrid_x + 1) * BOX_WIDTH - start_x;
			lattice_dy = (tgrid_y + 1) * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
				if (lattice_dy * dx < dy * lattice_dx) {
					if (wall(SOUTH, tgrid_x, tgrid_y))
						return FALSE;
					tgrid_y++;
					lattice_dy += BOX_HEIGHT;
				} else {
					if (wall(EAST, tgrid_x, tgrid_y))
						return FALSE;
					tgrid_x++;
					lattice_dx += BOX_WIDTH;
				}
			}
		} else if (fgrid_y < tgrid_y) {	/* Northeast */
			lattice_dx = (tgrid_x + 1) * BOX_WIDTH - start_x;
			lattice_dy = tgrid_y * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
				if (lattice_dy * dx > dy * lattice_dx) {
					if (wall(NORTH, tgrid_x, tgrid_y))
						return FALSE;
					tgrid_y--;
					lattice_dy -= BOX_HEIGHT;
				} else {
					if (wall(EAST, tgrid_x, tgrid_y))
						return FALSE;
					tgrid_x++;
					lattice_dx += BOX_WIDTH;
				}
			}
		} else {				/* East */
			for (; tgrid_x < fgrid_x; tgrid_x++)
				if (wall(EAST, tgrid_x, tgrid_y))
					return FALSE;
	} else if (fgrid_x < tgrid_x)
		if (fgrid_y > tgrid_y) {/* Southwest */
			lattice_dx = tgrid_x * BOX_WIDTH - start_x;
			lattice_dy = (tgrid_y + 1) * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
				if (lattice_dy * dx >= dy * lattice_dx) {
					if (wall(SOUTH, tgrid_x, tgrid_y))
						return FALSE;
					tgrid_y++;
					lattice_dy += BOX_HEIGHT;
				} else {
					if (wall(WEST, tgrid_x, tgrid_y))
						return FALSE;
					tgrid_x--;
					lattice_dx -= BOX_WIDTH;
				}
			}
		} else if (fgrid_y < tgrid_y) {	/* Northwest */
			lattice_dx = tgrid_x * BOX_WIDTH - start_x;
			lattice_dy = tgrid_y * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
				if (lattice_dy * dx < dy * lattice_dx) {
					if (wall(NORTH, tgrid_x, tgrid_y))
						return FALSE;
					tgrid_y--;
					lattice_dy -= BOX_HEIGHT;
				} else {
					if (wall(WEST, tgrid_x, tgrid_y))
						return FALSE;
					tgrid_x--;
					lattice_dx -= BOX_WIDTH;
				}
			}
		} else {				/* West */
			for (; tgrid_x > fgrid_x; tgrid_x--)
				if (wall(WEST, tgrid_x, tgrid_y))
					return FALSE;
	} else if (fgrid_y > tgrid_y) {	/* South */
		for (; tgrid_y < fgrid_y; tgrid_y++)
			if (wall(SOUTH, tgrid_x, tgrid_y))
				return FALSE;
	} else if (fgrid_y < tgrid_y) {	/* North */
		for (; tgrid_y > fgrid_y; tgrid_y--)
			if (wall(NORTH, tgrid_x, tgrid_y))
				return FALSE;
	}
	return TRUE;
}

/*
** Puts closest enemy vehicle with a clear path to it into enemy.
** Returns TRUE if such a vehicle exists, otherwise FALSE.
*/
Boolean
get_closest_enemy(Vehicle_info *enemy)
{
	Vehicle_info vinfo[MAX_VEHICLES];
	int num_vinfos;
	Vehicle_info *v, me;
	int dx, dy, range, min_range;
	int i;

	get_self(&me);
	get_vehicles(&num_vinfos, vinfo);

	/* Find the closest enemy with a clear path to it */
	min_range = 99999999;
	for (i = 0; i < num_vinfos; i++) {
		v = &vinfo[i];

		/* Ignore vehicles on my team, unless we're neutral */
		if (v->team == me.team && me.team != NEUTRAL)
			continue;

		/* Ignore vehicles that have no clear path to them */
		if (!clear_path(&me.loc, &v->loc))
			continue;

		dx = v->loc.x - me.loc.x;
		dy = v->loc.y - me.loc.y;
		range = dx * dx + dy * dy;
		if (range < min_range) {
			min_range = range;
			*enemy = *v;
		}
	}

	return (min_range == 99999999) ? FALSE : TRUE;
}

/*
   The Xtank blazingly fast line & rectangle intersection routine!
   (OK, OK, it's a little slower than the old point & rectangle code,
   but it has to do a least two point compares, obviously.)

   The following (written by lidl, and stripes) implements the
   Cohen-Sutherland clipping algorithm, as seen in "Fundamentals
   of Interactive Computer Graphics by Foley and Van Dam",
   page 144-149.
*/

#define csCode(temp, x, y, w1, h1, w2, h2) \
	temp = 0; \
	if (x < w1) temp |= 0x01; \
	if (x > w2) temp |= 0x02; \
	if (y < h1) temp |= 0x08; \
	if (y > h2) temp |= 0x04;

#define SwapTwo(temp, a, b) \
	temp = a; \
	a = b; \
	b = temp;

/*
   Return true is any part of line (ix1, iy1) to (ix2, iy2)
   intersects, or touches the rectangle from (w1, h1) to (w2, h2)

   Changed to return the result of the clip algorithm (HAK 3/93)
*/
int line_in_rect(int *ix1, int *iy1, int *ix2, int *iy2, int w1, int h1, int w2, int h2)
{
	int code1, code2;
	int converted = FALSE;
	int temp;
	float x1, y1, x2, y2;
	float ftemp;

	csCode(code1, *ix1, *iy1, w1, h1, w2, h2);
	csCode(code2, *ix2, *iy2, w1, h1, w2, h2);

	for (;;) {
		/* trivially reject */
		if (code1 & code2) {
			/* don't have to convert back to ints here, not
			 * needed (HAK)
			 */
			return (FALSE);
		}
		/* trivially accept */
		if (code1 == 0 && code2 == 0) {
			if (converted) {  /*convert back to ints (HAK 3/93) */
				*ix1 = (int) x1;
				*iy1 = (int) y1;
				*ix2 = (int) x2;
				*iy2 = (int) y2;
			}
			return (TRUE);
		}
		/* convert from ints to floats, after the easy cases */
		if (!converted) {
			converted = TRUE;	/* only do this once */
			x1 = (float) *ix1;
			y1 = (float) *iy1;
			x2 = (float) *ix2;
			y2 = (float) *iy2;
		}
		/* swap code1 and code2, and the x1,y1 and x2,y2 points */
		if (!(code1 & 0x0f)) {
			SwapTwo(temp, code1, code2);
			SwapTwo(ftemp, x1, x2);
			SwapTwo(ftemp, y1, y2);
		}
		if (code1 & 0x08) {
			x1 = x1 + (x2 - x1) * (h2 - y1) / (y2 - y1);
			y1 = h2;
		} else if (code1 & 0x04) {
			x1 = x1 + (x2 - x1) * (h1 - y1) / (y2 - y1);
			y1 = h1;
		} else if (code1 & 0x02) {
			y1 = y1 + (y2 - y1) * (w2 - x1) / (x2 - x1);
			x1 = w2;
		} else if (code1 & 0x01) {
			y1 = y1 + (y2 - y1) * (w1 - x1) / (x2 - x1);
			x1 = w1;
		}
		csCode(code1, x1, y1, w1, h1, w2, h2);
		csCode(code2, x2, y2, w1, h1, w2, h2);
	}
}
