/*  ========================================================================
    ========================================================================
    =====                  THE DIOPHANTINE EXPLORER                     ====
    =====                                                               ====
    ===== For use as a controlling program for the Xtank "Diophantom"   ====
    =====                                                               ====
    =====                                                               ====
    ===== Written by Steve Worley for the BATTLE OF THE XTANKS contest  ====
    =====                                                               ====
    ===== 4/24/88                                         v48.378.373   ====
    =====                                                               ====
    ===== Thanks to Terry Donahue for help with program coding problems ====
    ========================================================================
    ========================================================================

***** Modified to work with Xtank v0.95 by Dan Schmidt *****
***** Modified to work with Xtank v1.20 by Gordon Smith *****

*/

#include "../Src/xtanklib.h"
#include "math.h"
#include "stdio.h"
#include <assert.h>

#ifdef DEBUG
#define dbg(a)  indbg(a)
#define dbg1(a) outdbg(a)
#else
#define dbg(a)
#define dbg1(a)
#endif

static Diophantine_main();

/* Who knows what the hell these skills are? */

Prog_desc Dio_prog = {
	"DioMatt2.0",
	/* "war", */
	"Diophantom",
	"The Diophantine explorer is a fairly intelligent robot which has a \
decent dodging and wall avoiding routine. Although it's weaponry is merely \
machine guns, it should not be ignored. It also is ve ry fast and \
    manouverable, which, combined with its bullet dodging, makes it tough to \
    hit.",
	"Steven Worley",
	PLAYS_COMBAT | PLAYS_RACE | DOES_SHOOT | DOES_EXPLORE | DOES_REPLENISH | USES_TEAMS,
	7,
	Diophantine_main
};

#define SLOWER_THEN_MAX 3.0

struct g_struct
{
	int D_lastshotat, D_shootcounter;
	int ANYONE_HERE;
	float sav_WANTED_SPEED;
	float WANTED_SPEED;
	float acc_freedom;
	float crange;
	float inv_speed;
	int num_weap;
	int max_weap_range;
	int weap_range[6];
};


static Diophantine_main()
{
	Location my_loc;
	Weapon_info winfo[6];
	int i;
	struct g_struct gstruct;

	dbg("Diophantine_main()");

	memset((char *) &gstruct, 0, sizeof(gstruct));
	gstruct.ANYONE_HERE = 1;
	gstruct.sav_WANTED_SPEED = gstruct.WANTED_SPEED = max_speed();
	gstruct.acc_freedom = 2.0;
	gstruct.crange = 9999.0;
	gstruct.D_shootcounter = 0;
	gstruct.inv_speed = 0.0;

	set_abs_drive(9.0);

	/* initialize globals */
	gstruct.num_weap = num_weapons();

	/* Compute stuff about weapons */
	gstruct.max_weap_range = 0;
	for (i = 0; i < gstruct.num_weap; i++)
	{
		get_weapon(i, winfo + i);
		gstruct.inv_speed += winfo[i].ammo_speed;
		gstruct.weap_range[i] = winfo[i].range;
		if (gstruct.weap_range[i] > gstruct.max_weap_range)
			gstruct.max_weap_range = gstruct.weap_range[i];
	}
	gstruct.inv_speed = gstruct.num_weap / gstruct.inv_speed;

	while (1)
	{
		get_location(&my_loc);

		Diophantine_watch_out_t(&gstruct);

		if (gstruct.D_shootcounter == 0)
			Diophantine_shoot_suckers(&gstruct);
		else
			Diophantine_nuke_same_guy(&gstruct);

		Diophantine_watch_out_a(&gstruct);
		/* Diophantine_watch_out_t(&gstruct); */

		if (gstruct.D_shootcounter == 0)
			Diophantine_shoot_suckers(&gstruct);
		else
			Diophantine_nuke_same_guy(&gstruct);

		gstruct.WANTED_SPEED = gstruct.sav_WANTED_SPEED;
	}
	dbg1("Diophantine_main()");
}


