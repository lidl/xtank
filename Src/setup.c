/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "xtank.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "vstructs.h"
#include "message.h"
#include "terminal.h"
#include "cosell.h"
#include "thread.h"
#include "globals.h"
#include "assert.h"
#include "bullet.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif /* SOUND */

#include <stdlib.h>		/* for random() */

extern Map real_map;
extern Boolean game_paused;
extern int frame;
extern Settings settings;
extern int num_terminals;
extern Terminal *terminal[];
extern Terminal *term;
extern Engine_stat engine_stat[];

#define rnd(max)  (random() % (max))

int num_combatants;
Combatant combatant[MAX_VEHICLES];

Boolean game_running = False;


/*
** Sets up a combatant for each player and 5 + difficulty/2 robots.
*/
void
standard_combatants(void)
{
	Combatant *c;
	int i;

	num_combatants = num_terminals + 5 + settings.difficulty / 2;
	for (i = 0; i < num_combatants; i++) {
		c = &combatant[i];
		if (i < num_terminals) {
			/* Set up a vehicle for each player */
			c->num_players = 1;
			c->player[0] = i;
			c->num_programs = 0;
			c->team = (i + 1) % MAX_TEAMS;
			c->vdesc = terminal[i]->vdesc;
		} else {
			/* Set up everyone else as neutral robots */
			c->num_players = 0;
			c->num_programs = 1;
			c->program[0] = choose_program();
			c->team = NEUTRAL;
			c->vdesc = 0;
		}
	}
}

/*
** Chooses a program based on difficulty and game settings.
*/
int
choose_program(void)
{
	return 1;
}

/*
** Sets up 5 + difficulty robot combatants on different teams.
*/
void
robot_combatants(void)
{
	Combatant *c;
	int i;

	num_combatants = 5 + settings.difficulty;
	for (i = 0; i < num_combatants; i++) {
		c = &combatant[i];
		c->num_players = 0;
		c->num_programs = 1;
		c->program[0] = choose_program();
		c->team = i % MAX_TEAMS;
		c->vdesc = 0;
	}
}

/*
** Sets up a combatant for each player.
*/
void
player_combatants(void)
{
	Combatant *c;
	int i;

	num_combatants = num_terminals;
	for (i = 0; i < num_combatants; i++) {
		c = &combatant[i];
		c->num_players = 1;
		c->player[0] = i;
		c->num_programs = 0;
		c->team = (i + 1) % MAX_TEAMS;
		c->vdesc = terminal[i]->vdesc;
	}
}

/*
** Sets up a customized combatants list from the combatants grid.
*/
void
customized_combatants(void)
{
	Combatant *c;
	int i;

	num_combatants = 0;
	for (i = 0; i < MAX_VEHICLES; i++) {
		c = &combatant[num_combatants];
		if (!make_grid_combatant(c, i))
			num_combatants++;
	}
}

void
init_terms(void)
{
	int i;
	extern int num_terminals;
	extern Terminal *terminal[];

	for (i = 0; i < num_terminals; i++) {
		if (!terminal[i])
			continue;
		terminal[i]->observer = True;
		terminal[i]->vehicle = NULL;
	}
}

/*
** Initializes the number, score, kills, deaths, and money of all combatants.
*/
void
init_combatants(void)
{
	extern Vdesc *vdesc;
	extern Prog_desc *prog_desc[];
	Combatant *c;
	int max_cost = 0, i;

	num_teams = 0;
	for (i = 0; i < MAX_TEAMS; ++i) {
		teamdata[i].vehicle_count = 0;
		teamdata[i].treasury = 0;
	}

	for (i = 0; i < num_combatants; i++) {
		c = &combatant[i];
		/* name combatant after player, or failing that a program */
		if (c->num_players) {
			(void) strncpy(c->name, terminal[c->player[0]]->player_name,
						   MAX_STRING - 1);
			
		} else {
			assert(c->num_programs > 0);
			(void) strncpy(c->name, prog_desc[c->program[0]]->name,
						   MAX_STRING - 1);
			
		}

		max_cost = MAX(max_cost, vdesc[c->vdesc].cost);
		last_team = MAX(last_team, c->team);
		if ((teamdata[c->team].vehicle_count)++ == 0) {
			++num_teams;
		}
		c->number = i;

		c->keep_score += c->score;
		c->keep_kills += c->kills;
		c->keep_deaths += c->deaths;

		c->score = 0;
		c->kills = 0;
		c->deaths = 0;
	}

	init_terms();

	/* Give everyone $ 2 * max_cost - cost of their vehicle */
	for (i = 0; i < num_combatants; i++) {
		c = &combatant[i];
		c->money = max_cost * 2;
		if (!settings.si.pay_to_play)
			c->money -= vdesc[c->vdesc].cost;
		teamdata[c->team].treasury += c->money;
	}
}

