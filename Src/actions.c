/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** actions.c
*/

/*
$Author: lidl $
$Id: actions.c,v 2.11 1992/08/18 05:40:06 lidl Exp $

$Log: actions.c,v $
 * Revision 2.11  1992/08/18  05:40:06  lidl
 * added tac nuke changes
 *
 * Revision 2.10  1992/06/07  02:45:08  lidl
 * Post Adam Bryant patches and a manual merge of the rejects (ugh!)
 *
 * Revision 2.9  1992/05/19  22:57:19  lidl
 * post Chris Moore patches, and sqrt to SQRT changes
 *
 * Revision 2.8  1992/03/31  21:45:50  lidl
 * Post Aaron-3d patches, camo patches, march patches & misc PIX stuff
 *
 * Revision 2.7  1992/01/29  08:35:48  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.6  1991/12/15  21:10:07  aahz
 * improved previous and next live_tank calls.
 *
 * Revision 2.5  1991/12/10  03:41:44  lidl
 * changed float to FLOAT, for portability reasons
 *
 * Revision 2.4  1991/11/22  07:00:42  aahz
 * added functions to return the next/previous number
 *
 * Revision 2.3  1991/02/10  13:50:05  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:17  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:10:51  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:08:58  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:01:52  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "disc.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "vstructs.h"
#include "bullet.h"
#include "terminal.h"
#include "cosell.h"
#include "globals.h"
#include "assert.h"


extern Terminal *term;
extern Weapon_stat weapon_stat[];
extern Settings settings;
extern Boolean game_paused;


/*
** Adjusts the specified location by dx in the x direction, and
** dy in the y direction.
*/
adjust_loc(loc, dx, dy)
Loc *loc;
int dx, dy;
{
	/* adjust x coordinate */
	loc->x += dx;
	if ((loc->box_x += dx) < 0)
		do
		{
			loc->grid_x--;
		} while ((loc->box_x += BOX_WIDTH) < 0);
	else
	if (loc->box_x >= BOX_WIDTH)
		do
		{
			loc->grid_x++;
		} while ((loc->box_x -= BOX_WIDTH) >= BOX_WIDTH);

	/* adjust y coordinate */
	loc->y += dy;
	if ((loc->box_y += dy) < 0)
		do
		{
			loc->grid_y--;
		} while ((loc->box_y += BOX_HEIGHT) < 0);
	else
	if (loc->box_y >= BOX_HEIGHT)
		do
		{
			loc->grid_y++;
		} while ((loc->box_y -= BOX_HEIGHT) >= BOX_HEIGHT);
}

/*
** Creates a bullet from the specified weapon type at the given location,
** owned by the vehicle v, moving at the specified angle.
*/
/*
 * New function for generating smart (has an internal coordinate)
 * bullets.  The original function is replaced by a macro (someplace...)
 * that points here.
 */
make_smart_bullet(v, loc, type, angle, target)
Vehicle *v;
Loc *loc;
WeaponType type;
Angle angle;
lCoord *target;
{
    extern set_disc_team();
    Weapon_stat *ws;
    extern Bset *bset;
    Bullet *b;
    static int discteam = 0;

    if ((target == NULL) && (type == HARM))
	return;

    if (bset->number >= MAX_BULLETS)
	return;			/* too many bullets */

    /* Initialize the bullet's owner and location */
    b = bset->list[bset->number++];
    b->owner = v;
    b->thrower = -1;
    b->loc1 = b->loc2 = *loc;
    b->loc = &b->loc1;
    b->old_loc = &b->loc2;

/*
 *
 * Note that this "height" idea here is an extensive change. It moves a buncha
 * logic from hit.c over here for setting the "height" of a bullet. Has the
 * disadvantage that these "high" objects still hit the things they run
 * into, they just don't explode! Maybe i'll fix that sometime...
 * 
 */
    switch (type) {
      case SEEKER:
    		b->old_loc->z = b->loc->z = 1;
		break;
      case MINE:
    		b->old_loc->z = b->loc->z = -1;
		break;
      default:                                   /* HARM when launched too */
    		b->old_loc->z = b->loc->z = 0;
		break;
     }

    /* Find the weapon_stat structure */
    ws = &weapon_stat[(int)type];

    /* Compute the x and y components of the bullet's speed */
    b->xspeed = (FLOAT) ws->ammo_speed * cos(angle);
    b->yspeed = (FLOAT) ws->ammo_speed * sin(angle);

    /* Add the motion of the owner (if necessary) */
    if (settings.si.rel_shoot == TRUE && v != NULL)
    {
	b->xspeed += v->vector.xspeed;
	b->yspeed += v->vector.yspeed;
    }
    b->type = type;
    b->life = ws->frames;
    b->hurt_owner = FALSE;
    if (type == HARM) {
        b->target.y = target->y;
        b->target.x = target->x; 
    }
    if (type == DISC) {
	set_disc_team(b, discteam++);
    } else {
	discteam = 0;
    }
}

