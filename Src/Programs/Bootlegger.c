/*
** Xtank
**
** Copyright 1992 by Sean Barrett
**
** $Id$
*/

/*
		The tank is currently modeled with three crewmembers:

	member		role
	 Captain	 Chooses goals, destination
	 Pilot		 Steers
	 Gunner		 Aims and fires guns at enemy
	and also
	 Computer	 Generates prediction of enemy location


	COMPUTER:	Computer's model of enemy tank assumes
			constant acceleration and constant rotation.
			It will precisely predict the enemy's location
			so long as those constraints are met.

	GUNNER:		Gunner uses computer's predictions.  Gunner
			compensates for turning rate of turret by
			choosing the angle to turn the turret to
			for next TICKSZ frame and only firing if it's
			gotten there.  Gunner will only fire weapons
			with the correct speed to reach the target.
			Gunner knows about side mounts.  Gunner knows
			about weapon ranges.  Gunner knows about relative
			firing.  Gunner only uses side mounts at nearest
			enemy.  Gunner does not avoid shooting friends.
			Gunner doesn't realize our tank may have turned
			by next TICKSZ frame, ruining relative fire calc.

	PILOT:		Pilot attempts to avoid walls by turning or
			reversing.  If a collision occurs, attempts to
			drive in reverse.  Can drive to a goal destination
			if there are no walls in the way.

	CAPTAIN:	Captain uses a simple finite state machine to select
			actions.  Look for the captain to be heavily improved,
			and for an intermediate between the pilot and
			the captain, a navigator, to allow the pilot to select
			arbitrary destination points (such as replenishing
			locations).

Note:

	It might be an interesting idea to standardize this model
	to allow plug-in modules for each.  This would require
	some heavy standardization, and we make it difficult
	to optimize things which are called more than once.
	Basically the allp structure would be standardized to have
	four or five pointers, all of which would be pointers to
	locally defined structs, one for each crew member.
	The communication system already used (using the enumerated
	types Gunnery and Navigation) plus a standard method for
	specifying the destination would be a good idea.  Don't know
	whether an independent "communications officer" would be
	better or if letting the captain handle it is wise.
	It seems the gunners job is reasonably independent that the
	captain doesn't need to tell him much.  Maybe have a set
	of commands: SHOOT_AT_WILL, SHOOT_THIS_TARGET, and CONSERVE_AMMO
	(none of which any particular gunner HAS to obey, and no captain
	has to issue all three--are there any other commands the
	captain might wish to issue?  LAY_MINES and LAY_SLICKS perhaps
	if there's an overall strategic reason for those.  That will
	perhaps be independent, so we need either bit fields or just
	two basic parallel commands for the gunner.  Maybe the Captain
	knows he's going to get more ammo, and knows it's unguarded,
	so 'SHOOT_CONTINUOUSLY' or something--i.e. go ahead an shoot
	even if you're shot isn't very good.)
*/

#include <xtanklib.h>
#include "graphics.h"
#include "gr.h"
#include "vehicle.h"
#include "terminal.h"
#include <math.h>

/*
#define SOFT_DEBUG
#define HARD_DEBUG
*/

#ifdef HARD_DEBUG
#define debugmsg puts
#else
#define debugmsg
#endif

#ifdef TRACE
#define trace puts
#else
#define trace
#endif

/*

			Interface to Xtank server


*/


static void bootlegger_start();

Prog_desc Bootlegger_prog =
{
	"Bootlegger",
	"Vanguard",					/* prefer a tank with side mounts */
	"Crewed by an awesome targetting computer, a great gunner, \
a fair captain, and a poor pilot.",
	"Sean Barrett (buzzard@eng.umd.edu)",
	PLAYS_COMBAT | DOES_SHOOT | USES_TEAMS
	| USES_SIDE_MOUNTS
	| USES_MINES
	| USES_SLICKS
 /* | USES_RAMPLATE */
	,4,
	bootlegger_start
};



#define vehicle_size	20		/* estimated size for other objects */
#define radius		5			/* half of radius of vehicle */
#define radius_2	25			/* squared */
#define border		10			/* pixels of clearance to stay from walls */

#define MAX_FUTURE	17			/* max time I predict enemy location */
#define REACTION_TIME	5		/* max time I expect enemy to turn, and
				   min time before enemy will dodge mine */
#define FAN_OUT_FACTOR	0		/* aim turret at where target will be
				   within +- FAN_OUT_FACTOR frames from
				   predicted.  Uses uniform distribution,
				   normal (bell-curve) might be better */

#define MIN_ANGLE	0.10
#define MIN_ANGLE_2	0.20

#define MAX_TURN	(PI/2)		/* the furthest we expect a tank a turn */

