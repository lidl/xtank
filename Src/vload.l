/*-
 * Copyright (c) 1990 Gordon Smith
 * Copyright (c) 1992, 1993 Kurt Lidl
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

%option noyywrap

%{
#ifdef YY_DECL
#undef YY_DECL
#endif
#define YY_DECL int yylex(d) Vdesc *d;

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xtank.h"
#include "vdesc.h"
#include "vehicle.h"
#include "vstructs.h"
#include "setup.h"
#include "globals.h"
#include "team.h"
/* For "UNDEFINED" */
#include "interface.h"
#include "terminal.h"
#include "proto.h"

#define BUFLEN 256
#define SEP ",\t"

extern char *games_entries[];
extern Settings settings;
extern int num_terminals;
extern Terminal *terminal[];
extern char *Wnames[VMAX_WEAPONS];
extern char *Mnames[NUM_MOUNTS];
extern char *Snames[MAX_SIDES];

static int iGblRetCode;

char *zval(), *strtoke();
void cv_str(), cv_kw2i(), cv_bool(), cv_int(), cv_flt();
void cv_armr(), cv_body(), cv_bump(), cv_engine();
void cv_special(), cv_susp(), cv_tread(), cv_weap();
void cv_int_m();

%}

B		:[ \t]+
s		{B}.+
b		{B}[01]
i		{B}[0-9]+
f		{B}[0-9]+("."[0-9]*)?

%%

\n						{ }
^xtank:[ \t]+114336$	{ }
^Vehicle{s}/\n			{ strcpy(d->name, zval()); }
^Designer{s}/\n			{ strcpy(d->designer, zval()); }
^Body{s}/\n				{ cv_body(zval(), d); }
^Engine{s}/\n			{ cv_engine(zval(), d); }
^W#[0-5]{s}/\n			{ cv_weap(yytext[2], zval(), d); }
^Armor\ Type{s}/\n		{ cv_armr(zval(), d); }
^Front{s}/\n			{ cv_int_m(zval(), &d->armor.side[FRONT], &d->armor.max_side); }
^Back{s}/\n				{ cv_int_m(zval(), &d->armor.side[BACK], &d->armor.max_side); }
^Left{s}/\n				{ cv_int_m(zval(), &d->armor.side[LEFT], &d->armor.max_side); }
^Right{s}/\n			{ cv_int_m(zval(), &d->armor.side[RIGHT], &d->armor.max_side); }
^Top{s}/\n				{ cv_int_m(zval(), &d->armor.side[TOP], &d->armor.max_side); }
^Bottom{s}/\n			{ cv_int_m(zval(), &d->armor.side[BOTTOM], &d->armor.max_side); }
^Special{s}/\n			{ cv_special(zval(), d);}
^Heat\ sinks{s}/\n		{ cv_int(zval(), &d->heat_sinks);}
^Suspension{s}/\n		{ cv_susp(zval(), d); }
^Treads{s}/\n			{ cv_tread(zval(), d); }
^Bumpers{s}/\n			{ cv_bump(zval(), d);}

%%

void cv_int_m(vstr, addr, max_addr)
char *vstr;
int *addr;
int *max_addr;
{
	int v;

	v = atoi(vstr);

	if (v > *max_addr) {
		*max_addr = v;
	}

	*addr = v;
}

char *zval()
{
	char *cp;

	/* printf(":: '%s'\n", yytext); */

	for(cp = yytext; *cp != ':'; cp++) ;
	cp++;
	while(*cp == ' ' || *cp == '\t') cp++;

	return cp;
}

void cv_armr(val, d)
char *val;
Vdesc *d;
{
	int i;

	for (i = 0; i < sizeof(armor_stat)/sizeof(Armor_stat); i += 1) {
		if (!strcmp(armor_stat[i].type, val)) {
			d->armor.type = i;
			return;
		}
	}

	fprintf(stderr, "Invalid Armor Type : %s\n", val);
	iGblRetCode = DESC_BAD_FORMAT;
}

