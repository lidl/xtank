/* jeff's first robot.  It doesn't move, it talks, it eats your mushrooms. */

/*****************************************************************************
 Copyright (c) 1994 Jeff Snider Enterprizes
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. All advertising materials mentioning features or use of this software
    must display the following acknowledgement:
      This product includes software developed by Jeff Snider.
 4. The names Bob Stratton or Nat Howard may not be used to endorse or 
	promote products derived from this software without specific prior 
	written permission.
 5. Listings of this program may not be used as toilet paper or to contain 
	mucus of any kind.

 THIS SOFTWARE IS PROVIDED BY THE Jeff Snider ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE Jeff Snider BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

*****************************************************************************/

#include <xtanklib.h>
#include <tanktypes.h>
#include <lowlib.h>
#include <stdio.h>
#include <math.h>

#define DEBUG 0

#define SMART_SPACE			20
#define SMART_MIN_DIST		120

#define SMART_TIME			30.0 
/* this is just a sometimes-useful approximation */

#define MAX_MESSAGE_AGE		20
#define SPEED_BIAS			128
/* tagman uses this, so I do for compatibility */

#define ENEMY_HOW_OFTEN		4
#define OPEN_HOW_OFTEN		50
#define DELAY_OPEN_MESSAGE	8
#define CHECK_FOR_OUTPOSTS	30
#define HOW_MANY_OUTPOSTS	5
#define ANGLE_SLOP			1.1  /* this is a toughie to pick    */
                                 /* could use a better algorithm */
								 /* should depend on ammo left   */
								 /* and other crap               */

#define MAX_WEAPON_SPEED	61
/* currently the pulse laser.  Is this defined somewhere else? */

#define MY_MAX_ARRAY		(2 * MAX_VEHICLES)

static void main();

Prog_desc mmtf_prog = {
	"mmtf",
	"Jefticle",
	"Spins to protect armor, shoots, talks to it's friends.",
	"Jeff Snider",
	PLAYS_COMBAT,
	7,
	main
};

/* heh.  heh heh.  LOTS of global variables and malloced-once memory */
/* You don't like it?  Bite me.  Or rewrite it, but have fun...      */
typedef struct {
	Settings_info settings;
	Vehicle_info me;
	Weapon_info weapon[MAX_WEAPONS];
	double safe_dist[MAX_WEAPONS];
	Location myloc;

	/* things keyed on id require MY_MAX_ARRAY */
	double enemy_dx[MY_MAX_ARRAY];
	double enemy_dy[MY_MAX_ARRAY];
	double enemy_dist[MY_MAX_ARRAY];
	double enemy_xspd[MY_MAX_ARRAY];
	double enemy_yspd[MY_MAX_ARRAY];
	double enemy_xacc[MY_MAX_ARRAY];
	double enemy_yacc[MY_MAX_ARRAY];
	int enemy_frame[MY_MAX_ARRAY];
	int enemy_time[MY_MAX_ARRAY];
	Boolean have_calc[MY_MAX_ARRAY][MAX_WEAPONS];
	int smartdx[MY_MAX_ARRAY];
	int smartdy[MY_MAX_ARRAY];
	double smartdist[MY_MAX_ARRAY];
	double tdx[MY_MAX_ARRAY][MAX_WEAPONS];
	double tdy[MY_MAX_ARRAY][MAX_WEAPONS];
	double tdist[MY_MAX_ARRAY][MAX_WEAPONS];
	int ttime[MY_MAX_ARRAY][MAX_WEAPONS];

	double priority[MY_MAX_ARRAY]; /* priority value of target. key on id */
	int hitme[MAX_VEHICLES];       /* list of id's, in order of priority */
	int id2num[MY_MAX_ARRAY];      /* list of num in vehicle list. key on id */
	Vehicle_info vehicle_list[MAX_VEHICLES];
	Boolean inview[MY_MAX_ARRAY];  /* is the vehicle on the screen? key on id */
	int num_vehicles;
	int num_targets;
	Vehicle_info closest_vehicle;  /* crufty old stupid dumb crippled */

	int speed2idx[MAX_WEAPON_SPEED];
	int num_weap_speeds;
	int each_speed[MAX_WEAPONS];
	int max_calc_range[MAX_WEAPONS];
	int calc;
	int num_weap;
	int weap2tur[MAX_WEAPONS];
	int frame;
	int last_smart_shot;
	int next_enemy_msg;
	int last_open_msg;
	int min_armor_allow;
	int nobody_around;
	double my_abs_drive;
	Boolean need_new_lock;
	Boolean have_lock;
	Boolean enemy_around;
	Boolean have_smart_weapon;
	int more_outposts;
	int num_outposts;
} All;


/*  WELCOME TO THE WORLD OF SHITHOLE DEBUGGING!    *
 *  This is the sewer below all those nice places  *
 *  GDB or UPS might let us live.  But do those    *
 *  SCUM SUCKING SLUG LICKING MAGGOT INFESTED      *
 *  xtank code maintainers come up with a way      *
 *  for us to use them?  Nope.  So, we live        *
 *  IN THE STINKING SEWER.  enjoy.                 */

#if DEBUG > 0
#define l1debug_printf(s) printf("%d %d: ",allp->me.id,allp->frame); printf s
#else
#define l1debug_printf(s)
#endif

#if DEBUG > 1
#define l2debug_printf(s) printf("%d %d: ",allp->me.id,allp->frame); printf s
#else
#define l2debug_printf(s)
#endif

#if DEBUG > 2
#define l3debug_printf(s) printf("%d %d: ",allp->me.id,allp->frame); printf s
#else
#define l3debug_printf(s)
#endif

#if DEBUG > 3
#define l4debug_printf(s) printf("%d %d: ",allp->me.id,allp->frame); printf s
#else
#define l4debug_printf(s)
#endif

#if DEBUG > 4
#define l5debug_printf(s) printf("%d %d: ",allp->me.id,allp->frame); printf s
#else
#define l5debug_printf(s)
#endif

static void cleanup(allp)
	All *allp;
{
	free((char *)allp);
}