#define WATCH_STEP	1			/* granularity of look ahead (UNUSED) */



  typedef enum {
	  STANK_TURRET1,
	  STANK_TURRET2,
	  STANK_TURRET3,
	  DROPPED_WEAPON,
	  MOUNTED_WEAPON
  } Sweapon_type;

  typedef struct _Graphic {
	  struct _Graphic *next;	/* pointer to next in a linked list */
	  int x1, y1, x2, y2;		/* line endpoints */
	  int color;
	  int frame;				/* frame when drawn (needed to handle window
                                   refresh) */
  }
Graphic;

  struct allp {

/*
	Simulated global data
*/

	  /* CAPTAIN */

	  Settings_info this_game;
	  int me_neutral;			/* am I a neutral player? */


	  /* Shared by CAPTAIN and PILOT */

	  Location my_goal;
	  int nav_mode;				/* what mode the cap'n desires */


	  /* PILOT */

	  int no_navigate;			/* no driving allowed */
	  float mmax_turn, max_acc, top_speed;	/* one-time calculations */
	  int time_to_top, stopping_distance;

	  int halfwidth, halfheight;/* our current shape */
	  int drive_direction;		/* in forward or reverse? */
	  int stopping, stopped_last;	/* old pilot fsm */
	  int pilot_mode;			/* finite state machine */
	  int turn_number;			/* number of turns in this */


	  /* GUNNER */

	  Weapon_info weapons[MAX_WEAPONS];	/* copy of weapon data */
	  int wcount;
	  int range[MAX_WEAPONS][MAX_FUTURE];	/* range table */
	  int ammo[MAX_WEAPONS];
	  double desired_angle[MAX_WEAPONS];
	  double actual_angle[5];
	  Sweapon_type weapon_class[MAX_WEAPONS];
	  double previous_msx, previous_msy;	/* my previous speed */
	  /* used kludgishly by gunner to guess what
				   self speed&angle will be in TICKSZ frames
				   for proper leading relative fire */


	  /* Information about current target */

	  int exists_target;		/* do we have a current one? */
	  Vehicle_info current_target;	/* all data for the current one */
	  float previous_speedx, previous_speedy;	/* from last frame */
	  int old_id;


	  /* ONBOARD TARGETTING COMPUTER */

	  int newx[MAX_FUTURE], newy[MAX_FUTURE];
	  Graphic *glist;
	  Video *vid;


	  /* EVERBODY'S INFO */

	  Vehicle_info vehicles[MAX_VEHICLES];
	  int vcount;
	  int frame, previous_frame;
	  Vehicle_info my_vinfo;
	  float speed;				/* our speed, magnitude */

  };


#define Z 		struct allp *a
#define G 		a->
#define my_info		a->my_vinfo
#define drive		a->drive_direction
#define target		a->current_target
#define mloc		my_info.loc
#define relative	a->this_game.rel_shoot	/* why don't this work? */
#define head		my_info.heading
#define stop_dist	a->stopping_distance
#define max_turn	a->mmax_turn
#define goal		a->my_goal
#define pred_x		a->newx
#define pred_y		a->newy
#define want_ang	a->desired_angle

#define give_up()	for(;;)done()
#define EQUAL_ANGLES(x,y)  (fixed_angle((x)-(y)+MIN_ANGLE)<MIN_ANGLE_2)
#ifndef PI2
#define PI2		(2*PI)
#endif
#define turn_vehicle_human(x) turn_vehicle(drive>0 ? (x) : (x)+PI)

#ifndef QUART_CIRC
#define QUART_CIRC (PI/2)
#endif

#if TICKSZ==1
#undef TICKSZ
#define TICKSZ 2
#endif


/*
	Initialize our startup crap
*/


static void cleanup(a) Z;
{
	free(a);
}


static struct allp *setup(a) Z;
{
	int i, j;
#if defined(__alpha)
	extern void *calloc();
#else
	extern char *calloc();
#endif

#ifndef lint
	a = (struct allp *) calloc(1, sizeof(struct allp));

#endif
	if (!a) {
		send_msg(RECIPIENT_ALL, OP_TEXT, "Someone's hogging memory!");
		give_up();
	}
	set_cleanup_func(cleanup, a);
	get_settings(&G this_game);

	/* Precalculate CAPTAIN information */

	get_self(&my_info);
	G me_neutral = my_info.team == NEUTRAL;
	G top_speed = max_speed();

	max_turn = turn_rate(G top_speed);
	if (!max_turn) {
		send_msg(RECIPIENT_ALL, OP_TEXT, "This tank won't turn!");
		G no_navigate = 1;
	} else if (!has_special(MAPPER)) {
		send_msg(RECIPIENT_ALL, OP_TEXT, "I can't see the walls!");
		G no_navigate = 1;
	} else {
		G max_acc = acc();
		G no_navigate = 0;

		stop_dist = 0.5 * SQR(G top_speed) / tread_acc();
		G time_to_top = (int) (G top_speed / G max_acc + 0.5);
	}

