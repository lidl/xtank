/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** highlib.c
*/

/*
$Author: rpotter $
$Id: highlib.c,v 2.3 1991/02/10 13:50:40 rpotter Exp $

$Log: highlib.c,v $
 * Revision 2.3  1991/02/10  13:50:40  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:54  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:36  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:32  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:26  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "xtanklib.h"
#include "screen.h"
#include "vehicle.h"


extern Vehicle *cv;

/*
** Turns all turrets to the specified angle (in radians)
*/
void turn_all_turrets(angle)
Angle angle;
{
	int i;

	for (i = 0; i < cv->num_turrets; i++)
        turn_turret((TurretNum)i, angle);
}

/*
** Aims all turrets at a location dx away horizontally
** and dy away vertically from the vehicle
*/
void aim_all_turrets(dx, dy)
int dx, dy;
{
	int i;

	for (i = 0; i < cv->num_turrets; i++)
        aim_turret((TurretNum)i, dx, dy);
}

/*
** Attempts to fire all weapons in numerical order.
*/
int fire_all_weapons()
{
    int i;

    /* Check if the vehicle has any weapons */
    if (cv->num_weapons == 0)
    {
	check_time();
	return BAD_VALUE;
    }
    for (i = 0; i < cv->num_weapons; i++)
	fire_weapon(i);

    return 0;
}

/*
** Returns TRUE if there are no walls blocking the path from start to finish,
** otherwise returns FALSE.
*/
Boolean clear_path(start, finish)
Location *start, *finish;
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
	if (fgrid_y > tgrid_y)
	{			/* Southeast */
	    lattice_dx = (tgrid_x + 1) * BOX_WIDTH - start_x;
	    lattice_dy = (tgrid_y + 1) * BOX_HEIGHT - start_y;
	    while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
	    {
		if (lattice_dy * dx < dy * lattice_dx)
		{
		    if (wall(SOUTH, tgrid_x, tgrid_y))
			return FALSE;
		    tgrid_y++;
		    lattice_dy += BOX_HEIGHT;
		}
		else
		{
		    if (wall(EAST, tgrid_x, tgrid_y))
			return FALSE;
		    tgrid_x++;
		    lattice_dx += BOX_WIDTH;
		}
	    }
	}
	else if (fgrid_y < tgrid_y)
	{			/* Northeast */
	    lattice_dx = (tgrid_x + 1) * BOX_WIDTH - start_x;
	    lattice_dy = tgrid_y * BOX_HEIGHT - start_y;
	    while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
	    {
		if (lattice_dy * dx > dy * lattice_dx)
		{
		    if (wall(NORTH, tgrid_x, tgrid_y))
			return FALSE;
		    tgrid_y--;
		    lattice_dy -= BOX_HEIGHT;
		}
		else
		{
		    if (wall(EAST, tgrid_x, tgrid_y))
			return FALSE;
		    tgrid_x++;
		    lattice_dx += BOX_WIDTH;
		}
	    }
	}
	else
	{			/* East */
	    for (; tgrid_x < fgrid_x; tgrid_x++)
		if (wall(EAST, tgrid_x, tgrid_y))
		    return FALSE;
	}

    else if (fgrid_x < tgrid_x)
	if (fgrid_y > tgrid_y)
	{			/* Southwest */
	    lattice_dx = tgrid_x * BOX_WIDTH - start_x;
	    lattice_dy = (tgrid_y + 1) * BOX_HEIGHT - start_y;
	    while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
	    {
		if (lattice_dy * dx >= dy * lattice_dx)
		{
		    if (wall(SOUTH, tgrid_x, tgrid_y))
			return FALSE;
		    tgrid_y++;
		    lattice_dy += BOX_HEIGHT;
		}
		else
		{
		    if (wall(WEST, tgrid_x, tgrid_y))
			return FALSE;
		    tgrid_x--;
		    lattice_dx -= BOX_WIDTH;
		}
	    }
	}
	else if (fgrid_y < tgrid_y)
	{			/* Northwest */
	    lattice_dx = tgrid_x * BOX_WIDTH - start_x;
	    lattice_dy = tgrid_y * BOX_HEIGHT - start_y;
	    while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
	    {
		if (lattice_dy * dx < dy * lattice_dx)
		{
		    if (wall(NORTH, tgrid_x, tgrid_y))
			return FALSE;
		    tgrid_y--;
		    lattice_dy -= BOX_HEIGHT;
		}
		else
		{
		    if (wall(WEST, tgrid_x, tgrid_y))
			return FALSE;
		    tgrid_x--;
		    lattice_dx -= BOX_WIDTH;
		}
	    }
	}
	else
	{			/* West */
	    for (; tgrid_x > fgrid_x; tgrid_x--)
		if (wall(WEST, tgrid_x, tgrid_y))
		    return FALSE;
	}

    else if (fgrid_y > tgrid_y)
    {				/* South */
	for (; tgrid_y < fgrid_y; tgrid_y++)
	    if (wall(SOUTH, tgrid_x, tgrid_y))
		return FALSE;
    }
    else if (fgrid_y < tgrid_y)
    {				/* North */
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
Boolean get_closest_enemy(enemy)
    Vehicle_info *enemy;
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
	for (i = 0; i < num_vinfos; i++)
	{
		v = &vinfo[i];

		/* Ignore vehicles on my team, unless we're neutral */
		if (v->team == me.team && me.team != 0)
			continue;

		/* Ignore vehicles that have no clear path to them */
		if (!clear_path(&me.loc, &v->loc))
			continue;

		dx = v->loc.x - me.loc.x;
		dy = v->loc.y - me.loc.y;
		range = dx * dx + dy * dy;
		if (range < min_range)
		{
			min_range = range;
			*enemy = *v;
		}
	}

	return (min_range == 99999999) ? FALSE : TRUE;
}
