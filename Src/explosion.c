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

#include "sysdep.h"
#include "bullet.h"
#include "graphics.h"
#include "loc.h"
#include "terminal.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif /* SOUND */

#define EXP_SPREAD 15

/* Creates an explosion of the specified type at the specified location.  */
void
make_explosion(Loc *loc, int type, int index)
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
#endif /* SOUND */
	
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

void
explode_location(Loc *loc, int num, int type)
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