	/* Precalculate GUNNER information */

	for (i = 0; !get_weapon(i, &G weapons[i]); ++i) {
		G ammo[i] = G weapons[i].max_ammo;

		if (IS_TURRET(G weapons[i].mount))
			G weapon_class[i] = G weapons[i].mount;

		else if (G weapons[i].type == MINE || G weapons[i].type == SLICK)
			G weapon_class[i] = DROPPED_WEAPON;

		else
			G weapon_class[i] = MOUNTED_WEAPON;

		for (j = 0; j <= MAX_FUTURE; ++j) {
			G range[i][j] = G weapons[i].ammo_speed * j;

			if (G range[i][j] > G weapons[i].range
				+ G weapons[i].ammo_speed)
				G range[i][j] = -5000;
		}
	}
	G wcount = i;

	return a;
}



static void my_done(a) Z;
{
	int newframe, oldframe;

	trace("my_done");


	oldframe = G frame;
	done();
#ifdef SOFT_DEBUG
	newframe = frame_number();
	if (newframe > oldframe + TICKSZ)
		printf("Stank lost %d moves.\n", (newframe - oldframe) / TICKSZ - 1);
	G frame = newframe;

#endif
	trace(" my_done");
}


#if 0
static void predict(deltat, xnew, ynew, vx, vy, ox, oy, lx, ly)
int deltat, *xnew, *ynew, lx, ly;
double vx, vy, ox, oy;
{
	double angle, rot, speed, acci, ispeed;
	int i;

	speed = HYPOT(vx, vy);
	acc = (speed - HYPOT(ox, oy)) / deltat;
	ispeed = speed ? 1 / speed : 0;
	if (vx * oy == ox * vy) {
		acc *= 0.5;
		for (i = 0; i < MAX_FUTURE; ++i) {
			xnew[i] = lx + i * vx + i * i * acc * vx * ispeed;
			ynew[i] = ly + j * vy + i * i * acc * vy * ispeed;
		}
	} else {
		xnew[0] = lx;
		ynew[0] = ly;
		angle = ATAN2(vy, vx);
		rot = (angle - ATAN2(oy, ox)) / deltat;
		for (i = 1; i < MAX_FUTURE; ++i) {
			angle += rot;
			speed += acc;
			xnew[i] = xnew[i - 1] + speed * COS(angle);
			ynew[i] = ynew[i - 1] + speed * SIN(angle);
		}
	}
}
#else

/*
	 move multiplies and stuff out of the loop by hand --
	 probably no compiler in existence will know how to
	 handle COS(angle+=rot), SIN(angle+=rot); besides,
	 by default xtank tanks aren't compiled -O
*/

static void predict(deltat, xnew, ynew, vx, vy, ox, oy, lx, ly)
int deltat, *xnew, *ynew, lx, ly;
double vx, vy, ox, oy;
{
	double angle, rot, speed, acc, ispeed, dvx, dvy, sn, cs, tvx, tm;
	int i, stop = REACTION_TIME;

	trace("predict");
/*
	Should rewrite this to obey WATCH_STEP.  Worst case time
	estimate:

	2 sqrts, 2 atan2s, 1 sin, 1 cos, 3 divides, 6*MAX_FUTURE+4 multiplies

	If there are no weapons ready to fire this turn or next turn,
	and we have no ram plate, then this prediction is mostly useless.
*/

	speed = HYPOT(vx, vy);
	acc = (speed - HYPOT(ox, oy)) / deltat;
	if (acc < 0) {
		tm = speed / fabs(acc);
		if (tm < stop)
			stop = tm;
	}
	ispeed = speed ? 1 / speed : 0;
	if (vx * oy == ox * vy) {
		acc *= ispeed;
		dvx = acc * vx;
		dvy = acc * vy;
		for (i = 0; i < MAX_FUTURE; ++i) {
			*xnew++ = lx;
			*ynew++ = ly;
			if (i < stop) {
				vx += dvx;
				vy += dvy;
			}
			lx += vx;
			ly += vy;
		}
	} else {
		angle = ATAN2(vy, vx);
		rot = (angle - ATAN2(oy, ox)) / deltat;
		if (rot) {
			tm = MAX_TURN / fabs(rot);
			if (tm < stop)
				stop = tm;
		}
#ifdef HAS_SINCOS
		sincos(rot, &sn, &cs);
#else
		cs = COS(rot);
		sn = SIN(rot);
#endif
		vx *= ispeed;
		vy *= ispeed;
		for (i = 0; i < MAX_FUTURE; ++i) {
			*xnew++ = lx;
			*ynew++ = ly;
			if (i < stop) {
				speed += acc;
				tvx = vx;
				vx = vx * cs - vy * sn;
				vy = vy * cs + tvx * sn;
			}
			lx += vx * speed;
			ly += vy * speed;
		}
	}
	trace(" predict");
}
#endif


