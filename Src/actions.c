/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** actions.c
*/

/*
$Author: rpotter $
$Id: actions.c,v 2.3 1991/02/10 13:50:05 rpotter Exp $

$Log: actions.c,v $
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
make_bullet(v, loc, type, angle)
Vehicle *v;
Loc *loc;
WeaponType type;
Angle angle;
{
    Weapon_stat *ws;
    extern Bset *bset;
    Bullet *b;

    if (bset->number >= MAX_BULLETS)
	return;			/* too many bullets */

    /* Initialize the bullet's owner and location */
    b = bset->list[bset->number++];
    b->owner = v;
    b->thrower = -1;
    b->loc1 = b->loc2 = *loc;
    b->loc = &b->loc1;
    b->old_loc = &b->loc2;

    /* Find the weapon_stat structure */
    ws = &weapon_stat[(int)type];

    /* Compute the x and y components of the bullet's speed */
    b->xspeed = (float) ws->ammo_speed * cos(angle);
    b->yspeed = (float) ws->ammo_speed * sin(angle);

    /* Add the motion of the owner (if necessary) */
    if (settings.si.rel_shoot == TRUE && v != NULL)
    {
	b->xspeed += v->vector.xspeed;
	b->yspeed += v->vector.yspeed;
    }
    b->type = type;
    b->life = ws->frames;
    b->hurt_owner = FALSE;
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
    make_explosion(b->loc, type);

    /* I think this makes bullets that explode before they are displayed not
       not get erased, but I'm not sure... (sigh) */

    if (b->life == weapon_stat[(int)b->type].frames - 1)
	b->life = -2;		/* undisplayed bullet */
    else
	b->life = -1;
}

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

    if (special_num == REPAIR)
    {
        if (v->special[(int)REPAIR].status == SP_nonexistent)
	    return;
	if (settings.si.no_wear) /* GHS */
	    return;		/* GHS */
	if (v->vector.speed != 0.0) /* GHS */
	    return;		/* GHS */
    }				/* GHS */
    /* First make sure the special is really there */
    s = &v->special[(int)special_num];
    if (s->status == SP_nonexistent)
	return;

    switch (action)
    {
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
	    s->status = SP_on;
	    (*s->proc) (v, s->record, SP_activate);
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
	    s->status = SP_on;
	    (*s->proc) (v, s->record, SP_activate);
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
	s->status = SP_on;
	(*s->proc) (v, s->record, SP_activate);
	(*s->proc) (v, s->record, SP_draw);
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
** Disc functions
*/

/*
** Releases all discs owned by a vehicle.  Sets speed of discs to speed.
** Allows one update if delay is set.  This allows robots to throw to
** an accuracy of one frame.
*/
release_discs(v, dspeed, delay)
Vehicle *v;
float dspeed;
Boolean delay;
{
    extern Bset *bset;
    Bullet *b;
    float ratio;
    float curspeed;
    int i;

    if (v->num_discs > 0)
    {
	for (i = 0; i < bset->number; i++)
	{
	    b = bset->list[i];
	    if (b->type == DISC && b->owner == v)
	    {
		if (delay)
		    update_disc(b);
		curspeed = sqrt(b->xspeed * b->xspeed + b->yspeed * b->yspeed);
                ratio = dspeed / curspeed;
		b->xspeed *= ratio;
		b->yspeed *= ratio;
		/* display_bullets(ON); */
		b->owner = (Vehicle *) NULL;
		b->thrower = v->color;
		/* display_bullets(ON); */

		/* Inform the commentator about the throw */
		if (settings.commentator)
		    comment(COS_OWNER_CHANGE, COS_IGNORE, (Vehicle *) NULL,
			    (Vehicle *) NULL);
	    }
	}
	v->num_discs = 0;
    }
}

/*
** Makes all discs owned by the vehicle spin in the specified direction.
*/
set_disc_orbit(v, dir)
Vehicle *v;
Spin dir;
{
	switch (dir)
	{
		case CLOCKWISE:
			v->status &= ~VS_disc_spin;
			break;
		case COUNTERCLOCKWISE:
			v->status |= VS_disc_spin;
			break;
		case TOGGLE:
			v->status ^= VS_disc_spin;
			break;
	}
}

/*
** Sets the owner of the specified disc to the specified vehicle.
*/
set_disc_owner(b, v)
Bullet *b;
Vehicle *v;
{
	/* Take it away from previous owner */
	if (b->owner != (Vehicle *) NULL)
	{
		b->owner->num_discs--;
		b->thrower = b->owner->color;
	}
	else
	{
		if (b->thrower == -1)
			b->thrower = WHITE;
	}

	/* Give it to new owner */
	/* display_bullets(OFF); */
	b->owner = v;
	/* display_bullets(OFF); */
	if (b->owner != (Vehicle *) NULL)
		b->owner->num_discs++;
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
