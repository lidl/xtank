/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** setup.c
*/

/*
$Author: lidl $
$Id: setup.c,v 2.11 1992/06/07 02:45:08 lidl Exp $

$Log: setup.c,v $
 * Revision 2.11  1992/06/07  02:45:08  lidl
 * Post Adam Bryant patches and a manual merge of the rejects (ugh!)
 *
 * Revision 2.10  1992/05/19  22:57:19  lidl
 * post Chris Moore patches, and sqrt to SQRT changes
 *
 * Revision 2.9  1992/03/31  04:04:16  lidl
 * pre-aaron patches, post 1.3d release (ie mailing list patches)
 *
 * Revision 2.8  1992/01/30  07:43:15  aahz
 * added init_box_names to avoid a seg fault later.
 *
 * Revision 2.7  1992/01/30  05:25:52  aahz
 * moved the game_running flag lower (into setup.c)
 *
 * Revision 2.6  1991/12/15  22:36:31  aahz
 * used a macro for structure assignment in place_vehicle
 *
 * Revision 2.5  1991/12/10  03:41:44  lidl
 * changed float to FLOAT, for portability reasons
 *
 * Revision 2.4  1991/03/25  00:42:11  stripes
 * RS6K Patches (IBM is a rock sucker)
 *
 * Revision 2.3  1991/02/10  13:51:38  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:56  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:56  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:28  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:04  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
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

extern Map real_map;
extern Boolean game_paused;
extern int frame;
extern Settings settings;
extern int num_terminals;
extern Terminal *terminal[];
extern Terminal *term;
extern Engine_stat engine_stat[];

#if defined(_IBMR2)
extern long random();
#define rnd(max)  (random() % (max))
#endif


int num_combatants;
Combatant combatant[MAX_VEHICLES];

Boolean game_running = False;


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
	} else {
	    /* Set up everyone else as neutral robots */
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
	terminal[i]->observer = True;
	terminal[i]->vehicle = NULL;
    }
}

/*
** Initializes the number, score, kills, deaths, and money of all combatants.
*/
init_combatants()
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

    for (i = 0; i < num_combatants; i++)
    {
	c = &combatant[i];
	/* name combatant after player, or failing that a program */
	if (c->num_players) {
	    (void) strncpy(c->name, terminal[c->player[0]]->player_name,
			   MAX_STRING - 1);
	    c->mouse_speed = terminal[c->player[0]]->mouse_speed;
	} else {
	    assert(c->num_programs > 0);
	    (void) strncpy(c->name, prog_desc[c->program[0]]->name,
			   MAX_STRING - 1);
	    c->mouse_speed = FALSE;
	}

	max_cost = MAX(max_cost, vdesc[c->vdesc].cost);
	last_team = MAX(last_team, c->team);
	if ((teamdata[c->team].vehicle_count)++ == 0) {
	    ++num_teams;
	}
	c->number = i;

	c->keep_score += c -> score;
	c->keep_kills += c -> kills;
	c->keep_deaths += c -> deaths;

	c->score = 0;
	c->kills = 0;
	c->deaths = 0;
    }

    init_terms();

    /* Give everyone $ 2 * max_cost - cost of their vehicle */
    for (i = 0; i < num_combatants; i++)
    {
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
int play_game()
{
    unsigned int status;
    int i;
    Boolean newgame;

    if (num_combatants == 0)
	return GAME_FAILED;

    /* In an ultimate game, only reset at the beginning, since we want don't
       want scores to reset after every goal. */
    if (settings.si.game == ULTIMATE_GAME)
    {
	init_combatants();
	newgame = TRUE;
    }

    do {
	if (settings.si.game != ULTIMATE_GAME)
	{
	    init_combatants();
	    newgame = TRUE;
	}
	if (setup_game(newgame) == GAME_FAILED)
	    return GAME_FAILED;
	newgame = FALSE;

#ifdef X11
	button_up(ANIM_WIN, FALSE);
	follow_mouse(ANIM_WIN, FALSE);
#endif

	game_running = True;

	do {
	    status = animate();
	} while (status == GAME_RUNNING);

	game_running = False;

	status = display_game_stats(status);

	game_cleanup();		/* nuking all the vehicles here is overkill */
    } while (status == GAME_RESET);

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
    extern void disc_init_history();
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

setup_combatant(c)
Combatant *c;
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
    }

    return activate_vehicle(v);
}


/* sets up the given terminal */

setup_terminal(num, v)
    int num;
    Vehicle *v;			/* vehicle it'll control (or NULL) */
{
    int i;

    set_terminal(num);
    term->vehicle = v;
    for (i = 0; i < MAX_WINDOWS; i++)
	expose_win(i, TRUE);
    term->num_lines = 0;
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
    int tries, num, random;

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
    random = rnd(num_starts);
    for (;;)
    {
	/* Take the next start location, or a random one if there are none */
	if (num < num_starts)
	{
	    grid_x = start[(num + random) % num_starts].x;
	    grid_y = start[(num + random) % num_starts].y;
	    num++;
	}
	else
	{
	    grid_x = rnd(GRID_WIDTH);
	    grid_y = rnd(GRID_HEIGHT);
	}

	b = &real_map[grid_x][grid_y];

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

game_cleanup()
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

int all_terms(veh, func)
Vehicle *veh;
void (*func)();
{
    int ctri;
    extern Terminal *terminal[];
    extern int num_terminals;

    for (ctri = 0; ctri < num_terminals; ctri++)
    {
	if (terminal[ctri]->vehicle == veh)
	{
	    func(terminal[ctri]);
	}
    }

    return (0);
}
