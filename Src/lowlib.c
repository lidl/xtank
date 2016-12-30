/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include <stdlib.h>		/* for abort() */

#include "xtank.h"
#include "xtanklib.h"
#include "screen.h"
#include "map.h"
#include "vstructs.h"
#include "sysdep.h"
#include "bullet.h"
#include "cosell.h"
#include "outpost.h"
#include "globals.h"
#include "graphics.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif /* SOUND */

#if 1 || defined(__bsdi__)
#define Angle double
#define ID int
#endif

extern Weapon_stat weapon_stat[];
extern Armor_stat armor_stat[];
extern Engine_stat engine_stat[];
extern Body_stat body_stat[];
extern Settings settings;
extern Map real_map;
extern int frame;


#define legal_weapon(num) (num >= 0 && num < cv->num_weapons)
#define legal_turret(num) ((int)num>=(int)TURRET1 && (int)num<cv->num_turrets)
#define legal_armor(num) ((int)num >= (int)FRONT && (int)num <= (int)BOTTOM)
#define fix_angle(angle) angle -= (2*PI) * floor((double) angle/(2*PI))

/* Pointer to current vehicle */
Vehicle *cv;

/*
** The check_time function is used to determine when to switch control
** back to the program scheduler.
**
** In an action xtanklib function, check_time is called after the action
** occurs.  This improves the time consistency of a vehicles actions,
** since they will occur during the same frame that the call was made.
**
** In a query xtanklib function, check_time is called before the information
** is gleaned from the appropriate places.  This improves the time
** consistency of the information, since it is computed during the same
** frame that the information will be used.
*/

/*************
** Movement **
*************/

/* returns the vehicle's current location */

void
get_location(Location *loc)
{
	Loc *cvloc;

	check_time();

	cvloc = cv->loc;

	/* Copy all the values from the current vehicle's location structure */
	loc->grid_x = cvloc->grid_x;
	loc->grid_y = cvloc->grid_y;
	loc->box_x = (int) cvloc->box_x;
	loc->box_y = (int) cvloc->box_y;
	loc->x = (int) cvloc->x;
	loc->y = (int) cvloc->y;
}


/* returns the vehicle's maximum speed, in pixels per frame */

FLOAT
max_speed(void)
{
	check_time();
	return (cv->vdesc->max_speed);
}


/* returns the vehicle's current speed, in pixels per frame */

FLOAT
speed(void)
{
	check_time();
	return (cv->vector.speed);
}


/* returns the direction the body of the vehicle is currently pointing */

Angle
heading(void)
{
	Angle hdng;

	check_time();
	hdng = cv->vector.heading;
	fix_angle(hdng);
	return hdng;
}


/* return the maximum possible acceleration of the vehicle, taking engine power
   and ground friction into account */

FLOAT
acc(void)
{
	check_time();
	return cv->vdesc->acc;		/* assumes ground friction is 1.0 */
}


/* returns the acceleration limit imposed by the engine power.  This only
   affects speeding up. */

FLOAT
engine_acc(void)
{
	check_time();
	return cv->vdesc->engine_acc;
}


/* returns the acceleration limit imposed by the treads (which should be
   multiplied by settings.si.normal_friction or settings.si.slip_friction
   before use).  This affects both speeding up and slowing down. */

FLOAT
tread_acc(void)
{
	check_time();
	return cv->vdesc->tread_acc;
}


/* returns the vehicle's width and height at the current heading */

void
vehicle_size(int *width, int *height)
{
	Picture *pic;

	check_time();
	pic = &cv->obj->pic[cv->vector.rot];
	*width = pic->width;
	*height = pic->height;
}


/* sets the vehicle's desired heading, which will take a few frames to acheive,
   depending on the turning rate */

void
turn_vehicle(Angle desired_heading)
{
	FLOAT diff;
	Vector *vector;

	vector = &cv->vector;

	/* make sure the desired heading is between -PI and PI */
	vector->desired_heading = drem(desired_heading, 2 * PI);

	diff = vector->desired_heading - vector->heading;

	if (diff < 0.0) {
		if (diff <= -PI) {		/* -2*PI <= diff <= -PI */
			vector->heading -= 2 * PI;	/* make heading smaller than desired
					   heading */
			vector->heading_flag = CLOCKWISE;
		} else					/* -PI < diff < 0.0 */
			vector->heading_flag = COUNTERCLOCKWISE;

	} else if (diff > 0.0) {
		if (diff < PI)			/* 0.0 < diff < PI */
			vector->heading_flag = CLOCKWISE;
		else {					/* PI <= diff <= 2*PI */
			vector->heading += 2 * PI;	/* make angle heading than desired
					   heading */
			vector->heading_flag = COUNTERCLOCKWISE;
		}
	} else						/* diff = 0.0 */
		vector->heading_flag = NO_SPIN;

	check_time();
}

void
turn_vehicle_human(Angle desired_heading)
{
	if (cv->special[(int) NAVIGATION].status != SP_nonexistent &&
		cv->vector.drive < 0.0) {
		desired_heading -= PI;
	}
	turn_vehicle(desired_heading);
}


/* returns the maximum turning rate (in radians per frame) of the vehicle if it
   is moving at the given speed */