/*
** Animates a game until it is finished, and then cleans up allocated stuff.
** Returns final status of the game.
*/
int
play_game(void)
{
	unsigned int status;
	int i;
	Boolean newgame;

	if (num_combatants == 0)
		return GAME_FAILED;

	/* In an ultimate game, only reset at the beginning, since we want don't
       want scores to reset after every goal. */
	if (settings.si.game == ULTIMATE_GAME) {
		init_combatants();
		newgame = TRUE;
	}
	do {
#ifdef X11
		button_up(ANIM_WIN, FALSE);
		follow_mouse(ANIM_WIN, FALSE);
#endif
		if (settings.si.game != ULTIMATE_GAME) {
			init_combatants();
			newgame = TRUE;
		}
		if (setup_game(newgame) == GAME_FAILED)
			return GAME_FAILED;
		newgame = FALSE;


		game_running = True;

#ifdef SOUND
		play_all(START_SOUND);
#endif /* SOUND */

		do {
			status = animate();
		} while (status == GAME_RUNNING);

#ifdef SOUND
		play_all(END_SOUND);
#endif /* SOUND */

		game_running = False;

		status = display_game_stats(status);

		game_cleanup();			/* nuking all the vehicles here is overkill */
	} while (status == GAME_RESET);

	/* Unmap battle windows and clear all other windows on all the terminals */
	for (i = 0; i < num_terminals; i++) {
		set_terminal(i);
		unmap_battle_windows();
		clear_windows();
	}

	/* Return to terminal 0 for the user interface */
	set_terminal(0);

#ifdef X11
	button_up(ANIM_WIN, TRUE);
	follow_mouse(ANIM_WIN, TRUE);
#endif

	return status;
}

/*
** Sets up the combatants, and the world for a game.
** Returns either GAME_FAILED or GAME_RUNNING.
*/
int
setup_game(Boolean newgame)
{
	int i;

	frame = 0;

	/* Setup maze */
	setup_maze();

	/* Initialize bullets and explosions */
	init_bset();
	init_eset();

	/* Initialize box names */
	init_box_names();

	/* Initialize all terminals to observe vehicle 0 */
	for (i = 0; i < num_terminals; i++) {
		setup_terminal(i, &actual_vehicles[0]);
		terminal[i]->observer = True;
	}

	num_veh = num_veh_alive = num_veh_dead = 0;

	/* Set up combatants */
	for (i = 0; i < num_combatants; i++)
		if (setup_combatant(&combatant[i]) == GAME_FAILED)
			return GAME_FAILED;

	/* Initialize message menus for game */
	init_msg_game();

	/* Initialize status windows (based on vehicle information) */
	init_status();

	/* Initialize game rules and disc history */
	disc_init_history();
	game_rules(TRUE);

	/* Initialize the commentator */
	if (settings.commentator)
		comment(COS_INIT_MOUTH, (int) newgame, (Vehicle *) NULL,
				(Vehicle *) NULL, (Bullet *) NULL);

	game_paused = FALSE;

	/* Initialize the game speed */
	set_game_speed(settings.game_speed);

	/* Flush the input */
	sync_terminals(TRUE);

	return GAME_RUNNING;
}


/* Makes and initializes a vehicle for the specified combatant.  Returns
   GAME_RUNNING if successful, GAME_FAILED if vehicle could not be set up.  */
int
setup_combatant(Combatant *c)
{
	extern Vdesc *vdesc;
	extern Vehicle *make_vehicle();
	extern int team_color[];
	Vehicle *v;
	int i;

	v = make_vehicle(&vdesc[c->vdesc], c);
	if (v == NULL)
		return GAME_FAILED;

	c->vehicle = v;

	for (i = 0; i < c->num_players; i++) {
		terminal[c->player[i]]->observer = False;
		terminal[c->player[i]]->teleview = False;
	}

	return activate_vehicle(v);
}