static Diophantine_watch_out_t(gstruct)
struct g_struct *gstruct;
{
	static int hurtamt[14] = {0, 2, 4, 0, 2, 4, 2, 4, 6, 4, 8, 12, 8, 2};
	static float angles[11] = {0.0, 0.4, -0.4, 0.8, -0.8, -1.5, 1.5, 2.3, -2.3, PI};
	float spe, ang;
	Location my_loc;
	float xs[5], ys[5], angl[5];
	int dam[5];
	Bullet_info bullet[MAX_BULLETS];
	Bullet_info *b;
	int num_bullets, count;
	int ax, ay, t, dx, dy, dis, s, ldam, ws, num;
	float sx, sy, sumsq, diff;

	dbg("Diophantine_watch_out_t()");

	get_location(&my_loc);
	spe = speed();
	ang = heading();
	num = 1;
	count = -1;

	while (count < 11 && num < 4)
	{
		count++;

		if (!Diophantine_gonna_hit(gstruct, &my_loc, ang + angles[count], spe))
		{
			angl[num] = ang + angles[count];
			dam[num] = 3 * Diophantine_abs_ang(gstruct, angl[count], ang);
			num++;
		}
	}

	gstruct->acc_freedom = 2 - (count > 4) - (count > 6);
	count += (count > 7) + (count > 10);
	assert(count >= 0);
	gstruct->WANTED_SPEED = (gstruct->sav_WANTED_SPEED + SLOWER_THEN_MAX) - count;

	/* set_abs_speed(gstruct->WANTED_SPEED); */
	set_abs_drive(gstruct->WANTED_SPEED);

	if (gstruct->D_shootcounter > 0)
		Diophantine_nuke_same_guy(gstruct);

	for (count = 1; count < num + 1; count++)
	{
		xs[count] = gstruct->WANTED_SPEED * cos(angl[count]);
		ys[count] = gstruct->WANTED_SPEED * sin(angl[count]);
		dam[count] += random() % 3;
	}

	if (gstruct->D_shootcounter > 0)
		Diophantine_nuke_same_guy(gstruct);

	ax = my_loc.grid_x * BOX_WIDTH + my_loc.box_x;
	ay = my_loc.grid_y * BOX_HEIGHT + my_loc.box_y;

	get_bullets(&num_bullets, bullet);
	if (num_bullets > 0)
	{
		for (t = 0; t < num_bullets; t++)
		{
			b = &bullet[t];
			dx = b->loc.grid_x * BOX_WIDTH + b->loc.box_x - ax;
			dy = b->loc.grid_y * BOX_HEIGHT + b->loc.box_y - ay;
			dis = dx * dx + dy * dy;
			if (dis > 500.0 && dis < 40000.0)
				for (s = 1; s < num + 1; s++)
				{
					sx = b->xspeed - xs[s];
					sy = b->yspeed - ys[s];
					sumsq = sx * sx + sy * sy;
					if (dy * sy + dx * sx < 0.0 && sumsq > 0.0)
					{
						diff = dx * sy - dy * sx;
						if (diff * diff < sumsq * 400.0)
							dam[s] += hurtamt[b->type];
					}
				}
		}
	}
	ldam = 999;
	/* printf("dam: %d %d %d %d %d\n ",dam[1],dam[2],dam[3]); */

	for (s = 1; s < num + 1; s++)
		if (dam[s] < ldam)
		{
			ws = s;
			ldam = dam[s];
		}
	turn_vehicle(angl[ws]);
	dbg1("Diophantine_watch_out_t()");
}


