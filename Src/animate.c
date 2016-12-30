/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "malloc.h"
#include "xtank.h"
#include "bullet.h"
#include "vehicle.h"
#include "terminal.h"
#include "globals.h"
#include "clfkr.h"
#include "graphics.h"
#include "proto.h"

extern int frame;
extern CLFkr command_options;
extern int num_combatants;
extern Combatant combatant[MAX_VEHICLES];

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
int
animate(void)
{
	extern int num_terminals;
	extern Terminal *terminal[];
	Vehicle *v;
	unsigned int retval;
	int i;
	static int quit_frame;		/* frame that game ends */

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
			--i;				/* new vehicle fell into this slot */
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
			--i;				/* new dead vehicle fell into this slot */
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
	for (i = 0; i < num_terminals; i++) {
		set_terminal(i);
		if (get_input() == GAME_QUIT) {
			if (i == 0)
				return GAME_QUIT;	/* game god quit? */

			/* %%% I don't think this is complete enough -RDP */

			if (--terminal[i]->vehicle->owner->num_players <= 0) {
				int j;
				int flags = 0;
				int need = PLAYS_COMBAT | PLAYS_WAR | PLAYS_ULTIMATE |
				PLAYS_CAPTURE | PLAYS_RACE;

				for (j = 0; j < terminal[i]->vehicle->num_programs; j++) {
					flags |= terminal[i]->vehicle->program[j].desc->abilities;
					if (need & flags)
						break;
				}

				if (!(need & flags)) {   /* HAK 2/93 */
					Vehicle *temp;

					/* save the vehicle addr, since kill_vehicle */
					/* will set terminal[i]->vehicle to NULL */
					temp = terminal[i]->vehicle;

					kill_vehicle(terminal[i]->vehicle, (Vehicle *) NULL);

					/* he ain't comin' back */
					setflag(temp->status, VS_permanently_dead);
					if (i != num_combatants - 1)
						combatant[i] = combatant[num_combatants - 1];
				}
			}
			/*
			terminal[i]->vehicle->owner->num_players--;
			*/
			remove_player(i);	/* close the terminal */
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

	if (!command_options.NoIO) {
		/* Update screen locations, and display everything on each terminal */
		for (i = 0; i < num_terminals; i++) {
			set_terminal(i);
			update_screen_locs();
			display_terminal(REDISPLAY, i == num_terminals - 1);
		}
	}
#ifndef NO_CAMO
/*
 * kind of a funny place, eh?  It's because old_camod actually
 * refers to if it has been drawn as a "camod" vehicle yet by
 * display.c; I must have been up too late when I thought of
 * this scheme.
 *
 * This could be moved to update_vehicle(), but it would require that
 * the update_vehicle be called after display_terminal, not before.
 *
 * Seemed a little risky!
 *
 * The reson for updating everyone, instead of just the "live vehicles"
 * is that I'm not sure how the "death-delay" thing works, I didn't want
 * to risk a screwup if you blew up a camo'd tank.  Feel free to try
 * it with live_vehicles.
 *
 * I'm welcome to suggestions for alternate hacks, several occured
 * to me, but this was by far the simplest, though it is a bit
 * "ugly" on the surface.   --ane
 *
 */
	for (i = 0; i < MAX_VEHICLES; i++)
		actual_vehicles[i].old_camod = actual_vehicles[i].camod;

#endif /* !NO_CAMO */

	/* Synchronize all terminals every sync_rate frames */
	if ((frame % sync_rate) == 0)
		sync_terminals(FALSE);

	return GAME_RUNNING;
}