void cv_body(val, d)
char *val;
Vdesc *d;
{
	int i;

	for (i = 0; i < sizeof(body_stat)/sizeof(Body_stat); i += 1) {
		if (!strcmp(body_stat[i].type, val)) {
			d->body = i;
			return;
		}
	}

	fprintf(stderr, "Invalid Body Type : %s\n", val);
	iGblRetCode = DESC_BAD_FORMAT;
}

void cv_bump(val, d)
char *val;
Vdesc *d;
{
	int i;

	for (i = 0; i < sizeof(bumper_stat)/sizeof(Bumper_stat); i += 1) {
		if (!strcmp(bumper_stat[i].type, val)) {
			d->bumpers = i;
			return;
		}
	}

	fprintf(stderr, "Invalid Bumper Type : %s\n", val);
	iGblRetCode = DESC_BAD_FORMAT;
}

void cv_engine(val, d)
char *val;
Vdesc *d;
{
	int i;

	for (i = 0; i < sizeof(engine_stat)/sizeof(Engine_stat); i += 1) {
		if (!strcmp(engine_stat[i].type, val)) {
			d->engine = i;
			return;
		}
	}

	fprintf(stderr, "Invalid Engine Type : %s\n", val);
	iGblRetCode = DESC_BAD_FORMAT;
}

void cv_special(val, d)
char *val;
Vdesc *d;
{
	int i;

	for (i = 0; i < sizeof(special_stat)/sizeof(Special_stat); i += 1) {
		if (!strcmp(special_stat[i].type, val)) {
			d->specials |= 1 << i;
			return;
		}
	}
	fprintf(stderr, "Unknown Special Type : %s\n", val);
	iGblRetCode = DESC_BAD_FORMAT;
}

void cv_susp(val, d)
char *val;
Vdesc *d;
{
	int i;

	for (i = 0; i < sizeof(suspension_stat)/sizeof(Suspension_stat); i += 1) {
		if (!strcmp(suspension_stat[i].type, val)) {
			d->suspension = i;
			return;
		}
	}
	fprintf(stderr, "Invalid Suspension Type : %s\n", val);
	iGblRetCode = DESC_BAD_FORMAT;
}

void cv_tread(val, d)
char *val;
Vdesc *d;
{
	int i;

	for (i = 0; i < sizeof(tread_stat)/sizeof(Tread_stat); i += 1) {
		if (!strcmp(tread_stat[i].type, val)) {
			d->treads = i;
			return;
		}
	}
	fprintf(stderr, "Invalid Tread Type : %s\n", val);
	iGblRetCode = DESC_BAD_FORMAT;
}

void cv_weap(weapon_num, val, d)
char weapon_num;
char *val;
Vdesc *d;
{
	char *ptr = val;

	while (! isspace(*ptr) && *ptr)
	{
		ptr++;
	}
	*ptr = '\0';

	ptr++;
	while (isspace(*ptr) && *ptr)
	{
		ptr++;
	}

	if (*ptr)
	{
		cv_kw2i(val, &d->mount[d->num_weapons], Mnames, NUM_MOUNTS);
		cv_kw2i(ptr, &d->weapon[d->num_weapons], Wnames, VMAX_WEAPONS);
		d->num_weapons += 1;
	}
	else
	{
		iGblRetCode = DESC_BAD_FORMAT;
	}

	return;
}

int ReadVehicleFormat1(fp, d)
	FILE *fp;
	Vdesc *d;
{
	static int bCalled = FALSE;
	iGblRetCode = DESC_LOADED;

	if (bCalled) {
		yyrestart(fp);
		yylex(d);
	} else {
		if (Wnames[0] == (void *)0)
		{
			init_Wnames();
		}

		yyin = fp;
		yylex(d);

		bCalled = TRUE;
	}

	return (iGblRetCode);
}