Angle
turn_rate(FLOAT abs_speed)
{
	Angle turning_rate;
	FLOAT ground_friction;
	FLOAT traction;				/* the limit to acceleration imposed by ground
                 and tread friction */

	check_time();

	if (cv->safety == TRUE)
		return cv->max_turn_rate;

	/* determine the ground friction here */
	if (cv->vdesc->treads == HOVER_TREAD) {
		ground_friction = settings.si.normal_friction;
	} else {
		/* on slip square? */
		ground_friction =
		  real_map[cv->loc->grid_x][cv->loc->grid_y].type == SLIP ?
		  settings.si.slip_friction :
		  settings.si.normal_friction;
	}
	/* determine the traction between treads and ground */
	traction = ground_friction * cv->vdesc->tread_acc;

	if (traction < abs_speed) {
		turning_rate = asin(traction / abs_speed);
		if (turning_rate > cv->max_turn_rate)
			turning_rate = cv->max_turn_rate;
	} else {
		turning_rate = cv->max_turn_rate;
	}

	return turning_rate;
}


/* sets the vehicle's desired speed */

void
set_abs_drive(FLOAT abs_speed)
{
	cv->vector.drive = (ABS(abs_speed) > cv->vdesc->max_speed ?
						cv->vdesc->max_speed * SIGN(abs_speed) :
						abs_speed);
	check_time();
}

/* gets the vehicle's current speed */

FLOAT
get_abs_drive(void)
{
	check_time();
	return (FLOAT) cv->vector.drive;
}


/* sets the vehicle's desired speed relative to the maximum */

void
set_rel_drive(FLOAT rel_drive)
{
	/* rel_drive - in the range -9 to 9 */
	/* Make sure the drive is within the allowable range */
	if (rel_drive > 9.0)
		rel_drive = 9.0;
	else if (rel_drive < -9.0)
		rel_drive = -9.0;
	set_abs_drive(cv->vdesc->max_speed * rel_drive / 9);

	/* Don't do a check_time since one is done in set_abs_drive */
}


/* sets the safe-turning flag.  If true, the turning rate will be lower at
   higher speeds to prevent skidding */

void
set_safety(int status)
{
	if (cv->vdesc->treads != HOVER_TREAD && status)
		cv->safety = TRUE;
	else
		cv->safety = FALSE;

	check_time();
}

/* sets the teleport flag.  If true, vehicles passing through teleports
   which are either neutral, or on the vehicle's team will be teleported */

void
set_teleport(int status)
{
	if (status)
		cv->teleport = TRUE;
	else
		cv->teleport = FALSE;

	check_time();
}


/************
** Turrets **
************/

/* returns the current position of the turret relative to the vehicle's
   position */

void turret_position(TurretNum turret, int *xp, int *yp)
{
	check_time();

	/* Make sure the specified turret exists */
	if (!legal_turret(turret)) {
		*xp = *yp = 0;
	} else {
		Coord *cp;

		cp = &(cv->obj->picinfo[cv->vector.rot].turret_coord[(int) turret]);
		*xp = cp->x;
		*yp = cp->y;
	}
}


/*
** Returns the number of turrets on the vehicle
*/
int num_turrets(void)
{
	check_time();

	return cv->num_turrets;
}


/*
** Returns angle of the specified turret (in radians, 0 <= angle < 2*PI)
** If the specified turret doesn't exist, returns BAD_VALUE
*/
Angle
turret_angle(TurretNum num)
{
	FLOAT angle;

	check_time();

	/* Make sure the specified turret exists */
	if (!legal_turret(num))
		return ((FLOAT) BAD_VALUE);

	angle = cv->turret[(int) num].angle;
	fix_angle(angle);
	return (angle);
}

/*
** Returns turn rate of the specified turret (in radians/frame)
** If the specified turret doesn't exist, returns BAD_VALUE
*/
Angle
turret_turn_rate(TurretNum num)
{
	check_time();

	/* Make sure the specified turret exists */
	if (!legal_turret(num))
		return ((FLOAT) BAD_VALUE);

	return (cv->turret[(int) num].turn_rate);
}

/*
** Turns the turret numbered num to the specified angle (in radians)
*/
void
turn_turret(TurretNum num, Angle angle)
{
	FLOAT diff;
	Turret *t;

	if (legal_turret(num)) {
		t = &cv->turret[(int) num];

		/* make sure desired angle is between -PI and PI */
		t->desired_angle = drem(angle, 2 * PI);

		diff = t->desired_angle - t->angle;

		/* start the turret turning in the proper direction */
		if (diff < 0.0)
			if (diff < -PI) {	/* -2*PI < diff < -PI */
				t->angle -= 2 * PI;	/* make angle smaller than desired
					   angle */
				t->angle_flag = CLOCKWISE;
			} else				/* -PI <= diff < 0.0 */
				t->angle_flag = COUNTERCLOCKWISE;

		else if (diff > 0.0)
			if (diff < PI)		/* 0.0 < diff < PI */
				t->angle_flag = CLOCKWISE;
			else {				/* PI <= diff <= 2*PI */
				t->angle += 2 * PI;	/* make angle bigger than
					   desired_angle */
				t->angle_flag = COUNTERCLOCKWISE;
		} else					/* diff = 0.0 */
			t->angle_flag = NO_SPIN;
	}
	check_time();
}

/*
** Aims the turret numbered num at a location dx away horizontally
** and dy away vertically from the vehicle
*/
Angle
aim_turret(TurretNum num, int dx, int dy)
{
	FLOAT ang;
	Coord *tcoord;
	Picinfo *picinfo;

	if (legal_turret(num)) {
		picinfo = &cv->obj->picinfo[cv->vector.rot];
		tcoord = &picinfo->turret_coord[(int) num];

		/* compute the angle to turn the turret towards */
		ang = ATAN2(dy - tcoord->y,
					dx - tcoord->x);

		turn_turret(num, ang);
	}
	/* Don't do a check_time since one is done in turn_turret */

	/* Let 'em know where they are aiming at   JMO 3/17/90 fo shooter */
	return (ang);
}



/************
** Weapons **
************/
/*
 * Get the heat generated by a weapon num on the current vehicle.
 */
