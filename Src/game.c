#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** game.c
*/

#include <ctype.h>
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "vehicle.h"
#include "cosell.h"


extern Maze maze;
extern Vehicle *vehicle[];
extern int num_vehicles;
extern char *game_str[];
extern char *teams_entries[];
extern int team_color[];
extern int num_terminals;
extern Settings settings;
extern Map box;
extern int frame;


static Vehicle *winning_vehicle;
static int winning_team;
int num_teams;

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

	for (i = 0; i < num_vehicles; i++)
	{
		v = vehicle[i];

		if (settings.robots_dont_win &&
			v->owner->flesh == FALSE &&
			v->owner->num_programs != 0)
		{
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
                b = &box[x][y];
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

        for (vnum = num_vehicles - 1; vnum >= 0; --vnum) {
	    Vehicle *v = vehicle[vnum];
	    int old_team, k;

	    /* skip neutral vehicles */
            if (v->team == NEUTRAL)
				continue;

			x = v->loc->grid_x;
			y = v->loc->grid_y;
			b = &box[x][y];

	    /* skip vehicles that already own this box */
	    if (b->team == v->team)
				continue;

            /* Decrement time and check for takeover */
            if (--time[x][y][v->team] != (Byte) -1) 
		continue;	/* no takeover yet */

				old_team = b->team;
	    if (! change_box(b, x, y)) {
		/* well, I guess we can't change it after all */
		time[x][y][v->team] = 1;	/* try again next time */
		break;		/* skip all the rest of the vehicles, since
				   they won't be able to change anything either
				   */
	    }
	    b->team = v->team;

	    /* Fix team totals */
	    --total[old_team];
	    if (++total[v->team] >= winning_total) {	/* win? */
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

#if 1							/* THIS IS DEBUGING SHIT, REMOVE IT SOON */
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
	for (i = 0; i < num_teams; i++)
        ptr[i] = settings.si.takeover_time;

	/* Decrease times for this box depending on neighboring teams */
	for (i = 0; i < 8; i++)
	{
	Box *b = &box[x + neighbor_x[i]][y + neighbor_y[i]];

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
		make_bullet((Vehicle *) NULL, vehicle[rnd(num_vehicles)]->loc, DISC, 0.0);
	}
	else
	{
		/* When a vehicle is in an enemy goal, and has the disc, he wins */
		for (i = 0; i < num_vehicles; i++)
		{
			v = vehicle[i];
			if (v->num_discs > 0)
			{
				b = &box[v->loc->grid_x][v->loc->grid_y];
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
			for (j = 0; j < num_vehicles; j++)
				if (vehicle[j]->team == i)
				{
					make_bullet((Vehicle *) NULL, vehicle[j]->loc, DISC, 0.0);
					total_discs++;
					break;
				}
	}
	else
	{
		/* When a vehicle is in its own goal, and has all the discs, he wins */
		for (i = 0; i < num_vehicles; i++)
		{
			v = vehicle[i];
			if (v->num_discs == total_discs)
			{
				b = &box[v->loc->grid_x][v->loc->grid_y];
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
		for (i = 0; i < num_vehicles; i++)
		{
			v = vehicle[i];
			b = &box[v->loc->grid_x][v->loc->grid_y];
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
	extern int end_boundry;

	sprintf(fmt, "     %%%ds%%c: %%7d points   %%3d kills   %%3d deaths",
            MAX_STRING);        

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
		for (i = 0, k = 0; i < num_teams; i++, k = 0)
		{
			for (j = 0, m = l + 1; j < end_boundry; j++)
				if (vehicle[j]->team == i)
				{
					sprintf(s, fmt,
							vehicle[j]->owner->name,
							(settings.si.pay_to_play && vehicle[j]->is_dead
							 ? '*' : ' '),
							vehicle[j]->owner->score,
                            vehicle[j]->owner->kills,   
                            vehicle[j]->owner->deaths); 
					mprint(s, 5, m++);
					k += vehicle[j]->owner->score;
                    if (n == num_terminals - 1 &&       
							settings.si.game != ULTIMATE_GAME)
					{
                        vehicle[j]->owner->score = 0;   
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
