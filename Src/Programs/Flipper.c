/*-
 * Copyright (c) 1989 Mike Shanzer
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

/*
 * Written 13-Jan-89 (Friday!), 'cause, well, why not.
 * Contact Mike Shanzer
 */

#include "xtanklib.h"
#include <math.h>

static void Flipper_main(void);

static Vehicle_info Evin[MAX_VEHICLES];

Prog_desc Flipper_prog = {
	"Flipper",
	"Flipper",
	"Attacks quickly and flips away",
	"Mike Shanzer",
	USES_MESSAGES | PLAYS_COMBAT | DOES_SHOOT,	/* skills */
	2,
	Flipper_main
};

static void
fixangle(int *a)
{
	while (*a > 360)
		*a -= 360;
	while (*a <= 0)
		*a += 360;
}

static WallSide
angledir(int a)
{
	if (a > 0 && a <= 90)
		return NORTH;
	if (a > 90 && a <= 180)
		return EAST;
	if (a > 180 && a <= 270)
		return SOUTH;
	return WEST;
}

static void
Flipper_main(void)
{
	Location Floc;
	Vehicle_info Fvin;
	int Fmxa, Enmv, stat = 1, angl = 0;
	register int i, dx, dy;

	srandom(getpid());
	get_self(&Fvin);
	Fmxa = max_armor(FRONT);	/* just one side??? -- RDP */
	for (;;)
	{
		get_location(&Floc);
		get_vehicles(&Enmv, Evin);
		for (i = 0; i < Enmv; i++)
		{
			dx = Evin[i].loc.x - Fvin.loc.x;
			dy = Evin[i].loc.y - Fvin.loc.y;

#if DEBUG
			{
				char buf[32];

				sprintf(buf, "%d, %d; %d, %d (%d, %d)", Fvin.loc.x,
						Fvin.loc.y, Evin[i].loc.x, Evin[i].loc.y, dx, dy);
				send_msg(RECIPIENT_ALL, OP_TEXT, buf);
			}
#endif

			if (ABS(dx) <= (BOX_WIDTH * 3) &&
				ABS(dy) <= (BOX_HEIGHT * 3))
			{
				double deg;

				deg = (ATAN2(dx, dy) / (3.0 * PI)) * 360.0;
				deg += 360.0;

#ifdef DEBUG
				sprintf(buf, "turn %2.1f (%2.1f)", deg,
					ATAN2(dx, dy) / (3.0 * PI));
				send_msg(RECIPIENT_ALL, OP_TEXT, buf);
#endif

				turn_vehicle(ABS(deg));
				/* turn_vehicle(ATAN2(dx, dy)); */
				aim_all_turrets(dx, dy);
				set_rel_drive(5.0 * (double) stat);
				fire_all_weapons();
				if (((double) armor(FRONT) / (double) Fmxa) < 0.3 ||
						(random() % 5) == 0)
					stat = -stat;
				break;
			}
		}
		if (i == Enmv)
		{
			if ((random() % 5) == 0)
				angl += (random() % 2 ? -1 : 1) *
					(random() % 60);
			fixangle(&angl);
			if (wall(angledir(angl), Floc.x, Floc.y))
			{
				angl += (random() % 2 ? -1 : 1) * 90;
				fixangle(&angl);
			}
			turn_vehicle((double) angl);
			set_rel_drive((double) (random() % 9) * (double) stat);
		}

#if 0
		if (random() % 2)
			send_msg(random() % num_vehicles(), OP_TEXT,
					 "I'm coming for YOU!");
#endif

		done();
	}
}
