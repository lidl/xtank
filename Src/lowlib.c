/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** lowlib.c
*/

#include "malloc.h"
#include "xtank.h"
#include "xtanklib.h"
#include "screen.h"
#include "map.h"
#include "vstructs.h"
#include "sysdep.h"
#include "bullet.h"
#include "cosell.h"
#include "box.h"

extern int num_vehicles;
extern Vehicle *vehicle[];
extern Weapon_stat weapon_stat[];
extern Armor_stat armor_stat[];
extern Engine_stat engine_stat[];
extern Body_stat body_stat[];
extern Settings settings;
extern Map box;
extern int frame;


#define legal_weapon(num) (num >= 0 && num < cv->num_weapons)
#define legal_turret(num) ((int)num>=(int)TURRET1 && (int)num<cv->num_turrets)
#define legal_armor(num) ((int)num >= (int)FRONT && (int)num < (int)BOTTOM)
#define fix_angle(angle) angle -= (2*PI) * floor(angle/(2*PI))

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

 /************
 ** ACTIONS **
 ************/
/*************
** Movement **
*************/

turn_vehicle_human(desired_heading)
Angle desired_heading;
{
    if (cv->special[(int)NAVIGATION].status != SP_nonexistent &&
			cv->vector.drive < 0.0)
	{
		desired_heading -= PI;
	}
	turn_vehicle(desired_heading);
}

/*
** Starts turning the vehicle to the specified angle.
*/
turn_vehicle(desired_heading)
Angle desired_heading;
{
	float diff;
	Vector *vector;

	vector = &cv->vector;

	/* make sure the desired heading is between -PI and PI */
	vector->desired_heading = drem(desired_heading, 2 * PI);

	diff = vector->desired_heading - vector->heading;

	if (diff < 0.0)
		if (diff <= -PI)
		{						/* -2*PI <= diff <= -PI */
			vector->heading -= 2 * PI;	/* make heading smaller than desired
										   heading */
			vector->heading_flag = CLOCKWISE;
		}
		else					/* -PI < diff < 0.0 */
			vector->heading_flag = COUNTERCLOCKWISE;

	else if (diff > 0.0)
		if (diff < PI)			/* 0.0 < diff < PI */
			vector->heading_flag = CLOCKWISE;
		else
		{						/* PI <= diff <= 2*PI */
			vector->heading += 2 * PI;	/* make angle heading than desired
										   heading */
			vector->heading_flag = COUNTERCLOCKWISE;
		}

	else						/* diff = 0.0 */
        vector->heading_flag = NO_SPIN;

	check_time();
}


/* sets the vehicle's desired speed */

set_abs_drive(abs_speed)
float abs_speed;
{
    cv->vector.drive = (ABS(abs_speed) > cv->vdesc->max_speed ?
			cv->vdesc->max_speed * SIGN(abs_speed) :
			abs_speed);
	check_time();
}


/*
** Sets drive to rel_drive/9 * max_speed() as quickly as possible
** rel_drive can range from -9 to 9.
*/
set_rel_drive(rel_drive)
float rel_drive;
{
	/* Make sure the drive is within the allowable range */
	if (rel_drive > 9.0)
		rel_drive = 9.0;
	else if (rel_drive < -9.0)
		rel_drive = -9.0;
	set_abs_drive(cv->vdesc->max_speed * rel_drive / 9);

	/* Don't do a check_time since one is done in set_abs_drive */
}

/*
** Sets the safety of your vehicle.  When safety is on, the vehicle's
** turn rate is limited to reduce skidding.
*/
set_safety(status)
int status;
{
	if (status)
		cv->safety = TRUE;
	else
		cv->safety = FALSE;

	check_time();
}

/************
** Turrets **
************/