static Diophantine_gonna_hit(gstruct, loc, ang, spe)
struct g_struct *gstruct;
float ang, spe;
Location *loc;
{
	float xs, ys, xt, yt;
	int xi, yi, xd, yd;
	int ret;

	dbg("Diophantine_gonna_hit()");
#define RETURN(a)  {ret = (a); goto end;}

	spe += 3.0;
	if (spe < 3.0)
		spe -= 6.0;
#define JMO
#ifdef JMO
	/* JMOKludge */
	if (spe == 0 && !(random() % 50))
		RETURN(TRUE);
#endif
	spe *= 1.5;
	xs = spe * cos(ang ? ang : 0.1);
	ys = spe * sin(ang ? ang : 0.1);


	xt = 99999.0;
	if (xs < 0.0)
		xt = (10 - loc->box_x) / xs;
	if (xs > 0.0)
		xt = (BOX_WIDTH - 10 - loc->box_x) / xs;

	yt = 99999.0;
	if (ys < 0.0)
		yt = (10 - loc->box_y) / ys;
	if (ys > 0.0)
		yt = (BOX_HEIGHT - 10 - loc->box_y) / ys;


	/* if (xt>12.0 && yt >12.0) RETURN(0); */

	xi = loc->box_x + yt * xs;
	yi = loc->box_y + xt * ys;

	if (xs < 0.0)
		xd = -1;
	else
		xd = 0;
	if (ys < 0.0)
		yd = -1;
	else
		yd = 0;

	if (xt < 12.0)
	{
		if (yi < BOX_HEIGHT + 20 && yi > -20)
			if (wall(EAST, loc->grid_x + xd, loc->grid_y))
				RETURN(1);
		if (yi > BOX_HEIGHT - 20 && yi < 2 * BOX_HEIGHT + 20)
		{
			if (wall(EAST, loc->grid_x + xd, loc->grid_y + 1))
				RETURN(1);
			if (yi < BOX_HEIGHT + 20)
				if (wall(SOUTH, loc->grid_x + xd, loc->grid_y) ||
						wall(SOUTH, loc->grid_x + xd + 1, loc->grid_y))
					RETURN(1);
		}
		if (yi < 20 && yi > -BOX_HEIGHT - 20)
		{
			if (wall(EAST, loc->grid_x + xd, loc->grid_y - 1))
				RETURN(1);
			if (yi > -20)
				if (wall(NORTH, loc->grid_x + xd, loc->grid_y) ||
						wall(NORTH, loc->grid_x + xd + 1, loc->grid_y))
					RETURN(1);
		}
	}
	if (yt < 12.0)
	{
		if (xi < BOX_WIDTH + 20 && xi > -20)
			if (wall(SOUTH, loc->grid_x, loc->grid_y + yd))
				RETURN(1);
		if (xi > BOX_WIDTH - 20 && xi < 2 * BOX_WIDTH + 20)
		{
			if (wall(SOUTH, loc->grid_x + 1, loc->grid_y + yd))
				RETURN(1);
			if (xi < BOX_WIDTH + 20)
				if (wall(EAST, loc->grid_x, loc->grid_y + yd) ||
						wall(EAST, loc->grid_x, loc->grid_y + yd + 1))
					RETURN(1);
		}
		if (xi < 20 && xi > -BOX_WIDTH - 20)
		{
			if (wall(SOUTH, loc->grid_x - 1, loc->grid_y + yd))
				RETURN(1);
			if (xi > -20)
				if (wall(WEST, loc->grid_x, loc->grid_y + yd) ||
						wall(WEST, loc->grid_x, loc->grid_y + yd + 1))
					RETURN(1);
		}
	}
	ret = 0;
end:
	dbg1("Diophantine_gonna_hit()");
	return ret;
}




static Diophantine_watch_out_a(gstruct)
struct g_struct *gstruct;
{
	static int hurtamt[14] = {0, 1, 2, 0, 1, 2, 1, 2, 3, 2, 4, 6, 4, 1};
	float spe, ang;
	Location my_loc;
	float xs[4], ys[4];
	int dam[4];
	Bullet_info bullet[MAX_BULLETS];
	Bullet_info *b;
	int num_bullets;
	int ax, ay, t, dx, dy, dis, s, ldam, ws;
	float sx, sy, want, ca, sa, sumsq, diff;

