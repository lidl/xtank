#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** init.c
*/

#include "xtank.h"
#include "graphics.h"
#include "vstructs.h"
#include "bullet.h"


extern Weapon_stat weapon_stat[];
extern Settings settings;


/*
** Initializes the settings.
*/
init_settings()
{
	static Settings initial_settings = {
		SINGLE_MODE,			/* int mode */
		15,						/* int game_speed in frames per second */
		NULL,					/* Mdesc *mdesc */
		30,						/* int maze_density */
		FALSE,					/* Boolean point_bullets */
		FALSE,					/* Boolean commentator */
		FALSE,					/* Boolean robots_dont_win GHS */
		TRUE,					/* Boolean max_armor_scale GHS */
		0,						/* int difficulty */
		{						/* Settings_info */
			COMBAT_GAME,		/* Game game */
			FALSE,				/* Boolean ricochet */
			TRUE,				/* Boolean rel_shoot GHS */
			FALSE,				/* Boolean no_wear */
			TRUE,				/* Boolean restart */
			FALSE,				/* Boolean full_map */
			FALSE,				/* Boolean pay_to_play GHS */
			0,                  /* int shocker_walls GHS */
			10000,				/* int winning_score GHS */
	    20,			/* int takeover_time */
			5,					/* int outpost_strength */
			5.0,				/* float scroll_speed */
			0.5,				/* float slip_friction */
			1.0,				/* float normal_friction */
			0.99,				/* float disc_friction */
			0.5,				/* float box_slowdown */
			0.3,				/* float owner_slowdown */
		},
	};

	settings = initial_settings;
}

/*
** Initializes a vehicle for the start of a game.
*/
init_vehicle(v)
Vehicle *v;
{
	extern int team_color_bright[];
	Weapon *w;
	int i;

	v->status |= VS_functioning | VS_is_alive;
	v->status &= ~VS_was_alive;
	v->death_timer = 0;
	v->is_dead = FALSE;

	/* Initialize message information */
	init_messages(v);

	v->armor = v->vdesc->armor;
	v->fuel = v->max_fuel;
	v->heat = 0;

	for (i = 0; i < v->num_weapons; i++)
	{
		w = &v->weapon[i];
		w->status = WS_on | WS_func;
        w->ammo = weapon_stat[(int)w->type].max_ammo;
		w->reload_counter = 0;
	}

	init_specials(v);
	init_turrets(v);

	v->team = 0;
	v->num_programs = 0;
	v->num_discs = 0;
    v->color = team_color_bright[v->team];
}

/*
** Allocates memory for all the specials for the vehicle and activates them.
*/
init_specials(v)
Vehicle *v;
{
	extern int special_console(), special_mapper(), special_radar(), special_dummy(), special_repair();
	extern char *calloc();
	Special *s;
	Console *c;
	int i, j;

	for (i = 0; i < MAX_SPECIALS; i++)
	{
		s = &v->special[i];
		switch (i)
		{
			case CONSOLE:
				s->proc = special_console;
				s->record = calloc((unsigned) 1, sizeof(Console));
				c = (Console *) s->record;
				for (j = 0; j < MAX_ENTRIES; j++)
					c->_entry[j].color = WHITE;
				break;
			case MAPPER:
				s->proc = special_mapper;
				s->record = calloc((unsigned) 1, sizeof(Mapper));
				break;
			case RADAR:
				s->proc = special_radar;
				s->record = calloc((unsigned) 1, sizeof(Radar));
				break;
			case REPAIR:		/* REPAIR */
				s->proc = special_repair;
				s->record = calloc((unsigned) 1, 1);
				break;
			default:
				s->proc = special_dummy;
				s->record = calloc((unsigned) 1, 1);
				break;
		}
		if (s->status == SP_nonexistent)
			continue;
		s->status = SP_off;
	}
}

init_turrets(v)
Vehicle *v;
{
	extern Object *turret_obj[];
	Turret *t;
	Weapon *w;
	int turret_weight;
	int views;
	int i, j;

	if (v->num_turrets == 0)
		return;

	v->turret = (Turret *) calloc((unsigned) v->num_turrets, sizeof(Turret));

	for (i = 0; i < v->num_turrets; i++)
	{
		t = &v->turret[i];

		turret_weight = TURRET_WEIGHT;

		/* Add to that weight, the weights of all weapons mounted on it */
		if (v->num_weapons > 0)
			for (j = 0; j < v->num_weapons; j++)
			{
				w = &v->weapon[j];
                if ((int)w->mount == i)
                    turret_weight += weapon_stat[(int)w->type].weight;
			}

		/* Make the turret turn rate inversely proportional to its weight */
		t->turn_rate = 600.0 / (float) turret_weight;

		/* Give the turret a random initial angle */
		t->desired_angle = t->angle = (float) rnd(100) * (2 * PI) / 100 - PI;
		t->obj = turret_obj[0];
		views = t->obj->num_pics;
        t->old_rot = t->rot = ((int) ((t->angle) /
									  (2 * PI) * views + views + .5)) % views;
	}
}


init_bset()
{
	extern Bset *bset;
	int i;

	bset->number = 0;
	for (i = 0; i < MAX_BULLETS; i++)
		bset->list[i] = &bset->array[i];
}


init_eset()
{
	extern Eset *eset;
	int i;

	eset->number = 0;
	for (i = 0; i < MAX_EXPS; i++)
		eset->list[i] = &eset->array[i];
}
