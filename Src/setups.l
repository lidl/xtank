/*-
 * Copyright (c) 1993 Josh Osborne
 * Copyright (c) 1993 Kurt Lidl
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

%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xtank.h"
#include "vehicle.h"
#include "setup.h"
#include "globals.h"
#include "team.h"
/* For "UNDEFINED" */
#include "interface.h"
#include "terminal.h"
#include "vstructs.h"
#include "proto.h"

#define BUFLEN 256
#define SEP ",\t"

extern char *games_entries[];
extern Settings settings;
extern int num_terminals;
extern Terminal *terminal[];

/* function prototypes */
char *yval(void);
char *strtoke(char *in, char *sep);
void cv_str(char *val, char **store);
void cv_kw2i(char *val, int *ival, char *arr[], int max);
void cv_bool(char *val, int *bool);
void cv_int(char *val, int *ival);
void cv_flt(char *val, FLOAT *fval);
void cv_maze(char *val, Mdesc **maze);
void cv_force(char *val, int state);

%}

B		:[ \t]+
s		{B}.+
b		{B}[01]
i		{B}[0-9]+
f		{B}[0-9]+("."[0-9]*)?

%%

\n							{ }
^xtank:[ \t]+114336$		{ }
^Maze{s}/\n					{ cv_maze(yval(), &settings.mdesc); }
^Game{s}/\n					{ cv_kw2i(yval(), &settings.si.game,games_entries, MAX_GAMES);}
^Point\ Bullets{b}/\n		{ cv_bool(yval(), &settings.point_bullets); }
^Ricochet{b}/\n				{ cv_bool(yval(), &settings.si.ricochet); }
^Rel\ Fire{b}/\n			{ cv_bool(yval(), &settings.si.rel_shoot); }
^No\ Wear{b}/\n				{ cv_bool(yval(), &settings.si.no_wear); }
^Restart{b}/\n				{ cv_bool(yval(), &settings.si.restart); }
^Commentator{b}/\n			{ cv_bool(yval(), &settings.commentator); }
^Full\ Map{b}/\n			{ cv_bool(yval(), &settings.si.full_map); }
^Pay\ To\ Play{b}/\n		{ cv_bool(yval(), &settings.si.pay_to_play); }
^Robots\ Don't\ Win{b}/\n	{ cv_bool(yval(), &settings.robots_dont_win); }
^Max\ Armor\ Scale{b}/\n	{ cv_bool(yval(), &settings.max_armor_scale); }
^Nametags{b}/\n				{ cv_bool(yval(), &settings.si.no_nametags); }
^Team\ Score{b}/\n			{ cv_bool(yval(), &settings.si.team_score); }
^Player\ Teleport{b}/\n		{ cv_bool(yval(), &settings.si.player_teleport); }
^Disc\ Teleport{b}/\n		{ cv_bool(yval(), &settings.si.disc_teleport); }
^Teleport\ From\ Team{b}/\n	{ cv_bool(yval(), &settings.si.teleport_from_team); }
^Teleport\ From\ Neutral{b}/\n	{ cv_bool(yval(), &settings.si.teleport_from_neutral); }
^Teleport\ To\ Team{b}/\n	{ cv_bool(yval(), &settings.si.teleport_to_team); }
^Teleport\ To\ Neutral{b}/\n	{ cv_bool(yval(), &settings.si.teleport_to_neutral); }
^Teleport\ Any\ To\ Any{b}/\n	{ cv_bool(yval(), &settings.si.teleport_any_to_any); }
^War\ Goals\ Only{b}/\n		{ cv_bool(yval(), &settings.si.war_goals_only); }
^Relative\ Disc{b}/\n		{ cv_bool(yval(), &settings.si.relative_disc); }
^Ultimate\ Own\ Goal{b}/\n	{ cv_bool(yval(), &settings.si.ultimate_own_goal); }
^"Winning Score"{i}/\n		{ cv_int( yval(), &settings.si.winning_score); }
^"Outpost Strength"{i}/\n	{ cv_int( yval(), &settings.si.outpost_strength); }
^"Scroll Speed"{f}/\n		{ cv_flt( yval(), &settings.si.scroll_speed); }
^"Box slowdown"{f}/\n		{ cv_flt( yval(), &settings.si.box_slowdown); }
^"Disc Friction"{f}/\n		{ cv_flt( yval(), &settings.si.disc_friction); }
^"Disc Speed"{f}/\n			{ cv_flt( yval(), &settings.si.disc_speed); }
^"Disc Damage"{f}/\n		{ cv_flt( yval(), &settings.si.disc_damage); }
^"Disc Heat"{f}/\n			{ cv_flt( yval(), &settings.si.disc_heat); }
^"Owner Slowdown"{f}/\n		{ cv_flt( yval(), &settings.si.owner_slowdown); }
^"Slip Friction"{f}/\n		{ cv_flt( yval(), &settings.si.slip_friction); }
^"Normal Friction"{f}/\n	{ cv_flt( yval(), &settings.si.normal_friction); }
^"Shocker Walls"{i}/\n		{ cv_int( yval(), &settings.si.shocker_walls); }
^"Difficulty"{i}/\n			{ cv_int( yval(), &settings.difficulty); }
^"Force On"{s}/\n			{ cv_force(yval(), INT_FORCE_ON); }
^"Force Off"{s}/\n			{ cv_force(yval(), INT_FORCE_OFF); }

^"Program"{s}/\n			{
				/* we should load $2 if prog $1 doesn't exist */
				char *progname;
				char *progpath;
				int junk;
				
				strtoke(yytext, SEP);		/* "Program:	" */
				progname = strtoke(NULL, SEP);
				progpath = strtoke(NULL, SEP);

				if (DESC_LOADED != find_pdesc(progname, &junk)) {
					load_prog_desc(progpath, TRUE);
				}
}