/*
	heads up prediction display
*/

static void erase_graphics();
static void draw_screen_line();
static Video *find_terminal();
static void show_predict(a) Z;
{
	int x, y, i;

	if (!G vid)
		return;
	erase_graphics(a);
	if (G exists_target) {
		x = SCREEN_WIDTH / 2 - mloc.x;
		y = SCREEN_HEIGHT / 2 - mloc.y;
		for (i = 1; i < MAX_FUTURE - 2; ++i) {
			draw_screen_line(a, pred_x[i] + x, pred_y[i] + y,
							 pred_x[i + 1] + x, pred_y[i + 1] + y, 1);
		}
	}
}
/*
	our gunner module
*/

typedef enum {
	GUNNER_BORED = 0,
	GUNNER_BLOCKED,
	GUNNER_OUT_OF_RANGE,
	GUNNER_AIMING,
	GUNNER_FIRED,
} Gunnery;


#define FIRE_WEAPON(j)	 (fire_weapon(j) || !G ammo[j] || \
		(--G ammo[j], weapon_active[j]=1, RETURN(GUNNER_FIRED)))
#define RETURN(x) return_value = MAX(return_value,x)


static int maybe_fire_turrets(a) Z;
{
	int i, return_value = GUNNER_OUT_OF_RANGE;

	for (i = 0; i < 5; ++i)
		G actual_angle[i] = turret_angle(i);

	for (i = 0; i < G wcount; ++i)
		if (fabs(want_ang[i] - G actual_angle[G weapon_class[i]])
			< 0.05) {
			fire_weapon(i) || --G ammo[i];
			return_value = GUNNER_FIRED;
		}
	for (i = 0; i < G wcount; ++i)
		want_ang[i] = -BAD_ANGLE;
	return return_value;
}

/*
	How to check friendly fire:  use inaccurate straight line
	approximation.  Find approximate time

	if (a,b) + (c,d)t ~= (e,f) + (g,h)t, i.e. if

	a-e-r <= (g-c)t <= a-e+r
and	b-g-r <= (h-f)t <= b-g+r

	If this time range intersects (0..time-to-hit-target), don't shoot.
	If after, we might accidentally hit friend if we miss. (crossfire)
*/


