/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** shooter.c
*/

/*
$Author: aahz $
$Id: shooter.c,v 2.5 1991/09/28 23:32:05 aahz Exp $

$Log: shooter.c,v $
 * Revision 2.5  1991/09/28  23:32:05  aahz
 * no change.
 *
 * Revision 2.4  1991/09/17  17:01:08  lidl
 * caught a non-checked in change for the i860
 *
 * Revision 2.3  91/02/10  13:51:39  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 * 
 * Revision 2.2  91/01/20  09:58:58  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:58  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:31  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:05  aahz
 * Initial revision
 * 
*/

#include <xtanklib.h>
#include "malloc.h"
#include <stdio.h>
#if defined(i860)
#undef drem
#endif
#include <math.h>

#if defined(i860)
/* drem() not available so replace with wrapper around fmod() */
#define drem(a,b) ((temp_drem = fmod(a,b)) > (b)/2 ? temp_drem-(b) : temp_drem)
#endif

static void main();

Prog_desc shooter_prog = {
    "shooter",
    "BASE",
    "Sits in one place and fires at the nearest enemy.",
    "Stripes@eng.umd.edu (orig. Terry Donahue)",
    PLAYS_COMBAT | DOES_SHOOT | USES_TEAMS,
    1,
    main
};

#define ENEMY_NONE		0
#define ENEMY_RADAR		1
#define ENEMY_SCREEN	2


typedef struct
{
	int num;					/* #weps in turret */
	Weapon_info *winfo[MAX_WEAPONS];
} tinf;							/* "Turret info" */

typedef struct
{
	int nt;
	Weapon_info weap[MAX_WEAPONS];
	tinf tinfo[MAX_TURRETS];
	Team team;
} shooter_info;


static void main()
{
	Vehicle_info enemy;
	Location myloc;
	int enemy_flag;
	shooter_info data;
	shooter_info *sinfo = &data;
	extern int frame;
	int cur_frame;


	/* Find out info about weapons */
	shooter_init(sinfo);

	for (;;)
	{
		/* figure out where we are */
		get_location(&myloc);

		/* find the nearest enemy */
		enemy_flag = shooter_find(&myloc, sinfo->team, &enemy);

		/* try to shoot at enemy if on screen and we have a weapon */
		if (enemy_flag == ENEMY_SCREEN)
			shooter_shoot(sinfo, &myloc, &enemy);

		/* Give up remaining cpu time to improve speed of game */
		cur_frame = frame;
		done();
		if (cur_frame == frame)
		{
			char *crash = NULL;

			/* assert apparently doesn't work too well... */
			*crash = 0;
		}
	}
}

/*
** Looks for vehicles on the screen.  Puts closest vehicle with
** a clear path to it into t and returns ENEMY_SCREEN.  If no
** such vehicle, returns ENEMY_NONE.
*/
shooter_find(myloc, myteam, t)
Location *myloc;
Team myteam;
Vehicle_info *t;
{
	Vehicle_info vinfo[MAX_VEHICLES];
	int num_vinfos;
	Vehicle_info *v;
	int dx, dy, range, min_range;
	int i;

	get_vehicles(&num_vinfos, vinfo);

	/* * Find the closest vehicle with a clear path to it. */
	min_range = 99999999;
	for (i = 0; i < num_vinfos; i++)
	{
		v = &vinfo[i];

		/* ignore vehicles on the same team */
		if (myteam == v->team && myteam != NEUTRAL)
			continue;

		/* ignore vehicles that have no clear path to them */
		if (!clear_path(myloc, &v->loc))
			continue;

		dx = v->loc.x - myloc->x;
		dy = v->loc.y - myloc->y;
		range = dx * dx + dy * dy;
		if (range < min_range)
		{
			min_range = range;
			*t = *v;
		}
	}

	if (min_range == 99999999)
		return ENEMY_NONE;
	else
		return ENEMY_SCREEN;
}