int
weapon_heat(WeaponNum num)
{
  Weapon_stat *ws;
  Weapon *w;
  
  check_time();
  if (!legal_weapon(num))
    return BAD_VALUE;
  w = &cv->weapon[num];
  
  ws = &weapon_stat[(int) w->type];
  return (ws->heat);
}

Boolean
weapon_on(WeaponNum num)
{
	check_time();
	if (!legal_weapon(num))
		return BAD_VALUE;
	return ((Boolean) (cv->weapon[num].status & WS_on));
}

/*
** Turns on the specified weapon
*/
int
turn_on_weapon(WeaponNum num)
{
	Weapon *w;
	int retval;

	check_time();

	/* first make sure the specified weapon exists */
	if (!legal_weapon(num))
		retval = BAD_VALUE;
	else {
		retval = 0;
		w = &cv->weapon[num];
		if (w->status & WS_func)
			w->status |= WS_on;
	}

	check_time();
	return retval;
}

/*
** Turns off the specified weapon
*/
int
turn_off_weapon(WeaponNum num)
{
	Weapon *w;
	int retval;

	/* Make sure the specified weapon exists */
	if (!legal_weapon(num))
		retval = BAD_VALUE;
	else {
		retval = 0;
		w = &cv->weapon[num];
		if (w->status & WS_func)
			w->status &= ~WS_on;
	}

	check_time();
	return retval;
}

void
toggle_weapon(WeaponNum num)
{
	if (weapon_on(num)) {
		turn_off_weapon(num);
	} else {
		turn_on_weapon(num);
	}
}

void
anim_fire_weapon(WeaponNum num)
{
	unsigned long flgs;

	flgs = weapon_stat[(cv->weapon[num]).type].creat_flgs;

	if((flgs & F_BOTH) || !(flgs & F_MAP))
		fire_weapon(num);
}

void
map_fire_weapon(WeaponNum num)
{
	unsigned long flgs;

	flgs = weapon_stat[(cv->weapon[num]).type].creat_flgs;

	if(flgs & (F_MAP | F_BOTH))
		fire_weapon(num);
}

/*
** Fires a weapon
*/
WeaponStatus
fire_weapon(WeaponNum num)
{
	extern int num_terminals;
	Weapon *w;
	Weapon_stat *ws;
	Turret *t;
	FLOAT angle;
	Loc bloc;
	Coord *tcoord;
	WeaponStatus retval;
	int i;

	/* Make sure the specified weapon exists */
	if (!legal_weapon(num)) {
		check_time();
		return BAD_WEAPON_NUM;
	}
	w = &cv->weapon[num];

	if (w->reload_counter) {
#ifdef SOUND
		play_owner(cv, WEAPON_RELOADING_SOUND);
#endif /* SOUND */
		retval = RELOADING;		/* weapon has not reloaded yet */
	}
	else if (w->status & WS_no_ammo) {
#ifdef SOUND
		play_owner(cv, WEAPON_NO_AMMO_SOUND);
#endif /* SOUND */
		retval = NO_AMMO;		/* weapon has no ammo */
	}
	else if (!(w->status & WS_on)) {
#ifdef SOUND
		play_owner(cv, WEAPON_OFF_SOUND);
#endif /* SOUND */
		retval = WEAPON_OFF;	/* weapon is off */
	}
	else if (cv->heat > 100) {
#ifdef SOUND
		play_owner(cv, WEAPON_TOO_HOT_SOUND);
#endif /* SOUND */
		return TOO_HOT;			/* vehicle is too hot to shoot */
	}
	else {
#ifdef SOUND
		switch(w->type)
		{
			case LMG:
				play_in_view(cv->loc, LMG_SOUND);
				break;
			case MG:
				play_in_view(cv->loc, MG_SOUND);
				break;
			case HMG:
				play_in_view(cv->loc, HMG_SOUND);
				break;
			case LCANNON:
				play_in_view(cv->loc, LCANNON_SOUND);
				break;
			case CANNON:
				play_in_view(cv->loc, CANNON_SOUND);
				break;
			case HCANNON:
				play_in_view(cv->loc, HCANNON_SOUND);
				break;
			case LROCKET:
				play_in_view(cv->loc, LROCKET_SOUND);
				break;
			case ROCKET:
				play_in_view(cv->loc, ROCKET_SOUND);
				break;
			case HROCKET:
				play_in_view(cv->loc, HROCKET_SOUND);
				break;
			case ACID:
				play_in_view(cv->loc, ACID_SOUND);
				break;
			case FLAME:
				play_in_view(cv->loc, FLAME_SOUND);
				break;
			case MINE:
				play_in_view(cv->loc, MINE_SOUND);
				break;
			case SEEKER:
				play_in_view(cv->loc, SEEKER_SOUND);
				break;
			case BLAST:
				play_in_view(cv->loc, BLAST_SOUND);
				break;
			case LASER:
				play_in_view(cv->loc, LASER_SOUND);
				break;
			case SLICK:
				play_in_view(cv->loc, SLICK_SOUND);
				break;
			case PROCKET:
				play_in_view(cv->loc, PROCKET_SOUND);
				break;
			case UMISSLE:
				play_in_view(cv->loc, UMISSLE_SOUND);
				break;
			case TELE:
				play_in_view(cv->loc, TELEGUIDED_SOUND);
				break;
			case TOW:
				play_in_view(cv->loc, TOW_SOUND);
				break;
			case LTORP:
				play_in_view(cv->loc, LTORP_SOUND);
				break;
			case MORTAR:
				play_in_view(cv->loc, MORTAR_SOUND);
				break;
#ifdef NO_GAME_BALANCE
			case NUKE:
				play_in_view(cv->loc, NUKE_SOUND);
				break;
#endif
		}
#endif /* SOUND */

		retval = FIRED;
		ws = &weapon_stat[(int) w->type];

		/* Set the reload counter and decrement the ammo of the weapon */
		w->reload_counter = ws->reload_time;
		if (--w->ammo == 0)
			w->status |= WS_no_ammo;

		/* Heat up the vehicle by the proper amount */
		cv->heat += ws->heat;

		/* Copy the vehicle location for the starting bullet location */
		bloc = *cv->loc;

		/* Figure out what angle to shoot the bullet towards */

		switch (w->mount) {
		  case MOUNT_TURRET1:
		  case MOUNT_TURRET2:
		  case MOUNT_TURRET3:
		  case MOUNT_TURRET4:
			  /* Make the angle equal to where the turret will be this frame
	   */
			  t = &cv->turret[(int) w->mount];
			  angle = t->angle;
			  switch (t->angle_flag) {
				case NO_SPIN:
					break;
				case CLOCKWISE:
					if ((angle += t->turn_rate) >= t->desired_angle)
						angle = t->desired_angle;
					break;
				case COUNTERCLOCKWISE:
					if ((angle -= t->turn_rate) <= t->desired_angle)
						angle = t->desired_angle;
					break;
			  }

			  /* Adjust the bullet location to the correct turret */
			  tcoord = &cv->obj->picinfo[cv->vector.rot].
				turret_coord[(int) w->mount];
			  adjust_loc(&bloc, tcoord->x, tcoord->y);
			  for (i=0; i < num_terminals; i++) {
				bloc.screen_x[i] += tcoord->x;
				bloc.screen_y[i] += tcoord->y;
			  }
			  break;
		  case MOUNT_FRONT:
			  angle = cv->vector.heading;
			  break;
		  case MOUNT_BACK:
			  angle = cv->vector.heading + PI;
			  break;
		  case MOUNT_LEFT:
			  angle = cv->vector.heading - PI / 2;
			  break;
		  case MOUNT_RIGHT:
			  angle = cv->vector.heading + PI / 2;
		}

		if(ws->creat_func)		/* call special creation function. */
				/* this func should call make_bullet at some point. */
			(*ws->creat_func)(w, &bloc, angle);
		else {
			/* adds a slight random "error" (+/- 1.8 degrees) */
			/* in the bullet's direction.  Thus, bullets fired */
			/* simultaneously won't completely overlap (HAK) */

			angle += (rnd(101) - 50) * (PI / 100 / 50);

			if(ws->creat_flgs & F_CR3) {
				if (w->type == SLICK && settings.commentator)
					comment(COS_SLICK_DROPPED, 0, (Vehicle *) NULL,
							(Vehicle *) NULL, (Bullet *) NULL);

				for (i = 0; i < 3; i++)
					make_bullet(cv, &bloc, w->type, angle - PI / 12 + i * PI / 12, NULL);
			} else {
				make_bullet(cv, &bloc, w->type, angle, NULL);
			}
		}

		/* mark time weapon was fired */

		cv->frame_weapon_fired = frame;
	}
	check_time();
	return retval;
}