static int gunner(a, collide) Z;
int *collide;					/* when are we likely to hit this guy? */
{
	double rx, ry, tx, ty, xspd, yspd, turret_active[5], ang;
	double fx, fy, vx, vy, angle, newspd, newsx, newsy;
	int px, py, q;
	Location aim;
	int return_value, dist, weapon_active[MAX_WEAPONS], i, j, turr;

	trace("gunner");
	*collide = 0;
	if (!G exists_target)		/* nobody to shoot at! */
		return GUNNER_BORED;

	return_value = maybe_fire_turrets(a);

	xspd = relative ? my_info.xspeed : 0;
	yspd = relative ? my_info.yspeed : 0;
	rx = mloc.x + xspd;
	ry = mloc.y + yspd;

/*
	Try to guess what my speed will be in TICKSZ frames
	to properly handle aiming turreted weapons for next
	round.
*/

	angle = ATAN2(vy, vx);
	angle = angle +
	  (angle - ATAN2(G previous_msy, G previous_msx))
	  / (G frame - G previous_frame) * TICKSZ;
	newspd = G speed +
	  (G speed - HYPOT(G previous_msx, G previous_msy))
	  / (G frame - G previous_frame) * TICKSZ;
	newsx = COS(angle) * newspd;
	newsy = SIN(angle) * newspd;

	for (i = 0; i < MAX_WEAPONS; ++i)
		weapon_active[i] = 0;
	for (i = 0; i < 3; ++i)
		turret_active[i] = BAD_ANGLE;
	for (i = 1; i < MAX_FUTURE - FAN_OUT_FACTOR; ++i) {
/*
	Let's take a look at the state of things 'i' frames
	into the future.

	We're want to know whether dropping something *this*
	turn or firing a fix-mounted weapon *this* turn will
	work, and we want to know where to aim our turret
	in TICKSZ turns, which will hopefully fix our previous
	problem with shooting behind the target.
*/
		rx += xspd;
		ry += yspd;

/*
		assume testing for a clear path is cheaper then
		looping through all weapons
*/
		aim.x = pred_x[i];
		aim.y = pred_y[i];
		aim.grid_x = aim.x / BOX_WIDTH;
		aim.grid_y = aim.y / BOX_HEIGHT;

		if (abs(aim.x - mloc.x) < 5000 && abs(aim.y - mloc.y) < 5000 &&
			clear_path(&mloc, &aim)) {
/*
	check for a collision at this time, since we've got the
	data handy
*/
			tx = aim.x - rx;
			ty = aim.y - ry;
			dist = HYPOT(tx, ty);
			if (dist < vehicle_size && !*collide)
				*collide = i;

			for (j = 0; j < G wcount; ++j)
				if (!weapon_active[j] && G ammo[j])
					switch (G weapon_class[j]) {
					  case DROPPED_WEAPON:
/*
	If we're moving, and enemy will end up on this square in
	less than REACTION_TIME frames, drop something.  This
	doesn't seem to actually work currently.
*/
						  if (i < REACTION_TIME && G speed && SQR(aim.x
								- mloc.x) + SQR(aim.y - mloc.y) <= radius_2)
							  FIRE_WEAPON(j);
						  break;

					  case MOUNTED_WEAPON:
						  if (fabs(dist - G range[i][j] >= radius))
							  break;
						  ang = fixed_angle(ATAN2(ty, tx));
						  switch (G weapons[j].mount) {
/*
	Haven't ever tried a vehicle with any of these...
*/
							case MOUNT_FRONT:
								if (EQUAL_ANGLES(ang, head))
									FIRE_WEAPON(j);
								break;
							case MOUNT_LEFT:
								if (EQUAL_ANGLES(ang - QUART_CIRC, head))
									FIRE_WEAPON(j);
								break;
							case MOUNT_BACK:
								if (EQUAL_ANGLES(ang + PI, head))
									FIRE_WEAPON(j);
								break;
							case MOUNT_RIGHT:
								if (EQUAL_ANGLES(ang + QUART_CIRC, head))
									FIRE_WEAPON(j);
								break;
						  }
						  break;

					  default:
/* Assume we start shooting the bullet in TICKSZ turns */
						  if (i < TICKSZ || abs(dist - G range[j][i - TICKSZ])
							  > radius + G weapons[j].ammo_speed)
							  break;
						  turr = G weapons[j].mount;
						  turr += TURRET1 - MOUNT_TURRET1;
						  turret_position(turr, &px, &py);
/*
	to add a fan out, replace (tx,ty) with
	q = random(5)-2; (pred_x[i+q]-rx, pred_y[i+q]-ry)
*/

/*
	tx is target - rx.  rx used xspd but we want to use
	newsx for turrets, so adjustment factor is
	rx += (newsx-xspd)*i, or tx -= (news-xspd)*i, assuming
	relative fire.
*/
						  fx = tx - px;
						  fy = ty - py;
						  q = rnd(FAN_OUT_FACTOR * 2 + 1) - FAN_OUT_FACTOR;
						  if (q) {
							  fx += pred_x[i + q] - pred_x[i];
							  fy += pred_y[i + q] - pred_y[i];
						  }
						  if (relative) {
#if 0							/* this doesn't work, dunno why */
							  fx -= (newsx - xspd) * i;
							  fy -= (newsy - yspd) * i;
#endif
						  } else {
							  fx -= newsx * TICKSZ;
							  fy -= newsy * TICKSZ;
						  }
						  ang = fixed_angle(ATAN2(fy, fx));
/*
	if this turret hasn't been aimed this round, aim it for this weapon
*/
						  if (turret_active[turr] == BAD_ANGLE) {
							  draw_screen_line(a, SCREEN_WIDTH / 2 + px,
											   SCREEN_HEIGHT / 2 + py,
								   (int) (SCREEN_WIDTH / 2 + tx + xspd * i),
							   (int) (SCREEN_WIDTH / 2 + ty + yspd * i), 1);
							  RETURN(GUNNER_AIMING);
							  turn_turret(turr, ang);
							  turret_active[turr] = ang;
							  want_ang[j] = ang;
						  } else if (EQUAL_ANGLES(ang, turret_active[turr]))
/*
	otherwise, if it'll be right for this weapon, mark it for firing
*/
							  want_ang[j] = turret_active[turr];
						  weapon_active[j] = 1;
					}
		} else {
			RETURN(GUNNER_BLOCKED);
		}
	}
#ifdef GUNNER_RES
	if (return_value == GUNNER_FIRED)
		debugmsg("Fired.");
	if (return_value == GUNNER_OUT_OF_RANGE)
		debugmsg("Out of range.");
	if (return_value == GUNNER_BLOCKED)
		debugmsg("Blocked.");
	if (return_value == GUNNER_AIMING)
		debugmsg("Aiming.");
#endif
	trace(" gunner");
	return return_value;
}




