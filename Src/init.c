/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** init.c
*/

/*
$Author: lidl $
$Id: init.c,v 2.14 1992/06/07 02:45:08 lidl Exp $

$Log: init.c,v $
 * Revision 2.14  1992/06/07  02:45:08  lidl
 * Post Adam Bryant patches and a manual merge of the rejects (ugh!)
 *
 * Revision 2.13  1992/03/31  04:04:16  lidl
 * pre-aaron patches, post 1.3d release (ie mailing list patches)
 *
 * Revision 2.12  1992/01/30  03:43:20  aahz
 * removed ifdefs around no radar
 *
 * Revision 2.11  1992/01/30  03:25:48  aahz
 * team score and disable radar where juxtaposed in init.
 *
 * Revision 2.10  1992/01/29  08:37:01  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.9  1991/12/10  03:41:44  lidl
 * changed float to FLOAT, for portability reasons
 *
 * Revision 2.8  1991/11/27  06:45:25  aahz
 * fixed typo
 *
 * Revision 2.7  1991/11/27  06:22:48  aahz
 * default team score to on.
 *
 * Revision 2.6  1991/10/28  13:52:54  lidl
 * removed #ifdefs for NONAMETAGS -- they are now the default
 *
 * Revision 2.5  1991/09/19  05:30:48  lidl
 * run through indent, added NONAMETAGS default setting
 *
 * Revision 2.4  1991/03/25  01:38:23  stripes
 * Set defaults back to default (init_settings constants changed).
 *
 * Revision 2.3  1991/02/10  13:50:47  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:02  rpotter
 * complete rewrite of vehicle death, other tweaks
 *
 * Revision 2.1  91/01/17  07:11:48  rpotter
 * lint warnings and a fix to update_vector()
 *
 * Revision 2.0  91/01/17  02:09:39  rpotter
 * small changes
 *
 * Revision 1.2  90/12/30  03:00:34  aahz
 * made the default shocker wall strength = 1.
 *
 * Revision 1.1  90/12/29  21:02:32  aahz
 * Initial revision
 *
*/

#include "malloc.h"
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
	15,			/* int game_speed in frames per second */
	NULL,			/* Mdesc *mdesc */
	30,			/* int maze_density */
	FALSE,			/* Boolean point_bullets */
	FALSE,			/* Boolean commentator */
	FALSE,			/* Boolean robots_dont_win GHS */
	FALSE,			/* Boolean max_armor_scale GHS */
	0,			/* int difficulty */
	{			/* Settings_info */
	    COMBAT_GAME,	/* Game game */
	    FALSE,		/* Boolean ricochet */
	    TRUE,		/* Boolean rel_shoot GHS */
	    FALSE,		/* Boolean no_wear */
	    TRUE,		/* Boolean restart */
	    FALSE,		/* Boolean full_map */
	    FALSE,		/* Boolean pay_to_play GHS */
	    FALSE,		/* Boolean no_nametags */
	    TRUE,		/* Boolean team_score GHS */
	    FALSE,		/* Boolean no_radar */
	    TRUE,		/* Boolean player_teleport */
	    TRUE,		/* Boolean disc_teleport */
	    TRUE,		/* Boolean teleport_from_team */
	    FALSE,		/* Boolean teleport_from_neutral */
	    TRUE,		/* Boolean teleport_to_team */
	    TRUE,		/* Boolean teleport_to_neutral */
	    FALSE,		/* Boolean teleport_any_to_any */
	    TRUE,		/* Boolean war_goals_only */
	    FALSE,		/* Boolean relative_disc */
	    TRUE,		/* Boolean ultimate_own_goal */

	    10000,		/* int winning_score GHS */
	    20,			/* int takeover_time */
	    5,			/* int outpost_strength */
	    1,			/* int shocker_walls GHS */
	    5.0,		/* FLOAT scroll_speed */
	    0.5,		/* FLOAT slip_friction */
	    1.0,		/* FLOAT normal_friction */
	    0.99,		/* FLOAT disc_friction */
	    0.4,		/* FLOAT disc_speed */
	    0.0,		/* FLOAT disc_damage */
	    1.0,		/* FLOAT disc_heat */
	    0.5,		/* FLOAT box_slowdown */
	    0.3,		/* FLOAT owner_slowdown */
	},
    };

    settings = initial_settings;
}

init_turrets(v)
Vehicle *v;
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
