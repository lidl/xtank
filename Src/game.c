/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** game.c
*/

/*
$Author: lidl $
$Id: game.c,v 2.6 1991/09/15 23:35:15 lidl Exp $

$Log: game.c,v $
 * Revision 2.6  1991/09/15  23:35:15  lidl
 * helpsx if we pass the correct number of arguments to sprintf
 * we didn't before :-(
 *
 * Revision 2.5  1991/08/22  03:23:21  aahz
 * cast to avoid warning on i860.
 *
 * Revision 2.4  1991/03/25  00:25:59  stripes
 * RS6K patch, broke a sprintf in display_game_stats.  Will fix after a
 * trip to bathroom.
 *
 * Revision 2.3  1991/02/10  13:50:32  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:47  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:25  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:27  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:22  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include <ctype.h>
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "vehicle.h"
#include "cosell.h"
#include "globals.h"


extern Maze maze;
extern char *game_str[];
extern char *teams_entries[];
extern int team_color[];
extern int num_terminals;
extern Settings settings;
extern Map real_map;
extern int frame;


TeamData teamdata[MAX_TEAMS];
Team last_team;			/* the highest-numbered team that's playing
				   (NB: they are not necessarily the
				   low-numbered teams!)  */
int num_teams;			/* the number of teams with any vehicles left
				   in play (NB: they are not necessarily the
				   low-numbered teams!) */
Team winning_team;		/* team that has just won */
static Vehicle *winning_vehicle;

/* the delta x and y for each of the 8 neighbors of a box */
static int neighbor_x[] = { 0,  1, 1, 1, 0, -1, -1, -1};
static int neighbor_y[] = {-1, -1, 0, 1, 1,  1,  0, -1};


/*
** Applies the rules of the game during play.
** Returns one of GAME_RUNNING, GAME_RESET, GAME_OVER.
*/
game_rules(init)
Boolean init;
{
	switch (settings.si.game)
	{
		case COMBAT_GAME:
			return combat_rules(init);
		case WAR_GAME:
			return war_rules(init);
		case ULTIMATE_GAME:
			return ultimate_rules(init);
		case CAPTURE_GAME:
			return capture_rules(init);
		case RACE_GAME:
			return race_rules(init);
	}

	return GAME_RUNNING;
}

/*
** Vehicles fight each other for score.
** A team wins when a vehicle on that team gets the required score.
*/
combat_rules(init)
Boolean init;
{
    Vehicle *v;
    int i;

    if (init)
	return GAME_RUNNING;

    for (i = 0; i < num_veh_alive; i++) {
	v = live_vehicles[i];

	if (settings.robots_dont_win && v->owner->num_players == 0) {
	    continue;
	}
	if (v->owner->score >= settings.si.winning_score)
	{
	    winning_vehicle = v;
	    winning_team = v->team;
	    return GAME_RESET;
	}
    }
    return GAME_RUNNING;
}