/* creation functions for special bullets... (HAK) */
void
creat_harm(Weapon *w, Loc *bloc, Angle angle)
{
	/* ingore passed-in value for angle */

	angle = cv->vector.heading; /* harms face forward */
	make_bullet(cv, cv->loc, w->type, angle, &cv->target);
}

void
creat_mort(Weapon *w, Loc *bloc, Angle angle)
{
	angle = ATAN2((double)(cv->target.y - cv->loc->y),(double)(cv->target.x - cv->loc->x));
	/* (Icko) */

	make_bullet(cv, cv->loc, w->type, angle, &cv->target);
}

/*
** Returns the number of weapons on the vehicle
*/
int
num_weapons(void)
{
	check_time();
	return (cv->num_weapons);
}

/*
** Puts constant information about weapon into weapon info structure,
** Returns BAD_VALUE if specified weapon does not exist.
*/
int
get_weapon(WeaponNum num, Weapon_info *winfo)
{
	Weapon *w;
	Weapon_stat *ws;

	check_time();
	if (!legal_weapon(num))
		return BAD_VALUE;
	w = &cv->weapon[num];
	ws = &weapon_stat[(int) w->type];

	winfo->type = w->type;
	winfo->mount = w->mount;
	winfo->damage = ws->damage;
	winfo->heat = ws->heat;
#ifdef BOGO_RANGE
	winfo->range = ws->range;
#else /* BOGO_RANGE */
	winfo->range = ws->frames * ws->ammo_speed;
#endif
	winfo->reload = ws->reload_time;
	winfo->max_ammo = ws->max_ammo;
	winfo->ammo_speed = ws->ammo_speed;
	winfo->frames = ws->frames;
	winfo->safety = ws->safety;
	winfo->height = ws->height;
	return 0;
}

/*
** Returns number of frames before weapon can fire again.
*/
int
weapon_time(WeaponNum num)
{
	check_time();
	if (!legal_weapon(num))
		return BAD_VALUE;
	return (cv->weapon[num].reload_counter);
}

int
weapon_ammo(WeaponNum num)
{
	check_time();
	if (!legal_weapon(num))
		return BAD_VALUE;
	return (cv->weapon[num].ammo);
}


/************************************/
/* Information about one's own tank */
/************************************/
int
get_tread_type(void)
{
	check_time();
	return (cv->vdesc->treads);
}

int
get_bumper_type(void)
{
	check_time();
	return (cv->vdesc->bumpers);
}

int
get_vehicle_cost(void)
{
	check_time();
	return (cv->vdesc->cost);
}

int
get_engine_type(void)
{
	check_time();
	return (cv->vdesc->engine);
}