/* redundant, but right now clear_path doesn't work  8-(  */
static Boolean my_clear_path(dest, x0, y0, x1, y1, allp)
	Boolean dest;
	int x0, y0, x1, y1;
	All *allp;
{
	int vx, vy, grid_x, grid_y, end_grid_x, end_grid_y;
	int xadd, yadd, dxbox, dybox, dx, dy, lower_x, lower_y;
	double ydx, yhit;
	WallSide xwall, ywall;
	WallType walltype;

	l2debug_printf(("in my_clear_path\n"));
	vx = x1 - x0;
	vy = y1 - y0;
	grid_x = x0 / BOX_WIDTH;
	l2debug_printf(("x0: %d BW: %d grid_x: %d\n",x0,BOX_WIDTH,grid_x));
	grid_y = y0 / BOX_HEIGHT;
	l2debug_printf(("y0: %d BH: %d grid_y: %d\n",y0,BOX_HEIGHT,grid_y));
	end_grid_x = x1 / BOX_WIDTH;
	l2debug_printf(("x1: %d BW: %d grid_x: %d\n",x1,BOX_WIDTH,end_grid_x));
	end_grid_y = y1 / BOX_HEIGHT;
	l2debug_printf(("y1: %d BH: %d grid_y: %d\n",y1,BOX_HEIGHT,end_grid_y));
	/* Happy Happy trivial cases: */
	if (grid_x == end_grid_x) {
		if (grid_y == end_grid_y) {
			l2debug_printf(("begin == end\n"));
			return TRUE;
		} else {
			l2debug_printf(("straight vertical line case\n"));
			if (grid_y > end_grid_y) {
				end_grid_x = grid_y;
				grid_y = end_grid_y;
				end_grid_y = end_grid_x;
			}
			for ( ; grid_y < end_grid_y; grid_y++) {
				walltype = wall(SOUTH,grid_x,grid_y);
				if (walltype == MAP_WALL || !dest && walltype == MAP_DEST) {
					return FALSE;
				}
			}
			return TRUE;
		}
	} else {
		if (grid_y == end_grid_y) {
			l2debug_printf(("straight horizontal line case\n"));
			if (grid_x > end_grid_x) {
				end_grid_y = grid_x;
				grid_x = end_grid_x;
				end_grid_x = end_grid_y;
			}
			for ( ; grid_x < end_grid_x; grid_x++) {
				walltype = wall(EAST,grid_x,grid_y);
				if (walltype == MAP_WALL || !dest && walltype == MAP_DEST) {
					return FALSE;
				}
			}
			return TRUE;
		}
	}
	ydx = (double)vy / (double)vx;
	/* two "if"s now so you don't do them in each loop in the while */
	if (vy > 0) {
		ywall = SOUTH;
		dy = 1;
		yadd = BOX_HEIGHT;
		dybox = BOX_HEIGHT;
	} else {
		ywall = NORTH;
		dy = -1;
		yadd = 0;
		dybox = -BOX_HEIGHT;
	}
	if (vx > 0) {
		xwall = EAST;
		dx = 1;
		xadd = BOX_WIDTH - x0;
		dxbox = BOX_WIDTH;
	} else {
		xwall = WEST;
		dx = -1;
		xadd = -x0;
		dxbox = -BOX_WIDTH;
	}

	lower_x = grid_x * BOX_WIDTH;
	lower_y = grid_y * BOX_HEIGHT;

	while (grid_x != end_grid_x || end_grid_y != grid_y
		&& grid_x > 0 && grid_y > 0 && grid_x < GRID_WIDTH
		&& grid_y < GRID_HEIGHT
		) {
		l2debug_printf(("at %d %d\n",grid_x,grid_y));
		yhit = ydx * (double)(lower_x + xadd) + (double)y0;
		l2debug_printf(("yhit: %f\n",yhit));
		if (yhit > (lower_y + BOX_HEIGHT) || yhit < lower_y) {
			walltype = wall(ywall,grid_x,grid_y);
			if (walltype == MAP_WALL || !dest && walltype == MAP_DEST) {
				return FALSE;
			} else {
				grid_y += dy;
				lower_y += dybox;
			}
		} else {
			walltype = wall(xwall,grid_x,grid_y);
			if (walltype == MAP_WALL || !dest && walltype == MAP_DEST) {
				return FALSE;
			} else {
				grid_x += dx;
				lower_x += dxbox;
			}
		}
	}
	return TRUE;
}

static void insert_hitme(id, allp)
	int id;
	All *allp;
{
	int i, j;

	if (team(id) == allp->me.team && allp->me.team != 0) {
		l2debug_printf(("not inserting: id %d  team %d\n",id,team(id)));
		return;
	}
	l2debug_printf(("inserting: id %d priority %f dist: %f\n", id,
		allp->priority[id], allp->enemy_dist[id]));
	for (i = 0; i < allp->num_vehicles; i++) {
		l2debug_printf(("position %d: id: %d priority %f dist: %f\n", i,
			allp->hitme[i], allp->priority[allp->hitme[i]],
			allp->enemy_dist[allp->hitme[i]]));
		if (   allp->hitme[i] == -1
			|| allp->priority[id] > allp->priority[allp->hitme[i]]
			|| (allp->priority[id] == allp->priority[allp->hitme[i]]
			&& allp->enemy_dist[id] >= allp->enemy_dist[allp->hitme[i]])
			) {
			for (j = allp->num_vehicles - 1; j > i; j--) {
				allp->hitme[j] = allp->hitme[j-1];
			}
			l2debug_printf(("being put in position %d\n",i));
			allp->hitme[i] = id;
			allp->num_targets++;
			return;
		}
	}
	l2debug_printf(("NOT INSERTED!\n"));
}

static int w2t(mnt)
	MountLocation mnt;
{
	switch (mnt) {
		case MOUNT_TURRET1:
			return 0;
			break;
		case MOUNT_TURRET2:
			return 1;
			break;
		case MOUNT_TURRET3:
			return 2;
			break;
		case MOUNT_TURRET4:
			return 3;
			break;
		default:
			return -1;
			break;
	}
}

static int angle2side(angle)
	double angle;
{
	double foo;
	foo = fixed_angle(angle + 0.78539816);
	if (foo < PI) {
		if (foo < 1.5707963) {
			return FRONT;
		} else {
			return RIGHT;
		}
	} else {
		if (foo < 4.712389) {
			return BACK;
		} else {
			return LEFT;
		}
	}
}

static double side2angle(side)
	int side;
{
	switch (side) {
		case FRONT:
			return 0.0;
			break;
		case RIGHT:
			return 1.5707963;
			break;
		case BACK:
			return PI;
			break;
		case LEFT:
			return 4.712389;
			break;
		default:
			return 0.0;
			break;
	}
}