/*
** Teams take over boxes by remaining in them for a period of time.
** The time required decreases as the # neighbors on your team increases.
** The more vehicles in a box, the faster the takeover.
** A team wins the game if they control the required percentage of boxes.
*/
war_rules(init)
Boolean init;
{
    static Byte time[GRID_WIDTH][GRID_HEIGHT][MAX_TEAMS];
    static int total[MAX_TEAMS];
    static int winning_total;
    static int percent;
    static int num_boxes;
    Box *b;
    int x, y;
    int temp_team;

    if (init)
    {
	/* Clear total # boxes in play and # owned by each team */
        for (temp_team = num_teams - 1; temp_team; --temp_team)
            total[temp_team] = 0;
        num_boxes = 0;

        for (x = GRID_WIDTH - 1; x >= 0; --x)
	    for (y = GRID_HEIGHT - 1; y >= 0; --y)
	    {
                b = &real_map[x][y];
		if (!(b->flags & INSIDE_MAZE))
		    continue;

                /* Count # boxes in play and # owned by each team */
                num_boxes++;
                total[b->team]++;

		/* Initialize time based on neighboring teams */
                war_init_time(time, x, y);
	    }

        /* Compute winning total (fixed 3/17/90 JMO & GHS) */
	percent = settings.si.winning_score % 100;
	if (percent == 0) percent = 75;
	if (settings.si.winning_score > 100)
	{
	    printf("Warning, winning_score is being used as a percentage (%d%%)\n",
		   percent);
	}
        winning_total = (percent * num_boxes) / 100;
    }
    else
    {
	int vnum;

        for (vnum = num_veh_alive - 1; vnum >= 0; --vnum) {
	    Vehicle *v = live_vehicles[vnum];
	    int old_team, k;

	    /* skip neutral vehicles */
            if (v->team == NEUTRAL)
		continue;

	    x = v->loc->grid_x;
	    y = v->loc->grid_y;
	    b = &real_map[x][y];

	    /* skip vehicles that already own this box */
	    if (b->team == v->team)
		continue;

            /* Decrement time and check for takeover */
            if (--time[x][y][v->team] != (Byte) -1) 
		continue;	/* no takeover yet */

	    old_team = b->team;
	    if (! change_box(b, x, y)) {
		/* well, I guess we can't change it after all */
		time[x][y][v->team] = 1; /* try again next time */
		break;		/* skip all the rest of the vehicles, since
				   they won't be able to change anything either
				   */
	    }
	    b->team = v->team;

	    /* Fix team totals */
	    --total[old_team];
	    if (++total[v->team] >= winning_total) { /* win? */
		winning_vehicle = (Vehicle *) NULL;
		winning_team = v->team;
		return GAME_RESET;
	    }

	    /* re-initialize team time counts for this box */
	    war_init_time(time, x, y);

	    /* update team time counts of surrounding boxes */
	    for (k = 0; k < 8; k++) {
		Byte *times = time[x + neighbor_x[k]][y + neighbor_y[k]];

		/* neighboring squares of old team get their times increased by
		   a factor of 1.5 */
		times[old_team] = (((int) times[old_team]) * 3) >> 1;
		/* neighboring squares of new team get their times reduced by a
		   factor of 1.5 */
		times[v->team] = (((int) times[v->team]) << 8) / 384;
	    }

#if 1				/* THIS IS DEBUGING SHIT, REMOVE IT SOON */
#include "message.h"
	    {
		Message m;

		m.sender = SENDER_COM;
		m.recipient = RECIPIENT_ALL;
		m.opcode = OP_TEXT;
		sprintf((char *)m.data, "%s has %d%%",
			teams_entries[v->team],
			total[v->team] * 100 / num_boxes);
		dispatch_message(&m);
	    }
#endif
	}
    }
    return GAME_RUNNING;
}


/*
** Initializes the time values for a square based on the neighboring teams.
*/

war_init_time(time, x, y)
Byte time[GRID_WIDTH][GRID_HEIGHT][MAX_TEAMS];
int x, y;
{
    Byte *ptr = time[x][y];	/* this box's array of team counts */
    int i;

    /* Initialize times for this box to beginning value for 0 neighbors */
    for (i = 0; i < (int)last_team; i++)
	{
		ptr[i] = settings.si.takeover_time;
	}

    /* Decrease times for this box depending on neighboring teams */
    for (i = 0; i < 8; i++)
    {
	Box *b = &real_map[x + neighbor_x[i]][y + neighbor_y[i]];

	if (!(b->flags & INSIDE_MAZE))
	    continue;

	/* neighboring squares of the same team reduce time by a factor of 1.5
	 */
	ptr[b->team] = (((int) ptr[b->team]) << 8) / 384;
    }
}


/*
** One disc in game, disc owned in enemy goal wins.
*/
ultimate_rules(init)
Boolean init;
{
    Vehicle *v;
    Box *b;
    int i;

    if (init)
    {
	/* Start up 1 disc on a random vehicle */
	make_bullet((Vehicle *) NULL, live_vehicles[rnd(num_veh_alive)]->loc,
		    DISC, 0.0);
    }
    else
    {
	/* When a vehicle is in an enemy goal, and has the disc, he wins */
	for (i = 0; i < num_veh_alive; i++)
	{
	    v = live_vehicles[i];
	    if (v->num_discs > 0)
	    {
		b = &real_map[v->loc->grid_x][v->loc->grid_y];
		if (b->type == GOAL && b->team != v->team)
		{
		    winning_vehicle = v;
		    winning_team = v->team;
		    v->owner->score++;
		    if (settings.commentator)
			comment(COS_GOAL_SCORED, 0, v, (Vehicle *) NULL);
		    return GAME_RESET;
		}
	    }
	}
    }
    return GAME_RUNNING;
}