	dbg("Diophantine_watch_out_a()");

	want = gstruct->WANTED_SPEED;


	if (gstruct->acc_freedom == 0)
		goto end;

	if (gstruct->D_shootcounter > 0)
		Diophantine_nuke_same_guy(gstruct);

	spe = speed();
	ang = heading();
	get_location(&my_loc);


	dam[1] = ((want - spe) * 0.5);
	dam[2] = 0;
	dam[3] = ((spe - want) * 0.5);


	get_bullets(&num_bullets, bullet);

	if (num_bullets > 0)
	{

		ca = cos(ang);
		sa = sin(ang);

		want = gstruct->WANTED_SPEED;

		xs[3] = (spe + gstruct->acc_freedom) * ca;
		ys[3] = (spe + gstruct->acc_freedom) * sa;

		xs[2] = spe * ca;
		ys[2] = spe * sa;

		xs[1] = (spe - gstruct->acc_freedom) * ca;
		ys[1] = (spe - gstruct->acc_freedom) * sa;


		ax = my_loc.grid_x * BOX_WIDTH + my_loc.box_x;
		ay = my_loc.grid_y * BOX_HEIGHT + my_loc.box_y;

		for (t = 0; t < num_bullets; t++)
		{
			b = &bullet[t];
			dx = b->loc.grid_x * BOX_WIDTH + b->loc.box_x - ax;
			dy = b->loc.grid_y * BOX_HEIGHT + b->loc.box_y - ay;
			dis = dx * dx + dy * dy;
			if (dis > 500.0 && dis < 100000.0)
				for (s = 1; s < 4; s++)
				{
					sx = b->xspeed - xs[s];
					sy = b->yspeed - ys[s];
					sumsq = sx * sx + sy * sy;
					if (dy * sy + dx * sx < 0.0 && sumsq > 0.0)
					{
						diff = dx * sy - dy * sx;
						if (diff * diff < sumsq * 400.0)
							dam[s] += hurtamt[b->type];
					}
				}
		}
	}
	ldam = 999;

	for (s = 1; s < 4; s++)
		if (dam[s] < ldam)
		{
			ws = s;
			ldam = dam[s];
		}
	set_abs_drive(spe + (ws - 2) * gstruct->acc_freedom);
end:
	dbg1("Diophantine_watch_out_a()");
}


static Diophantine_nuke_same_guy(gstruct)
struct g_struct *gstruct;
{
	Location my_loc;
	int i, num, range;
	float xsp, ysp, leadang;
	float trans;
	float dy, dx, dis;
	Vehicle_info vehicle[MAX_VEHICLES];
	int num_vehicles;

	dbg("Diophantine_nuke_same_guy()");


	get_location(&my_loc);
	get_vehicles(&num_vehicles, vehicle);

	if (num_vehicles == 0)
	{
		gstruct->D_shootcounter = 0;
		gstruct->ANYONE_HERE = 0;
		goto end;
	}
	num = 99;
	gstruct->ANYONE_HERE = 1;

	for (i = 0; i < num_vehicles; ++i)
	{
		if (vehicle[i].id == gstruct->D_lastshotat)
		{
			num = i;
			i = 99;
		}
	}
	if (num == 99)
	{
		gstruct->D_shootcounter = 0;
		goto end;
	}
	xsp = vehicle[num].xspeed;
	ysp = vehicle[num].yspeed;

	dx = BOX_WIDTH * (vehicle[num].loc.grid_x - my_loc.grid_x) +
		vehicle[num].loc.box_x - my_loc.box_x + 1.5 * xsp;

	dy = BOX_HEIGHT * (vehicle[num].loc.grid_y - my_loc.grid_y) +
		vehicle[num].loc.box_y - my_loc.box_y + 1.5 * ysp;

