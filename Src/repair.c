/*
** Xtank
**
** Copyright 1990 by Gordon Smith
**
** repair.c
*/

#include "xtank.h"
#include "vstructs.h"
#include "sysdep.h"
#include "vehicle.h"


extern Settings settings;
extern int frame;


#define FUEL_SWAP 1

/*ARGSUSED*/
special_repair(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	int *side, *max_side;
	int i;

	if (settings.si.no_wear)
		return;
	if (v->vector.speed != 0.0)
		return;

	/* Add one armor point to all sides every 12 frames */
	if (frame % 12 != 0)
		return;

	side = v->armor.side;
	max_side = v->vdesc->armor.side;

	for (i = 0; i < MAX_SIDES; i++)
	{
		if (v->fuel >= FUEL_SWAP && side[i] < max_side[i])
		{
			side[i]++;
			v->fuel -= FUEL_SWAP;		/* make it fuel type & armor type */
		}
	}
}
