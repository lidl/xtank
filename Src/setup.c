#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** setup.c
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

extern Map box;
extern Boolean game_paused;
extern int frame;
extern Settings settings;
extern int num_vehicles, num_terminals;
extern Terminal *terminal[];
extern Terminal *term;
extern Engine_stat engine_stat[];


int num_combatants;
Combatant combatant[MAX_VEHICLES];


/*
** Sets up a combatant for each player and 5 + difficulty/2 robots.
*/
standard_combatants()
{
	Combatant *c;
	int i;

	num_combatants = num_terminals + 5 + settings.difficulty / 2;
	for (i = 0; i < num_combatants; i++)
	{
		c = &combatant[i];
		if (i < num_terminals)
		{
			/* Set up a vehicle for each player */
			c->num_players = 1;
			c->player[0] = i;
			c->num_programs = 0;
			c->team = (i + 1) % MAX_TEAMS;
			c->vdesc = terminal[i]->vdesc;
		}
		else
		{
			/* Set up everyone else as neutral robots some fighters some
			   talkers */
			c->num_players = 0;
			c->num_programs = 1;
			c->program[0] = choose_program();
			c->team = 0;
			c->vdesc = 0;
		}
	}
}

/*
** Chooses a program based on difficulty and game settings.
*/
choose_program()
{
	return 1;
}

