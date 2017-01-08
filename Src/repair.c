/*-
 * Copyright (c) 1990 Gordon Smith
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
#include "vstructs.h"
#include "sysdep.h"
#include "vehicle.h"
#include "proto.h"

extern Settings settings;
extern int frame;

#define FUEL_SWAP 1

SpecialStatus
special_repair(Vehicle *v, char *record, int action)
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