int
get_handling(void)
{
	check_time();
	return (cv->vdesc->handling);
}

int
get_suspension_type(void)
{
	check_time();
	return (cv->vdesc->suspension);
}


/**********
** Armor **
**********/

/*
** Returns amount of armor on the specified side
*/
int
armor(Side num)
{
	check_time();

	if (legal_armor(num))
		return (cv->armor.side[(int) num]);
	else
		return BAD_VALUE;
}

/*
** Returns maximum amount of armor on the specified side as above
*/
int
max_armor(Side num)
{
	check_time();

	if (legal_armor(num))
		return (cv->vdesc->armor.side[(int) num]);
	else
		return BAD_VALUE;
}

/*
** Returns protection of the type of armor on the vehicle
*/
int
protection(void)
{
	check_time();
	return armor_stat[cv->armor.type].defense;
}


/****************
** Environment **
****************/

/* return a pointer to the vehicle's map.  Here's an example of use:
** {
**     Box (*my_map)[GRID_HEIGHT];
**
**     my_map = map_get();
**
**     if (my_map[9][4].strength > 1) ...
** }
*/
Box (*map_get())[GRID_HEIGHT] {
	return ((Mapper *) cv->special[(int) MAPPER].record)->map;
}


/*
** Return the status of a wall of the specified box (x,y) in the specified
** direction.
**
** Returns one of the following values:
**  MAP_NONE    no wall there that you know of
**  MAP_WALL    indestructible wall present
**  MAP_DEST    destructible wall present
**
** You can only detect the presence of a wall if you have a mapper and it is
** on your map.
*/

WallType
wall(WallSide dir, int x, int y)
{
	Special *s = &cv->special[(int) MAPPER];

	check_time();

	/* Make sure the vehicle has a mapper */
	if (s->status == SP_nonexistent)
		return MAP_NONE;

	return map_wall(((Mapper *) s->record)->map, dir, x, y);
}


/* tells you what kind of landmark is in the given box, or BAD_MAPPER if you
   don't have a mapper */

LandmarkType
landmark(int x, int y)
{
	Special *s;
	Mapper *m;

	check_time();

	/* If the box they are looking for is outside the grid, return NORMAL */
	if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
		return NORMAL;

	/* Make sure the vehicle has a mapper */
	s = &cv->special[(int) MAPPER];
	if (s->status == SP_nonexistent)
		return BAD_MAPPER;

	m = (Mapper *) s->record;

	return m->map[x][y].type;
}


/*
** Puts information about all landmarks your mapper has seen into the
** landmark_info array.  Returns BAD_VALUE if vehicle has no mapper.
*/
int
get_landmarks(int *num_landmark_infos, Landmark_info landmark_info[])
{
	/* landmark_info[] -  Must have size >= MAX_LANDMARKS */
	Special *s = &cv->special[(int) MAPPER];
	Mapper *m = (Mapper *) s->record;

	check_time();

	/* Make sure the vehicle has a mapper */
	if (s->status == SP_nonexistent) {
		*num_landmark_infos = 0;
		return BAD_VALUE;
	}
	*num_landmark_infos = m->num_landmarks;

	/* Copy the landmark information into the landmark_info array */
	bcopy((char *) m->landmark,
		  (char *) landmark_info,
		  *num_landmark_infos * sizeof(Landmark_info));

	return 0;
}

/* return location of an outpost at the specified time
** (future locations are only accurate if the outpost's
** strength doesn't change)
**
** x, y :         box coords of the outpost
** frame_num :    when
** *xret, *yret : return pixel coords (absolute)
*/

void
get_outpost_loc(int x, int y, int frame_num, int *xret, int *yret)
{
	Special *s = &cv->special[(int) MAPPER];
	Mapper *m = (Mapper *) s->record;
	Coord *cp;

	check_time();

	/* make sure the vehicle has a mapper, that the indicated box is on the
       map, and that it is an outpost */
	if (s->status == SP_nonexistent ||
		(x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) ||
		m->map[x][y].type != OUTPOST) {
		*xret = *yret = 0;
		return;
	}
	cp = get_outpost_coord(&(m->map[x][y]), frame_num);

	*xret = cp->x + x * BOX_WIDTH;
	*yret = cp->y + y * BOX_HEIGHT;
}

int
get_blips(int *num_blip_infos, Blip_info blip_info[])
{
	Special *s, *ns;
	Radar *r;
	newRadar *nr;
	int veh, i;
	newBlip *b;
	Vehicle *bv;


	check_time();

	*num_blip_infos = 0;

	s = &cv->special[(int) RADAR];
	r = (Radar *) s->record;

	ns = &cv->special[(int) NEW_RADAR];
	nr = (newRadar *) ns->record;

	if (cv->special[(int) RADAR].status == SP_on) {
		for (i = 0; i < r->num_blips; i++) {
			blip_info[i].x = map2grid(r->blip[i].x - MAP_BOX_SIZE / 4);
			blip_info[i].y = map2grid(r->blip[i].y - MAP_BOX_SIZE / 4);
		}
		*num_blip_infos = r->num_blips;
		return 0;
	} else if (cv->special[(int) NEW_RADAR].status == SP_on
			   || cv->special[(int) TACLINK].status == SP_on) {
		for (veh = 0; veh < num_veh_alive; veh++) {
			b = &nr->blip[veh];
			bv = &actual_vehicles[veh];
			if (b->draw_tactical || b->draw_radar) {
				blip_info[*num_blip_infos].tactical = b->draw_tactical;
				blip_info[*num_blip_infos].radar = b->draw_radar;
				blip_info[*num_blip_infos].friend = b->draw_friend;
				if (b->draw_friend) {
					blip_info[*num_blip_infos].team = bv->team;
					blip_info[*num_blip_infos].number = bv->number;
				}
				blip_info[*num_blip_infos].x = bv->loc->grid_x;
				blip_info[*num_blip_infos].y = bv->loc->grid_y;
				++*num_blip_infos;
			}
		}
		return 0;
	} else
		return BAD_VALUE;

}