/*
** Sets up 5 + difficulty robot combatants on different teams.
*/
robot_combatants()
{
	Combatant *c;
	int i;

	num_combatants = 5 + settings.difficulty;
	for (i = 0; i < num_combatants; i++)
	{
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
player_combatants()
{
	Combatant *c;
	int i;

	num_combatants = num_terminals;
	for (i = 0; i < num_combatants; i++)
	{
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
customized_combatants()
{
	Combatant *c;
	int i;

	num_combatants = 0;
	for (i = 0; i < MAX_VEHICLES; i++)
	{
		c = &combatant[num_combatants];
		if (!make_grid_combatant(c, i))
			num_combatants++;
	}
}

init_terms()
{
	int i;
	extern int num_terminals;
	extern Terminal *terminal[];

	for (i = 0; i < num_terminals; i++)
	{
		if (!terminal[i])
			continue;
		terminal[i]->is_dead = FALSE;
		terminal[i]->vehicle = NULL;
	}
}

/*
** Initializes the number, score, kills, deaths, and money of all combatants.
** Initializes mode to single, battle, or multi depending on number of players.
*/
init_combatants()
{
	extern Vdesc *vdesc;
	extern Prog_desc *prog_desc[];
	extern int num_teams;
	Combatant *c;
	Combatant tmp_c;
	int max_cost, max_team, max_players, i;

	max_cost = 0;
	max_team = 0;
	max_players = 0;
	/* Sort combatants, so deaths don't mess us up :-) */
	for (i = 1; i < num_combatants; i++)
	{
		if (combatant[i].number < combatant[i - 1].number)
		{
            memcpy((char *)&tmp_c, (char *)&combatant[i - 1],
		   sizeof(Combatant));
            memcpy((char *)&combatant[i - 1], (char *)&combatant[i],
		   sizeof(Combatant));
            memcpy((char *)&combatant[i], (char *)&tmp_c,
		   sizeof(Combatant));
		}
	}
	for (i = 0; i < num_combatants; i++)
	{
		c = &combatant[i];
		if (c->num_players)
		{
			max_players++;
			(void) strncpy(c->name, terminal[c->player[0]]->player_name, MAX_STRING - 1);
			c->flesh = TRUE;
		}
		else
		{
			(void) strncpy(c->name, prog_desc[c->program[0]]->name, MAX_STRING - 1);
			c->flesh = FALSE;
		}

		max_cost = MAX(max_cost, vdesc[c->vdesc].cost);
		max_team = MAX(max_team, c->team);
		c->number = i;
		c->score = 0;
		c->kills = 0;
		c->deaths = 0;
	}

	init_terms();

	switch (max_players)
	{
		case 0:
			settings.mode = BATTLE_MODE;
			break;
		case 1:
			settings.mode = SINGLE_MODE;
			break;
		default:
			settings.mode = MULTI_MODE;
			break;
	}

	/* Give everyone $ 1.5 * max_cost - cost of their vehicle */
	for (i = 0; i < num_combatants; i++)
	{
		c = &combatant[i];
		c->money = max_cost * 1.5;
		if (!settings.si.pay_to_play)
			c->money -= vdesc[c->vdesc].cost;
	}
	num_teams = max_team + 1;
}

/*
** Animates a game until it is finished, and then cleans up allocated stuff.
** Returns final status of the game.
*/
play_game()
{
	unsigned int status;
	int i;
	Boolean newgame;
	extern int end_boundry;

	if (num_combatants == 0)
		return GAME_FAILED;

	/* In an ultimate game, only reset at the beginning, since we want don't
	   want scores to reset after every goal. */
	if (settings.si.game == ULTIMATE_GAME)
	{
		init_combatants();
		newgame = TRUE;
	}

	do
	{
		if (settings.si.game != ULTIMATE_GAME)
		{
			init_combatants();
			newgame = TRUE;
		}
		/* Setup game */
		if (setup_game(newgame) == GAME_FAILED)
			return GAME_FAILED;
		newgame = FALSE;

		end_boundry = num_vehicles;

#ifdef X11
		button_up(ANIM_WIN, FALSE);
		follow_mouse(ANIM_WIN, FALSE);
#endif

		/* Animate until game is finished */
		do
		{
			status = animate();
		}
		while (status == GAME_RUNNING);

		/* Display stats about the games state */
		status = display_game_stats(status);

		/* Free all allocated memory for game */
		free_game_mem();
	}
	while (status == GAME_RESET);

	/* Unmap battle windows and clear all other windows on all the terminals */
	for (i = 0; i < num_terminals; i++)
	{
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
setup_game(newgame)
Boolean newgame;
{
	extern Vehicle *vehicle[];
	int i;

	frame = 0;
	num_vehicles = 0;

	/* Setup maze */
	setup_maze();

	/* Initialize bullets and explosions */
	init_bset();
	init_eset();

	/* Initialize all terminals to vehicle 0 */
	for (i = 0; i < num_terminals; i++)
		setup_terminal(i, vehicle[0]);

	/* Set up combatants */
	for (i = 0; i < num_combatants; i++)
		if (setup_combatant(&combatant[i]) == GAME_FAILED)
			return GAME_FAILED;

	/* Initialize message menus for game */
	init_msg_game();

	/* Initialize status windows (based on vehicle information) */
	init_status();

	/* Initialize game rules */
	game_rules(TRUE);

	/* Initialize the commentator */
	if (settings.commentator)
		comment(COS_INIT_MOUTH, (int) newgame, (Vehicle *) NULL, (Vehicle *) NULL);

	/* Start game in non-pausing mode */
	game_paused = FALSE;

	/* Initialize the game speed */
	set_game_speed(settings.game_speed);

	/* Flush the input */
	sync_terminals(TRUE);

	return GAME_RUNNING;
}

/*
** Makes and initializes the vehicle for the specified combatant.
** Returns GAME_RUNNING if successful, GAME_FAILED if vehicle could not
** be set up.
*/
setup_combatant(c)
Combatant *c;
{
	Mapper *m;
	extern char team_char[];
	extern Vdesc *vdesc;
	extern Vehicle *make_vehicle();
	Vehicle *v;
    Landmark_info *s;
	int i, j;
	extern int team_color[];


	/* Make and initialize the vehicle */
	v = make_vehicle(&vdesc[c->vdesc]);
	if (v == (Vehicle *) NULL)
		return GAME_FAILED;

	/* Assign number to vehicle before init, so init_messages() will work */
	v->number = c->number;
	init_vehicle(v);

	/* Assign the combatant specific information to the vehicle */
	v->flag = VEHICLE_0 << v->number;
	v->team = c->team;
	v->color = team_color[c->team];
	assert(c->team < MAX_TEAMS);

	sprintf(v->disp, "%c%d %s", team_char[c->team], c->number, c->name);
	v->owner = c;
	c->vehicle = v;
	/* Make and initialize the programs */
	make_programs(v, c->num_programs, c->program);
	init_programs(v);

	/* Set up each player's terminal */
	for (i = 0; i < c->num_players; i++)
		setup_terminal(c->player[i], v);

	/* Place vehicle in maze */
	if (place_vehicle(v) == -1)
		return GAME_FAILED;

	/* Activate the specials */
	for (i = 0; i < MAX_SPECIALS; i++)
        do_special(v, (SpecialType)i, SP_activate);

	/* Copy maze into mapper if full_map is on */
    if (v->special[(int)MAPPER].status != SP_nonexistent &&
	settings.si.full_map)
	{
        m = (Mapper *) v->special[(int)MAPPER].record;
		bcopy((char *) box, (char *) m->map, sizeof(Map));

		/* Initialize landmarks */
		m->num_landmarks = 0;
		for (i = 0; i < GRID_WIDTH; i++)
			for (j = 0; j < GRID_HEIGHT; j++)
                if ((m->map[i][j].type != NORMAL) &&
					(m->num_landmarks < MAX_LANDMARKS))
				{
					s = &m->landmark[m->num_landmarks++];
					s->type = m->map[i][j].type;
					s->x = i;
					s->y = j;
				}
	}
	return GAME_RUNNING;
}

/*
** Sets the vehicles and exposes the windows of the numbered terminal.
** Sets the number of lines drawn on the terminal to 0.
*/
setup_terminal(num, v)
int num;
Vehicle *v;
{
	int i;

	set_terminal(num);
	term->vehicle = v;
	for (i = 0; i < MAX_WINDOWS; i++)
		expose_win(i, TRUE);
	term->num_lines = 0;
}

/*
** Makes a vehicle structure from the specified vehicle description.
** Returns NULL if no room in list, otherwise returns pointer to vehicle.
*/
Vehicle *make_vehicle(d)
Vdesc *d;
{
	extern int num_vehicles;
	extern Vehicle *vehicle[];
	extern Object *vehicle_obj[];
	static int turn_divider[MAX_SPEED] = {
		8, 8, 8, 8, 10, 12, 16, 20, 24, 28, 34, 40, 46, 52, 60, 68, 78, 88,
	100, 114, 130, 146, 162, 180, 200};
	Vehicle *v;
	Weapon *w;
	int i;

	if (num_vehicles >= MAX_VEHICLES)
		return (Vehicle *) NULL;

	v = vehicle[num_vehicles++];

	v->vdesc = d;
	v->name = d->name;

	v->obj = vehicle_obj[d->body];
	v->num_turrets = v->obj->num_turrets;
	v->max_fuel = (float) engine_stat[d->engine].fuel_limit;

	v->num_weapons = d->num_weapons;

	for (i = 0; i < d->num_weapons; i++)
	{
		w = &v->weapon[i];
		w->type = d->weapon[i];
		w->mount = d->mount[i];
	}

	for (i = 0; i < MAX_SPECIALS; i++)
		v->special[i].status = (d->specials & (1 << i)) ? SP_off : SP_nonexistent;

	/* Turn rate for each speed */
	for (i = 0; i < MAX_SPEED; i++)
		v->turn_rate[i] = (float) d->handling / (float) turn_divider[i];

	/* Vehicle not restricted to making safe turns */
	v->safety = FALSE;

	return v;
}

/*
** Tries to put the vehicle at each of its team starting locations.
** If those are all taken, random coordinates are used.
** Returns -1 if vehicle could not be placed.
*/
place_vehicle(v)
Vehicle *v;
{
	extern Maze maze;
	Vector *vector;
	int views;
	Box *b;
	int num_starts;
	Coord *start;
	int grid_x, grid_y;
	int tries, num;

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
	for (;;)
	{
		/* Take the next start location, or a random one if there are none */
		if (num < num_starts)
		{
			grid_x = start[num].x;
			grid_y = start[num].y;
			num++;
		}
		else
		{
			grid_x = rnd(GRID_WIDTH);
			grid_y = rnd(GRID_HEIGHT);
		}

		b = &box[grid_x][grid_y];

		if (b->flags & INSIDE_MAZE &&
				!(b->flags & ANY_VEHICLE))
		{
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

	*v->old_loc = *v->loc;

	vector = &v->vector;
	vector->drive = 0.0;
	assign_speed(vector, 0.0, 0.0);
	/* orient the vehicle randomly */
	vector->desired_heading = vector->heading = vector->old_heading =
		(float) rnd(100) * (2 * PI) / 100 - PI;
	views = v->obj->num_pics;
	vector->old_rot = vector->rot =
		((int) ((vector->heading) / (2 * PI) * views + views + .5)) % views;
    vector->heading_flag = NO_SPIN;

	/* Set flag in box to show that the vehicle is there */
	box[v->loc->grid_x][v->loc->grid_y].flags |= v->flag;

	return 0;
}

int set_is_dead(trm)
Terminal *trm;
{
	trm->is_dead = TRUE;
}


/*
** Removes ith vehicle from the list, trying to restart a new one.
** Returns GAME_OVER if the game should end, otherwise GAME_RUNNING.
*/
remove_vehicle(num)
int num;
{
	extern int num_vehicles;
	extern Vehicle *vehicle[];
	extern int end_boundry;
	Vehicle *v;
	Combatant *c;
    int i, revive, team_count, ctr1;
	int swapped = FALSE;
	char teams[MAX_TEAMS];
	static int kill_frame = 0;

	v = vehicle[num];
	if (v->is_dead)
		return (GAME_RUNNING);

	/* MEGA KLUDGE  Josh made me do it!!! */
	if (v->special[0].record == NULL)
	{
		fprintf(stderr, "dead vehicle being removed more than once\n");
		return (GAME_RUNNING);
	}
	/* MEGA KLUDGE  Josh made me do it!!! */



	if (v->death_timer > 0)
		v->death_timer--;

	/* fprintf(stderr, "frame:  %d of %s@%d\n",frame, v->disp,
	   v->death_timer); */

	/* Rearrange vehicles in array to shrink it by one */
	switch (v->death_timer)
	{
		case (QUIT_DELAY - 1):
			if (num < num_vehicles - 1)
			{
				vehicle[num] = vehicle[num_vehicles - 1];
				vehicle[num_vehicles - 1] = v;
				swapped = TRUE;
			}
			num_vehicles--;
			/* v->status &= ~VS_was_alive; */
			break;
		case (QUIT_DELAY - 2):
			v->status &= ~VS_was_alive;
			break;
		case (1):
			/* Kludge around simultaneous death bug */
			if (kill_frame == frame)
			{
				fprintf(stderr, "Waiting 1 frame %s\n", v->disp);
				v->death_timer++;
				kill_frame = frame;
			}
			break;
		case (0):
			revive = TRUE;
			memset(teams, 0, sizeof(char) * MAX_TEAMS);

			if (settings.si.pay_to_play)
			{
				/* make 'em pay for a new tank */
				v->owner->money -= v->vdesc->cost;
				if (v->owner->money < 0)
				{				/* can't afford it? */
					char str[32];

					sprintf(str, "%c is broke.", v->number);
                    compose_message(SENDER_DEAD, RECIPIENT_ALL, OP_TEXT,
				    (Byte *) str);
					all_terms(v, set_is_dead);
					v->is_dead = TRUE;
					revive = FALSE;
				}
			}
			/* This little chunk untested */
			if (!settings.si.restart)
			{
				all_terms(v, set_is_dead);
				v->is_dead = TRUE;
				revive = FALSE;
			}
			for (ctr1 = 0; ctr1 < end_boundry; ctr1++)
			{
				if (!vehicle[ctr1]->is_dead)
				{
					teams[vehicle[ctr1]->owner->team] = 1;
				}
			}
            for (team_count = ctr1 = 0; ctr1 < MAX_TEAMS; ctr1++)
			{
				if (teams[ctr1])
				{
                    team_count++;
				}
			}
            if (team_count == 1 && !teams[0])    /* not neutral team */
				return (GAME_OVER);

			/* Let the terminals know that they have no vehicle to track */
			for (i = 0; i < v->owner->num_players; i++)
			{
				terminal[v->owner->player[i]]->vehicle = (Vehicle *) NULL;

#if 0
				printf("Clearing term[%d]  (%d)\n", v->owner->player[i], i);
#endif
			}

			/* Free the memory allocated for the vehicle */
			free_vehicle_mem(v);
			v->owner->deaths++;

			/* num_vehicles--; */

			/* put this guy at the first after boundry pos */
			if (num != num_vehicles)
			{
				vehicle[num] = vehicle[num_vehicles];
				vehicle[num_vehicles] = v;
			}
			/* If restart is on, restart the vehicle, otherwise check for
			   game over */
			if (settings.si.restart)
			{
				if (revive)
				{
					c = v->owner;
					if (setup_combatant(c) == GAME_FAILED)
						return (GAME_FAILED);
					v = c->vehicle;
					init_vehicle_status(v);
				}
			}
			else
			{
				if (settings.mode == SINGLE_MODE && v->owner->num_players > 0)
				{
					return GAME_OVER;
				}
				if (num_vehicles <= 1)
				{
					return GAME_OVER;
				}
			}
			break;
	}

	return (swapped ? SWAPPED : GAME_RUNNING);
}

/*
** Frees allocated memory for all remaining vehicles.
*/
free_game_mem()
{
	extern int num_vehicles;
	extern Vehicle *vehicle[];
	extern int end_boundry;
	int i;

	/* Free all the allocated memory for every vehicle */
	for (i = 0; i < end_boundry; i++)
		free_vehicle_mem(vehicle[i]);
}

/*
** Frees allocated memory and threads for the specified vehicle.
*/
free_vehicle_mem(v)
Vehicle *v;
{
	int i;

	/* Free the memory for the thread and buffer */
	for (i = 0; i < v->num_programs; i++)
	{
		if (v->program[i].thread == NULL)
		{
			fprintf(stderr, "NULL thread, %d %s: %s\n", i, v->name, v->disp);
			continue;
		}
        thread_kill((Thread *)v->program[i].thread);
		free((char *) v->program[i].thread);
		v->program[i].thread = NULL;;
	}

	/* Free the special record pointers */
	for (i = 0; i < MAX_SPECIALS; i++)
	{
		free((char *) v->special[i].record);
		v->special[i].record = NULL;
	}

	/* Free the turret array */
	if (v->num_turrets > 0)
	{
		free((char *) v->turret);
		v->turret = NULL;
	}
}

int all_terms(veh, func)
Vehicle *veh;
int (*func) ();

{
	int ctri;
	extern Terminal *terminal[];
	extern int num_terminals;

	for (ctri = 0; ctri < num_terminals; ctri++)
	{
		fprintf(stderr, "checking term # %d\n", ctri);
		if (terminal[ctri]->vehicle == veh)
		{
			fprintf(stderr, "setting is_dead\n");
			(*func) (terminal[ctri]);
		}
	}

	return (0);
}
