/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ctype.h>
#include "xtank.h"
#include "bullet.h"
#include "graphics.h"
#include "gr.h"
#include "vehicle.h"
#include "cosell.h"
#include "globals.h"
#include "clfkr.h"
#include "message.h"
#include "terminal.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif /* SOUND */

extern Maze maze;
extern char *games_entries[];
extern char *teams_entries[];
extern int team_color[];
extern int num_terminals;
extern Settings settings;
extern Map real_map;
extern int frame;
extern CLFkr command_options;


TeamData teamdata[MAX_TEAMS];
Team last_team;					/* the highest-numbered team that's playing
				   (NB: they are not necessarily the
				   low-numbered teams!)  */
int num_teams;					/* the number of teams with any vehicles left
				   in play (NB: they are not necessarily the
				   low-numbered teams!) */
Team winning_team;				/* team that has just won */
static Vehicle *winning_vehicle;

/* the delta x and y for each of the 8 neighbors of a box */
static int neighbor_x[] =
{0, 1, 1, 1, 0, -1, -1, -1};
static int neighbor_y[] =
{-1, -1, 0, 1, 1, 1, 0, -1};


/*
** Applies the rules of the game during play.
** Returns one of GAME_RUNNING, GAME_RESET, GAME_OVER.
*/
int
game_rules(Boolean init)
{
	switch (settings.si.game) {
	  case MADMAN_GAME:
		  return madman_rules(init);
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
	  default:
		  return GAME_RUNNING;
	}

}

/*
** Vehicles fight each other for score.
** A team wins when a vehicle on that team gets the required score.
*/
int
combat_rules(Boolean init)
{
	Vehicle *v;
	int i, j;
	int TeamScores[MAX_TEAMS];
	int HighestPlayer[MAX_TEAMS];
	int retcode = GAME_RUNNING;

	if (!init) {
		if (settings.si.team_score) {
			for (i = 0; i < MAX_TEAMS; i++) {
				TeamScores[i] = 0;
				HighestPlayer[i] = -1;
			}

			for (i = 0; i < num_veh; i++) {
				v = &actual_vehicles[i];

				if (settings.robots_dont_win && v->owner->num_players == 0) {
					continue;
				}
				TeamScores[v->team] += v->owner->score;

				if (HighestPlayer[v->team] == -1) {
					HighestPlayer[v->team] = i;
				} else {
					if (v->owner->score >
					 actual_vehicles[HighestPlayer[v->team]].owner->score) {
						HighestPlayer[v->team] = i;
					}
				}
			}

			for (i = 0; i < MAX_TEAMS; i++) {
				/* skip neutral vehicles */
				if (i != NEUTRAL && TeamScores[i] >= settings.si.winning_score) {
					winning_vehicle = &actual_vehicles[HighestPlayer[i]];
					winning_team = i;
					retcode = GAME_RESET;
				}
			}
		} else {
			for (i = 0; i < num_veh_alive; i++) {
				v = live_vehicles[i];

				if (settings.robots_dont_win && v->owner->num_players == 0) {
					continue;
				}
				if (v->owner->score >= settings.si.winning_score) {
					winning_vehicle = v;
					winning_team = v->team;
					retcode = GAME_RESET;
					break;
				}
			}
		}
	}
	return (retcode);
}

int
madman_rules(Boolean init)
{
	static int starter;
	Coord *start;
	int num_starts, random, i;
	Loc loc;
	int ret = combat_rules(init);

	if (init) {
		start = maze.start[NEUTRAL];
		num_starts = maze.num_starts[NEUTRAL];

/* Increase the number of disks (HAK and MEL, Jan 93) */
/* Made it a setting (HAK 3/93) */
		for(i=settings.si.num_discs; i>0; i--)
			if (num_starts != 0) {
				random = rnd(num_starts);
				/* Start up 1 disc in a random neutral start point */
				loc.grid_x = start[random].x;
				loc.grid_y = start[random].y;

				loc.box_x = BOX_WIDTH / 2;
				loc.box_y = BOX_HEIGHT / 2;

				loc.x = loc.grid_x * BOX_WIDTH + loc.box_x;
				loc.y = loc.grid_y * BOX_HEIGHT + loc.box_y;

				make_bullet((Vehicle *) NULL, &loc, DISC, 0.0, NULL);
			} else {
				/* Start up 1 disc on the starter's vehicle */
				make_bullet((Vehicle *) NULL, live_vehicles[starter++
				  % num_veh_alive]->loc, DISC, 0.0, NULL);
			}
	}
	return ret;
}

