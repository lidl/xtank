/*
  file: tuna.c
  date: 25 april 1988
  auth: kirk johnson
  what: xtanks contest entry
*/

#include <math.h>
#include "/mit/games/src/vax/xtank/Contest/xtanklib.h"


#define TUNA_FAST   9.0
#define TUNA_NORM   6.0
#define TUNA_SLOW   4.0
#define TUNA_STOP   0.0
#define TUNA_BACK  -3.0

#define TUNA_HOLD  -1
#define TUNA_E      0
#define TUNA_S      1
#define TUNA_W      2
#define TUNA_N      3

#define TUNA_LOOK   2
#define TUNA_FAR    9999


static Blip_info tuna_blip[MAX_BLIPS];
static int       tuna_brng[MAX_BLIPS];
static int       tuna_near;
static int       tuna_nblips;
static int       tuna_act_map[16];
static int       tuna_some_dir;
static char      tuna_buf[80];
static int       tuna_dummy_x, tuna_dummy_y;


tuna_main()
{
  tuna_startup();
  
  while (1)
  {
    tuna_fire_control();
    
    /* choose movement mode */
    if (tuna_stuck())
      tuna_get_unstuck();
    
    else if (tuna_needs_armor())
      tuna_get_armor();

    else if (tuna_needs_fuel())
      tuna_get_fuel();

    else if (tuna_needs_ammo())
      tuna_get_ammo();

    else if (tuna_too_hot())
      tuna_cool_off();

    else if (tuna_no_enemy())
      tuna_chase();

    else
      tuna_engage();
  }
}


tuna_startup()
{
  tuna_nblips = 0;

  tuna_some_dir = random() % 4;

  tuna_dummy_x = random() % GRID_WIDTH;
  tuna_dummy_y = random() % GRID_HEIGHT;

  tuna_act_map[ 0] = TUNA_S;
  tuna_act_map[ 1] = TUNA_E;
  tuna_act_map[ 2] = TUNA_N;
  tuna_act_map[ 3] = TUNA_E;
  tuna_act_map[ 4] = TUNA_E;
  tuna_act_map[ 5] = TUNA_S;
  tuna_act_map[ 6] = TUNA_E;
  tuna_act_map[ 7] = TUNA_N;
  tuna_act_map[ 8] = TUNA_S;
  tuna_act_map[ 9] = TUNA_W;
  tuna_act_map[10] = TUNA_N;
  tuna_act_map[11] = TUNA_W;
  tuna_act_map[12] = TUNA_W;
  tuna_act_map[13] = TUNA_S;
  tuna_act_map[14] = TUNA_W;
  tuna_act_map[15] = TUNA_N;
}


tuna_stuck()
{
  return (speed() == 0);
}


tuna_needs_armor()
{
  int i, rslt;
  float ratio;

  rslt = 0;

  for (i=0; i<4; i++)
  {
    ratio  = armor(i);
    ratio /= max_armor(i);

    if (ratio < 0.25)
    {
      rslt = 1;
      break;
    }
  }

  return rslt;
}


tuna_needs_fuel()
{
  float ratio;

  ratio  = fuel();
  ratio /= max_fuel();

  return (ratio < 0.25);
}


tuna_needs_ammo()
{
  int i, nw;
  int rslt;
  float num, max;

  nw = num_weapons();
  rslt = 1;

  num = 0.0;
  max = 0.0;
  
  for (i=0; i<nw; i++)
  {
    num += weapon_ammo(i);
    max += weapon_max_ammo(i);
  }

  return (num/max < 0.25);
}


tuna_too_hot()
{
  return (heat() > 80);
}


tuna_no_enemy()
{
  int i, nv;
  int rslt;
  Location loc;
  Vehicle_info veh[MAX_VEHICLES];

  get_location(&loc);
  get_vehicles(&nv, veh);

  rslt = 0;

  for (i=0; i<nv; i++)
    if ((loc.grid_x == veh[i].loc.grid_x) &&
	(loc.grid_y == veh[i].loc.grid_y))
    {
      rslt = 1;
      break;
    }

  return (!rslt);
}