/*
** Destroys bullet b, and creates an explosion in its place.
** The type of the explosion is dependent on the damage.
*/
explode(b, damage)
Bullet *b;
int damage;
{
    unsigned int type;

    /* Determine type of explosion from the amount of damage the bullet did */
    type = (damage <= 4) ? (EXP_DAM0 + damage) : EXP_DAM4;
#ifdef NO_GAME_BALANCE
    if (b->type == NUKE) {
	expl_nuke(b);
    } else
#endif
        make_explosion(b->loc, type);

    /* I think this makes bullets that explode before they are displayed
       not get erased, but I'm not sure... (sigh) */
    if (b->type != DISC) {
	if (b->life == weapon_stat[(int)b->type].frames - 1)
	    b->life = -2;		/* undisplayed bullet */
	else
	    b->life = -1;
    }
}

#ifdef NO_GAME_BALANCE
#define NUKE_DIST 215
expl_nuke(b)
Bullet *b;
{
	double dist;
	int dx, dy, i, j, damage;

	make_explosion(b->loc, EXP_NUKE);
	for (i = 0; i < num_veh_alive; i++) {
	dx = live_vehicles[i]->loc->x - b->loc->x;
	dy = live_vehicles[i]->loc->y - b->loc->y;
	dist = sqrt((double)(dx*dx + dy*dy));
	if ((int)dist <= NUKE_DIST) {
		damage = (int)(200*(1.0-dist/NUKE_DIST));
		for ( j = FRONT; j <= BOTTOM; j++) {
			live_vehicles[i]->armor.side[j] -= damage;
			if (live_vehicles[i]->armor.side[j] < 0) {
				live_vehicles[i]->armor.side[j] = 0;
				kill_vehicle(live_vehicles[i], b->owner);
				}
			}
		}
	}
}
#endif

/*
** Does the specified action for the specified special in the
** specified vehicle.
*/
do_special(v, special_num, action)
Vehicle *v;
SpecialType special_num;
unsigned int action;
{
    Special *s;

    /* deleted all of the REPAIR crap, let the special handle it -ane */

    /* First make sure the special is really there */
    s = &v->special[(int)special_num];
    if (s->status == SP_nonexistent)
	return;

    switch (action)
    {
/*
 * Use the "activate" and "deactivate" for robots,
 * the "on" and "off" for humans.
 */

 /*
  * Well, calling do_special from inside of a special
  * was a big mistake.  hacka hacka hacka...
  *
  * In order for a special to refuse
  * to activate, the specials can now return a value
  * in the case of an SP_activate & return either a 
  * SP_on or a SP_off.             --ane
  */

/*
 * To answer RDP's question, the tests for SP_on & SP_off
 * inhibit anything but a SP_repair from changing the state
 * of a broken special.
 */

      case SP_update:
      case SP_redisplay:
      case SP_draw:
      case SP_erase:
	if (s->status == SP_on)
	    (*s->proc) (v, s->record, action);
	break;
      case SP_activate:
	if (s->status == SP_off)	/* why this test? -RDP */
	{
	    s->status = (*s->proc) (v, s->record, SP_activate);
	}
	break;
      case SP_deactivate:
	if (s->status == SP_on)
	{
	    s->status = SP_off;
	    (*s->proc) (v, s->record, SP_deactivate);
	}
	break;
      case SP_toggle:
	if (s->status == SP_off)
	{
	    if ((s->status = (*s->proc) (v, s->record, SP_activate)) == SP_on)
		(*s->proc) (v, s->record, SP_draw);

	}
	else if (s->status == SP_on)
	{
	    s->status = SP_off;
	    (*s->proc) (v, s->record, SP_deactivate);
	    (*s->proc) (v, s->record, SP_erase);
	}
	break;
      case SP_break:
	if (s->status == SP_on)
	{
	    s->status = SP_off;
	    (*s->proc) (v, s->record, SP_deactivate);
	    (*s->proc) (v, s->record, SP_erase);
	}
	s->status = SP_broken;
	break;
      case SP_repair:
	    if ((s->status = (*s->proc) (v, s->record, SP_activate)) == SP_on)
		(*s->proc) (v, s->record, SP_draw);
	break;
      case SP_on:
	if (s->status == SP_off) {
	    if ((s->status = (*s->proc) (v, s->record, SP_activate)) == SP_on)
		(*s->proc) (v, s->record, SP_draw);
	}
	break;
      case SP_off:
	if (s->status == SP_on) {
	    s->status = SP_off;
	    (*s->proc) (v, s->record, SP_deactivate);
	    (*s->proc) (v, s->record, SP_erase);
	}
	break;
    }
}