/* bite me if you think tagman's frantic code is better */
static void protect_armor(allp)
	All *allp;
{
	int dx, dy, time, most, second, current, opposite;
	double turnto, angle2enemy, side;
	FLOAT dabs;

	dabs = get_abs_drive();
	l2debug_printf(("dabs: %f %d\n",dabs,dabs));
	dabs -= allp->my_abs_drive;
	dabs = dabs > 0 ? dabs : -dabs;
	if (dabs > 3.0) {
		l2debug_printf(("Hey! Somebody's drivin! (%f)\n",dabs));
		return;
	} else {
		l2debug_printf(("Nobody drivin (%f)\n",dabs));
	}
	dx = allp->closest_vehicle.loc.x - allp->myloc.x;
	dy = allp->closest_vehicle.loc.y - allp->myloc.y;
	/* OK, OK, so this is a touch anal... */
	if (allp->settings.rel_shoot) {
		if (allp->closest_vehicle.xspeed != 0.0
			|| allp->closest_vehicle.yspeed != 0.0
			|| allp->me.xspeed != 0.0 || allp->me.yspeed != 0.0
			) {
			time = sqrt(dx * dx + dy * dy) / 17;
			dx = allp->closest_vehicle.loc.x + (allp->closest_vehicle.xspeed - allp->me.xspeed) * time - allp->myloc.x;
			dy = allp->closest_vehicle.loc.y + (allp->closest_vehicle.yspeed - allp->me.xspeed) * time - allp->myloc.y;
		}
	} else {
		if (allp->closest_vehicle.xspeed != 0.0
			|| allp->closest_vehicle.yspeed != 0.0
			) {
			time = sqrt(dx * dx + dy * dy) / 17;
			dx = allp->closest_vehicle.loc.x
				+ allp->closest_vehicle.xspeed * time - allp->myloc.x;
			dy = allp->closest_vehicle.loc.y
				+ allp->closest_vehicle.yspeed * time - allp->myloc.y;
		}
	}
	angle2enemy = ATAN2(dy,dx);
	side = angle2enemy - allp->me.heading;
	current = angle2side(side);
	opposite = angle2side(side + PI);

	if (armor(FRONT) > armor(BACK)) {
		most = FRONT;
		second = BACK;
	} else {
		most = BACK;
		second = FRONT;
	}
	/* dx and dy are reused just to save on variables */
	if (armor(RIGHT) > armor(LEFT)) {
		dx = RIGHT;
		dy = LEFT;
	} else {
		dx = LEFT;
		dy = RIGHT;
	}
	if (armor(most) < armor(dx)) {
		second = most;
		most = dx;
		if (armor(second) < armor(dy)) {
			second = dy;
		}
	} else {
		if (armor(second) < armor(dx)) {
			second = dx;
		}
	}

	if (most != opposite) {
		turnto = side2angle(most);
	} else {
		turnto = side2angle(second);
	}

	turn_vehicle((angle2enemy - turnto));
}

static void target_bastard_enemies(allp)
	All *allp;
{
	int id, i, j;

	for (i = 0; i < MY_MAX_ARRAY; i++) {
		allp->hitme[i] = -1;
		allp->enemy_dist[i] = -2000000.0;
		allp->smartdist[i] = -1.0;
		l5debug_printf(("priority: before: %f\n",allp->priority[i]));
		allp->priority[i] = (allp->priority[i] > 0.1) ?  (0.99 * (allp->priority[i] - 0.1)) : 0.0;
		l5debug_printf(("priority: after: %f\n",allp->priority[i]));
		allp->inview[i] = FALSE;
	}
	if (!get_closest_enemy(&(allp->closest_vehicle))) {
		allp->closest_vehicle.id = -1;
	}
	l2debug_printf(("closest enemy id: %d\n",allp->closest_vehicle.id));
	get_vehicles(&(allp->num_vehicles),allp->vehicle_list);
	allp->num_targets = 0;
	l2debug_printf(("%d vehicles\n",allp->num_vehicles));
	for (i = 0; i < allp->num_vehicles; i++) {
		id = allp->vehicle_list[i].id;
		allp->enemy_dx[id] = (double)(allp->vehicle_list[i].loc.x - allp->myloc.x);
		allp->enemy_dy[id] = (double)(allp->vehicle_list[i].loc.y - allp->myloc.y);
		allp->enemy_dist[id] = -sqrt(allp->enemy_dx[id] * allp->enemy_dx[id] + allp->enemy_dy[id] * allp->enemy_dy[id]);
		l2debug_printf(("inserting vehicle %d in hitme\n",i));
		insert_hitme(allp->vehicle_list[i].id,allp);
		allp->id2num[allp->vehicle_list[i].id] = i;
		allp->inview[allp->vehicle_list[i].id] = TRUE;
	}
	l2debug_printf(("%d targets\n",allp->num_targets));
}

static void setup_weapon_arrays(allp)
	All *allp;
{
	Boolean already_in;
	int i, j;

	allp->num_weap = num_weapons();
	allp->num_weap_speeds = 0;
	for (i = 0; i< allp->num_weap; i++) {
		get_weapon(i, &(allp->weapon[i]));
		allp->safe_dist[i] = (double)(allp->weapon[i].safety * allp->weapon[i].ammo_speed);
		already_in = FALSE;
		if (allp->weapon[i].type == MORTAR || allp->weapon[i].type == HARM) {
			allp->weap2tur[i] = -2;
			allp->have_smart_weapon = TRUE;
			continue;
		}
		for (j = 0; j < allp->num_weap_speeds; j++) {
			if (allp->each_speed[j] == (double)allp->weapon[i].ammo_speed) {
				already_in = TRUE;
				if (allp->max_calc_range[j] < allp->weapon[i].range) {
					allp->max_calc_range[j] = allp->weapon[i].range;
				}
			}
		}
		if (!already_in) {
			l2debug_printf(("weap %d: speed %d idx %d\n",i,allp->weapon[i].ammo_speed,allp->num_weap_speeds));
			allp->each_speed[allp->num_weap_speeds] = (double)allp->weapon[i].ammo_speed;
			allp->max_calc_range[allp->num_weap_speeds] = allp->weapon[i].range;
			allp->speed2idx[allp->weapon[i].ammo_speed] = allp->num_weap_speeds;
			allp->num_weap_speeds++;
		}
		if IS_TURRET(allp->weapon[i].mount) {
			allp->weap2tur[i] = w2t(allp->weapon[i].mount);
		} else {
			allp->weap2tur[i] = -1;
		}
	}
/* this value needs to be reviewed (lowered?) under multi-targeting */
	if (allp->num_weap_speeds == 0) {
		allp->calc = 0;
	} else {
		allp->calc = (int)(12.0 / (double)allp->num_weap_speeds + 0.7);
	}
	l2debug_printf(("calc value: %d\n",allp->calc));
	for (i = 0; i < MY_MAX_ARRAY; i++) {
		allp->enemy_frame[i] = allp->frame - 100;
		allp->priority[i] = 0.0;
		allp->smartdist[i] = -1.0;
	}
}

