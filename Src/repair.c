/*
** Xtank
**
** Copyright 1990 by Gordon Smith
**
** repair.c
*/

/*
$Author: rpotter $
$Id: repair.c,v 2.3 1991/02/10 13:51:33 rpotter Exp $

$Log: repair.c,v $
 * Revision 2.3  1991/02/10  13:51:33  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:51  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:49  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:24  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:01  aahz
 * Initial revision
 * 
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