/*
** Puts information about all visible vehicles (excluding your own)
** into the vehicle_info array.
*/
void
get_vehicles(int *num_vehicle_infos, Vehicle_info vehicle_info[])
{
	/* vehicle_info[] -  Must have size >= MAX_VEHICLES */

	Vehicle *v;
	Vehicle_info *v_info;
	Picture *pic;
	int dx, dy;
	int i;
	int tur;

	check_time();

	/* Initialize number of vehicle_infos to zero */
	*num_vehicle_infos = 0;

	/* Go through all vehicles, adding visible ones to the vehicle_info array
       */
	for (i = 0; i < num_veh_alive; i++) {
		v = live_vehicles[i];

		/* if the vehicle is our own, don't put it in the list */
		if (cv == v)
			continue;

		/* see if the vehicle is visible */
		dx = v->loc->x - cv->loc->x;
		dy = v->loc->y - cv->loc->y;
		if (dx > -ANIM_WIN_WIDTH / 2 && dx < ANIM_WIN_WIDTH / 2 &&
#ifndef NO_CAMO
			!v->camod &&
#endif /* !NO_CAMO */
			dy > -ANIM_WIN_HEIGHT / 2 && dy < ANIM_WIN_HEIGHT / 2) {

			/* add a vehicle to the list */
			v_info = &vehicle_info[(*num_vehicle_infos)++];

			/* copy the location information */
			v_info->loc.grid_x = v->loc->grid_x;
			v_info->loc.grid_y = v->loc->grid_y;
			v_info->loc.box_x = (int) v->loc->box_x;
			v_info->loc.box_y = (int) v->loc->box_y;
			v_info->loc.x = (int) v->loc->x;
			v_info->loc.y = (int) v->loc->y;

			/* copy the vector information */
			v_info->xspeed = v->vector.xspeed;
			v_info->yspeed = v->vector.yspeed;
			v_info->heading = v->vector.heading;
			fix_angle(v_info->heading);

			/* copy the id, team, and body type */
			v_info->id = v->number;
			v_info->team = v->team;
			v_info->body = v->vdesc->body;

			/* copy the turret information */
			v_info->num_turrets = v->num_turrets;
			for (tur = v->num_turrets - 1; tur >= 0; --tur) {
				v_info->turret_angle[tur] = v->turret[tur].angle;
			}

			pic = v->obj->pic + v->vector.rot;
			v_info->bwidth = pic->width;
			v_info->bheight = pic->height;
		}
	}
}

/*
** Puts information about the current vehicle into the vehicle_info structure.
*/
void
get_self(Vehicle_info *v_info)
{
	int tur;
	Picture *pic;

	check_time();

	/* Copy the location, vector, id, team, and body type */
	v_info->loc.grid_x = cv->loc->grid_x;
	v_info->loc.grid_y = cv->loc->grid_y;
	v_info->loc.box_x = (int) cv->loc->box_x;
	v_info->loc.box_y = (int) cv->loc->box_y;
	v_info->loc.x = (int) cv->loc->x;
	v_info->loc.y = (int) cv->loc->y;
	v_info->xspeed = cv->vector.xspeed;
	v_info->yspeed = cv->vector.yspeed;
	v_info->heading = cv->vector.heading;
	fix_angle(v_info->heading);
	v_info->id = cv->number;
	v_info->team = cv->team;
	v_info->body = cv->vdesc->body;

	/* copy the turret information */
	v_info->num_turrets = cv->num_turrets;
	for (tur = cv->num_turrets - 1; tur >= 0; --tur) {
		v_info->turret_angle[tur] = cv->turret[tur].angle;
	}

	pic = cv->obj->pic + cv->vector.rot;
	v_info->bwidth = pic->width;
	v_info->bheight = pic->height;
}

/*
** Puts information about all visible bullets into the bullet_info array.
*/
void
get_bullets(int *num_bullet_infos, Bullet_info bullet_info[])
{
	/* bullet_info[] -	Must have size >= MAX_BULLETS */
	extern Bset *bset;
	Bullet *b;
	Bullet_info *b_info;
	int dx, dy;
	int i;

	check_time();

	/* Initialize number of bullet_infos to zero */
	*num_bullet_infos = 0;

	/* Go through all bullets, adding visible ones to the bullet_info array */
	for (i = 0; i < bset->number; i++) {
		b = bset->list[i];
		if (b->type == DISC)
			break;

		/* See if the bullet is visible */
		dx = b->loc->x - cv->loc->x;
		dy = b->loc->y - cv->loc->y;
		if (dx > -ANIM_WIN_WIDTH / 2 && dx < ANIM_WIN_WIDTH / 2 &&
			dy > -ANIM_WIN_HEIGHT / 2 && dy < ANIM_WIN_HEIGHT / 2) {

			/* Add a bullet to the list */
			b_info = &bullet_info[(*num_bullet_infos)++];
			if (*num_bullet_infos > MAX_BULLETS) {
				fprintf(stderr, "need to increse MAX_BULLETS!\n");
				abort();
			}
			/* Copy the location, vector, and type */
			b_info->loc.grid_x = b->loc->grid_x;
			b_info->loc.grid_y = b->loc->grid_y;
			b_info->loc.box_x = (int) b->loc->box_x;
			b_info->loc.box_y = (int) b->loc->box_y;
			b_info->loc.x = (int) b->loc->x;
			b_info->loc.y = (int) b->loc->y;
			b_info->xspeed = b->xspeed;
			b_info->yspeed = b->yspeed;
			b_info->type = b->type;
			/* ID is offset in bullet array, which never changes */
			b_info->id = b - bset->array;
		}
	}
}