static void independance(num, id, idx, weap, allp)
	int num;
	int id;
	int idx;
	int weap;
	All *allp;
{
	int j, x, y, time, ntime, ltime;
	int dist, bx, by;

	l2debug_printf(("in outpost calc_aim (independace) (%d %d)\n",
		weap, allp->weapon[weap].ammo_speed));
	j = 0;
	bx = allp->vehicle_list[num].loc.grid_x;
	by = allp->vehicle_list[num].loc.grid_y;
	ntime = (int)allp->tdist[id][idx] / allp->weapon[weap].ammo_speed
		+ allp->frame;
	ltime = -1;
	time = 0;
	while (ntime != time && j < 6 && ltime != ntime) {
		j++;
		l2debug_printf(("iteration %d, ntime %d\n", j, ntime));
		ltime = time;
		time = ntime;
		get_outpost_loc(bx, by, time, &x, &y);
		x = x - allp->myloc.x;
		y = y - allp->myloc.y;
		if (allp->settings.rel_shoot) {
			x -= (int)(allp->me.xspeed * (double)time);
			y -= (int)(allp->me.yspeed * (double)time);
		}
		dist = (int)sqrt((double)(x*x + y*y));
		ntime = dist / allp->weapon[weap].ammo_speed + allp->frame;
	}
	allp->tdist[id][idx] = (double)dist;
	allp->tdx[id][idx] = (double)x;
	allp->tdy[id][idx] = (double)y;
	l2debug_printf(("dist: %d dx: %d dy: %d\n", dist, x, y));
}