/*
	since we don't have one main loop, we need to put all
	the stuff we do every time in one place.  This is it.
*/
static void update(a) Z;
{
	trace("update");
	G previous_frame = G frame;
	G previous_speedx = target.xspeed;
	G previous_speedy = target.yspeed;
	G old_id = target.id;

	my_done(a);
	G exists_target = get_closest_enemy(&target);

	trace("got enemy");
	if (target.id != G old_id) {
		G previous_speedx = target.xspeed;
		G previous_speedy = target.yspeed;
	}
	G previous_msx = my_info.xspeed;
	G previous_msy = my_info.yspeed;

	trace("get_self");
	get_self(&my_info);
	trace(" get_self");
	G speed = speed();

/*
	if (!G exists_target)
		find_nearest_outpost();
*/
	if (G exists_target) {
		predict(G frame - G previous_frame,
				G newx,
				G newy,
				target.xspeed,
				target.yspeed,
				G previous_speedx,
				G previous_speedy,
				target.loc.x,
				target.loc.y
		  );
		show_predict(a);
	} else {
		trace("erase_graphivs");
		erase_graphics(a);
		trace(" erase_graphics");
	}
	trace(" update");
}

/*
	pilot module

	two major modes:
		near destination
		combat-engagement

	In both modes try to avoid walls; in the first try to get to
	the location, in the second, try to stay perpendicular to
	the target.

	"near destination" means it has to be one that the way isn't
	blocked to.  If the way is blocked, this pilot needs you
	to specify subgoals.  Perhaps this should be the "steerer"
	and the "pilot" should handle long-range goal reaching
	with RacerX-style analysis.  Possibly just copped from RacerX?
*/

typedef enum {
	NAV_OK = 0,
	NAV_EVADING,				/* try to evade nearest enemy */
	NAV_STOPPING,				/* panic to avoid hitting a wall */
	NAV_WALL_TURN,				/* turning to miss a wall */
	NAV_BORED,					/* at goal or no enemies */
	NAV_BLOCK,					/* pilot doesn't know how to get there */
	NAV_DEAD					/* can't navigate */
} Navigation;

/*
	Turning distance is: (speed-|ourvx|)/max_turn
*/

  typedef enum {
	  PILOT_OK,					/* just use nav_mode */
	  PILOT_STOPPING,			/* currently trying to stop */
	  PILOT_REVERSING,			/* currently reversing */
	  PILOT_BOOTLEGGING,		/* currently bootlegging */
	  PILOT_WALL_TURN			/* currently turning from wall */
  } Piloting;

static int pilot(a) Z;
{
	int already = 0;			/* if already is non-zero, avoid two walls */
	float vx, vy;
	double fake_speed;
	int x, y, gx, gy, hw, vw;
	Location temp;

	trace("pilot");
	if (G no_navigate)
		return NAV_DEAD;

	if (G stopping == 1 && fabs(G speed) >= 0.1)
		return NAV_STOPPING;

	trace("check stop");
	if (fabs(G speed) < 0.1) {
/*
			We've come to a full stop, possibly
			to avoid a wall or because we hit
			something.
*/
		G stopped_last += 1;

		if (G stopped_last < 4)
			return NAV_STOPPING;
		G stopped_last = 0;

		set_rel_drive(0);
		G stopping = 0;

		drive = -drive;
		set_rel_drive(9.0 * drive);
		if (has_special(RAMPLATE) || !rnd(2))
			/* do a bootleg outta here */
			G stopping = 2;

		return NAV_STOPPING;
	}
	G stopped_last = 0;

	trace("not stopped");

	if (G stopping) {			/* bootleg FSM */
		++G stopping;
		if (G stopping >= G time_to_top / TICKSZ) {
			turn_vehicle(head + PI);
			drive = -drive;
			set_rel_drive(9.0 * drive);
			G stopping = 0;
		}
	}
	trace("check walls");
	gx = mloc.grid_x;
	gy = mloc.grid_y;
	x = mloc.box_x;
	y = mloc.box_y;
	vx = my_info.xspeed;
	vy = my_info.yspeed;
	hw = G halfwidth + border;
	vw = G halfheight + border;

#define CHECK_PANIC \
	if (already){ G stopping=1; set_rel_drive(0.0); \
		debugmsg("Panic!");return NAV_STOPPING; }\
	already = 1;

#define WALL(side) wall(side,gx,gy)

	fake_speed = G speed;
	if (fake_speed < 5)
		fake_speed = 5;
	if (vx <= 0 && WALL(WEST)
		&& fake_speed - fabs(vy) > (x - hw) * max_turn) {
		already = 1;
		turn_vehicle_human(
					   (WALL(SOUTH) != WALL(NORTH) ? WALL(SOUTH) : vy < 0) ?
							  3 * PI / 2 : QUART_CIRC);
	} else if (vx >= 0 && WALL(EAST)
			   && fake_speed - fabs(vy) > (BOX_WIDTH - x - hw) * max_turn) {
		already = 1;
		turn_vehicle_human(
					   (WALL(SOUTH) != WALL(NORTH) ? WALL(SOUTH) : vy < 0) ?
							  3 * PI / 2 : QUART_CIRC);
	}
	if (vy <= 0 && WALL(NORTH)
		&& fake_speed - fabs(vx) > (y - hw) * max_turn) {
		CHECK_PANIC
		  turn_vehicle_human(
						  (WALL(EAST) != WALL(WEST) ? WALL(EAST) : vx < 0) ?
								PI : 0);
	} else if (vy >= 0 && WALL(SOUTH)
			   && fake_speed - fabs(vx) > (BOX_HEIGHT - y - hw) * max_turn) {
		CHECK_PANIC
		  turn_vehicle_human(
						  (WALL(EAST) != WALL(WEST) ? WALL(EAST) : vx < 0) ?
								PI : 0);
	}
	trace("checked walls");

	if (G pilot_mode) {
		if (--G pilot_mode)
			return NAV_WALL_TURN;
	}
	G pilot_mode = already * 5;

	if (already)
		return NAV_WALL_TURN;
	if (G nav_mode == NAV_EVADING && !rnd(500)) {
		set_rel_drive(0);
		G stopping = 1;

		return NAV_STOPPING;
	}
	trace("try aim");

/*
	This will give us stupid movement:
		turn towards wall, turn away from the wall, etc. etc.
		over and over again.

	Might be better to make a wall-avoidance module, and do the
	following:
		handle stopping
		check for walls
		try to reach goal/evade, but check new angle
			for walls and don't use it if so
*/

	if (G nav_mode == NAV_EVADING) {
		if (!G exists_target)
			return NAV_BORED;
		y = target.loc.x - mloc.x;
		x = mloc.y - target.loc.y;
		if (fixed_angle(my_info.heading + PI * (drive < 0)
						- ATAN2(y, x) + QUART_CIRC) > PI) {
			x = -x;
			y = -y;
		}
	} else {
		trace("try clear path");
		if (goal.grid_x >= 0 && goal.grid_y >= 0 &&
			goal.grid_x < GRID_WIDTH && goal.grid_y
			< GRID_HEIGHT &&
			clear_path(&mloc, &goal)) {
			x = goal.x - mloc.x;
			y = goal.y - mloc.y;
			trace("draw screen line");
			draw_screen_line(a, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
							 SCREEN_WIDTH / 2 + x, SCREEN_HEIGHT / 2 + y, 2);
		} else {
			return NAV_BLOCK;
		}
	}
	trace("try turn vehicle");
	turn_vehicle_human(ATAN2(y, x));
	trace(" pilot");
	return G nav_mode;
}