/*
** Returns the team number for the given vehicle id.
*/
Team
team(ID vid)
{
	int i, retval;

	check_time();
	retval = BAD_VALUE;
	for (i = 0; i < num_veh; ++i)
		if (actual_vehicles[i].number == vid) {
			retval = actual_vehicles[i].team;
			break;
		}
	return retval;
}

/*
** Returns the number of vehicles currently in the maze.
*/
int
number_vehicles(void)
{
	check_time();
	return num_veh_alive;
}

/*
** Puts the important settings into the given settings structure.
*/
void
get_settings(Settings_info *si)
{
	check_time();
	*si = settings.si;			/* structure copy */
}

/******************
** Miscellaneous **
******************/

/**********
** Money **
**********/

/* amount of money the vehicle has */

int
get_money(void)
{
	check_time();
	return (cv->owner->money);
}


/* how much one unit of fuel costs */

int
get_fuel_cost(void)
{
	check_time();
	return engine_stat[cv->vdesc->engine].fuel_cost;
}


/* how much one unit of armor costs */

int
get_armor_cost(void)
{
	check_time();
	return armor_stat[cv->armor.type].cost * body_stat[cv->vdesc->body].size;
}


/* how much one bullet for the indicated weapon costs */

int
get_ammo_cost(WeaponNum wn)
{
	check_time();
	if (!legal_weapon(wn)) {
		return BAD_VALUE;
	}
	return weapon_stat[(int) cv->weapon[(int) wn].type].ammo_cost;
}


/**********
** Discs **
**********/

/*
** Throws all discs owned by vehicle at the desired speed.
** Speed must be between 0 and 25.
** If delay is TRUE, the discs will move in next frame's direction.
**
*/
void
throw_discs(FLOAT dspeed, Boolean delay)
{
	if (dspeed < 0.0)
		dspeed = 0.0;
	else if (dspeed > 25.0)
		dspeed = 25.0;
	release_discs(cv, dspeed, delay);
	check_time();
}

/*
** Spins all discs owned by vehicle in the specified direction
** dir can be one of COUNTERCLOCKWISE, CLOCKWISE, or TOGGLE.
*/
void
spin_discs(Spin dir)
{
	set_disc_orbit(cv, dir);
	check_time();
}

/*
** Returns the number of discs owned by the vehicle
*/
int num_discs(void)
{
	check_time();
	return (cv->num_discs);
}

/*
** Puts information about all visible discs into the disc_info array.
*/
void get_discs(int *num_disc_infos, Disc_info *disc_info)
{
	/* Disc_info disc_info[] --	Must have size >= MAX_DISCS */
	extern Bset *bset;
	Bullet *b;
	Disc_info *b_info;
	int dx, dy;
	int i;

	check_time();

	/* Initialize number of disc_infos to zero */
	*num_disc_infos = 0;

	/* Go through all bullets, adding visible discs to the disc_info array */
	for (i = 0; i < bset->number; i++) {
		b = bset->list[i];
		if (b->type != DISC)
			break;

		/* See if the disc is visible */
		dx = b->loc->x - cv->loc->x;
		dy = b->loc->y - cv->loc->y;
		if (dx > -ANIM_WIN_WIDTH / 2 && dx < ANIM_WIN_WIDTH / 2 &&
			dy > -ANIM_WIN_HEIGHT / 2 && dy < ANIM_WIN_HEIGHT / 2) {

			/* Add a disc to the list */
			b_info = &disc_info[(*num_disc_infos)++];

			/* Copy the location, vector, owner, angle, and spin */
			b_info->loc.grid_x = b->loc->grid_x;
			b_info->loc.grid_y = b->loc->grid_y;
			b_info->loc.box_x = (int) b->loc->box_x;
			b_info->loc.box_y = (int) b->loc->box_y;
			b_info->loc.x = (int) b->loc->x;
			b_info->loc.y = (int) b->loc->y;
			b_info->xspeed = b->xspeed;
			b_info->yspeed = b->yspeed;
			if (b->owner == (Vehicle *) NULL) {
				b_info->owner = NO_OWNER;
				b_info->angle = 0.0;
				b_info->spin = NO_SPIN;
			} else {
				b_info->owner = b->owner->number;
				b_info->angle = ATAN2(b->loc->y - b->owner->loc->y,
									  b->loc->x - b->owner->loc->x);
				if (b_info->angle < 0.0)
					b_info->angle += 2 * PI;

				b_info->spin = (b->owner->status & VS_disc_spin) ?
				  COUNTERCLOCKWISE : CLOCKWISE;
			}
		}
	}
}


/*************
** Messages **
*************/

/*
** Returns number of messages waiting to be read.
*/
int
messages(void)
{
	check_time();
	return ((cv->next_message - cv->current_prog->next_message + MAX_MESSAGES)
			% MAX_MESSAGES);
}

/*
** Sends a message to the recipient with the given opcode and data.
** Only MAX_DATA_LEN-1 bytes of the data are put into the message,
** to ensure that a text message will be null-terminated.
*/
void
send_msg(Byte recipient, Opcode opcode, Byte *data)
{
	cv->sending.sender = cv->number;
	cv->sending.recipient = recipient;
	cv->sending.opcode = opcode;
	cv->sending.frame = frame;
	bcopy((char *) data, (char *) cv->sending.data, MAX_DATA_LEN);
	send_message(cv);

	check_time();
}

/* Copies the information from the next received messsage into the given
   message.  Returns FALSE if there are no messages pending.  */