tuna_get_unstuck()
{
  int fnum;
  
  sprintf(tuna_buf, "get_unstuck %d", frame_number());
  message(tuna_buf);

  while (tuna_stuck())
  {
    set_rel_speed(TUNA_BACK);
    fnum = frame_number() + 3;
    while (frame_number() < fnum)
      tuna_fire_control();

    if (!tuna_stuck()) break;

    set_rel_speed(TUNA_FAST);
    turn_vehicle(angle() + PI/10);
    fnum = frame_number() + 3;
    while (frame_number() < fnum)
      tuna_fire_control();
  }
}


tuna_get_armor()
{
  int fnum;
  int dx, dy;
  Location loc;

  get_location(&loc);
  tuna_landmark(&dx, &dy, ARMOR);

  sprintf(tuna_buf, "get_armor %d (%2d, %2d)", frame_number(), dx, dy);
  message(tuna_buf);

  tuna_towards(dx, dy);
  
  if ((dx == loc.grid_x) && (dy == loc.grid_y))
  {
    while (!tuna_find_landmark())
      tuna_fire_control();

    fnum = frame_number() + 50;
    while (frame_number() < fnum)
      tuna_fire_control();
  }
}


tuna_get_fuel()
{
  int fnum;
  int dx, dy;
  Location loc;

  get_location(&loc);  
  tuna_landmark(&dx, &dy, FUEL);

  sprintf(tuna_buf, "get_fuel %d (%2d, %2d)", frame_number(), dx, dy);
  message(tuna_buf);

  tuna_towards(dx, dy);
  
  if ((dx == loc.grid_x) && (dy == loc.grid_y))
  {
    while (!tuna_find_landmark())
      tuna_fire_control();

    fnum = frame_number() + 50;
    while (frame_number() < fnum)
      tuna_fire_control();
  }
}


tuna_get_ammo()
{
  int fnum;
  int dx, dy;
  Location loc;

  get_location(&loc);
  tuna_landmark(&dx, &dy, AMMO);

  sprintf(tuna_buf, "get_ammo %d (%2d, %2d)", frame_number(), dx, dy);
  message(tuna_buf);

  tuna_towards(dx, dy);
  
  if ((dx == loc.grid_x) && (dy == loc.grid_y))
  {
    while (!tuna_find_landmark())
      tuna_fire_control();

    fnum = frame_number() + 50;
    while (frame_number() < fnum)
      tuna_fire_control();
  }
}


tuna_find_landmark()
{
  int dx, dy;
  float spd;
  double rg;
  Location loc;

  sprintf(tuna_buf, "homing on landmark");
  message(tuna_buf);

  get_location(&loc);

  dx = BOX_WIDTH/2  - loc.box_x;
  dy = BOX_HEIGHT/2 - loc.box_y;
  rg = sqrt(dx*dx + dy*dy);

  spd = BOX_WIDTH * TUNA_NORM / rg;
  if (spd > TUNA_NORM) spd = TUNA_NORM;
  if (rg < 25) spd = 0.0;

  turn_vehicle(atan2(dy, dx));
  set_rel_speed(spd);

  return (rg < 25);
}


tuna_cool_off()
{
  int dx, dy;
  
  sprintf(tuna_buf, "cool_off %d", frame_number());
  message(tuna_buf);

  tuna_landmark(&dx, &dy, ARMOR);

  /* check if we're there and take appropriate action */
  tuna_towards(dx, dy);
}


