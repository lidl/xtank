/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** lowlib.c
*/

#include "xtank.h"
#define LIB
#include "xtanklib.h"
#include "map.h"
#include "vstructs.h"

extern int num_vehicles;
extern Vehicle *vehicle[];
extern Weapon_stat weapon_stat[];
extern Armor_stat armor_stat[];

#ifdef AMIGA
double temp_lowc;
#define drem(a,b) ( (temp_lowc = fmod(a,b)) > b/2 ? temp_lowc-b : temp_lowc )
#endif

#define legal_weapon(num) (num >= 0 && num < cv->num_weapons)
#define legal_turret(num) (num >= 0 && num < cv->num_turrets)
#define legal_armor(num) (num >= 0 && num < MAX_SIDES)
#define fix_angle(angle) angle -= (2*PI) * floor(angle/(2*PI))

/* Owner of disc if it is not orbiting anyone */
#define NO_OWNER        255

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
  vector->desired_heading = drem(desired_heading,2*PI);

  diff = vector->desired_heading - vector->heading;

  if(diff < 0.0)
    if(diff <= -PI) {		/* -2*PI <= diff <= -PI */
      vector->heading -= 2*PI;	/* make heading smaller than desired heading */
      vector->heading_flag = CLOCKWISE;
    }
    else			/* -PI < diff < 0.0 */
      vector->heading_flag = COUNTERCLOCKWISE;

  else if(diff > 0.0)
    if(diff < PI)		/* 0.0 < diff < PI */
      vector->heading_flag = CLOCKWISE;
    else {			/* PI <= diff <= 2*PI */
      vector->heading += 2*PI;	/* make angle heading than desired heading */
      vector->heading_flag = COUNTERCLOCKWISE;
    }

  else				/* diff = 0.0 */
    vector->heading_flag = NO_ROTATION;

  check_time();
}

/*
** Sets drive to abs_drive as quickly as possible
** abs_speed can vary from -max_speed() to max_speed()
*/
set_abs_drive(abs_drive)
     float abs_drive;
{
  Vector *vector;
  float diff;
  float max_drive;

  /* Make sure the drive is within the allowable range */
  max_drive = cv->vdesc->max_speed;
  if(abs_drive > max_drive) abs_drive = max_drive;
  else if(abs_drive < -max_drive) abs_drive = -max_drive;

  vector = &cv->vector;

  vector->desired_drive = abs_drive;
  diff = vector->desired_drive - vector->drive;

  if(diff < 0.0)
    vector->drive_flag = DECELERATING;
  else if(diff > 0.0)
    vector->drive_flag = ACCELERATING;
  else
    vector->drive_flag = NO_ACCELERATION;

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
  if(rel_drive > 9.0) rel_drive = 9.0;
  else if(rel_drive < -9.0) rel_drive = -9.0;
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
  if(status) cv->safety = TRUE;
  else cv->safety = FALSE;

  check_time();
}

/************
** Turrets **
************/

/*
** Turns the turret numbered num to the specified angle (in radians)
** where num is
**	0 = turret #1		1 = turret #2		2 = turret #3
**
** This is consistent with what the weapon_mount function returns
*/
turn_turret(num,angle)
     TurretNum num;
     Angle angle;
{
  float diff;
  Turret *t;

  if(legal_turret(num)) {
    t = &cv->turret[num];

    /* make sure desired angle is between -PI and PI */
    t->desired_angle = drem(angle,2*PI);

    diff = t->desired_angle - t->angle;

    /* start the turret turning in the proper direction */
    if(diff < 0.0)
      if(diff < -PI) {		/* -2*PI < diff < -PI */
	t->angle -= 2*PI;	/* make angle smaller than desired angle */
	t->angle_flag = CLOCKWISE;
      }
      else			/* -PI <= diff < 0.0 */
	t->angle_flag = COUNTERCLOCKWISE;

    else if(diff > 0.0)
      if(diff < PI)		/* 0.0 < diff < PI */
	t->angle_flag = CLOCKWISE;
      else {			/* PI <= diff <= 2*PI */
	t->angle += 2*PI;	/* make angle bigger than desired_angle */
	t->angle_flag = COUNTERCLOCKWISE;
      }

    else				/* diff = 0.0 */
      t->angle_flag = NO_ROTATION;
  }

  check_time();
}