	dis = sqrt(dx * dx + dy * dy);

	trans = ((dx * ysp - dy * xsp) * gstruct->inv_speed / dis);

	if (trans >= -1.0 && trans <= 1.0)
	{
		leadang = asin(trans) * (.5 + random() % 101 * .005) + atan2(dy, dx);

		if (dy == 0)
			range = (dx / (cos(leadang) - xsp * gstruct->inv_speed));
		else
			range = (dy / (sin(leadang) - ysp * gstruct->inv_speed));

		if (range < gstruct->max_weap_range)
		{
			turn_all_turrets(leadang);	/* Bastard was only useing #0
										   JMO&GHS... */
			gstruct->D_shootcounter--;
			trans = Diophantine_abs_ang(gstruct, turret_angle(0), leadang);
			if (trans < 0.6)
				Diophantine_fire(gstruct, range);
		}
		else
			gstruct->D_shootcounter = 0;
	}
	else
		gstruct->D_shootcounter = 0;
end:
	dbg1("Diophantine_nuke_same_guy()");
}



static Diophantine_shoot_suckers(gstruct)
struct g_struct *gstruct;
{
	Vehicle_info vehicle[MAX_VEHICLES];
	Vehicle_info *v;
	int num_vehicles;
	Location my_loc;
	int ax, ay, not_done;
	int i, absx, absy;
	int qual[MAX_VEHICLES];
	float dx, dy, ang;
	float xsp, ysp, highang;
	float leadang[MAX_VEHICLES];
	float trans, ta, r, rng;
	int hrange, range[MAX_VEHICLES];
	int hid, highqual, di[MAX_VEHICLES], ohighqual;

	dbg("Diophantine_shoot_suckers()");

	get_vehicles(&num_vehicles, vehicle);

	if (num_vehicles == 0)
	{
		gstruct->ANYONE_HERE = 0;
		gstruct->crange = 9999;
		goto end;
	}
	else
		gstruct->ANYONE_HERE = 1;

	ta = turret_angle(0);
	ang = heading();

	get_location(&my_loc);

	absx = BOX_WIDTH * my_loc.grid_x + my_loc.box_x;
	absy = BOX_HEIGHT * my_loc.grid_y + my_loc.box_y;

	ohighqual = 9999;

	for (i = 0; i < num_vehicles; ++i)
	{
		v = &vehicle[i];
		xsp = v->xspeed;
		ysp = v->yspeed;
		dx = BOX_WIDTH * v->loc.grid_x + v->loc.box_x - absx;
		dy = BOX_HEIGHT * v->loc.grid_y + v->loc.box_y - absy;

		r = sqrt(dx * dx + dy * dy);
		if (r < gstruct->crange)
		{
			gstruct->crange = r;
		}
		dx += 3.0 * xsp;
		dy += 3.0 * ysp;

		rng = sqrt(dx * dx + dy * dy);
		if (rng > 0.0)
		{
			trans = (dx * ysp - dy * xsp) * gstruct->inv_speed / rng;

			if (trans >= -1.0 && trans <= 1.0)
			{
				leadang[i] = asin(trans) + atan2(dy, dx);
				if (dy == 0.0)
					range[i] = (dx / (cos(leadang[i]) - xsp * gstruct->inv_speed));
				else
					range[i] = (dy / (sin(leadang[i]) - ysp * gstruct->inv_speed));

				qual[i] = -range[i] - 20 * Diophantine_abs_ang(gstruct, ang, leadang[i])
					- 3000 * (range[i] > gstruct->max_weap_range);

				di[i] = v->id;
			}
		}
	}