tuna_landmark(dx, dy, type)
int *dx, *dy;
int type;
{
  int i, nlm, idx;
  Landmark_info lmi[MAX_LANDMARKS];
  Location loc;

  get_location(&loc);
  get_landmarks(&nlm, lmi);

  idx = -1;
  for (i=0; i<nlm; i++)
    if (lmi[i].type == type)
    {
      i = idx;
      break;
    }

  if (idx >= 0)
  {
    *dx = lmi[idx].x;
    *dy = lmi[idx].y;
  }
  else
  {
    if ((loc.grid_x == tuna_dummy_x) &&
	(loc.grid_y == tuna_dummy_y))
    {
      tuna_dummy_x = random() % GRID_WIDTH;
      tuna_dummy_y = random() % GRID_HEIGHT;
    }

    *dx = tuna_dummy_x;
    *dy = tuna_dummy_y;
  }    
}


tuna_chase()
{
  int dx, dy;

  sprintf(tuna_buf, "chase %d", frame_number());
  message(tuna_buf);
  
  tuna_nearest_blip(&dx, &dy);

  tuna_towards(dx, dy);
}


tuna_engage()
{
  int act;
  Location loc;

  sprintf(tuna_buf, "engage %d", frame_number());
  message(tuna_buf);
  
  get_location(&loc);

  set_rel_speed(TUNA_SLOW);
  tuna_do(tuna_act_map[(loc.box_x*4/BOX_WIDTH)*4+
		       (loc.box_y*4/BOX_HEIGHT)]);
  set_rel_speed(TUNA_NORM);
}


tuna_blip_at(gx, gy)
int gx, gy;
{
  tuna_update_blips();

  return ((tuna_nblips != 0) &&
	  (tuna_brng[tuna_near] == 0));
}


tuna_nearest_blip(gx, gy)
int *gx, *gy;
{
  tuna_update_blips();

  if (tuna_nblips != 0)
  {
    *gx = tuna_blip[tuna_near].x;
    *gy = tuna_blip[tuna_near].y;
  }
  else
  {
    *gx = random() % GRID_WIDTH;
    *gy = random() % GRID_HEIGHT;
  }
}


tuna_update_blips()
{
  int i, nb;
  Location loc;

  get_location(&loc);
  get_blips(&nb, tuna_blip);
  
  if (nb != 0)
  {
    tuna_nblips = nb;

    for (i=0; i<nb; i++)
      tuna_brng[i] = tuna_d2(loc.grid_x, loc.grid_y,
			     tuna_blip[i].x, tuna_blip[i].y);

    tuna_near = 0;
    for (i=1; i<nb; i++)
      if (tuna_brng[i] < tuna_brng[tuna_near])
	tuna_near = i;
  }
}


tuna_towards(dx, dy)
int dx, dy;
{
  int act;
  Location loc;

  get_location(&loc);

  tuna_search(loc.grid_x, loc.grid_y, dx, dy, &act, TUNA_LOOK);
  tuna_do(act);
  if (act != TUNA_HOLD)
    set_rel_speed(TUNA_NORM);

  sprintf(tuna_buf, "towards %d", act);
  message(tuna_buf);

  return act;
}


tuna_search(sx, sy, dx, dy, act, level)
int sx, sy;
int dx, dy;
int *act;
int level;
{
  int i, dir;
  int dst[4];
  int rslt;

  /* if we are there, stop */
  if ((sx == dx) && (sy == dy))
  {
    rslt = 0;
    *act = TUNA_HOLD;
  }

  /* else if level is non-negative, recurse */
  else if (level >= 0)
  {
    if (wall(EAST, sx, sy))
      dst[TUNA_E] = TUNA_FAR;
    else
      dst[TUNA_E] = tuna_search(sx+1, sy, dx, dy, act, level-1);

    if (wall(WEST, sx, sy))
      dst[TUNA_W] = TUNA_FAR;
    else
      dst[TUNA_W] = tuna_search(sx-1, sy, dx, dy, act, level-1);

    if (wall(SOUTH, sx, sy))
      dst[TUNA_S] = TUNA_FAR;
    else
      dst[TUNA_S] = tuna_search(sx, sy+1, dx, dy, act, level-1);

    if (wall(NORTH, sx, sy))
      dst[TUNA_N] = TUNA_FAR;
    else
      dst[TUNA_N] = tuna_search(sx, sy-1, dx, dy, act, level-1);

    dir = tuna_some_dir;
    for (i=0; i<4; i++)
      if (dst[i] < dst[dir])
	dir = i;

    rslt = dst[dir];
    *act = dir;
  }

  /* else base case it */
  else
  {
    rslt = tuna_d2(sx, sy, dx, dy);
    *act = TUNA_HOLD;
  }

  rslt += 1;

  return rslt;
}