/*
** Shoots at vehicle t with all weapons.  Uses quick leading fan algorithm.
*/
shooter_shoot(sinfo, myloc, t)
    shooter_info *sinfo;
    Location *myloc;
    Vehicle_info *t;
{
    extern float turret_angle(), aim_turret();
    extern int frame;
    int dx, dy, range;
    float lead_factor, ang;
    float kludge;
    int i, j, k;
    WeaponType weap_type;
    Weapon_info *wi, *twi;

    dx = t->loc.x - myloc->x;
    dy = t->loc.y - myloc->y;

    range = dx * dx + dy * dy;
    for (i = 0; i < sinfo->nt; i++)
    {
	for (j = 0; j < sinfo->tinfo[i].num; j++)
	{
	    wi = sinfo->tinfo[i].winfo[j];
	    if (range > wi->range)
		continue;
	    /* Lead the target approximately, shoot fanning */
	    lead_factor = 2 * sqrt(0.0 + range) / wi->ammo_speed;
	    dx += (int) (t->xspeed * lead_factor * (float) rnd(20) / 19.0);
	    dy += (int) (t->yspeed * lead_factor * (float) rnd(20) / 19.0);

	    /* Point the turret towards where he is going to be */
            ang = aim_turret((TurretNum)i, dx, dy);

	    /* Forget shooting if you won't hit... */
	    /* (I want a better algo..) */
            kludge = ang - turret_angle((TurretNum)i);
	    ang = drem(kludge, (2.0 * PI)); /* Needs to become drem, I
					       think */
	    /* ang = (ang - turret_angle(i)) % 2.0*PI; */
	    ang = (ang <= 0) ? -ang : ang;

#ifdef DEBUG
	    if (ang >= PI / 8)
	    {
		fprintf(stderr, "turret %d off by %f, frame %d skiping\n", i, ang, frame);
		break;
	    }
#endif

	    /* Shoot the weapon */
	    weap_type = wi->type;
	    fire_weapon(wi - sinfo->weap);
	    for (twi = wi + 1, k = j + 1; k < sinfo->tinfo[i].num; k++, twi++)
	    {
		if (twi->type == weap_type)
		    fire_weapon(twi - sinfo->weap);
	    }
	    break;
	}
    }
}

shooter_init(sinfo)
shooter_info *sinfo;
{
	int i, tnum, j, numweaps;
	Weapon_info *w, *wx;
	tinf *ti;
	Vehicle_info v;

	get_self(&v);
	sinfo->team = v.team;

	for (i = 0; i < MAX_TURRETS; i++)
	{
		sinfo->tinfo[i].num = 0;
	}
	sinfo->nt = 0;

	numweaps = num_weapons();
	for (i = 0; i < numweaps; i++)
	{
		w = &sinfo->weap[i];
		get_weapon(i, w);

		switch (w->mount)
		{
			case (MOUNT_TURRET1):
				tnum = 0;
				break;
			case (MOUNT_TURRET2):
				tnum = 1;
				break;
			case (MOUNT_TURRET3):
				tnum = 2;
				break;
			default:
				fprintf(stderr, "shooter_init bad mount=%d, ignored\n", w->mount);
		}
		w->range *= w->range;
		ti = sinfo->tinfo + tnum;
		for (j = 0; j < ti->num; j++)
		{
			if (ti->winfo[j]->range > w->range)
			{
				/* struct assign is ANSI std, you can memcy if you have a
				   losing compiler... or get gcc */
				wx = w;
				w = ti->winfo[j];
				ti->winfo[j] = wx;
			}
		}
		ti->winfo[j] = w;
		ti->num += 1;
		if (tnum >= sinfo->nt)
			sinfo->nt = tnum + 1;
	}
	/* Next do secondary sort baised on sinfo->tinfo[*].winfo[*].damage */
	/* (want big to low) */

#if 0
	for (i = 0; i < sinfo->nt; i++)
	{
		ti = sinfo->tinfo;
		for (j = 0; j < ti->num; j++)
		{
			fprintf(stderr, "turret %d, wep %d, range %d\n", i, j, ti->winfo[j]->range);
		}
	}
#endif
}