static void ram(a, c) Z;
int c;
{
	int who, omode, ox, oy, ogx, ogy, x, y, nres;

	if (!has_special(RAMPLATE)) {
		turn_vehicle(head - 0.25);	/* avoid collision */
		return;
	}
	debugmsg("in ram");

	omode = G nav_mode;
	G nav_mode = NAV_OK;

	who = target.id;
	ox = goal.x;
	oy = goal.y;
	ogx = goal.grid_x;
	ogy = goal.grid_y;
	nres = NAV_OK;
	for (; nres == NAV_OK && target.id == who && c;) {
		goal.x = pred_x[c * 3 / 4];	/* try to force it headfirst */
		goal.y = pred_y[c * 3 / 4];
		goal.grid_x = x / BOX_WIDTH;
		goal.grid_y = y / BOX_HEIGHT;
		nres = pilot(a);
		update(a);
		gunner(a, &c);
	}

	G nav_mode = omode;

	goal.x = ox;
	goal.y = oy;
	goal.grid_x = ogx;
	goal.grid_y = ogy;
	debugmsg("out of ram");
}


/*
	play sequence is gunner, pilot, update
*/

static void captain(a) Z;
{
	int c;
	int gres, nres;
	char where[2];
	Message msg;

	trace("pilot");

  engage:						/* drive defensively from 'target' if it exists */
	debugmsg("engage");
	G nav_mode = NAV_EVADING;

	for (;;) {
		gres = gunner(a, &c);
		if (c)
			ram(a, c);
		nres = pilot(a);
		update(a);

		if (gres == GUNNER_BORED)
			goto explore;
		if (gres == GUNNER_OUT_OF_RANGE)
			goto charge;
	}


  charge:						/* drive towards target until gunner can get him */
	debugmsg("charge");
	G nav_mode = NAV_OK;

	for (;;) {
		gres = gunner(a, &c);
		if (c)
			ram(a, c);
		goal.x = target.loc.x;
		goal.y = target.loc.y;
		goal.grid_x = target.loc.x / BOX_WIDTH;
		goal.grid_y = target.loc.y / BOX_HEIGHT;
		nres = pilot(a);
		update(a);

		if (gres == GUNNER_BLOCKED || gres == GUNNER_BORED)
			goto explore;
		if (gres != GUNNER_OUT_OF_RANGE)
			goto engage;
		if (nres == NAV_BLOCK || nres == NAV_STOPPING)
			goto explore;
	}

  explore:						/* wander around aimlessly until something exciting happens */
	/* should make this jump on supplies if it's with some */
	debugmsg("explore");

	G nav_mode = NAV_OK;

	for (;;) {
		gres = gunner(a, &c);
		nres = pilot(a);
		update(a);
		if (receive_msg(&msg) && !G me_neutral) {
			if (msg.sender_team == my_info.team
				&& (msg.opcode == OP_GOTO || msg.opcode == OP_HELP)) {
				goal.grid_x = msg.data[0];
				goal.grid_y = msg.data[1];
				goal.x = goal.grid_x * BOX_WIDTH + BOX_WIDTH / 2;
				goal.y = goal.grid_y * BOX_HEIGHT + BOX_HEIGHT / 2;
				where[0] = msg.opcode;
				send_msg(msg.sender, OP_ACK, where);
			} else if (msg.sender_team != my_info.team && rnd(2))
				send_msg(msg.sender, OP_TEXT, "Stuff it.");
		}
		if (gres != GUNNER_BLOCKED && gres != GUNNER_BORED) {
			if (!G me_neutral) {
				where[0] = mloc.grid_x;
				where[1] = mloc.grid_y;
				send_msg(my_info.team + MAX_VEHICLES,
						 OP_GOTO, where);
			}
			goto engage;
		}
		if (nres == NAV_BLOCK ||
			mloc.grid_x == goal.grid_x && mloc.grid_y == goal.grid_y)
			/* pick a new target square */
		{
			trace("pick square");
			goal.grid_x = mloc.grid_x + rnd(5) - 2;
			goal.grid_y = mloc.grid_y + rnd(5) - 2;
			goal.x = goal.grid_x * BOX_WIDTH + BOX_WIDTH / 2;
			goal.y = goal.grid_y * BOX_HEIGHT + BOX_HEIGHT / 2;
		}
	}
}