Boolean
receive_msg(Message *m)
{
	Program *p = cv->current_prog;

	check_time();

	/* If no new messages return 0, else copy in message and increment counter
       */
	if (p->next_message == cv->next_message)
		return FALSE;

	*m = cv->received[p->next_message];
	if (++p->next_message >= MAX_MESSAGES)
		p->next_message = 0;

	return TRUE;
}

/*
** Returns the current amount of fuel in the vehicle
*/
FLOAT
fuel(void)
{
	check_time();
	return (cv->fuel);
}

/*
** Returns the maximum amount of fuel the vehicle can hold
*/
FLOAT
max_fuel(void)
{
	check_time();
	return (cv->max_fuel);
}

/*
** Returns the heat of the vehicle
*/
int
heat(void)
{
	check_time();
	return (cv->heat);
}

/*
** Returns the number of heat sinks in the vehicle
*/
int
heat_sinks(void)
{
	check_time();
	return (cv->vdesc->heat_sinks);
}

/*
** Tells whether or not the vehicle has a particular special
*/
Boolean
has_special(SpecialType st)
{
	check_time();

	return (Boolean) (cv->special[(int) st].status != SP_nonexistent);
}

/*
 * Get the state of a special
 *                          -ane
 */

SpecialStatus
query_special(SpecialType st)
{
	check_time();

	return cv->special[(int) st].status;
}

/*
 * Tries to change the state of a special,
 * returns the state.                -ane
 *
 * Only valid inputs are SP_on and SP_off.
 */

SpecialStatus
switch_special(SpecialType st, int action)
{
	check_time();

	if (st != (int) STEALTH)
		if (st == SP_on || st == SP_off)
			do_special(cv, st, action);

	return cv->special[(int) st].status;
}



/*
** Returns the animation frame number
*/
int frame_number(void)
{
	check_time();
	return frame;
}

/*
** Returns number of kills the vehicle has accrued this battle
*/
int num_kills(void)
{
	check_time();
	return (cv->owner->kills);
}

/*
** Returns score of the vehicle
*/
int score(void)
{
	check_time();
	return (cv->owner->score);
}

/*
** This function will not return until the next frame of execution,
** so calling it means giving up your remaining cpu time for this frame.
*/
void done(void)
{
	stop_program();
}


/* register the given function and argument to be called when the vehicle dies
   (or at the end of the game).  This is needed so programs can free() any
   memory they malloc().  To remove the cleanup function call with funcp equal
   to NULL.  Don't try to do anything fancy in the cleanup function, I don't
   know what will work and what won't. */

void set_cleanup_func(funcp, argp)
void (*funcp) ();				/* pointer to function to call */
void *argp;						/* this pointer is passed to it */
{
	cv->current_prog->cleanup = funcp;
	cv->current_prog->cleanup_arg = argp;
}

void
aim_smart_weapon(int x, int y)
{
	cv->target.x = x;
	cv->target.y = y;

}



/*
 * compiler may warn about "statement not reached", ignore.
 */

#define SGN(a)  (((a)<0) ? -1 : 1)

void
rdf_map(Box **map)
/* Box map[][GRID_HEIGHT]; */
{
	int i, j;
	int first_veh, end_veh;
	Trace *t;
	Special *ms = &cv->special[(int) MAPPER];
	Special *rs = &cv->special[(int) RDF];
	Special *ts = &cv->special[(int) TACLINK];

	Mapper *m = (Mapper *) ms->record;
	Rdf *r = (Rdf *) rs->record;

	int d, x, y, ax, ay, sx, sy, dx, dy;
	unsigned int flags;
	int x1, y1, x2, y2;

	check_time();


	if (ms->status != SP_nonexistent && rs->status == SP_on) {

/*
 * Only scan myself if taclink isn't running.
 */

		if (ts->status == SP_on) {
			first_veh = 0;
			end_veh = MAX_VEHICLES;
		} else {
			first_veh = cv->number;
			end_veh = cv->number + 1;
		}

/*
 * Rasterize the RDF traces onto the map
 */

		for (j = first_veh; j < end_veh; j++) {
			for (i = 0; i < MAX_VEHICLES; i++) {

				t = &r->trace[j][i];

				if (t->to_draw) {
					x1 = t->draw.start_x;
					y1 = t->draw.start_y;
					x2 = t->draw.end_x;
					y2 = t->draw.end_y;

					/*
		     * RED is always set explicty independant of mono
		     */
					if (j != cv->number)
						flags = YELLOW_RDF;
					else if (t->draw.color == RED)
						flags = RED_RDF;
					else
						flags = GREEN_RDF;


					dx = x2 - x1;
					ax = ABS(dx) << 1;
					sx = SGN(dx);
					dy = y2 - y1;
					ay = ABS(dy) << 1;
					sy = SGN(dy);

					x = x1;
					y = y1;
					if (ax > ay) {
						d = ay - (ax >> 1);
						for (;;) {
							if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
								map[x][y].flags = ((map[x][y].flags & ANY_RDF) ? X_RDF : 0) | flags | map[x][y].flags;
							}
							if (x == x2)
								goto complete;
							if (d >= 0) {
								y += sy;
								d -= ax;
							}
							x += sx;
							d += ay;
						}
					} else {
						d = ax - (ay >> 1);
						for (;;) {
							if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
								map[x][y].flags = ((map[x][y].flags & ANY_RDF) ? X_RDF : 0) | flags | map[x][y].flags;
							}
							if (y == y2)
								goto complete;
							if (d >= 0) {
								x += sx;
								d -= ay;
							}
							y += sy;
							d += ax;
						}
					}
				}
			  complete:
				;
			}
		}
	}
	return;
}