tuna_do(act)
{
  int rslt;

  switch (act)
  {
  case TUNA_E:
    turn_vehicle(0*PI/2);
    rslt = 0;
    break;
    
  case TUNA_S:
    turn_vehicle(1*PI/2);
    rslt = 0;
    break;
    
  case TUNA_W:
    turn_vehicle(2*PI/2);
    rslt = 0;
    break;
    
  case TUNA_N:
    turn_vehicle(3*PI/2);
    rslt = 0;
    break;
    
  case TUNA_HOLD:
    rslt = 1;
    break;
  }

  return rslt;
}


tuna_fire_control()
{
  int i, nw;
  int range;

  sprintf(tuna_buf, "fire_control %d", frame_number());
  message(tuna_buf);

  nw = num_weapons();
  if (tuna_aim_turret(&range))
  {
    sprintf(tuna_buf, "fire_control, range %d, %d", range, frame_number());
    message(tuna_buf);

    for (i=0; i<nw; i++)
      if (range < weapon_range(i))
	fire_weapon(i);
  }
}


tuna_aim_turret(rtn)
int *rtn;
{
  int i, nv;
  int rslt, idx;
  Vehicle_info veh[MAX_VEHICLES];
  double rng[MAX_VEHICLES];
  double ang[MAX_VEHICLES];
  double delay, accy;
  Location loc;
  
  get_location(&loc);
  get_vehicles(&nv, veh);

  for (i=0; i<nv; i++)
  {
    veh[i].loc.box_x -= loc.box_x;
    veh[i].loc.box_y -= loc.box_y;
    
    veh[i].loc.grid_x -= loc.grid_x;
    veh[i].loc.grid_y -= loc.grid_y;

    veh[i].loc.box_x += veh[i].loc.grid_x * BOX_WIDTH;
    veh[i].loc.box_y += veh[i].loc.grid_y * BOX_HEIGHT;

    rng[i] = sqrt((double) veh[i].loc.box_x * veh[i].loc.box_x +
		           veh[i].loc.box_y * veh[i].loc.box_y);

    delay = rng[i] / weapon_ammo_speed(0);

    veh[i].loc.box_x += veh[i].xspeed * delay;
    veh[i].loc.box_y += veh[i].yspeed * delay;
    
    rng[i] = sqrt((double) veh[i].loc.box_x * veh[i].loc.box_x +
		           veh[i].loc.box_y * veh[i].loc.box_y);

    ang[i] = atan2((double) veh[i].loc.box_y,
		   (double) veh[i].loc.box_x);
  }

  if (nv == 0)
  {
    rslt = 0;
    *rtn = TUNA_FAR;
  }
  else
  {
    idx = 0;
    for (i=1; i<nv; i++)
      if (rng[i] < rng[idx])
	idx = i;

    turn_turret(0, ang[idx]);
    ang[idx] -= turret_angle(0);

    accy = 10 / rng[idx];
    rslt = ((ang[idx] > -accy) && (ang[idx] < accy));
    *rtn = (int) rng[idx];
  }

  return rslt;
}


tuna_d2(x1, y1, x2, y2)
int x1, y1;
int x2, y2;
{
  int dx, dy;
  int rslt;

  dx = x1 - x2;
  dy = y1 - y2;
  rslt = dx*dx + dy*dy;

  return rslt;
}
