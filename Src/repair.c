
/*
** Xtank
**
** Copyright 1990 by Gordon Smith
**
** repair.c
*/

/*
$Author: lidl $
$Id: repair.c,v 1.1.1.1 1995/02/01 00:25:37 lidl Exp $
*/

#include "xtank.h"
#include "vstructs.h"
#include "sysdep.h"
#include "vehicle.h"
#include "proto.h"

extern Settings settings;
extern int frame;

#define FUEL_SWAP 1

/*ARGSUSED*/
SpecialStatus special_repair(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	int *side, *max_side;
	int i;

	if (action != SP_update || settings.si.no_wear
		|| v->vector.speed != 0.0 || (frame % 12 != 0))
		return SP_on;

	/* Add one armor point to all sides every 12 frames */

	side = v->armor.side;
	max_side = v->vdesc->armor.side;

	for (i = 0; i < MAX_SIDES; i++) {
		if (v->fuel >= FUEL_SWAP && side[i] < max_side[i]) {
			side[i]++;
			v->fuel -= FUEL_SWAP;	/* make it fuel type & armor type */
		}
	}
}
