/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** animate.c
*/

/*
$Author: lidl $
$Id: animate.c,v 2.4 1991/09/19 05:27:57 lidl Exp $

$Log: animate.c,v $
 * Revision 2.4  1991/09/19  05:27:57  lidl
 * added LOCK_GAME_CONTROLS defines
 *
 * Revision 2.3  1991/02/10  13:50:06  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:19  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:10:54  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:00  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:01:53  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "terminal.h"
#include "globals.h"

extern int frame;

/* # frames between display synchronizations */
#ifndef LOCK_GAME_CONTROLS
int sync_rate = 1;
#else
int sync_rate = 16;
#endif

/*
** Removes dead vehicles, updates everything, checks for collisions,
** and displays all terminals, for one frame of animation.
**
** Returns one of GAME_RUNNING, GAME_QUIT, GAME_OVER, or GAME_RESET.
*/
animate()
{
    extern int num_terminals;
    extern Terminal *terminal[];
    Vehicle *v;
    unsigned int retval;
    int i;
    static int quit_frame;	/* frame that game ends */

    /* Check for paused or slowed game */
    check_game_speed();

    /* Increment frame counter */
    ++frame;

    /* Reset quit frame if we are starting a new game */
    if (frame <= 1)
	quit_frame = frame - 1;

    /* Check for internal end-of-game */
    if (frame == quit_frame)
	return GAME_OVER;

    /* check living vehicles */
    for (i = 0; i < num_veh_alive; ++i) {
	v = live_vehicles[i];

	if (tstflag(v->status, VS_is_alive)) {
	    setflag(v->status, VS_was_alive);
	} else {
	    if (tstflag(v->status, VS_permanently_dead)) {
		if (--(teamdata[v->team].vehicle_count) == 0) {
		    --num_teams;
		}
		/* everybody left on the same side? */
		if (num_teams < 2 && teamdata[NEUTRAL].vehicle_count < 2) {
		    quit_frame = frame + QUIT_DELAY;
		}
		unmake_vehicle(v);
	    } else {
		v->death_timer = DEATH_DELAY;
		/* put vehicle on dead list */
		dead_vehicles[num_veh_dead++] = v;
	    }
	    /* remove vehicle from live list */
	    live_vehicles[i] = live_vehicles[--num_veh_alive];
	    --i;		/* new vehicle fell into this slot */
	}
    }

    /* check dead vehicles */
    for (i = 0; i < num_veh_dead; ++i) {
	v = dead_vehicles[i];

	/* time to resurrect yet? */
	if (--(v->death_timer) <= 0) {
	    if (activate_vehicle(v) == GAME_OVER) {
		printf("animate(): couldn't resurrect_vehicle()\n");
		return GAME_OVER;
	    }
	    /* remove vehicle from dead list */
	    dead_vehicles[i] = dead_vehicles[--num_veh_dead];
	    --i;		/* new dead vehicle fell into this slot */
	}
    }

    /* Initialize the changed boxes in the maze */
    init_changed_boxes();

    /* Update the old screen locations for all the terminals */
    for (i = 0; i < num_terminals; i++)
	terminal[i]->old_loc = terminal[i]->loc;

    /* Clear the number of new messages for all the vehicles */
    for (i = 0; i < num_veh_alive; i++)
	live_vehicles[i]->new_messages = 0;

    /* Process input from all the programs */
    run_all_programs();

    /* Process input from all the terminals */
    for (i = 0; i < num_terminals; i++)
    {
	set_terminal(i);
	if (get_input() == GAME_QUIT) {
	    if (i == 0) return GAME_QUIT;	    /* game god quit? */

	    /* %%% I don't think this is complete enough -RDP */

	    kill_vehicle(terminal[i]->vehicle, (Vehicle *) NULL);
	    terminal[i]->vehicle->owner->num_players--;
	    remove_player(i);
	    i--;		/* new terminal has been moved in */
	}
    }

    /* Update locations and vectors of all vehicles, and bullets */
    for (i = 0; i < num_veh_alive; i++)
	update_vehicle(live_vehicles[i]);
    update_bullets();

    /* Check for collisions between all vehicles */
    coll_vehicles_vehicles();

    /* Check for collisions between each vehicle and the walls */
    for (i = 0; i < num_veh_alive; i++)
	coll_vehicle_walls(live_vehicles[i]);

    /* Check for bullet collisions against the maze and vehicles */
    coll_bullets_maze();
    coll_bullets_vehicles();

    /* Update vehicle rotations after collisions */
    for (i = 0; i < num_veh_alive; i++)
	update_rotation(live_vehicles[i]);

    /* Update explosions after bullet colls, since they might create some */
    update_explosions();

    /* Update maze flags after vehicle colls, to get new vehicle positions */
    update_maze_flags();

    /* Update specials after maze flags, since some specials use them */
    update_specials();

    /* Apply rules of the current game */
    if ((retval = game_rules(FALSE)) != GAME_RUNNING)
	return retval;

    /* Update screen locations, and display everything on each terminal */
    for (i = 0; i < num_terminals; i++)
    {
	set_terminal(i);
	update_screen_locs();
	display_terminal(REDISPLAY, i == num_terminals - 1);
    }

    /* Synchronize all terminals every sync_rate frames */
    if ((frame % sync_rate) == 0)
	sync_terminals(FALSE);

    return GAME_RUNNING;
}