static Boolean new_calc_aim(num, weap, allp)
	int num;  /* number of enemy in vehicle_list */
	int weap; /* which of weapon */
	All *allp;
{
	int idx, j, id;
	int end_x, end_y;
	double special_long_range;
	double ax, ay, vx, vy, x, y, S, A, B, C, D, E, B3, lerr;
	double t0, t1, t2, P0, P1, P2, err, firstguess, firsterr, tmax;
	Location targetloc;

	id = allp->vehicle_list[num].id;
	l2debug_printf(("made it into calc_aim, calc value: %d\n",allp->calc));
	if (allp->weapon[weap].type == MORTAR || allp->weapon[weap].type == HARM) {
		l2debug_printf(("is a special weapon\n"));
		if (allp->smartdist[id] > 0.0) {
			l2debug_printf(("already did smart calc, had solution\n"));
		} else {
			l2debug_printf(("doing smart weapon calcuation\n"));
			if (allp->vehicle_list[num].team == MAX_VEHICLES) {
				get_outpost_loc(allp->vehicle_list[num].loc.grid_x,
					allp->vehicle_list[num].loc.grid_y,
					allp->frame + allp->weapon[weap].frames,
					&(end_x), &(end_y));
				allp->smartdx[id] = (double)(end_x - allp->myloc.x);
				allp->smartdy[id] = (double)(end_y - allp->myloc.y);
			} else {
				allp->smartdx[id] = (int)(allp->enemy_dx[id]
					+ SMART_TIME * allp->enemy_xspd[id]);
				allp->smartdy[id] = (int)(allp->enemy_dy[id]
					+ SMART_TIME * allp->enemy_yspd[id]);
			}
			allp->smartdist[id] =
				sqrt((double)(allp->smartdx[id] * allp->smartdx[id]
				+ allp->smartdy[id] * allp->smartdy[id]));
			l2debug_printf(("done with smart weapon calcuation\n"));
		}
		return TRUE;
	}
	l2debug_printf(("is NOT a special weapon\n"));

	idx = allp->speed2idx[allp->weapon[weap].ammo_speed];
/* XXX This should be redundant: please make it so */
	if (num == -1 || (team(id) == allp->me.team && allp->me.team != 0)) {
		l1debug_printf(("BAD TARGET: num: %d team: %d\n",num,team(id)));
		return FALSE;
	}
	l2debug_printf(("enemy %d wspd %d idx %d id %d\n",num,allp->weapon[weap].ammo_speed,idx,id));
	if (id == -1) {
		l1debug_printf(("id == -1 !!!!! THIS SUCKS!!\n"));
		return FALSE;
	}

	if (allp->have_calc[id][idx] == allp->frame) {
		if (allp->tdist[id][idx] == 0.0) { 
			l2debug_printf(("already found no solution\n"));
			return FALSE;
		} else {
			l2debug_printf(("already had solution\n"));
			return TRUE;
		}
	}
	allp->have_calc[id][idx] = allp->frame;
	if (allp->enemy_dist[id] < 0.0) {
		allp->enemy_time[id] = allp->frame - allp->enemy_frame[id];
		allp->enemy_frame[id] = allp->frame;
		if (allp->enemy_time[id] > 8) {
			allp->enemy_xacc[id] = 0.0;
			allp->enemy_yacc[id] = 0.0;
		} else {
			allp->enemy_xacc[id] = (allp->vehicle_list[num].xspeed
				- allp->enemy_xspd[id]) / allp->enemy_time[id];
			allp->enemy_yacc[id] = (allp->vehicle_list[num].yspeed
				- allp->enemy_yspd[id]) / allp->enemy_time[id];
		}
		allp->enemy_dist[id] = -allp->enemy_dist[id];
		allp->enemy_xspd[id] = allp->vehicle_list[num].xspeed;
		allp->enemy_yspd[id] = allp->vehicle_list[num].yspeed;
		l2debug_printf(("init: %f %f, %f away / spd: x %f y %f\n",
			allp->enemy_dx[id], allp->enemy_dy[id], allp->enemy_dist[id],
			allp->vehicle_list[num].xspeed, allp->vehicle_list[num].yspeed));
	} else {
		l2debug_printf(("already did init (dist %f)\n",allp->enemy_dist[id]));
	}

	allp->tdist[id][idx] = allp->enemy_dist[id];
	l2debug_printf(("before iterations (dist %f)\n", allp->tdist[id][idx]));
	special_long_range = allp->max_calc_range[idx] * 1.5;
	if (allp->vehicle_list[num].team == MAX_VEHICLES) {
		independance(num,id,idx,weap,allp);
		l2debug_printf(("out of independace\n"));
		return TRUE;
	}
	ax = 0.5 * allp->enemy_xacc[id];
	ay = 0.5 * allp->enemy_yacc[id];
	vx = allp->enemy_xspd[id];
	vy = allp->enemy_yspd[id];
	if (allp->settings.rel_shoot) {
		vx -= allp->me.xspeed;
		vy -= allp->me.yspeed;
	}
	x = allp->enemy_dx[id];
	y = allp->enemy_dy[id];
	S = (double)allp->weapon[weap].ammo_speed;
	tmax = 1.15 * (double)allp->max_calc_range[idx] / S;
	l2debug_printf(("tmax: %f\n",tmax));
	A = ax * ax + ay * ay;
	B = 2.0 * (vx * ax + vy * ay);
	C = x * ax + vx * vx + y * ay + vy * vy - S * S;
	D = 2.0 * (x * vx + y * vy);
	E = x * x + y * y;
	l2debug_printf(("P4 coefficients: %f %f %f %f %f\n",A,B,C,D,E));
	if (A == 0.0 && B == 0.0) {
		l2debug_printf(("Using quickie because A and B == 0\n"));
		A = D * D - 4.0 * C * E;
		l2debug_printf(("new A = %f\n",A));
		if (A < 0.0) {
			l2debug_printf(("No Roots!\n"));
			allp->tdist[id][idx] = 0.0;
			return FALSE;
		}
		t1 = ( - D - sqrt( A ) ) / ( 2.0 * C );
		l2debug_printf(("lower root: t = %f\n",t1));
		if (t1 <= 0.0 && A != 0.0) {
			t1 = ( - D + sqrt( A ) ) / ( 2.0 * C );
			l2debug_printf(("upper root: t = %f\n",t1));
		}
		if (t1 < 0.0) {
			l2debug_printf(("No solution in range\n"));
			allp->tdist[id][idx] = 0.0;
			return FALSE;
		}
	} else {
#define P4(t) (((t * A + B) * t + C) * t + D) * t + E
		B3 = 3.0 * B;
		lerr = err = 2.0;
		j = 0;
		t0 = 0.0;
		P0 = P4(t0);
		t1 = 7.0;
		while ((err > 0.5 || lerr > 0.5) && j < allp->calc && t1 >= 0.0) {
			j++;
			P1 = P4(t1);
			if (P1 == P0) { break; }
			t2 = t1 - P1 * ( t1 - t0 ) / ( P1 - P0 );
			lerr = err;
			err = t2 - t1;
			err = err > 0 ? err : -err;
			l2debug_printf(("t0: %f t1: %f t2: %f err: %f P0: %f P1: %f\n",
				t0, t1, t2, err, P0, P1));
			if (t0 > tmax && t1 > tmax && (t2 > t1 || (t2 - err) > tmax)) {
				l2debug_printf(("Bailing! will try root of derivative\n"));
				err = 100.0;
				break;
			}
			t0 = t1;
			P0 = P1;
			t1 = t2;
		}
		if (err > 0.5) {
			l2debug_printf(("trying from dy/dy2 == 0\n"));
			firstguess = t1;
			firsterr = err;
			B3 = 3.0 * B;
			t1 = B3 * B3 - 24.0 * A * C;
			t1 = t1 < 0 ? -t1 : t1;
			t0 = (-B3 + sqrt(t1)) / ( 12.0 * A );
			P0 = P4(t0);
			t1 = t0 + 7.0;
			j = 0;
			lerr = err = 2.0;
			while ((err > 0.5 || lerr > 0.5) && j < allp->calc && t1 >= 0.0) {
				j++;
				P1 = P4(t1);
				if (P1 == P0) {
					l2debug_printf(("P1 == P0, this sucks\n"));
					break;
				}
				t2 = t1 - P1 * ( t1 - t0 ) / ( P1 - P0 );    
				lerr = err;
				err = t2 - t1;
				err = err > 0 ? err : -err;
				l2debug_printf(("t0: %f t1: %f t2: %f err: %f P0: %f P1: %f\n",t0,t1,t2,err,P0,P1));
				if (t0 > tmax && t1 > tmax && (t2 > t1 || (t2 - err) > tmax)) {
					l2debug_printf(("Bailing! NO ROOT FOUND\n"));
					err = 100.0;
					break;
				}
				t0 = t1;
				P0 = P1;
				t1 = t2;
			}
			if (err > firsterr) { /* choose the closer of the two tries */
				t1 = firstguess;
				err = firsterr;
			}
/* in a perfect world... */
#if 0
			if (err > 6.0) { /* selecting some reasonable margin is difficult */
				l2debug_printf(("Error too large (%f)\n",err));
				return FALSE;
			}
#endif
		}
	}
	l2debug_printf(("resultant t: %f\n",t1));
	if (t1 <= 0.0) {
		l2debug_printf(("t not in acceptable range\n"));
		allp->tdist[id][idx] = 0.0;
		return FALSE;
	}
	allp->ttime[id][idx] = (int)t1;
	allp->tdx[id][idx] = (ax * t1 + vx) * t1 + x;
	allp->tdy[id][idx] = (ay * t1 + vy) * t1 + y;
	allp->tdist[id][idx] = sqrt(allp->tdx[id][idx] * allp->tdx[id][idx]
		+ allp->tdy[id][idx] * allp->tdy[id][idx]);
	l2debug_printf(("done with calculation\n"));
	return TRUE;
}

