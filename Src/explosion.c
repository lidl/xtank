/*
** Xtank
**
** Copyright 1993 by Pix Technologies, Corp.
**
** explosion.c
*/

/*
$Author: lidl $
$Id: explosion.c,v 1.1.1.1 1995/02/01 00:25:34 lidl Exp $
*/

#include "sysdep.h"
#include "bullet.h"
#include "graphics.h"
#include "loc.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif SOUND

#define EXP_SPREAD 15

/* Creates an explosion of the specified type at the specified location.  */

make_explosion(loc, type, index)
Loc *loc;
unsigned int type;
int index;
{
	extern int num_terminals;
	extern Object *exp_obj[];
	extern Eset *eset;
	Exp *e;
	int tnum;

	if (eset->number >= MAX_EXPS)
		return;

#ifdef SOUND
	switch (type)
	{
		case EXP_TANK:
			play_in_view(loc, TANK_EXPLOSION_SOUND);
			break;
		case EXP_GLEAM:
			play_in_view(loc, GLEAM_EXPLOSION_SOUND);
			break;
		case EXP_DAM0:
			play_in_view(loc, DAM0_EXPLOSION_SOUND);
			break;
		case EXP_DAM1:
			play_in_view(loc, DAM1_EXPLOSION_SOUND);
			break;
		case EXP_DAM2:
			play_in_view(loc, DAM2_EXPLOSION_SOUND);
			break;
		case EXP_DAM3:
			play_in_view(loc, DAM3_EXPLOSION_SOUND);
			break;
		case EXP_DAM4:
			play_in_view(loc, DAM4_EXPLOSION_SOUND);
			break;
		case EXP_EXHAUST:
			play_in_view(loc, EXHAUST_EXPLOSION_SOUND);
			break;
		case EXP_ELECTRIC:
			play_in_view(loc, ELECTRIC_EXPLOSION_SOUND);
			break;
#ifdef NO_GAME_BALANCE
		case EXP_NUKE:
			play_in_view(loc, NUKE_EXPLOSION_SOUND);
			break;
#endif
		default:
			play_in_view(loc, DAMAGE_EXPLOSION_SOUND);
	}
#endif SOUND
	
	e = eset->list[eset->number++];
	e->x = (int) loc->x;
	e->y = (int) loc->y;
	e->z = (int) loc->z;
	for (tnum = 0; tnum < num_terminals; tnum++) {
		e->old_screen_x[tnum] = e->screen_x[tnum] = loc->screen_x[tnum];
		e->old_screen_y[tnum] = e->screen_y[tnum] = loc->screen_y[tnum];
	}
	e->obj = exp_obj[type];
	e->life = (e->obj->num_pics + 1) - index;
	e->color = CUR_COLOR;
}


/* Makes the given number of explosions of the given type around the
   location.  */

explode_location(loc, num, type)
Loc *loc;
int num;
unsigned int type;
{
	Loc exp_loc;
	int exp_dx, exp_dy;
	int i;

	for (i = 0; i < num; i++) {
		exp_loc = *loc;
		exp_dx = rnd(EXP_SPREAD << 1) - EXP_SPREAD;
		exp_dy = rnd(EXP_SPREAD << 1) - EXP_SPREAD;
		update_loc(&exp_loc, &exp_loc, exp_dx, exp_dy);
		make_explosion(&exp_loc, type, 0);
	}
}