/*
** Aims the turret numbered num at a location dx away horizontally
** and dy away vertically from the vehicle
*/
aim_turret(num,dx,dy)
     TurretNum num;
     int dx,dy;
{
  float ang;
  Coord *tcoord;
  Picinfo *picinfo;

  if(legal_turret(num)) {
    picinfo = &cv->obj->picinfo[cv->vector.rot];
    tcoord = &picinfo->turret_coord[num];
    
    /* compute the angle to turn the turret towards */
    ang = atan2((float) (dy - tcoord->y),
		(float) (dx - tcoord->x));

    turn_turret(num,ang);
  }

  /* Don't do a check_time since one is done in turn_turret */
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
  if(!legal_weapon(num)) retval = BAD_VALUE;
  else {
    retval = 0;
    w = &cv->weapon[num];
    if(w->status & WS_func)
      w->status |= WS_on;
  }

  check_time();
  return(retval);
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
  if(!legal_weapon(num)) retval = BAD_VALUE;
  else {
    retval = 0;
    w = &cv->weapon[num];
    if(w->status & WS_func)
      w->status &= ~WS_on;
  }

  check_time();
  return(retval);
}

/*
** Fires weapon numbered num.  Returns one of BAD_VALUE, FIRED,
** RELOADING, NO_AMMO, WEAPON_OFF or TOO_HOT.
*/
fire_weapon(num)
     WeaponNum num;
{
  Weapon *w;
  Weapon_stat *ws;
  Turret *t;
  float angle;
  Loc bloc;
  Coord *tcoord;
  int retval,i;

  /* Make sure the specified weapon exists */
  if(!legal_weapon(num)) {
    check_time();
    return BAD_VALUE;
  }

  w = &cv->weapon[num];

  if(w->reload_counter) retval = RELOADING; /* weapon has not reloaded yet */
  else if(w->status & WS_no_ammo) retval = NO_AMMO; /* weapon has no ammo */
  else if(!(w->status & WS_on)) retval = WEAPON_OFF; /* weapon is off */
  else if(cv->heat > 100) return TOO_HOT; /* vehicle is too hot to shoot */
  else {
    retval = FIRED;
    ws = &weapon_stat[w->type];

    /* Set the reload counter and decrement the ammo of the weapon */
    w->reload_counter = ws->reload_time;
    if(--w->ammo == 0)
      w->status |= WS_no_ammo;

    /* Heat up the vehicle by the proper amount */
    cv->heat += ws->heat;

    /* Copy the vehicle location for the starting bullet location */
    bloc = *cv->loc;

    /* Figure out what angle to shoot the bullet towards */
    switch(w->mount) {
      case MOUNT_TURRET1:
      case MOUNT_TURRET2:
      case MOUNT_TURRET3:
        /* Make the angle equal to where the turret will be this frame */
        t = &cv->turret[w->mount];
	angle = t->angle;
	switch(t->angle_flag) {
	  case NO_ROTATION:
	    break;
	  case CLOCKWISE:
	    if((angle += t->turn_rate) >= t->desired_angle)
	      angle = t->desired_angle;
	    break;
	  case COUNTERCLOCKWISE:
	    if((angle -= t->turn_rate) <= t->desired_angle)
	      angle = t->desired_angle;
	    break;
	  }
      
	/* Adjust the bullet location to the correct turret */
	tcoord = &cv->obj->picinfo[cv->vector.rot].turret_coord[w->mount];
	adjust_loc(&bloc,tcoord->x,tcoord->y);
	break;
      case MOUNT_FRONT:
	angle = cv->vector.heading;
	break;
      case MOUNT_BACK:
	angle = cv->vector.heading + PI;
	break;
      case MOUNT_LEFT:
	angle = cv->vector.heading - PI/2;
	break;
      case MOUNT_RIGHT:
	angle = cv->vector.heading + PI/2;
      }

    if(w->type == SLICK) {
      /* Make 3 oil slick bullets in a fan */
      for(i = 0 ; i < 3 ; i++)
	make_bullet(cv,&bloc,w->type,angle - PI/12 + i * PI/12);

      if(settings.commentator)
	comment(COS_SLICK_DROPPED,0,(Vehicle *) NULL,(Vehicle *) NULL);
    }
    else {
      /* Create the bullet with slight random fanning (1.8 degrees) */
      make_bullet(cv,&bloc,w->type,angle + PI/100 * (50 - rnd(101)) / 50);
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

  return(cv->vdesc->max_speed);
}

/*
** Returns the current speed of the vehicle
*/
float speed()
{
  check_time();

  return(cv->vector.speed);
}

/*
** Returns the current heading of the vehicle (in radians, 0 <= heading < 2*PI)
*/
Angle heading()
{
  float heading;

  check_time();

  heading = cv->vector.heading;
  fix_angle(heading);
  return(heading);
}

/*
** Returns the acceleration of the vehicle
*/
float acc()
{
  check_time();

  return(cv->vdesc->acc);
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
  if(abs(spd) > MAX_SPEED) return 0;
  return(cv->turn_rate[spd]);
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
vehicle_size(width,height)
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

/*
** Returns the number of turrets on the vehicle
*/
num_turrets()
{
  return cv->num_turrets;
}

/*
** Returns angle of the specified turret (in radians, 0 <= angle < 2*PI)
** If the specified turret doesn't exist, returns ((float) BAD_VALUE)
*/
Angle turret_angle(num)
     TurretNum num;
{
  float angle;
  check_time();

  /* Make sure the specified turret exists */
  if(!legal_turret(num)) return ((float) BAD_VALUE);

  angle = cv->turret[num].angle;
  fix_angle(angle);
  return(angle);
}

/*
** Returns turn rate of the specified turret (in radians/frame)
** If the specified turret doesn't exist, returns ((float) BAD_VALUE)
*/
float turret_turn_rate(num)
     TurretNum num;
{
  check_time();

  /* Make sure the specified turret exists */
  if(!legal_turret(num)) return ((float) BAD_VALUE);

  return(cv->turret[num].turn_rate);
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
  return(cv->num_weapons);
}

/*
** Puts constant information about weapon into weapon info structure,
** Returns BAD_VALUE if specified weapon does not exist.
*/
get_weapon(num,winfo)
     WeaponNum num;
     Weapon_info *winfo;
{
  Weapon *w;
  Weapon_stat *ws;

  check_time();
  if(!legal_weapon(num)) return BAD_VALUE;
  w = &cv->weapon[num];
  ws = &weapon_stat[w->type];

  winfo->type = w->type;
  winfo->mount = w->mount;
  winfo->damage = ws->damage;
  winfo->heat = ws->heat;
  winfo->range = ws->range;
  winfo->reload = ws->reload_time;
  winfo->max_ammo = ws->max_ammo;
  winfo->ammo_speed = ws->ammo_speed;
  return 0;
}

/*
** Returns number of frames before weapon can fire again.
*/
weapon_time(num)
     WeaponNum num;
{
  check_time();
  if(!legal_weapon(num)) return BAD_VALUE;
  return(cv->weapon[num].reload_counter);
}

weapon_ammo(num)
     WeaponNum num;
{
  check_time();
  if(!legal_weapon(num)) return BAD_VALUE;
  return(cv->weapon[num].ammo);
}

weapon_on(num)
     WeaponNum num;
{
  check_time();
  if(!legal_weapon(num)) return BAD_VALUE;
  return(cv->weapon[num].status & WS_on);
}


/**********
** Armor **
**********/

/*
** Returns amount of armor on the specified side as follows:
**	0	front		1	back
**	2	left		3	right
**	4	top		5	bottom
**
** If num < 0 or num > 5, returns BAD_VALUE
*/
armor(num)
     Side num;
{
  check_time();

  if(legal_armor(num))
    return(cv->armor.side[num]);
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

  if(legal_armor(num))
    return(cv->vdesc->armor.side[num]);
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

/*
** Returns pointer to the vehicle's map.
*/
Box **map_get()
{
  return (Box **) (((Mapper *) cv->special[MAPPER].record)->map);
}

/*
** Return the status of a wall of the specified box (x,y) in the specified
** direction, where the direction is one of the following values:
**        0      north
**        1      east
**        2      south
**        3      west
**
** Returns one of the following values:
**	0    no wall there that you know of
**	1    indestructible wall present
**	2    destructible wall present
**
** You can only detect the presence of a wall if you have a mapper and it is
** on your map.
*/
wall(dir,x,y)
     WallSide dir;
     int x,y;
{
  Special *s;
  Mapper *m;

  check_time();

  /* If the wall they are looking for is outside the grid, return 0 */
  if(x < 0 || x >= ((dir != EAST) ? GRID_WIDTH : GRID_WIDTH - 1) ||
     y < 0 || y >= ((dir != SOUTH) ? GRID_HEIGHT : GRID_HEIGHT - 1))
    return MAP_NONE;

  /* Make sure the vehicle has a mapper */
  s = &cv->special[MAPPER];
  if(s->status == SP_nonexistent) return MAP_NONE;

  m = (Mapper *) s->record;

  /* Check the appropriate wall on the map */
  switch(dir) {
    case NORTH:
      if(m->map[x][y].flags & NORTH_WALL) return MAP_WALL;
      break;
    case EAST:
      if(m->map[x+1][y].flags & WEST_WALL) return MAP_WALL;
      break;
    case SOUTH:
      if(m->map[x][y+1].flags & NORTH_WALL) return MAP_WALL;
      break;
    case WEST:
      if(m->map[x][y].flags & WEST_WALL) return MAP_WALL;
      break;
    }

  return MAP_NONE;
}

/*
** Tells you if there is a landmark in box (x,y).
** Returns one of the following values:
**	0  normal	4  goal		8  scroll e	12 scroll w
**	1  fuel		5  outpost	9  scroll se	13 scroll nw
**      2  ammo		6  scroll n	10 scroll s	14 slow
**      3  armor	7  scroll ne	11 scroll sw	15 slip	
**
** Returns BAD_VALUE if you don't have a mapper.
*/
landmark(x,y)
     int x,y;
{
  Special *s;
  Mapper *m;

  check_time();

  /* If the box they are looking for is outside the grid, return NORMAL */
  if(x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
    return NORMAL;

  /* Make sure the vehicle has a mapper */
  s = &cv->special[MAPPER];
  if(s->status == SP_nonexistent) return 0;

  m = (Mapper *) s->record;

  return((int) m->map[x][y].type);
}

/*
** Puts information about all landmarks your mapper has seen into the
** landmark_info array.  Returns BAD_VALUE if vehicle has no mapper.
*/
get_landmarks(num_landmark_infos,landmark_info)
     int *num_landmark_infos;
     Landmark_info landmark_info[]; /* Must have size >= MAX_LANDMARKS */
{
  Special *s;
  Mapper *m;
  int i;

  check_time();

  /* Make sure the vehicle has a mapper on */
  s = &cv->special[MAPPER];
  if(s->status == SP_nonexistent) return BAD_VALUE;

  m = (Mapper *) s->record;
  *num_landmark_infos = m->num_landmarks;

  /* Copy the landmark information into the landmark_info array */
  for(i = 0 ; i < m->num_landmarks ; i++) {
    landmark_info[i].x = m->landmark[i].x;
    landmark_info[i].y = m->landmark[i].y;
    landmark_info[i].type = m->landmark[i].type;
    landmark_info[i].team = box[landmark_info[i].x][landmark_info[i].y].team;
  }
  return 0;
}

/*
** Puts information about all radar blips into the blip_info array.
** Returns BAD_VALUE if vehicle has no radar.
*/
get_blips(num_blip_infos,blip_info)
     int *num_blip_infos;
     Blip_info blip_info[];	/* Must have size >= MAX_BLIPS */
{
  Special *s;
  Radar *r;
  int i;

  check_time();

  /* Make sure the vehicle has radar */
  s = &cv->special[RADAR];
  if(s->status == SP_nonexistent) return BAD_VALUE;

  r = (Radar *) s->record;
  *num_blip_infos = r->num_blips;

  /* Copy location information of each blip into the blip_info array */
  i = 0;
  while(i < r->num_blips) {
    blip_info[i].x = (r->blip[i].x - 5)/MAP_BOX_SIZE;
    blip_info[i].y = (r->blip[i].y - 5)/MAP_BOX_SIZE;
    i++;
  }
  return 0;
}

/*
** Puts information about all visible vehicles (excluding your own)
** into the vehicle_info array.
*/
get_vehicles(num_vehicle_infos,vehicle_info)
     int *num_vehicle_infos;
     Vehicle_info vehicle_info[]; /* Must have size >= MAX_VEHICLES */
{
  Vehicle *v;
  Vehicle_info *v_info;
  int dx,dy;
  int i;

  check_time();

  /* Initialize number of vehicle_infos to zero */
  *num_vehicle_infos = 0;

  /* Go through all vehicles, adding visible ones to the vehicle_info array */
  for(i = 0 ; i < num_vehicles ; i++) {
    v = vehicle[i];

    /* if the vehicle is our own, don't put it in the list */
    if(cv == v) continue;

    /* see if the vehicle is visible */
    dx = v->loc->x - cv->loc->x;
    dy = v->loc->y - cv->loc->y;
    if(dx > -ANIM_WIN_WIDTH/2 && dx < ANIM_WIN_WIDTH/2 &&
       dy > -ANIM_WIN_HEIGHT/2 && dy < ANIM_WIN_HEIGHT/2) {

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
get_bullets(num_bullet_infos,bullet_info)
     int *num_bullet_infos;
     Bullet_info bullet_info[];	/* Must have size >= MAX_BULLETS */
{
  extern Bset *bset;
  Bullet *b;
  Bullet_info *b_info;
  int dx,dy;
  int i;

  check_time();

  /* Initialize number of bullet_infos to zero */
  *num_bullet_infos = 0;

  /* Go through all bullets, adding visible ones to the bullet_info array */
  for(i = 0 ; i < bset->number ; i++) {
    b = bset->list[i];
    if(b->type == DISC) break;

    /* See if the bullet is visible */
    dx = b->loc->x - cv->loc->x;
    dy = b->loc->y - cv->loc->y;
    if(dx > -ANIM_WIN_WIDTH/2 && dx < ANIM_WIN_WIDTH/2 &&
       dy > -ANIM_WIN_HEIGHT/2 && dy < ANIM_WIN_HEIGHT/2) {

      /* Add a bullet to the list */
      b_info = &bullet_info[(*num_bullet_infos)++];

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
    }
  }
}

/*
** Puts information about all visible discs into the disc_info array.
*/
get_discs(num_disc_infos,disc_info)
     int *num_disc_infos;
     Disc_info disc_info[];	/* Must have size >= MAX_DISCS */
{
  extern Bset *bset;
  Bullet *b;
  Disc_info *b_info;
  int dx,dy;
  int i;

  check_time();

  /* Initialize number of disc_infos to zero */
  *num_disc_infos = 0;

  /* Go through all bullets, adding visible discs to the disc_info array */
  for(i = 0 ; i < bset->number ; i++) {
    b = bset->list[i];
    if(b->type != DISC) break;

    /* See if the disc is visible */
    dx = b->loc->x - cv->loc->x;
    dy = b->loc->y - cv->loc->y;
    if(dx > -ANIM_WIN_WIDTH/2 && dx < ANIM_WIN_WIDTH/2 &&
       dy > -ANIM_WIN_HEIGHT/2 && dy < ANIM_WIN_HEIGHT/2) {

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
      if(b->owner == (Vehicle *) NULL) {
	b_info->owner = NO_OWNER;
	b_info->angle = 0.0;
	b_info->spin = 0;
      }
      else {
	b_info->owner = b->owner->number;
	b_info->angle = atan2(b->loc->y - b->owner->loc->y,
			      b->loc->x - b->owner->loc->x);
	if(b_info->angle < 0.0) b_info->angle += 2*PI;

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
  int i,retval;

  check_time();
  retval = BAD_VALUE;
  for(i = 0 ; i < num_vehicles ; ++i)
    if(vehicle[i]->number == vid)
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
  si->ricochet         = settings.ricochet;
  si->no_wear          = settings.no_wear;
  si->full_map         = settings.full_map;
  si->game             = settings.game;
  si->winning_score    = settings.winning_score;
  si->outpost_strength = settings.outpost_strength;
  si->scroll_speed     = settings.scroll_speed;
  si->box_slowdown     = settings.box_slow;
  si->disc_friction    = settings.disc_friction;
  si->owner_slowdown   = settings.disc_slow;
}

/******************
** Miscellaneous **
******************/

/**********
** Discs **
**********/

/*
** Throws all discs owned by vehicle at the desired speed.
** Speed must be between 0 and 25.
** If delay is TRUE, the discs will move in next frame's direction.
**
*/
throw_discs(speed,delay)
     float speed;
     Boolean delay;
{
  if(speed < 0.0) speed = 0.0;
  else if(speed > 25.0) speed = 25.0;
  release_discs(cv,speed,delay);
  check_time();
}

/*
** Spins all discs owned by vehicle in the specified direction
** dir can be one of COUNTERCLOCKWISE, CLOCKWISE, or TOGGLE.
*/
spin_discs(dir)
{
  set_disc_orbit(cv,dir);
  check_time();
}

/*
** Returns the number of discs owned by the vehicle
*/
num_discs()
{
  check_time();
  return(cv->num_discs);
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
  return((cv->current_prog->next_message - cv->next_message + MAX_MESSAGES)
	 % MAX_MESSAGES);
}

/*
** Sends a message to the recipient with the given opcode and data.
** Only MAX_DATA_LEN-1 bytes of the data are put into the message,
** to ensure that a text message will be null-terminated.
*/
send(recipient,opcode,data)
     Byte recipient, opcode;
     Byte *data;
{
  int i;

  cv->sending.sender = cv->number;
  cv->sending.recipient = recipient;
  cv->sending.opcode = opcode;
  for(i = 0 ; i < MAX_DATA_LEN-1 ; i++)
    cv->sending.data[i] = data[i];
  send_message(cv);

  check_time();
}

/*
** Copies the information from the next received messsage into the given
** message.
*/
receive(m)
     Message *m;
{
  Program *p;

  check_time();

  /* If no new messages return, else copy in message and increment counter */
  p = cv->current_prog;
  if(p->next_message == cv->next_message) return;
  *m = cv->received[p->next_message];
  if(++p->next_message >= MAX_MESSAGES) p->next_message = 0;
}

/*
** Returns the current amount of fuel in the vehicle
*/
float fuel()
{
  check_time();
  return(cv->fuel);
}

/*
** Returns the maximum amount of fuel the vehicle can hold
*/
float max_fuel()
{
  check_time();
  return(cv->max_fuel);
}

/*
** Returns the heat of the vehicle
*/
heat()
{
  check_time();
  return(cv->heat);
}

/*
** Returns the number of heat sinks in the vehicle
*/
heat_sinks()
{
  check_time();
  return(cv->vdesc->heat_sinks);
}

/*
** Tells whether or not the vehicle has a particular special,
** where num is one of the following:
**       0       Console
**       1       Mapper
**       2       Radar
*/
has_special(num)
     int num;
{
  check_time();

  if(cv->special[num].status == SP_nonexistent) return 0;
  else return 1;
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
  return(cv->owner->kills);
}

/*
** This function will not return until the next frame of execution,
** so calling it means giving up your remaining cpu time for this frame.
*/
done()
{
  stop_program();
}