/*
** Moves the view on the current terminal by dx vertically and dy horizontally.
** The terminal stops tracking.
*/
move_view(dx, dy)
int dx, dy;
{
    Intloc new;

    /* switch view to a non-existent vehicle */
    switch_view(-1);

    /* compute the new viewing location */
    new.x = term->loc.x + dx;
    new.y = term->loc.y + dy;

    new.grid_x = new.x / BOX_WIDTH;
    new.grid_y = new.y / BOX_HEIGHT;

    /* If the new location is in the grid, move the view there */
    if (new.grid_x >= 0 && new.grid_x < GRID_WIDTH - 4 &&
	new.grid_y >= 0 && new.grid_y < GRID_HEIGHT - 4)
	term->loc = new;
}

#define INCR_WRAP(a,b) {if ((a) == (b)) (a) = 0; else (a)++;}
#define DECR_WRAP(a,b) {if ((a) == 0) (a) = (b); else (a)--;}
int next_live_tank()
{
	int iCurrentTank = -1;

	if (term->vehicle)
	{
		iCurrentTank = term->vehicle->number;

		INCR_WRAP(iCurrentTank, num_veh - 1);

		while (! IsVehicleAlive(iCurrentTank))
		{
			INCR_WRAP(iCurrentTank, num_veh - 1);
		}
	}

	return (iCurrentTank);
}

int previous_live_tank()
{
	int iCurrentTank = -1;

	if (term->vehicle)
	{
		iCurrentTank = term->vehicle->number;

		DECR_WRAP(iCurrentTank, num_veh - 1);

		while (! IsVehicleAlive(iCurrentTank))
		{
			DECR_WRAP(iCurrentTank, num_veh - 1);
		}
	}

	return (iCurrentTank);
}

int IsVehicleAlive(num)
	int num;
{
	int i;

	for (i = 0; i < num_veh_alive; i++) 
	{
	    if (live_vehicles[i]->number == num) 
		{
			break;
	    }
	}

	return ((i != num_veh_alive));
}


/*
** Switches the view on the current terminal to the perspective of
** the vehicle numbered num.  If num is not a legal vehicle number,
** the terminal stops tracking.
*/

switch_view(num)
    int num;
{
    extern int num_terminals;
    extern Terminal *terminal[];
    Vehicle *old_vehicle;
    int i;

    if (num >= 0)
    {
	/* remember who we were tracking */
	old_vehicle = term->vehicle;

	/* find the owner of the vehicle with that number */
	for (i = 0; i < num_veh_alive; i++) {
	    if (live_vehicles[i]->number == num) {
		term->vehicle = live_vehicles[i];
		break;
	    }
	}

	/* If we didn't find the vehicle number, or found the one that we
	   used to be tracking, don't expose the windows */
	if (old_vehicle == term->vehicle)
	    return;

	if (old_vehicle)
	    old_vehicle->owner->num_players--;
	for (i = 0; i < num_terminals && terminal[i] != term; i++)
	    ;
	assert(i != num_terminals);
	term->vehicle->owner->player[term->vehicle->owner->num_players++] = i;
    }
    else
    {
	if (term->vehicle != NULL)
	    term->vehicle->owner->num_players--;
	term->vehicle = NULL;	/* stop tracking */
    }

    /* Expose all the windows which need to be redisplayed after the switch */
    expose_win(ANIM_WIN, TRUE);
    expose_win(CONS_WIN, TRUE);
    expose_win(MAP_WIN, TRUE);
}

/*
** Game speed control functions
*/

#define PAUSE_X ANIM_WIN_WIDTH/2
#define PAUSE_Y 10

/*
** Xors the pause message onto the animation window.
*/
display_pause_message()
{
    draw_text(ANIM_WIN, PAUSE_X, PAUSE_Y,
	      "Game paused -- RETURN to unpause  SPACE to single step",
	      M_FONT, DRAW_XOR, WHITE);
}

/*
** If state is TRUE, sets game_speed to 0 to pause.
** If state is FALSE, restores game_speed to old value.
*/
pause_game(state)
    Boolean state;
{
    if (state == TRUE)
    {
	/* If not already paused, set game_speed to 0 */
	if (!game_paused)
	{
	    display_pause_message();
	    game_paused = TRUE;
	}
    }
    else
    {
	/* If paused, restore old game_speed */
	if (game_paused)
	{
	    display_pause_message();
	    game_paused = FALSE;
	}
    }
}

/*
** Sets game speed to specified value if it is reasonable.
*/
set_game_speed(spd)
int spd;
{
	if (spd > 0 && spd <= MAX_GAME_SPEED)
		settings.game_speed = spd;
	start_real_counter(1 + 999999 / settings.game_speed);
}

/*
** Deals with game pause.  Waits if game is running faster than it should.
*/
check_game_speed()
{
	char reply;

	if (settings.game_speed == MAX_GAME_SPEED && !game_paused)
		return;
	if (game_paused)
	{
		/* Game is paused, so wait for a space bar or return */
		do
		{
			reply = get_reply();
		} while (reply != ' ' && reply != '\r');

		/* Unpause game on a return, single step on a space bar */
		if (reply == '\r')
			pause_game(FALSE);
	}
	wait_for_real_counter();
}