/* sets up the given terminal */
void
setup_terminal(int num, Vehicle *v)
{
	int i;

	set_terminal(num);
	term->vehicle = v;
	for (i = 0; i < MAX_WINDOWS; i++)
		expose_win(i, TRUE);
	term->num_lines = 0;
	
	/* 
	 * Check whether to setup terminal with mouse_drive active on game
	 * start up.
	 */
	if (term->mouse_drive == 0)
	  {
	    follow_mouse(ANIM_WIN, FALSE);
	    button_up(ANIM_WIN, FALSE);
	    term->mouse_drive_active = FALSE;
	  }
	else
	  {
	    term->mouse_drive_active = (term->mouse_drive > 0) ? FALSE : TRUE;
	    
	    button_up(ANIM_WIN, TRUE);
	    follow_mouse(ANIM_WIN, TRUE);
	  }
	
}

/*
** Tries to put the vehicle at each of its team starting locations.
** If those are all taken, random coordinates are used.
** Returns -1 if vehicle could not be placed.
*/
int
place_vehicle(Vehicle *v)
{
	extern Maze maze;
	Vector *vector;
	int views;
	Box *b;
	int num_starts;
	Coord *start;
	int grid_x, grid_y;
	int tries, num, rand;

	/* Check if the desired box is in the maze, and unoccupied.  If not,
       start picking random boxes until we find one that is.  Once we find a
       "reasonable" box, try to get one that is the same team as the vehicle
       and normal.  If this doesn't seem possible, accept a neutral normal
       box.  If this doesn't seem possible, accept a box of any team that
       isn't an outpost.  If this doesn't seem possible, the vehicle cannot
       be placed. */
	num_starts = maze.num_starts[v->team];
	start = maze.start[v->team];
	tries = 0;
	num = 0;
	/* XXX - kludge to avoid division by zero */
	if (num_starts == 0)
		num_starts = 1;
	rand = rnd(num_starts);
	for (;;) {
		/* Take the next start location, or a random one if there are none */
		if (num < num_starts) {
			grid_x = start[(num + rand) % num_starts].x;
			grid_y = start[(num + rand) % num_starts].y;
			num++;
		} else {
			grid_x = rnd(GRID_WIDTH);
			grid_y = rnd(GRID_HEIGHT);
		}

		b = &real_map[grid_x][grid_y];

		if (b->flags & INSIDE_MAZE &&
			!(b->flags & ANY_VEHICLE)) {
			if (b->team == v->team && b->type == NORMAL)
				break;
			else if (tries > 500 && b->team == 0 && b->type == NORMAL)
				break;
			else if (tries > 1000 && b->type != OUTPOST)
				break;
		}
		/* Give up after 1500 tries */
		if (tries++ == 1500)
			return -1;
	}

	/* Put the vehicle in the center of the box */
	v->loc = &v->loc1;
	v->old_loc = &v->loc2;

	v->loc->grid_x = grid_x;
	v->loc->grid_y = grid_y;

	v->loc->box_x = BOX_WIDTH / 2;
	v->loc->box_y = BOX_HEIGHT / 2;

	v->loc->x = v->loc->grid_x * BOX_WIDTH + v->loc->box_x;
	v->loc->y = v->loc->grid_y * BOX_HEIGHT + v->loc->box_y;

	STRUCT_ASSIGN(*v->old_loc, *v->loc, Loc);

	vector = &v->vector;
	vector->speed = vector->xspeed = vector->yspeed = vector->drive = 0.0;
	/* orient the vehicle randomly */
	vector->angle = vector->desired_heading = vector->heading =
	  vector->old_heading = (FLOAT) rnd(100) * (2 * PI) / 100 - PI;
	views = v->obj->num_pics;
	vector->old_rot = vector->rot =
	  ((int) ((vector->heading) / (2 * PI) * views + views + .5)) % views;
	vector->heading_flag = NO_SPIN;

	/* Set flag in box to show that the vehicle is there */
	real_map[v->loc->grid_x][v->loc->grid_y].flags |= v->flag;

	return 0;
}


/* clean up after a game, unmaking all the vehicles */
void
game_cleanup(void)
{
	extern void inactivate_vehicle();
	int i;

	for (i = 0; i < num_veh_alive; ++i) {
		inactivate_vehicle(live_vehicles[i]);
		unmake_vehicle(live_vehicles[i]);
	}
	for (i = 0; i < num_veh_dead; ++i) {
		unmake_vehicle(dead_vehicles[i]);
	}
	/* permanently dead vehicles have already been freed */
}


/* apply func to all the terminals attached to the given vehicle */
int
all_terms(Vehicle *veh, void (*func)())
{
	int ctri;
	extern Terminal *terminal[];
	extern int num_terminals;

	for (ctri = 0; ctri < num_terminals; ctri++) {
		if (terminal[ctri]->vehicle == veh) {
			func(terminal[ctri]);
		}
	}

	return (0);
}