static void bootlegger_start()
{
	Z;
	a = setup(a);
#ifdef SOFT_DEBUG
	if (!(G vid = find_terminal()))
		printf("No terminal attached.\n");
#endif
	G glist = 0;
	G frame = frame_number();

	drive = has_special(RAMPLATE) ? 1 : -1;
	update(a);					/* wait just a little */

	captain(a);
	/* NOTREACHED */
}



/*
	display code lifted from hud3.c
*/

/* erase all the graphic objects we have drawn since the previous erase */

static void erase_graphics(a) Z;
{
	Graphic *gp;

	if (!G vid)
		return;

	while (G glist != NULL) {
		gp = G glist;
		G glist = G glist->next;

		/* only erase it if the window hasn't been refreshed since we drew it
           (which would have erased it for us already) */
		if (gp->frame > G vid->last_expose_frame) {
			/* erase it */
			XDrawLine(G vid->dpy, G vid->win[ANIM_WIN].id,
					  G vid->graph_gc[DRAW_XOR][gp->color],
					  gp->x1, gp->y1, gp->x2, gp->y2);
		}
		free((char *) gp);
	}
}


/* draw a line on the user's X window */

static void draw_screen_line(a, x1, y1, x2, y2, color) Z;
int x1, y1, x2, y2;
int color;
{
	Graphic *gp;

	if (!G vid)
		return;
	gp = (Graphic *) malloc(sizeof(Graphic));
	if (!gp) {
		printf("out of memory!");
		return;
	}
/*	truncate a pixel off each end to make the display look
	segmented
*/
	if (x1 < x2 - 1)
		++x1, --x2;
	else if (x1 > x2 + 1)
		--x1, ++x2;

	if (y1 < y2 - 1)
		++y1, --y2;
	else if (y1 > y2 + 1)
		--y1, ++y2;

	XDrawLine(G vid->dpy, G vid->win[ANIM_WIN].id,
			  G vid->graph_gc[DRAW_XOR][color],
			  x1, y1, x2, y2);

	/* link into list, for later erasure */
	gp->x1 = x1;
	gp->y1 = y1;
	gp->x2 = x2;
	gp->y2 = y2;
	gp->color = color;
	gp->frame = G frame;
	gp->next = G glist;
	G glist = gp;
}

static Video *find_terminal()
{
	extern int num_terminals;
	extern Terminal *terminal[];
	extern Vehicle *cv;			/* xtank's current vehicle */
	int tn;

	/* find a terminal attached to this vehicle */
	for (tn = 0; tn < num_terminals; ++tn) {
		if (terminal[tn]->vehicle == cv)
			break;
	}

	if (tn == num_terminals)
		return NULL;			/* failure */

	return (Video *) terminal[tn]->video;
}