static Boolean new_please_fire_weapon(weap, num, allp)
	int weap;
	int num; /* enemy num to kill, or -1 for best shot */
	All *allp;
{
	int pos, idx, id; /* position in hitme, weap speed idx, id */
	int turret_dx, turret_dy;
	int target_value[MY_MAX_ARRAY];
	double angle, angle1;
	Boolean did_fire, did_calc, in_range;

	l3debug_printf(("in please_fire_weapon\n"));
	idx = allp->speed2idx[allp->weapon[weap].ammo_speed];
	if (num == -1) {
		l3debug_printf(("looking for good target\n"));
		for (pos = 0; pos < allp->num_targets; pos++) {
			id = allp->hitme[pos];
			l2debug_printf(("Id: %d id2num: %d num2id: %d idx: %d\n",
				id, allp->id2num[id],
				allp->vehicle_list[allp->id2num[id]].id, idx));
			did_calc = new_calc_aim(allp->id2num[id],weap,allp);
			if (!did_calc) {
				l1debug_printf(("no solution for %d\n", id));
				target_value[pos] = 4;
				continue;
			}
			l3debug_printf(("safe_dist: %f  dist: %f\n",
				allp->safe_dist[weap], allp->tdist[id][idx]));
			in_range = (allp->tdist[id][idx] >= allp->safe_dist[weap])
				&& (allp->tdist[id][idx] <= allp->weapon[weap].range);
			l3debug_printf(("did first 1/2 of in_range (x %d y %d)\n",
				allp->myloc.x, allp->myloc.y));
			if (!in_range) {
				l1debug_printf(("too close/far to %d\n", id));
				target_value[pos] = 3;
				continue;
			}
			l3debug_printf(("doing clear_path...\n"));
			turret_position(allp->weap2tur[weap], &turret_dx, &turret_dy);
			in_range = my_clear_path(FALSE,
				allp->myloc.x + turret_dx, allp->myloc.y + turret_dy,
				allp->myloc.x + (int)allp->tdx[id][idx],
				allp->myloc.y + (int)allp->tdy[id][idx], allp);
			if (!in_range) {
				in_range = my_clear_path(TRUE,
					allp->myloc.x + turret_dx, allp->myloc.y + turret_dy,
					allp->myloc.x + (int)allp->tdx[id][idx],
					allp->myloc.y + (int)allp->tdy[id][idx], allp);
				if (!in_range) {
					l1debug_printf(("NCP to %d\n", id));
					target_value[pos] = 3; /* NCP at all */
					continue;
				}
				l1debug_printf(("path through dest walls to %d\n", id));
				target_value[pos] = 2; /* clear dest path */
				continue;
			}
#ifdef ANGLE_SLOP
			angle = ATAN2(allp->tdy[id][idx],allp->tdx[id][idx]);
			angle1 = turret_angle(allp->weap2tur[weap]);
			l2debug_printf(("enemy angle: %f tur angle: %f (dx: %f dy: %f)\n",
				angle, angle1, allp->tdx[id][idx], allp->tdy[id][idx]));
			angle = angle - angle1;
			angle = fixed_angle(angle);
			angle = (angle > PI) ? (PI + PI - angle) : angle;
			angle1 = ANGLE_SLOP * turret_turn_rate(allp->weap2tur[weap]);
			l1debug_printf(("final angle: %f  SLOP: %f\n", angle,
				angle1));
			in_range = angle < angle1;
			if (!in_range) {
				l1debug_printf(("have to turn long way to hit %d\n", id));
				target_value[pos] = 1; /* long turret turn clear path */
				continue;
			}
#endif
			l1debug_printf(("good clean path to shoot %d\n",id));
			target_value[pos] = 0;
			break;
		}
		if (!in_range) {
#ifdef ANGLE_SLOP
			l3debug_printf(("looking for no-good target to aim at\n"));
			for (pos = 0; pos < allp->num_targets; pos++) {
				if (target_value[pos] == 1) {
					l2debug_printf(("Selected target %d\n",id));
					in_range = TRUE;
					break;
				}
			}
#endif
			if (!in_range) {
				l3debug_printf(("(sigh) finding no-good target to aim at\n"));
				for (pos = 0; pos < allp->num_targets; pos++) {
					if (target_value[pos] == 2) {
						l2debug_printf(("Selected target %d\n",id));
						target_value[pos] = 0;
						in_range = TRUE;
						break;
					}
				}
			}
			if (!in_range) {
				l3debug_printf(("finding really-no-good target to aim at\n"));
				for (pos = 0; pos < allp->num_targets; pos++) {
					if (target_value[pos] == 3) {
						l2debug_printf(("Selected target %d\n",id));
						in_range = TRUE;
						break;
					}
				}
			}
		}
	} else {
		id = allp->vehicle_list[num].id;
		l2debug_printf(("trying for target target %d\n",id));
		in_range = new_calc_aim(num,weap,allp);
		/* XXX check for range and stuff                          */
		/* right now, this never gets called this way, so it's OK */
	}
	if (!in_range) {
		l2debug_printf(("NO SOLUTION!\n"));
		return FALSE;
	}
	id = allp->hitme[pos];
	did_fire = FALSE;
	if (allp->weap2tur[weap] == -2) {
		if ((allp->last_smart_shot + SMART_SPACE <= allp->frame)
				&& !weapon_time(weap) && weapon_ammo(weap) 
				&& (allp->smartdist[id] >= SMART_MIN_DIST)
				&& (allp->smartdist[id] <= allp->weapon[weap].range)) {
/* XXX check map_off_grid(grid_x, grid_y) of target */
			aim_smart_weapon((int)(allp->myloc.x + allp->smartdx[id]),
				(int)(allp->myloc.y+allp->smartdy[id]));
			turn_on_weapon(weap);
			fire_weapon(weap);
			l2debug_printf(("FIRE! FIRE! FIRE!\n"));
			allp->last_smart_shot = allp->frame;
			did_fire = TRUE;
			allp->priority[id] += (double)allp->weapon[weap].damage;
			l2debug_printf(("add dam to priority: after: %f\n",
				allp->priority[id]));
		} else {
			l2debug_printf(("weapon not ready / target too close\n"));
			turn_off_weapon(weap);
		}
	} else {
		l2debug_printf(("shooting for id %d (dist %f) (wspd idx %d)\n",
			id, allp->tdist[id][idx], idx));
		aim_turret(allp->weap2tur[weap], (int)allp->tdx[id][idx],
			(int)allp->tdy[id][idx]);
		if (target_value[pos] == 0
			&& (weapon_time(weap) == 0) && weapon_ammo(weap)
			) {
			l1debug_printf(("FIRING! (%d)\n",weap));
			turn_on_weapon(weap);
			fire_weapon(weap);
			allp->priority[id] += (double)allp->weapon[weap].damage;
			l2debug_printf(("add dam to priority: after: %f\n",
				allp->priority[id]));
			did_fire = TRUE;
		} else {
			l1debug_printf(("weapon not ready and aimed\n"));
			turn_off_weapon(weap);
		}
	}
	return did_fire;
}

/* totally stolen from tagman.c
   Of course, this is just so that fine robot can
   benefit fron this robot's presence...
*/
static void VehicleInfo2Message(pstVehicle, pbBuffer)
	Vehicle_info *pstVehicle;
	Byte *pbBuffer;
{
	pbBuffer[0]  = (int) pstVehicle->loc.grid_x;
	pbBuffer[1]  = (int) pstVehicle->loc.grid_y;
	pbBuffer[2]  = (int) pstVehicle->loc.box_x;
	pbBuffer[3]  = (int) pstVehicle->loc.box_y;
	pbBuffer[6]  = (int) pstVehicle->xspeed + SPEED_BIAS;
	pbBuffer[7]  = (int) pstVehicle->yspeed + SPEED_BIAS;
	pbBuffer[8]  = (int) pstVehicle->id;
	pbBuffer[9]  = (int) pstVehicle->team;
	pbBuffer[10] = (int) pstVehicle->body;
	pbBuffer[11] = (int) pstVehicle->num_turrets;
	pbBuffer[12] = 0x00;
}