/*
** Turns the turret numbered num to the specified angle (in radians)
*/
turn_turret(num, angle)
TurretNum num;
Angle angle;
{
	float diff;
	Turret *t;

	if (legal_turret(num))
	{
        t = &cv->turret[(int)num];

		/* make sure desired angle is between -PI and PI */
		t->desired_angle = drem(angle, 2 * PI);

		diff = t->desired_angle - t->angle;

		/* start the turret turning in the proper direction */
		if (diff < 0.0)
			if (diff < -PI)
			{					/* -2*PI < diff < -PI */
				t->angle -= 2 * PI;		/* make angle smaller than desired
										   angle */
				t->angle_flag = CLOCKWISE;
			}
			else				/* -PI <= diff < 0.0 */
				t->angle_flag = COUNTERCLOCKWISE;

		else if (diff > 0.0)
			if (diff < PI)		/* 0.0 < diff < PI */
				t->angle_flag = CLOCKWISE;
			else
			{					/* PI <= diff <= 2*PI */
				t->angle += 2 * PI;		/* make angle bigger than
										   desired_angle */
				t->angle_flag = COUNTERCLOCKWISE;
			}

		else					/* diff = 0.0 */
            t->angle_flag = NO_SPIN;
	}
	check_time();
}

/*
** Aims the turret numbered num at a location dx away horizontally
** and dy away vertically from the vehicle
*/
Angle aim_turret(num, dx, dy)
TurretNum num;
int dx, dy;
{
	float ang;
	Coord *tcoord;
	Picinfo *picinfo;

	if (legal_turret(num))
	{
		picinfo = &cv->obj->picinfo[cv->vector.rot];
        tcoord = &picinfo->turret_coord[(int)num];

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
** Turns on the specified weapon
*/
turn_on_weapon(num)
WeaponNum num;
{
	Weapon *w;
	int retval;

	check_time();

	/* first make sure the specified weapon exists */
	if (!legal_weapon(num))
		retval = BAD_VALUE;
	else
	{
		retval = 0;
		w = &cv->weapon[num];
		if (w->status & WS_func)
			w->status |= WS_on;
	}

	check_time();
	return (retval);
}

/*
** Turns off the specified weapon
*/
turn_off_weapon(num)
WeaponNum num;
{
	Weapon *w;
	int retval;

	/* Make sure the specified weapon exists */
	if (!legal_weapon(num))
		retval = BAD_VALUE;
	else
	{
		retval = 0;
		w = &cv->weapon[num];
		if (w->status & WS_func)
			w->status &= ~WS_on;
	}

	check_time();
	return (retval);
}

/*
** Fires a weapon
*/
WeaponStatus fire_weapon(num)
WeaponNum num;
{
	Weapon *w;
	Weapon_stat *ws;
	Turret *t;
	float angle;
	Loc bloc;
	Coord *tcoord;
	WeaponStatus retval;
	int i;

	/* Make sure the specified weapon exists */
	if (!legal_weapon(num))
	{
		check_time();
        return BAD_WEAPON_NUM;
	}
	w = &cv->weapon[num];

	if (w->reload_counter)
		retval = RELOADING;		/* weapon has not reloaded yet */
	else if (w->status & WS_no_ammo)
		retval = NO_AMMO;		/* weapon has no ammo */
	else if (!(w->status & WS_on))
		retval = WEAPON_OFF;	/* weapon is off */
	else if (cv->heat > 100)
		return TOO_HOT;			/* vehicle is too hot to shoot */
	else
	{
		retval = FIRED;
        ws = &weapon_stat[(int)w->type];

		/* Set the reload counter and decrement the ammo of the weapon */
		w->reload_counter = ws->reload_time;
		if (--w->ammo == 0)
			w->status |= WS_no_ammo;

		/* Heat up the vehicle by the proper amount */
		cv->heat += ws->heat;

		/* Copy the vehicle location for the starting bullet location */
		bloc = *cv->loc;

		/* Figure out what angle to shoot the bullet towards */
		switch (w->mount)
		{
			case MOUNT_TURRET1:
			case MOUNT_TURRET2:
			case MOUNT_TURRET3:
				/* Make the angle equal to where the turret will be this
				   frame */
                t = &cv->turret[(int)w->mount];
				angle = t->angle;
				switch (t->angle_flag)
				{
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
		    turret_coord[(int)w->mount];
				adjust_loc(&bloc, tcoord->x, tcoord->y);
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

		if (w->type == SLICK)
		{
			/* Make 3 oil slick bullets in a fan */
			for (i = 0; i < 3; i++)
				make_bullet(cv, &bloc, w->type, angle - PI / 12 + i * PI / 12);

			if (settings.commentator)
				comment(COS_SLICK_DROPPED, 0, (Vehicle *) NULL, (Vehicle *) NULL);
		}
		else
		{
			/* Create the bullet with slight random fanning (1.8 degrees) */
			make_bullet(cv, &bloc, w->type, angle + PI / 100 * (50 - rnd(101)) / 50);
		}
	}

	check_time();
	return retval;
}

 /************
 ** QUERIES **
 ************/

/*************
** Movement **
*************/

/*
** Returns the maximum speed of the vehicle
*/
float max_speed()
{
	check_time();

	return (cv->vdesc->max_speed);
}

/*
** Returns the current speed of the vehicle
*/
float speed()
{
	check_time();

	return (cv->vector.speed);
}

/*
** Returns the current heading of the vehicle (in radians, 0 <= heading < 2*PI)
*/
Angle heading()
{
    Angle hdng;

	check_time();

    hdng = cv->vector.heading;
    fix_angle(hdng);
    return hdng;
}

/*
** Returns the acceleration limit of the vehicle
*/
float acc()
{
	check_time();
    return cv->vdesc->acc;	/* unfortunately, this assumes the ground
				   friction is 1.0 */
}


/* returns the acceleration limit imposed by the engine power.  This only
   affects speeding up. */

float engine_acc()
{
    check_time();
    return cv->vdesc->engine_acc;
}


/* returns the acceleration limit imposed by the treads (which should be
   multiplied by settings.si.normal_friction or settings.si.slip_friction
   before use).  This affects both speeding up and slowing down. */

float tread_acc()
{
    check_time();
    return cv->vdesc->tread_acc;
}


/*
** Returns the turn rate of the vehicle for the specified absolute speed
*/
float turn_rate(abs_speed)
float abs_speed;
{
	int spd;

	check_time();

	/* Make sure the abs_speed is within the appropriate values */
	spd = (int) abs_speed;
    if (ABS(spd) > MAX_SPEED)
		/* return 0; */
		spd = MAX_SPEED;
	return (cv->turn_rate[spd]);
}

/*
** Puts your vehicle's current location into the location structure.
*/
get_location(loc)
Location *loc;
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

/*
** Returns width and height of current vehicle at current heading.
*/
vehicle_size(width, height)
int *width;
int *height;
{
	Picture *pic;

	check_time();
	pic = &cv->obj->pic[cv->vector.rot];
	*width = pic->width;
	*height = pic->height;
}


/************
** Turrets **
************/


/****************************/
/* Donated by Robert Potter */
/****************************/
void turret_position(turret, xp, yp)
TurretNum turret;				/* which turret */
int *xp, *yp;					/* for returning position of turret (relative
								   to vehicle position) */
{
	check_time();

	/* Make sure the specified turret exists */
	if (!legal_turret(turret))
	{
		*xp = *yp = 0;
	}
	else
	{
		Coord *cp;

        cp = &(cv->obj->picinfo[cv->vector.rot].turret_coord[(int)turret]);
		*xp = cp->x;
		*yp = cp->y;
	}
}


/*
** Returns the number of turrets on the vehicle
*/
num_turrets()
{
	return cv->num_turrets;
}

/*
** Returns angle of the specified turret (in radians, 0 <= angle < 2*PI)
** If the specified turret doesn't exist, returns BAD_VALUE
*/
Angle turret_angle(num)
TurretNum num;
{
	float angle;

	check_time();

	/* Make sure the specified turret exists */
	if (!legal_turret(num))
		return ((float) BAD_VALUE);

    angle = cv->turret[(int)num].angle;
	fix_angle(angle);
	return (angle);
}

/*
** Returns turn rate of the specified turret (in radians/frame)
** If the specified turret doesn't exist, returns BAD_VALUE
*/
Angle turret_turn_rate(num)
TurretNum num;
{
	check_time();

	/* Make sure the specified turret exists */
	if (!legal_turret(num))
		return ((float) BAD_VALUE);

    return (cv->turret[(int)num].turn_rate);
}


/************
** Weapons **
************/

/*
** Returns the number of weapons on the vehicle
*/
num_weapons()
{
	check_time();
	return (cv->num_weapons);
}

/*
** Puts constant information about weapon into weapon info structure,
** Returns BAD_VALUE if specified weapon does not exist.
*/
get_weapon(num, winfo)
WeaponNum num;
Weapon_info *winfo;
{
	Weapon *w;
	Weapon_stat *ws;

	check_time();
	if (!legal_weapon(num))
		return BAD_VALUE;
	w = &cv->weapon[num];
    ws = &weapon_stat[(int)w->type];

	winfo->type = w->type;
	winfo->mount = w->mount;
	winfo->damage = ws->damage;
	winfo->heat = ws->heat;
	winfo->range = ws->range;
	winfo->reload = ws->reload_time;
	winfo->max_ammo = ws->max_ammo;
	winfo->ammo_speed = ws->ammo_speed;
	winfo->frames = ws->frames;
	return 0;
}

/*
** Returns number of frames before weapon can fire again.
*/
weapon_time(num)
WeaponNum num;
{
	check_time();
	if (!legal_weapon(num))
		return BAD_VALUE;
	return (cv->weapon[num].reload_counter);
}

weapon_ammo(num)
WeaponNum num;
{
	check_time();
	if (!legal_weapon(num))
		return BAD_VALUE;
	return (cv->weapon[num].ammo);
}

weapon_on(num)
WeaponNum num;
{
	check_time();
	if (!legal_weapon(num))
		return BAD_VALUE;
	return (cv->weapon[num].status & WS_on);
}


/**********
** Armor **
**********/

/*
** Returns amount of armor on the specified side
*/
armor(num)
Side num;
{
	check_time();

	if (legal_armor(num))
        return (cv->armor.side[(int)num]);
	else
		return BAD_VALUE;
}

/*
** Returns maximum amount of armor on the specified side as above
*/
max_armor(num)
Side num;
{
	check_time();

	if (legal_armor(num))
        return (cv->vdesc->armor.side[(int)num]);
	else
		return BAD_VALUE;
}

/*
** Returns protection of the type of armor on the vehicle
*/
protection()
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
Box (*map_get())[GRID_HEIGHT]
{
    return ((Mapper *) cv->special[(int)MAPPER].record)->map;
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

WallType wall(dir, x, y)
WallSide dir;
int x, y;
{
    Special *s = &cv->special[(int)MAPPER];

	check_time();

	/* Make sure the vehicle has a mapper */
	if (s->status == SP_nonexistent)
		return MAP_NONE;

	return map_wall(((Mapper *) s->record)->map, dir, x, y);
}


/* tells you what kind of landmark is in the given box, or BAD_MAPPER if you
   don't have a mapper */

LandmarkType landmark(x, y)
int x, y;
{
	Special *s;
	Mapper *m;

	check_time();

	/* If the box they are looking for is outside the grid, return NORMAL */
	if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
		return NORMAL;

	/* Make sure the vehicle has a mapper */
    s = &cv->special[(int)MAPPER];
	if (s->status == SP_nonexistent)
        return BAD_MAPPER;

	m = (Mapper *) s->record;

    return m->map[x][y].type;
}


/* return location of an outpost at the specified time (future locations are
   only accurate if the outpost's strength doesn't change) */

void get_outpost_loc(x, y, frame_num, xret, yret)
    int x, y;			/* box coords of the outpost */
    int frame_num;		/* when */
    int *xret, *yret;		/* return pixel coords (absolute) */
{
    Special *s = &cv->special[(int)MAPPER];
    Mapper *m = (Mapper *) s->record;
    Coord *cp;

    check_time();

    /* make sure the vehicle has a mapper, that the indicated box is on the
       map, and that it is an outpost */
    if (s->status == SP_nonexistent &&
	(x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) &&
	m->map[x][y].type == OUTPOST) {
	*xret = *yret = 0;
	return;
    }

    cp = get_outpost_coord(&(m->map[x][y]), frame_num);

    *xret = cp->x + x * BOX_WIDTH;
    *yret = cp->y + y  * BOX_HEIGHT;
}


/*
** Puts information about all landmarks your mapper has seen into the
** landmark_info array.  Returns BAD_VALUE if vehicle has no mapper.
*/
get_landmarks(num_landmark_infos, landmark_info)
int *num_landmark_infos;
Landmark_info landmark_info[];	/* Must have size >= MAX_LANDMARKS */
{
    Special *s = &cv->special[(int)MAPPER];
    Mapper *m = (Mapper *) s->record;

	check_time();

    /* Make sure the vehicle has a mapper */
    if (s->status == SP_nonexistent) {
		*num_landmark_infos = 0;
		return BAD_VALUE;
	}

	*num_landmark_infos = m->num_landmarks;

	/* Copy the landmark information into the landmark_info array */
    bcopy((char *)m->landmark,
	  (char *)landmark_info,
	  *num_landmark_infos * sizeof(Landmark_info));

	return 0;
}

/*
** Puts information about all radar blips into the blip_info array.
** Returns BAD_VALUE if vehicle has no radar.
*/
get_blips(num_blip_infos, blip_info)
int *num_blip_infos;
Blip_info blip_info[];			/* Must have size >= MAX_BLIPS */

{
	Special *s;
	Radar *r;
	int i;

	check_time();

	/* Make sure the vehicle has radar */
    s = &cv->special[(int)RADAR];
	if (s->status == SP_nonexistent)
		return BAD_VALUE;

	r = (Radar *) s->record;
	*num_blip_infos = r->num_blips;

	/* Copy location information of each blip into the blip_info array */
	i = 0;
	while (i < r->num_blips)
	{
		blip_info[i].x = map2grid(r->blip[i].x - MAP_BOX_SIZE / 4);
		blip_info[i].y = map2grid(r->blip[i].y - MAP_BOX_SIZE / 4);
		i++;
	}
	return 0;
}

/*
** Puts information about all visible vehicles (excluding your own)
** into the vehicle_info array.
*/
get_vehicles(num_vehicle_infos, vehicle_info)
int *num_vehicle_infos;
Vehicle_info vehicle_info[];	/* Must have size >= MAX_VEHICLES */
{
	Vehicle *v;
	Vehicle_info *v_info;
	int dx, dy;
	int i;
    int tur;

	check_time();

	/* Initialize number of vehicle_infos to zero */
	*num_vehicle_infos = 0;

    /* Go through all vehicles, adding visible ones to the vehicle_info array
       */
	for (i = 0; i < num_vehicles; i++)
	{
		v = vehicle[i];

		/* if the vehicle is our own, don't put it in the list */
		if (cv == v)
			continue;

		/* see if the vehicle is visible */
		dx = v->loc->x - cv->loc->x;
		dy = v->loc->y - cv->loc->y;
		if (dx > -ANIM_WIN_WIDTH / 2 && dx < ANIM_WIN_WIDTH / 2 &&
				dy > -ANIM_WIN_HEIGHT / 2 && dy < ANIM_WIN_HEIGHT / 2)
		{

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
			for(tur = v->num_turrets - 1; tur >= 0 ; --tur) {
				v_info->turret_angle[tur] = v->turret[tur].angle;
			}
		}
	}
}

/*
** Puts information about the current vehicle into the vehicle_info structure.
*/
get_self(v_info)
Vehicle_info *v_info;
{
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
}

/*
** Puts information about all visible bullets into the bullet_info array.
*/
get_bullets(num_bullet_infos, bullet_info)
int *num_bullet_infos;
Bullet_info bullet_info[];		/* Must have size >= MAX_BULLETS */

{
	extern Bset *bset;
	Bullet *b;
	Bullet_info *b_info;
	int dx, dy;
	int i;

	check_time();

	/* Initialize number of bullet_infos to zero */
	*num_bullet_infos = 0;

	/* Go through all bullets, adding visible ones to the bullet_info array */
	for (i = 0; i < bset->number; i++)
	{
		b = bset->list[i];
		if (b->type == DISC)
			break;

		/* See if the bullet is visible */
		dx = b->loc->x - cv->loc->x;
		dy = b->loc->y - cv->loc->y;
		if (dx > -ANIM_WIN_WIDTH / 2 && dx < ANIM_WIN_WIDTH / 2 &&
				dy > -ANIM_WIN_HEIGHT / 2 && dy < ANIM_WIN_HEIGHT / 2)
		{

			/* Add a bullet to the list */
			b_info = &bullet_info[(*num_bullet_infos)++];
			if (*num_bullet_infos > MAX_BULLETS)
			{
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
** Puts information about all visible discs into the disc_info array.
*/
get_discs(num_disc_infos, disc_info)
int *num_disc_infos;
Disc_info disc_info[];			/* Must have size >= MAX_DISCS */

{
	extern Bset *bset;
	Bullet *b;
	Disc_info *b_info;
	int dx, dy;
	int i;

	check_time();

	/* Initialize number of disc_infos to zero */
	*num_disc_infos = 0;

	/* Go through all bullets, adding visible discs to the disc_info array */
	for (i = 0; i < bset->number; i++)
	{
		b = bset->list[i];
		if (b->type != DISC)
			break;

		/* See if the disc is visible */
		dx = b->loc->x - cv->loc->x;
		dy = b->loc->y - cv->loc->y;
		if (dx > -ANIM_WIN_WIDTH / 2 && dx < ANIM_WIN_WIDTH / 2 &&
				dy > -ANIM_WIN_HEIGHT / 2 && dy < ANIM_WIN_HEIGHT / 2)
		{

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
			if (b->owner == (Vehicle *) NULL)
			{
				b_info->owner = NO_OWNER;
				b_info->angle = 0.0;
                b_info->spin = NO_SPIN;
			}
			else
			{
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

/*
** Returns the team number for the given vehicle id.
*/
Team team(vid)
ID vid;
{
	int i, retval;

	check_time();
	retval = BAD_VALUE;
	for (i = 0; i < num_vehicles; ++i)
		if (vehicle[i]->number == vid)
			retval = vehicle[i]->team;

	return retval;
}

/*
** Returns the number of vehicles in the maze.
*/
int number_vehicles()
{
	check_time();
	return num_vehicles;
}

/*
** Puts the important settings into the given settings structure.
*/
int get_settings(si)
Settings_info *si;
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

int get_money()
{
	check_time();
	return (cv->owner->money);
}


/* how much one unit of fuel costs */

int get_fuel_cost()
{
	check_time();
	return engine_stat[cv->vdesc->engine].fuel_cost;
}


/* how much one unit of armor costs */

int get_armor_cost()
{
	check_time();
	return armor_stat[cv->armor.type].cost * body_stat[cv->vdesc->body].size;
}


/* how much one bullet for the indicated weapon costs */

int get_ammo_cost(wn)
WeaponNum wn;
{
	check_time();
	if (!legal_weapon(wn))
	{
		return BAD_VALUE;
	}
    return weapon_stat[(int)cv->weapon[(int)wn].type].ammo_cost;
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
void throw_discs(dspeed, delay)
float dspeed;
Boolean delay;
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
void spin_discs(dir)
    Spin dir;
{
	set_disc_orbit(cv, dir);
	check_time();
}

/*
** Returns the number of discs owned by the vehicle
*/
num_discs()
{
	check_time();
	return (cv->num_discs);
}

/*************
** Messages **
*************/

/*
** Returns number of messages waiting to be read.
*/
messages()
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
send_msg(recipient, opcode, data)
Byte recipient;
Opcode opcode;
char *data;
{
	int i;

	cv->sending.sender = cv->number;
	cv->sending.recipient = recipient;
	cv->sending.opcode = opcode;
	cv->sending.frame = frame;
	for (i = 0; i < MAX_DATA_LEN - 1; i++)
		cv->sending.data[i] = data[i];
	send_message(cv);

	check_time();
}

/*
** Copies the information from the next received messsage into the given
** message.
*/
receive_msg(m)
Message *m;
{
	Program *p;

	check_time();

	/* If no new messages return, else copy in message and increment counter */
	p = cv->current_prog;
	if (p->next_message == cv->next_message)
		return;
	*m = cv->received[p->next_message];
	if (++p->next_message >= MAX_MESSAGES)
		p->next_message = 0;
}

/*
** Returns the current amount of fuel in the vehicle
*/
float fuel()
{
	check_time();
	return (cv->fuel);
}

/*
** Returns the maximum amount of fuel the vehicle can hold
*/
float max_fuel()
{
	check_time();
	return (cv->max_fuel);
}

/*
** Returns the heat of the vehicle
*/
heat()
{
	check_time();
	return (cv->heat);
}

/*
** Returns the number of heat sinks in the vehicle
*/
heat_sinks()
{
	check_time();
	return (cv->vdesc->heat_sinks);
}

/*
** Tells whether or not the vehicle has a particular special
*/
has_special(st)
    SpecialType st;
{
	check_time();

    return (cv->special[(int)st].status != SP_nonexistent);
}

/*
** Returns the animation frame number
*/
frame_number()
{
	check_time();
	return frame;
}

/*
** Returns number of kills the vehicle has accrued this battle
*/
num_kills()
{
	check_time();
	return (cv->owner->kills);
}

/*
** Returns score of the vehicle
*/
score()
{
	check_time();
	return (cv->owner->score);
}

/*
** This function will not return until the next frame of execution,
** so calling it means giving up your remaining cpu time for this frame.
*/
done()
{
	stop_program();
}