/*
** Teams take over boxes by remaining in them for a period of time.
** The time required decreases as the # neighbors on your team increases.
** The more vehicles in a box, the faster the takeover.
** A team wins the game if they control the required percentage of boxes.
*/
int
war_rules(Boolean init)
{
	static Byte time[GRID_WIDTH][GRID_HEIGHT][MAX_TEAMS];
	static int total[MAX_TEAMS];
	static int winning_total;
	static int percent;
	static int num_boxes;
	Box *b;
	int x, y;
	int temp_team;

	if (init) {
		/* Clear total # boxes in play and # owned by each team */
		for (temp_team = num_teams - 1; temp_team; --temp_team)
			total[temp_team] = 0;
		num_boxes = 0;

		for (x = GRID_WIDTH - 1; x >= 0; --x)
			for (y = GRID_HEIGHT - 1; y >= 0; --y) {
				b = &real_map[x][y];
				if (!(b->flags & INSIDE_MAZE))
					continue;
				if ((settings.si.war_goals_only) &&
					(b->type != GOAL))
					continue;

				/* Count # boxes in play and # owned by each team */
				num_boxes++;
				total[b->team]++;

				/* Initialize time based on neighboring teams */
				war_init_time(time, x, y);
			}

		/* Compute winning total (fixed 3/17/90 JMO & GHS) */
		percent = settings.si.winning_score % 100;
		if (percent == 0)
			percent = 75;
		if (settings.si.winning_score > 100) {
			printf("Warning, winning_score is being used as a percentage (%d%%)\n",
				   percent);
		}
		winning_total = (percent * num_boxes) / 100;
	} else {
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

			/* skip non-goal boxes */
			if ((settings.si.war_goals_only) &&
				(b->type != GOAL))
				continue;

			/* skip vehicles that already own this box */
			if (b->team == v->team)
				continue;

			/* Decrement time and check for takeover */
			if (--time[x][y][v->team] > (Byte) - 1)
				continue;		/* no takeover yet */

			old_team = b->team;
			if (!change_box(b, x, y)) {
				/* well, I guess we can't change it after all */
				time[x][y][v->team] = 1;	/* try again next time */
				break;			/* skip all the rest of the vehicles, since
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
				sprintf((char *) m.data, "%s has %d%%",
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
void
war_init_time(Byte time[GRID_WIDTH][GRID_HEIGHT][MAX_TEAMS], int x, int y)
{
	Byte *ptr = time[x][y];		/* this box's array of team counts */
	int i;

	/* Initialize times for this box to beginning value for 0 neighbors */
	for (i = 0; i < (int) last_team; i++) {
		ptr[i] = settings.si.takeover_time;
	}

	/* Decrease times for this box depending on neighboring teams */
	for (i = 0; i < 8; i++) {
		Box *b = &real_map[x + neighbor_x[i]][y + neighbor_y[i]];

		if (!(b->flags & INSIDE_MAZE))
			continue;

		/* neighboring squares of the same team reduce time by a factor of 1.5
	 */
		ptr[b->team] = (((int) ptr[b->team]) << 8) / 384;
	}
}


/*
** One disc in game, disc owned in own (or opponents') goal wins.
*/
int
ultimate_rules(Boolean init)
{
	Vehicle *v;
	Box *b;
	int i;
	static int starter;
	Coord *start;
	int num_starts, random;
	Loc loc;

	if (init) {
		start = maze.start[NEUTRAL];
		num_starts = maze.num_starts[NEUTRAL];

		if (num_starts != 0) {
			random = rnd(num_starts);
			/* Start up 1 disc in a random neutral start point */
			loc.grid_x = start[random].x;
			loc.grid_y = start[random].y;

			loc.box_x = BOX_WIDTH / 2;
			loc.box_y = BOX_HEIGHT / 2;

			loc.x = loc.grid_x * BOX_WIDTH + loc.box_x;
			loc.y = loc.grid_y * BOX_HEIGHT + loc.box_y;

			make_bullet((Vehicle *) NULL, &loc, DISC, 0.0, NULL);
		} else
			/* Start up 1 disc on the starter's vehicle */
			make_bullet((Vehicle *) NULL, live_vehicles[starter++ % num_veh_alive]->loc,
						DISC, 0.0, NULL);
	} else {
		/* When a vehicle is in an enemy goal, and has the disc, he wins */
		for (i = 0; i < num_veh_alive; i++) {
			v = live_vehicles[i];
			if (v->num_discs > 0) {
				b = &real_map[v->loc->grid_x][v->loc->grid_y];
				if (b->type == GOAL &&
				  ((b->team == v->team) == settings.si.ultimate_own_goal)) {
#ifdef SOUND
					play_all(GOAL_SOUND);
#endif /* SOUND */
					winning_vehicle = v;
					winning_team = v->team;
					v->owner->score++;
					if (settings.commentator)
						comment(COS_GOAL_SCORED, 0, v, (Vehicle *) NULL,
								(Bullet *) NULL);
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
int
capture_rules(Boolean init)
{
	static int total_discs;
	Vehicle *v;
	Box *b;
	int i, j;

	if (init) {
		/* Start up 1 disc in the first starting box for each team */
		total_discs = 0;
		for (i = 1; i <= num_teams; i++)
			for (j = 0; j < num_veh_alive; j++)
				if (live_vehicles[j]->team == i) {
					make_bullet((Vehicle *) NULL, live_vehicles[j]->loc, DISC,
								0.0, NULL);
					total_discs++;
					break;
				}
	} else {
		/* When a vehicle is in its own goal, and has all the discs, he wins */
		for (i = 0; i < num_veh_alive; i++) {
			v = live_vehicles[i];
			if (v->num_discs == total_discs) {
				b = &real_map[v->loc->grid_x][v->loc->grid_y];
				if (b->type == GOAL && b->team == v->team) {
#ifdef SOUND
					play_all(GOAL_SOUND);
#endif /* SOUND */
					winning_vehicle = v;
					winning_team = v->team;
					v->owner->score++;
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
int
race_rules(Boolean init)
{
	Vehicle *v;
	Box *b;
	int i;

	if (!init) {
		/* When a vehicle is a goal, he wins */
		for (i = 0; i < num_veh_alive; i++) {
			v = live_vehicles[i];
			b = &real_map[v->loc->grid_x][v->loc->grid_y];
			if (b->type == GOAL) {
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
int
display_game_stats(int status)
{
	int n, reply;

	for (n = 0; n < num_terminals; n++) {
		set_terminal(n);

		display_game_stats_to_current(status, n);

		flush_output();
	}

	set_terminal(0);
	if (settings.si.game == ULTIMATE_GAME ||
		settings.si.game == CAPTURE_GAME)
		mprint("'q' to end game, 's' to start", 20, 40);
	else
		mprint("'q' to end game", 20, 40);

	if (command_options.AutoExit) {
		reply = 'Q';
	} else {
		int iNumEvents;
		Event aeEvents[1];

		do {
			iNumEvents = 1;

			get_events(&iNumEvents, aeEvents);
			if (iNumEvents) {
				if (aeEvents[0].type == EVENT_KEY &&
					(aeEvents[0].key == 'Q' || aeEvents[0].key == 'q')) {
					reply = 'Q';
					break;
				}
				if (aeEvents[0].type == EVENT_KEY &&
					(settings.si.game == CAPTURE_GAME ||
					 settings.si.game == ULTIMATE_GAME) &&
					(aeEvents[0].key == 'S' || aeEvents[0].key == 's')) {
					reply = 'S';
					break;
				}
				if (aeEvents[0].type == EVENT_KEY &&
					(settings.si.game == CAPTURE_GAME ||
					 settings.si.game == ULTIMATE_GAME) &&
					(aeEvents[0].key == 'S' || aeEvents[0].key == 's')) {
					reply = 'S';
					break;
				}
			}
			if (win_exposed(ANIM_WIN)) {
				display_game_stats_to_current(status, 0);
				/* mprint("'q' to end game, 's' to start", 20, 40); */
				mprint("'q' to end game", 20, 40);
				flush_output();
				expose_win(ANIM_WIN, FALSE);
			}
		}
		while (TRUE);
	}

	if (command_options.PrintScores) {
		display_game_stats_to_current(status, -1);
	}
	if (settings.si.game != ULTIMATE_GAME) {
		int j;

		for (j = 0; j < num_veh; j++) {
			Vehicle *v = &actual_vehicles[j];

			v->owner->score = 0;
		}
	}
	return ((reply == 'Q') ? GAME_QUIT : GAME_RESET);
}


void
ScreenOut(char *str, int x, int y)
{
	mprint(str, x, y);
}

void
ScreenOutColor(char *str, int x, int y, int color)
{
	mprint_color(str, x, y, color);
}

void
StandardOut(char *str, int x, int y)
{
	puts(str);
}

void
StandardOutColor(char *str, int x, int y, int color)
{
	puts(str);
}

int
display_game_stats_to_current(int status, int n)
{
	int i, j, k, l, m;
	char s[80];
	char fmt[80];
	void (*plain_out) ();
	void (*color_out) ();

	if (n >= 0) {
		plain_out = ScreenOut;
		color_out = ScreenOutColor;
	} else {
		plain_out = StandardOut;
		color_out = StandardOutColor;
	}

	if (n < 0) {
		(*plain_out) ("", 0, 0);
		(*plain_out) ("", 0, 0);
	}
	sprintf(fmt, "%%11s%%c: %%-11s  %%7d points   %%3d kills   %%3d deaths");

	clear_window(ANIM_WIN);

	sprintf(s, "Game: %s     Frame: %d",
			games_entries[(int) settings.si.game],
			frame);
	(*plain_out) (s, 5, 2);

	/* If we just reset the game, say who scored what */
	if (status == GAME_RESET) {
		switch (settings.si.game) {
		  case CAPTURE_GAME:
			  sprintf(s, "All discs collected by the %s team",
					  teams_entries[winning_team]);
			  break;
		  case ULTIMATE_GAME:
			  sprintf(s, "A goal for the %s team scored by %s",
					  teams_entries[winning_team], winning_vehicle->disp);
			  break;
		  case MADMAN_GAME:
			  sprintf(s, "Insanity squashed by %s", winning_vehicle->disp);
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
		  default:
			  (*plain_out) (s, 5, 4);
			  break;
		}
	}
	/* Print out the total scores for all the teams */
	(*plain_out) ("Current score:", 5, 6);
	l = 8;
	for (i = 0, k = 0; i < MAX_TEAMS; i++, k = 0) {
		for (j = 0, m = l + 1; j < num_veh; j++) {
			Vehicle *v = &actual_vehicles[j];

			if (v->team == i) {
				sprintf(s, fmt, v->owner->name,
						((settings.si.pay_to_play &&
						  !tstflag(v->status, VS_is_alive))
						 ? '*' : ' '),
						v->name, v->owner->score, v->owner->kills,
						v->owner->deaths);
				(*plain_out) (s, 5, m++);
				k += v->owner->score;
			}
		}

		if (m != l + 1) {
			sprintf(s, "  %s: %d points", teams_entries[i], k);
			(*color_out) (s, 5, l, team_color[i]);
			l = m + 1;
		}
		if (n < 0) {
			(*plain_out) ("", 0, 0);
		}
	}

	if (n < 0) {
		(*plain_out) ("", 0, 0);
	}
	return (0);
}