^Tanks{s}$					{ /* Who cares... */ }

V#[0-9]+{s}$				{
			/* V#N: owner->name, disp, vptr->name, team, #progs, progs... */
				char buf[BUFLEN];
				char *cvn;
				char *cplayer;
				char *cdisp;
				char *cvname;
				char *cteam;
				char *cnprogs;
				char *cprog1;
				extern Combatant combatant[];
				extern char *teams_entries[];
				int vn, nprogs;
				int num;
				int i;

				cvn		=	2+strtoke(yytext, SEP);
				cplayer	=	strtoke(NULL, SEP);
				cdisp	=	strtoke(NULL, SEP);
				cvname	=	strtoke(NULL, SEP);
				cteam	=	strtoke(NULL, SEP);
				cnprogs	=	strtoke(NULL, SEP);
				cprog1	=	strtoke(NULL, SEP);
				
				/* printf("%s, %s, %s, %s, %s, %s, %s\n",
					cvn, cplayer, cdisp, cvname, cteam, cnprogs, cprog1); */
				
				vn = atoi(cvn);

				/* See get_player_info */

				combatant[vn].num_players = 1;
				strcpy(combatant[vn].name, cplayer);
				combatant[vn].player[0] = UNDEFINED;
				for(i = 0; i < num_terminals; i++) {
					if (strcmp(terminal[i]->player_name, cplayer)) continue;
					combatant[vn].player[0] = i;
				}

				if (DESC_LOADED == make_vdesc(cvname, &num)) {
					combatant[vn].vdesc = num;
				}
				else
				{
					fprintf(stderr, "Warning!  Cannot load vehicle %s\n",
							cvname);
				}

				nprogs = atoi(cnprogs);
				combatant[vn].program[0] = UNDEFINED;
				combatant[vn].num_programs = 0;
				if (nprogs && DESC_LOADED == find_pdesc(cprog1, &num)) {
					combatant[vn].num_programs = atoi(cnprogs);
					combatant[vn].program[0] = num;
				}

				/* printf("(%d: %s/%d)\n", combatant[vn].num_programs,
					cprog1, combatant[vn].program[0]); */

				cv_kw2i(cteam, &num, teams_entries, MAX_TEAMS);
				combatant[vn].team = num;

				combatant_to_grid(combatant + vn, atoi(cvn));
			}

.*			{
				fprintf(stderr, "Bad setups file (%s)\n", yytext);
				exit(2);
			}

%%

char *
yval(void)
{
	char *cp;

	/* printf(":: %s\n", yytext); */

	for(cp = yytext; *cp != ':'; cp++) ;
	cp++;
	while(*cp == ' ' || *cp == '\t') cp++;

	return cp;
}

void
cv_maze(char *val, Mdesc **maze)
{
	int iCtr;
	extern int num_mdescs;
	extern Mdesc *mdesc;

	/* set to the random maze for now */
	*maze = (Mdesc *) NULL;

	/* need to search the mdesc[] to find the correct maze */
	for (iCtr = 0; iCtr < num_mdescs; iCtr++)
	{
		if (mdesc[iCtr].name)
		{
			if (! strcmp(mdesc[iCtr].name, val))
			{
				*maze = & mdesc[iCtr];
				break;
			}
		}
	}

	if (*maze == (Mdesc *) NULL)
	{
		fprintf(stderr, "Yo butthead!  Using the RANDOM maze.\n");
	}
}

void
cv_str(char *val, char **store)
{
	*store = strdup(val);
}

void
cv_kw2i(char *val, int *ival, char *arr[], int max)
{
	for(*ival = 0; *ival < max; *ival += 1) {
		if (!strcmp(arr[*ival], val)) return;
	}

	*ival = 0;
	return;
}

void
cv_force(char *val, int state)
{
	extern char force_states[];
	int i;

	for(i = 0; i < MAX_SPECIALS; i++) {
		if (!strcmp(val, special_stat[i].type)) {
			force_states[i] = state;
			return;
		}
	}

	fprintf(stderr, "Couldn't force %s to %d\n", val, state);
}

void
cv_bool(char *val, int *bool)
{
	*bool = (*val == '1');

	if (*val == '1' || *val == '0' && '\0' == *(val+1)) return;

	fprintf(stderr, "Bizare bool ('%c%c", *val, *(val+1));
	fprintf(stderr, "')!\n");
}

void
cv_flt(char *val, FLOAT *fval)
{
	*fval = atof(val);
}

void
cv_int(char *val, int *ival)
{
	*ival = atoi(val);
}

int
load_settings(char *pcFileName)
{
	extern char force_states[];
	int i;
	static int bCalled = FALSE;
	FILE *fp;

	if (! *pcFileName)
	{
		return (0);
	}

	fp = fopen(pcFileName, "r");
	if (!fp) {
		fprintf(stderr, "Can't find setups file\n");
		return (0);
	}

	for(i = 0; i < MAX_SPECIALS; i++) {
		force_states[i] = INT_FORCE_DONT;
	}

	if (bCalled)
	{
		yyrestart(fp);
		yylex();
	}
	else
	{
		yyin = fp;
		yylex();

		bCalled = TRUE;
	}

	fclose(fp);

	return(1);
}

/* Like ANSI strtok, but won't return an empty string */
char *
strtoke(char *in, char *sep)
{
	char *res;

	do {
		res = strtok(in, sep);
		in = NULL;
	} while(res && !*res);

	return res;
}