	not_done = 1;
	do
	{
		highqual = -9999;

		for (i = 0; i < num_vehicles; ++i)
		{
			if ((qual[i] > highqual) && (qual[i] < ohighqual))
			{
				highqual = qual[i];
				highang = leadang[i];
				hrange = range[i];
				hid = di[i];
			}
		}

		if (highqual < -2500)
			not_done = 0;
		else
		{
			ax = absx + hrange * cos(highang);
			ay = absy + hrange * sin(highang);
			if (Diophantine_clear_path(gstruct, absx, absy, ax, ay))
			{
				not_done = 0;
				turn_turret(0, highang);
				trans = Diophantine_abs_ang(gstruct, ta, highang);
				if (trans < 0.6)
				{
					Diophantine_fire(gstruct, hrange);
				}
				gstruct->D_lastshotat = hid;
				gstruct->D_shootcounter = 5;
			}
			ohighqual = highqual;
		}
	}
	while (not_done);
end:
	dbg1("Diophantine_shoot_suckers()");
}


static Diophantine_fire(gstruct, range)
struct g_struct *gstruct;
int range;
{
	int ctri, ctrj;

	dbg("Diophantine_fire()");

	if (range > gstruct->max_weap_range)
		goto end;

	if (range + heat() < 300)
	{
		for (ctri = 0; ctri < gstruct->num_weap; ctri++)
		{
			if (range < gstruct->weap_range[ctri])
				fire_weapon(ctri);
			else
			{
				for (ctrj = gstruct->num_weap - (random() & 1); ctrj > 0; ctrj -= 2)
				{
					if (range < gstruct->weap_range[ctrj])
						fire_weapon(ctrj);
				}
			}
		}
	}
end:
	dbg1("Diophantine_fire()");
}


