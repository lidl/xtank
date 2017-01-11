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

#include "xtank.h"
#include "graphics.h"
#include "vstructs.h"
#include "bullet.h"
#include "terminal.h"
#include "proto.h"

extern Weapon_stat weapon_stat[];
extern Settings settings;

/*
** Initializes the settings.
*/
void
init_settings(void)
{
	static Settings initial_settings =
	{
		15,						/* int game_speed in frames per second */
		NULL,					/* Mdesc *mdesc */
		30,						/* int maze_density */
		FALSE,					/* Boolean point_bullets */
		FALSE,					/* Boolean commentator */
		TRUE,					/* Boolean robots_dont_win GHS */
		FALSE,					/* Boolean max_armor_scale GHS */
		0,						/* int difficulty */
		{						/* Settings_info */
			COMBAT_GAME,		/* Game game */
			FALSE,				/* Boolean ricochet */
			TRUE,				/* Boolean rel_shoot GHS */
			FALSE,				/* Boolean no_wear */
			TRUE,				/* Boolean restart */
			FALSE,				/* Boolean full_map */
			FALSE,				/* Boolean pay_to_play GHS */
			FALSE,				/* Boolean no_nametags */
			TRUE,				/* Boolean team_score GHS */
			TRUE,				/* Boolean player_teleport */
			TRUE,				/* Boolean disc_teleport */
			TRUE,				/* Boolean teleport_from_team */
			TRUE,				/* Boolean teleport_from_neutral */
			TRUE,				/* Boolean teleport_to_team */
			TRUE,				/* Boolean teleport_to_neutral */
			TRUE,				/* Boolean teleport_any_to_any */
			TRUE,				/* Boolean war_goals_only */
			FALSE,				/* Boolean relative_disc */
			TRUE,				/* Boolean ultimate_own_goal */

			10000,				/* int winning_score GHS */
			20,					/* int takeover_time */
			5,					/* int outpost_strength */
			1,					/* int shocker_walls GHS */
			5.0,				/* FLOAT scroll_speed */
			0.5,				/* FLOAT slip_friction */
			1.0,				/* FLOAT normal_friction */
			0.99,				/* FLOAT disc_friction */
			0.4,				/* FLOAT disc_speed */
			0.0,				/* FLOAT disc_damage */
			1.0,				/* FLOAT disc_heat */
			0.5,				/* FLOAT box_slowdown */
			0.3,				/* FLOAT owner_slowdown */
			1				/* int num_discs */
		},
	};

	settings = initial_settings;
}

void
init_turrets(Vehicle *v)
{
	int i;

	for (i = 0; i < v->num_turrets; i++) {
		Turret *t = &v->turret[i];
		int views = t->obj->num_pics;

		/* Give the turret a random initial angle */
		t->desired_angle = t->angle = (FLOAT) rnd(100) * (2 * PI) / 100 - PI;
		t->old_rot = t->rot = ((int) ((t->angle) /
									(2 * PI) * views + views + .5)) % views;
#ifdef TEST_TURRETS
		t->old_end.x = t->end.x = cos(t->angle) * TURRET_LENGTH;
		t->old_end.y = t->end.y = sin(t->angle) * TURRET_LENGTH;
#endif /* TEST_TURRETS */

	}
}

void
init_bset(void)
{
	extern Bset *bset;
	int i;

	bset->number = 0;
	for (i = 0; i < MAX_BULLETS; i++)
		bset->list[i] = &bset->array[i];
}

void
init_eset(void)
{
	extern Eset *eset;
	int i;

	eset->number = 0;
	for (i = 0; i < MAX_EXPS; i++)
		eset->list[i] = &eset->array[i];
}