/*
** One disc per non-neutral team, all discs in own goal wins.
*/
capture_rules(init)
Boolean init;
{
    static int total_discs;
    Vehicle *v;
    Box *b;
    int i, j;

    if (init)
    {
	/* Start up 1 disc in the first starting box for each team */
	total_discs = 0;
	for (i = 1; i < num_teams; i++)
	    for (j = 0; j < num_veh_alive; j++)
		if (live_vehicles[j]->team == i)
		{
		    make_bullet((Vehicle *) NULL, live_vehicles[j]->loc, DISC,
				0.0);
		    total_discs++;
		    break;
		}
    }
    else
    {
	/* When a vehicle is in its own goal, and has all the discs, he wins */
	for (i = 0; i < num_veh_alive; i++)
	{
	    v = live_vehicles[i];
	    if (v->num_discs == total_discs)
	    {
		b = &real_map[v->loc->grid_x][v->loc->grid_y];
		if (b->type == GOAL && b->team == v->team)
		{
		    winning_vehicle = v;
		    winning_team = v->team;
		    return GAME_RESET;
		}
	    }
	}
    }
    return GAME_RUNNING;
}

/*
** First one to a goal wins.
*/
race_rules(init)
Boolean init;
{
	Vehicle *v;
	Box *b;
	int i;

	if (!init)
	{
		/* When a vehicle is a goal, he wins */
		for (i = 0; i < num_veh_alive; i++)
		{
			v = live_vehicles[i];
			b = &real_map[v->loc->grid_x][v->loc->grid_y];
			if (b->type == GOAL)
			{
				winning_vehicle = v;
				winning_team = v->team;
				return GAME_RESET;
			}
		}
	}
	return GAME_RUNNING;
}

#define mprint(str,x,y) \
  (display_mesg2(ANIM_WIN,str,x,y,M_FONT))

#define mprint_color(str,x,y,color) \
  (display_mesg1(ANIM_WIN,str,x,y,M_FONT,color))

/*
** Displays the current state of the game, who scored, and the current score.
*/
display_game_stats(status)
unsigned int status;
{
    char s[80];
    char fmt[80];
    int i, j, k, l, m, n, reply;

    /* sprintf(fmt, "     %%%ds%%c: %%7d points   %%3d kills   %%3d deaths",
            MAX_STRING);        		*/
	sprintf(fmt, "%%11s%%c: %%-11s  %%7d points   %%3d kills   %%3d deaths");

    for (n = 0; n < num_terminals; n++)
    {
	set_terminal(n);

	clear_window(ANIM_WIN);

        sprintf(s, "Game: %s     Frame: %d",
		game_str[(int)settings.si.game],
		frame);
	mprint(s, 5, 2);

	/* If we just reset the game, say who scored what */
	if (status == GAME_RESET)
	{
	    switch (settings.si.game)
	    {
	      case CAPTURE_GAME:
		sprintf(s, "All discs collected by the %s team",
			teams_entries[winning_team]);
		break;
	      case ULTIMATE_GAME:
		sprintf(s, "A goal for the %s team scored by %s",
			teams_entries[winning_team], winning_vehicle->disp);
		break;
	      case COMBAT_GAME:
		sprintf(s, "Battle won by %s", winning_vehicle->disp);
		break;
	      case WAR_GAME:
		sprintf(s, "War won by the %s team",
			teams_entries[winning_team]);
		break;
	      case RACE_GAME:
		sprintf(s, "Race won by %s", winning_vehicle->disp);
		break;
	    }
	    mprint(s, 5, 4);
	}
	/* Print out the total scores for all the teams */
	mprint("Current score:", 5, 6);
	l = 8;
	for (i = 0, k = 0; i < MAX_TEAMS; i++, k = 0)
	{
	    for (j = 0, m = l + 1; j < num_veh; j++) {
		Vehicle *v = &actual_vehicles[j];

		if (v->team == i) {
		    sprintf(s, fmt,
			    v->owner->name,
			    ((settings.si.pay_to_play &&
			      !tstflag(v->status, VS_is_alive))
			     ? '*' : ' '),
			    v->name,v->owner->score, v->owner->kills,
			    v->owner->deaths); 
		    mprint(s, 5, m++);
		    k += v->owner->score;
                    if (n == num_terminals - 1 &&
			settings.si.game != ULTIMATE_GAME)
		    {
                        v->owner->score = 0;   
		    }
		}
	    }
	    if (m != l + 1)
	    {
		sprintf(s, "  %s: %d points", teams_entries[i], k);
		mprint_color(s, 5, l, team_color[i]);
		l = m + 1;
	    }
	}
	flush_output();
    }

    set_terminal(0);
    /* mprint("'q' to end game, 's' to start", 20, 40); */
    mprint("'q' to end game", 20, 40);
    do
    {
	reply = get_reply();
	reply = islower(reply) ? toupper(reply) : reply;
    }
    /* while (reply != 'Q' && reply != 'S'); */
    while (reply != 'Q');
    return ((reply == 'Q') ? GAME_QUIT : GAME_RESET);
}