static Diophantine_clear_path(gstruct, start_x, start_y, finish_x, finish_y)
struct g_struct *gstruct;
int start_x, start_y, finish_x, finish_y;
{
	int dx, dy, lattice_dx, lattice_dy;
	int tgrid_x, tgrid_y, fgrid_x, fgrid_y;
	int ret;

	dbg("Diophantine_clear_path()");

	/* Compute absolute x coordinate in maze */
	/* Computed x and y differences from start to finish */
	dx = finish_x - start_x;
	dy = finish_y - start_y;

	/* Set up temporary and final box coordinates */
	tgrid_x = start_x / BOX_WIDTH;
	tgrid_y = start_y / BOX_HEIGHT;
	fgrid_x = finish_x / BOX_WIDTH;
	fgrid_y = finish_y / BOX_HEIGHT;

	/* if start and finish are in the same box, no wall is intersected */
	if (tgrid_x == fgrid_x && tgrid_y == fgrid_y)
		RETURN(1);

	/* Figure out the general direction that the line is travelling in * so
	   that we can write specific code for each case. *
	
	In the NE, SE, NW, and SW cases, * lattice_dx and lattice_dy are the
	   deltas from the starting * location to the lattice point that the path
	   is heading towards. * The slope of the line is compared to the slope
	   to the lattice point * This determines which wall the path intersects. *
	   Instead of comparing dx/dy with lattice_dx/lattice_dy, I multiply *
	   both sides by dy * lattice_dy, which lets me do 2 multiplies instead *
	   of 2 divides. */
	if (fgrid_x > tgrid_x)
		if (fgrid_y > tgrid_y)
		{						/* Southeast */
			lattice_dx = (tgrid_x + 1) * BOX_WIDTH - start_x;
			lattice_dy = (tgrid_y + 1) * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
			{
				if (lattice_dy * dx < dy * lattice_dx)
				{
					if (wall(SOUTH, tgrid_x, tgrid_y))
						RETURN(0);
					tgrid_y++;
					lattice_dy += BOX_HEIGHT;
				}
				else
				{
					if (wall(EAST, tgrid_x, tgrid_y))
						RETURN(0);
					tgrid_x++;
					lattice_dx += BOX_WIDTH;
				}
			}
		}
		else if (fgrid_y < tgrid_y)
		{						/* Northeast */
			lattice_dx = (tgrid_x + 1) * BOX_WIDTH - start_x;
			lattice_dy = tgrid_y * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
			{
				if (lattice_dy * dx > dy * lattice_dx)
				{
					if (wall(NORTH, tgrid_x, tgrid_y))
						RETURN(0);
					tgrid_y--;
					lattice_dy -= BOX_HEIGHT;
				}
				else
				{
					if (wall(EAST, tgrid_x, tgrid_y))
						RETURN(0);
					tgrid_x++;
					lattice_dx += BOX_WIDTH;
				}
			}
		}
		else
		{						/* East */
			for (; tgrid_x < fgrid_x; tgrid_x++)
				if (wall(EAST, tgrid_x, tgrid_y))
					RETURN(0);
		}

	else if (fgrid_x < tgrid_x)
		if (fgrid_y > tgrid_y)
		{						/* Southwest */
			lattice_dx = tgrid_x * BOX_WIDTH - start_x;
			lattice_dy = (tgrid_y + 1) * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
			{
				if (lattice_dy * dx > dy * lattice_dx)
				{
					if (wall(SOUTH, tgrid_x, tgrid_y))
						RETURN(0);
					tgrid_y++;
					lattice_dy += BOX_HEIGHT;
				}
				else
				{
					if (wall(WEST, tgrid_x, tgrid_y))
						RETURN(0);
					tgrid_x--;
					lattice_dx -= BOX_WIDTH;
				}
			}
		}
		else if (fgrid_y < tgrid_y)
		{						/* Northwest */
			lattice_dx = tgrid_x * BOX_WIDTH - start_x;
			lattice_dy = tgrid_y * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
			{
				if (lattice_dy * dx < dy * lattice_dx)
				{
					if (wall(NORTH, tgrid_x, tgrid_y))
						RETURN(0);
					tgrid_y--;
					lattice_dy -= BOX_HEIGHT;
				}
				else
				{
					if (wall(WEST, tgrid_x, tgrid_y))
						RETURN(0);
					tgrid_x--;
					lattice_dx -= BOX_WIDTH;
				}
			}
		}
		else
		{						/* West */
			for (; tgrid_x > fgrid_x; tgrid_x--)
				if (wall(WEST, tgrid_x, tgrid_y))
					RETURN(0);
		}

	else if (fgrid_y > tgrid_y)
	{							/* South */
		for (; tgrid_y < fgrid_y; tgrid_y++)
			if (wall(SOUTH, tgrid_x, tgrid_y))
				RETURN(0);
	}
	else if (fgrid_y < tgrid_y)
	{							/* North */
		for (; tgrid_y > fgrid_y; tgrid_y--)
			if (wall(NORTH, tgrid_x, tgrid_y))
				RETURN(0);
	}
	ret = 1;
end:
	dbg1("Diophantine_clear_path()");
	return (ret);
}


static Diophantine_abs_ang(gstruct, a, b)
struct g_struct *gstruct;
float a, b;
{
	float t;

	dbg("Diophantine_abs_ang()");

	a -= floor(a / (2 * PI)) * (2 * PI);
	b -= floor(b / (2 * PI)) * (2 * PI);

	t = a - b;
	if (t < 0.0)
		t = -t;

	if (t > PI)
		t = 2 * PI - t;

	dbg1("Diophantine_abs_ang()");
	return (t);
}

static int indent = -1;

static indbg(string)
char *string;
{
	int ctri;

	indent++;
	fprintf(stderr, "\n");
	for (ctri = 0; ctri < indent; ctri++)
		fprintf(stderr, ".");
	fprintf(stderr, "in  %s", string);
}

static outdbg(string)
char *string;
{
	int ctri;

	fprintf(stderr, "\n");
	for (ctri = 0; ctri < indent; ctri++)
		fprintf(stderr, ".");
	fprintf(stderr, "out %s", string);
	indent--;
}
