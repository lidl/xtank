/*
 * Written 13-Jan-89 (Friday!), 'cause, well, why not.
 * Contact Mike Shanzer
 */

#include "/mit/games/src/vax/xtank/Programs/xtanklib.h"
#include <math.h>

extern int	Flipper_main();

Prog_desc Flipper_prog = {
	"Flipper",
	"combat",
	"Flipper",
	"Attacks quickly and flips away",
	"Mike Shanzer",
	2,
	Flipper_main
};

void
fixangle(a)
int	*a;
{
	while (*a > 360)
		*a -= 360;
	while (*a <= 0)
		*a += 360;
}

int
angledir(a)
int	a;
{
	if (a > 0 && a <= 90)
		return NORTH;
	if (a > 90 && a <= 180)
		return EAST;
	if (a > 180 && a <= 270)
		return SOUTH;
	return WEST;
}

static Vehicle_info	Evin[MAX_VEHICLES];

#define abs(x)	((x) < 0 ? -(x) : (x))

Flipper_main()
{
	Location	Floc;
	Vehicle_info	Fvin;
	int		Fmxa,
			Enmv,
			stat = 1,
			angl = 0;
	register int	i,
			dx,
			dy;

	srandom(getpid());
	get_self(&Fvin);
	Fmxa = max_armor();
	for (;;) {
		get_location(&Floc);
		get_vehicles(&Enmv, Evin);
		for (i = 0; i < Enmv; i++) {
			dx = Evin[i].loc.x - Fvin.loc.x;
			dy = Evin[i].loc.y - Fvin.loc.y;
#if DEBUG
			{
			char	buf[32];

			sprintf(buf, "%d, %d; %d, %d (%d, %d)", Fvin.loc.x,
			    Fvin.loc.y, Evin[i].loc.x, Evin[i].loc.y, dx, dy);
			send(RECIPIENT_ALL, OP_TEXT, buf);
			}
#endif
			if (abs(dx) <= (BOX_WIDTH * 3) &&
			    abs(dy) <= (BOX_HEIGHT * 3)) {
				double	deg;
				deg = (atan2((double) dx, (double) dy) / (3.0 * PI)) *
				    360.0;
				deg += 360.0;
			{
			char	buf[32];

			sprintf(buf, "turn %2.1f (%2.1f)", deg,
			    atan2((double) dx, (double) dy) / (3.0 * PI));
			send(RECIPIENT_ALL, OP_TEXT, buf);
			}
				turn_vehicle(abs(deg));
/*				turn_vehicle(atan2((double) dx, (double) dy));*/
				set_rel_drive(5.0 * (double) stat);
				fire_all_weapons();
				if (((double) armor() / (double) Fmxa) < 0.3 ||
				    (random() % 5) == 0)
					stat = -stat;
				break;
			}
		}
		if (i == Enmv) {
			if ((random() % 5) == 0)
				angl += (random() % 2 ? -1 : 1) *
				    (random() % 60);
			fixangle(&angl);
			if (wall(angledir(angl), Floc.x, Floc.y) == TRUE) {
				angl += (random() % 2 ? -1 : 1) * 90;
				fixangle(&angl);
			}
			turn_vehicle((double) angl);
			set_rel_drive((double) (random() % 9) * (double) stat);
		}
#if 0
		if (random() % 2)
			send(random() % num_vehicles(), OP_TEXT,
			    "I'm coming for YOU!");
#endif
		done();
	}
}