static void Message2VehicleInfo(pbBuffer, pstVehicle)
Byte *pbBuffer;
Vehicle_info *pstVehicle;
{
	pstVehicle->loc.grid_x  = (int)   pbBuffer[0]; 
	pstVehicle->loc.grid_y  = (int)   pbBuffer[1];
	pstVehicle->loc.box_x   = (int)   pbBuffer[2];
	pstVehicle->loc.box_y   = (int)   pbBuffer[3];
	pstVehicle->loc.x       = (int)   pstVehicle->loc.grid_x * BOX_WIDTH + pstVehicle->loc.box_x;
	pstVehicle->loc.y       = (int)   pstVehicle->loc.grid_y * BOX_HEIGHT + pstVehicle->loc.box_y;
	pstVehicle->xspeed      = (float) pbBuffer[6] - SPEED_BIAS;
	pstVehicle->yspeed      = (float) pbBuffer[7] - SPEED_BIAS;
	pstVehicle->id          = (ID)    pbBuffer[8];
	pstVehicle->team        = (Team)  pbBuffer[9];
	pstVehicle->body        = (int)   pbBuffer[10];
	pstVehicle->num_turrets = (int)   pbBuffer[11];
}

static void process_messages(allp, how_many, get_targets)
	All *allp;
	int how_many;
	Boolean get_targets;
{
	int i, j, num_messages, id;
	Message readme;

	l2debug_printf(("processing messages\n"));
	num_messages = messages();
	how_many = (how_many > num_messages) ? num_messages : how_many;
	for (i = 0; i < how_many; i++) {
		l2debug_printf(("receiving message %d\n",i));
		receive_msg(&readme);
		l2debug_printf(("done receiving, now processing\n"));
		if (readme.sender != SENDER_COM
			&& (readme.sender_team != allp->me.team 
				|| allp->me.team == 0 
				|| readme.sender == allp->me.id)
		) {
			l2debug_printf(("won't read, not from anyone we trust\n"));
			continue;
		}
		switch (readme.opcode) {
			case OP_ENEMY_AT:
				l2debug_printf(("info on enemy (had %d vehicles)\n",allp->num_vehicles));
				j = allp->num_vehicles + 1;
				if (j < MAX_VEHICLES && get_targets) {
					l2debug_printf(("getting info\n"));
					Message2VehicleInfo(readme.data, &(allp->vehicle_list[j]));
					id = allp->vehicle_list[j].id;
					if (allp->inview[id]) {
						l2debug_printf(("already knew about him\n"));
					} else {
						l2debug_printf(("now %d vehicles\n",j));
						allp->num_vehicles++;
						insert_hitme(id,allp);
						allp->enemy_dx[id] = (double)(allp->vehicle_list[j].loc.x - allp->myloc.x);
						allp->enemy_dy[id] = (double)(allp->vehicle_list[j].loc.y - allp->myloc.y);
						allp->enemy_dist[id] = -sqrt(allp->enemy_dx[id]*allp->enemy_dx[id]+allp->enemy_dy[id]*allp->enemy_dy[id]);
						allp->id2num[allp->vehicle_list[j].id] = j;
						allp->inview[allp->vehicle_list[j].id] = FALSE;
					}
				} else {
					l2debug_printf(("too many vehicles, not adding\n"));
				}
				break;
			case OP_DEATH:
				l2debug_printf(("info about player death (%d)\n",readme.data[0]));
				allp->priority[readme.data[0]] = 0.0;
				break;
			default:
				l2debug_printf(("unknown message type\n"));
				break;
		}
	}
}

/* old name, used to do something different */
static Boolean kill_if_outpost(x,y,allp)
	int x;
	int y;
	All *allp;
{
	int id;

	l2debug_printf(("looking for outpost @ %d %d\n",x,y));
	if (landmark(x,y) == OUTPOST) {
		allp->more_outposts = allp->frame;
		if (allp->num_vehicles < MAX_VEHICLES) {
			/* ooh, I HATE special cases */
			if (allp->num_vehicles > 0) {
				id = allp->vehicle_list[allp->num_vehicles - 1].id;
				id = (id >= MAX_VEHICLES) ? (id + 1) : MAX_VEHICLES;
			} else {
				id = MAX_VEHICLES;
			}
			allp->vehicle_list[allp->num_vehicles].xspeed = 0.0;
			allp->vehicle_list[allp->num_vehicles].yspeed = 0.0;
			allp->vehicle_list[allp->num_vehicles].loc.grid_x = x;
			allp->vehicle_list[allp->num_vehicles].loc.grid_y = y;
			get_outpost_loc(x, y, allp->frame,
				&(allp->vehicle_list[allp->num_vehicles].loc.x),
				&(allp->vehicle_list[allp->num_vehicles].loc.y));
			allp->vehicle_list[allp->num_vehicles].loc.box_x = 
				allp->vehicle_list[allp->num_vehicles].loc.x - x * BOX_WIDTH;
			allp->vehicle_list[allp->num_vehicles].loc.box_y =
				allp->vehicle_list[allp->num_vehicles].loc.y - y * BOX_HEIGHT;
			allp->vehicle_list[allp->num_vehicles].heading = 0.0;
			/* ugh.  special cases suck massive tagman wonkie */
			allp->vehicle_list[allp->num_vehicles].id = id;
			allp->vehicle_list[allp->num_vehicles].team = MAX_VEHICLES;
			allp->vehicle_list[allp->num_vehicles].body = 0;
			allp->vehicle_list[allp->num_vehicles].num_turrets = 0;
			allp->id2num[id] = allp->num_vehicles;
			allp->enemy_dx[id] =
				(double)(allp->vehicle_list[allp->num_vehicles].loc.x
					- allp->myloc.x);
			allp->enemy_dy[id] =
				(double)(allp->vehicle_list[allp->num_vehicles].loc.y
					- allp->myloc.y);
			allp->enemy_dist[id] = -sqrt(allp->enemy_dx[id]*allp->enemy_dx[id]
				+ allp->enemy_dy[id]*allp->enemy_dy[id]);
			allp->priority[id] += 10.0 / (1.0 - allp->enemy_dist[id]);
			allp->inview[id] = TRUE;
			allp->enemy_dx[id] = allp->vehicle_list[allp->num_vehicles].loc.x
				- allp->myloc.x;
			allp->enemy_dy[id] = allp->vehicle_list[allp->num_vehicles].loc.y
				- allp->myloc.y;
			allp->num_vehicles++;
			insert_hitme(id,allp);
			if (allp->num_outposts++ < HOW_MANY_OUTPOSTS) {
				l2debug_printf(("Got one!  looking for others\n"));
				return TRUE;
			} else {
				l2debug_printf(("Got one!  not looking for others\n"));
				return FALSE;
			}
		} else {
			l2debug_printf(("no more space in vehicle_list\n"));
			/* no more space in vehicle_list, no point in checking */
			return FALSE;
		}
	} else {
		l2debug_printf(("no outpost here\n"));
		return TRUE; /* sure, this isn't an outpost, but there might be others */
	}
}

/* BRUTE FORCE! YEAH! */
static void look_for_outposts(allp)
	All *allp;
{
	int h, i, j, myx, myy;

	allp->num_outposts = 0;
	myx = allp->myloc.grid_x;
	myy = allp->myloc.grid_y;

	/* this gets set in kill_if_outpost if there are any more around */
	allp->more_outposts = allp->frame + CHECK_FOR_OUTPOSTS;

	/* trivial case */
	if (!kill_if_outpost(myx,myy,allp)) { return; }

	/* a box of expanding dimensions */
	for (h = 1; h < 4; h++) {
		/* across the top and bottom */
		for (i = - h; i < h + 1; i++) {
			for (j = - h; j <= h ; j += h + h) { 
				if (!kill_if_outpost(myx+i,myy+j,allp)) { return; }
			}
		}
		/* down the left and right, not repeating the corners */
		for (i = - h; i <= h ; i += h + h) {
			for (j = - h + 1; j < h; j++) { 
				if (!kill_if_outpost(myx+i,myy+j,allp)) { return; }
			}
		}
	}
}

static void main()
{
	All *allp;
	int i, nframe, pos, missed, totmiss, fframe;
	Byte msgData[MAX_DATA_LEN];
	char msg[30];

	allp = (All *) calloc(1, sizeof(*allp));
	set_cleanup_func(cleanup, (void *) allp);

	get_settings(&allp->settings);
	setup_weapon_arrays(allp);

	missed = totmiss = 0;
	allp->my_abs_drive = 0.0;
	allp->next_enemy_msg = allp->more_outposts = allp->frame = fframe = frame_number();
	allp->have_lock = FALSE;
	allp->nobody_around = fframe - DELAY_OPEN_MESSAGE;
	allp->need_new_lock = TRUE;
	allp->last_open_msg = fframe - OPEN_HOW_OFTEN;
	allp->min_armor_allow = (max_armor(FRONT) > max_armor(BACK)) ?  max_armor(FRONT) : max_armor(BACK);
	i = (max_armor(RIGHT) > max_armor(LEFT)) ?  max_armor(RIGHT) : max_armor(LEFT);
	allp->min_armor_allow = (allp->min_armor_allow > i) ?  allp->min_armor_allow : i;
	allp->min_armor_allow = allp->min_armor_allow / 5;

	while (1) {
		get_self(&allp->me);
		nframe = frame_number();
		l2debug_printf(("I'm alive!\n"));
		missed = (nframe - allp->frame) / TICKSZ - 1;
		totmiss += missed;
		if (missed) {
			sprintf(msg,"missed %d%% of %d turns",(int)(100.0*(double)totmiss/(double)(nframe-fframe)),(nframe-fframe));
			send_msg(allp->me.id, OP_TEXT, msg);
		}
		allp->frame = nframe;
		get_location(&(allp->myloc));

		target_bastard_enemies(allp);
		if (allp->num_targets == 0) {
			if (allp->more_outposts <= allp->frame) {
				l2debug_printf(("looking for outposts\n"));
				look_for_outposts(allp);
				l2debug_printf(("now %d vehicles\n",allp->num_vehicles));
			}
		}
		process_messages(allp, 8, (allp->num_targets == 0));
		/* XXX what about walls we can't see between us and remote targets? */

		l2debug_printf(("%d vehicles %d targets\n",allp->num_vehicles,allp->num_targets));
		if (allp->num_targets > 0) {
			if (allp->next_enemy_msg <= allp->frame && allp->me.team != 0) {
				allp->next_enemy_msg = allp->frame + ENEMY_HOW_OFTEN;
				l3debug_printf(("enemy in view to send message about? (checking %d targets)\n",allp->num_targets));
				for (pos = 0; pos < allp->num_targets; pos++) {
					l3debug_printf(("checking position %d to see if in view\n",pos));
					if (allp->inview[allp->hitme[pos]] && allp->hitme[pos] < MAX_VEHICLES) {
						l3debug_printf(("He's it\n"));
						break;
					}
				}
				if (pos < allp->num_targets) {
					l2debug_printf(("creating message about id %d\n",allp->hitme[pos]));
					VehicleInfo2Message(&(allp->vehicle_list[allp->id2num[allp->hitme[pos]]]), msgData);
					l3debug_printf(("sending message (id %d) (opcode %d)\n",allp->vehicle_list[allp->id2num[allp->hitme[pos]]].id,OP_ENEMY_AT));
					send_msg(MAX_VEHICLES + allp->me.team, OP_ENEMY_AT, msgData);
					l3debug_printf(("sent message\n"));
				}
			}
			l2debug_printf(("trying to fire weapons now\n"));
			for (i = 0; i < allp->num_weap; i++) {
				l3debug_printf(("trying weapon %d\n",i));
				if (!new_please_fire_weapon(i, -1, allp)) {
					l3debug_printf(("couldn't fire # %d\n",i));
					turn_off_weapon(i);
				}
			}
			l2debug_printf(("calling protect_armor\n"));
			protect_armor(allp);
		} else {
			if (allp->last_open_msg <= allp->frame) {
				if (allp->me.team != 0) {
					allp->last_open_msg = allp->frame + OPEN_HOW_OFTEN;
					msgData[0] = allp->myloc.grid_x;
					msgData[1] = allp->myloc.grid_y;
					msgData[2] = 0;
					send_msg(MAX_VEHICLES+allp->me.team, OP_OPEN, msgData);
				}
				for (i = 0; i < allp->num_weap; i++) {
					turn_off_weapon(i);
				}
			}
		}
		l3debug_printf(("done with turn\n\n"));
		done();
	}
}
