/***  bing.c - a hunter/killer xtank program ***/
/***   written by Richard S. Crosby for the  ***/
/***      Battle of the Xtanks tournament    ***/

#include <stdio.h>

#include "/mit/games/src/vax/xtank/Contest/xtanklib.h"

#include "math.h"

Location bing_loc, bing_tar_loc;
Blip_info bing_abs_loc, bing_tar_move, bing_tar_fire, bing_tar_abs_loc;
Blip_info bing_blip_info[MAX_BLIPS], bing_grid_loc, bing_tar_grid_loc;
Blip_info bing_tar_spd, bing_no_go[8], bing_search, bing_supply[4];
float bing_tar_angle;
int bing_tar, bing_vis, bing_blip, bing_stuck, bing_blip_on;
Vehicle_info bing_veh_info[MAX_VEHICLES];
char bing_message[40];

/**  the main loop  **/

bing_main ()
{
  bing_init_proc ();
  while (1) {
    bing_random ();
    get_vehicles (&bing_vis, bing_veh_info);
    get_blips (&bing_blip, bing_blip_info);
    if (bing_vis > 0)
      bing_vis_proc ();
    else if (bing_armor_proc () < 20)
      bing_supply_proc (3);
    else if ((bing_blip > 0) || (bing_blip_on == 1))
      bing_blip_proc ();
    else if (fuel () < (max_fuel () * 0.5))
      bing_supply_proc (1);
    else if (weapon_ammo (0) < (weapon_max_ammo (0) * 0.5))
      bing_supply_proc (2);
    else if (bing_armor_proc () < 60)
      bing_supply_proc (3);
    else bing_search_proc ();
    turn_vehicle (bing_tar_angle);
    turn_all_turrets (bing_tar_fire.x, bing_tar_fire.y);
    bing_fire_proc (bing_tar_fire);
    bing_move_proc ();
    message (bing_message);
  }
}

/*   check for landmarks   */

bing_chk_landmark_proc ()
{
  int x, y, z;
  for (x = bing_grid_loc.x - 1; x <= bing_grid_loc.x + 1; ++x)
    for (y = bing_grid_loc.y - 1; y <= bing_grid_loc.y +1; ++y) {
      z = landmark (x, y);
      bing_supply[z].x = x;
      bing_supply[z].y = y;
    }
}

/*   check for weakest armor   */

int bing_armor_proc ()
{
  int z = 100, i;
  for (i = 0; i < 4; ++i)
    if (armor (i) < z)
      z = armor (i);
  return (z);
}

/*   get angle from displacement   */

float bing_angle_proc (dloc)
Blip_info dloc;
{
  float ang;
  if (dloc.x == 0) {
    if (dloc.y > 0)
      return (PI / 2);
    else return (PI * 1.5);
  }
  ang = atan (dloc.y / dloc.x * 1.0);
  if (dloc.x < 0)
    ang = ang + PI;
  if (ang < 0)
    ang = ang + 2 * PI;
  return (ang);
}

/*   get range from displacement   */

float bing_range_proc (dloc)
Blip_info dloc;
{
  return (sqrt (1.0 * dloc.x * dloc.x + 1.0 * dloc.y * dloc.y));
}

/*   fire if in range   */

bing_fire_proc (dloc)
Blip_info dloc;
{
  int z;
  if ((bing_range_proc (dloc) <= weapon_range (0)) && (bing_vis > 0)) {
    for (z = 0; z < 6; ++z)
      turn_on_weapon (z);
    fire_all_weapons ();
  }
}

/*   move the vehicle   */

bing_move_proc ()
{
  float turn;
  int z;
  turn = fabs (bing_tar_angle - angle ());
  if (turn > PI)
    turn = 2 * PI - turn;
  if (fuel () <= (max_fuel () * .1))
    set_rel_speed (0.0);
  else if (bing_range_proc (bing_tar_move) == 0)
    set_rel_speed (0.0);
  else if (turn <= (PI / 4))
    set_abs_speed (max_speed ());
  else if (turn <= (PI / 2))
    set_rel_speed (5.0);
  else if (turn <= (PI * 0.75))
    set_rel_speed (2.0);
  else set_rel_speed (0.0);
  get_location (&bing_loc);
  bing_abs_loc.x = bing_loc.grid_x * BOX_WIDTH + bing_loc.box_x;
  bing_abs_loc.y = bing_loc.grid_y * BOX_HEIGHT + bing_loc.box_y;
  bing_grid_loc.x = bing_loc.grid_x;
  bing_grid_loc.y = bing_loc.grid_y;
  if (bing_grid_loc.x == bing_no_go[1].x)
    if (bing_grid_loc.y == bing_no_go[1].y)
      if (bing_grid_loc.x == bing_no_go[3].x)
	if (bing_grid_loc.y == bing_no_go[3].y)
	  bing_stuck = 1;
  if (bing_grid_loc.x == bing_no_go[0].x)
    if (bing_grid_loc.y == bing_no_go[0].y)
      return;
  bing_chk_landmark_proc ();
  if (bing_stuck = 1)
    for (z = 7; z > 3; --z)
      bing_no_go[z] = bing_no_go[z-1];
  for (z = 3; z > 0; --z)
    bing_no_go[z] = bing_no_go[z-1];
  bing_no_go[0] = bing_grid_loc;
}

/*   targets visible decision-maker   */

bing_vis_proc ()
{
  Blip_info dloc;
  int z, range;
  for (z = 0, bing_tar = 0, range = 9999; z < bing_vis ; ++z) {
    dloc.x = bing_veh_info[z].loc.grid_x * BOX_WIDTH
      + bing_veh_info[z].loc.box_x - bing_abs_loc.x;
    dloc.y = bing_veh_info[z].loc.grid_y * BOX_HEIGHT
      + bing_veh_info[z].loc.box_y - bing_abs_loc.y;
    if (bing_range_proc (dloc) < range) {
      bing_tar = z;
      range = bing_range_proc (dloc);
      bing_tar_abs_loc = dloc;
    }
  }
  bing_tar_loc = bing_veh_info[bing_tar].loc;
  bing_tar_grid_loc.x = bing_tar_loc.grid_x;
  bing_tar_grid_loc.y = bing_tar_loc.grid_y;
  bing_tar_spd.x = (int) bing_veh_info[bing_tar].xspeed;
  bing_tar_spd.y = (int) bing_veh_info[bing_tar].yspeed;
  bing_tar_move = bing_tar_abs_loc;
  bing_tar_fire = bing_tar_abs_loc;
  bing_nav_proc ();
  bing_tar_angle = bing_angle_proc (bing_tar_move);
  bing_blip_on = 0;
  sprintf (bing_message, "target in sight");
}

/*   radar blips decision-maker   */

bing_blip_proc ()
{
  Blip_info dloc;
  int z, range;
  if (bing_blip > 0) {
    for (z = 0, bing_tar = 0, range = 9999; z < bing_blip; ++z) {
      dloc.x = (bing_blip_info[z].x + 0.5) * BOX_WIDTH - bing_abs_loc.x;
      dloc.y = (bing_blip_info[z].y + 0.5) * BOX_HEIGHT - bing_abs_loc.y;
      if (bing_range_proc (dloc) < range) {
	bing_tar = z;
	range = bing_range_proc (dloc);
	bing_tar_abs_loc = dloc;
      }
    }
    bing_blip_on = 1;
    bing_tar_grid_loc = bing_blip_info[bing_tar];
  }
  bing_tar_move.x = (bing_tar_grid_loc.x + 0.5) * BOX_WIDTH
    - bing_abs_loc.x;
  bing_tar_move.y = (bing_tar_grid_loc.y + 0.5) * BOX_HEIGHT
    - bing_abs_loc.y;
  bing_tar_fire = bing_tar_move;
  bing_nav_proc();
  bing_tar_angle = bing_angle_proc (bing_tar_move);
  sprintf (bing_message, "homing on radar contact");
}

/*   look for supplies   */

bing_supply_proc (z)
int z;
{
  bing_disarm_proc ();
  if (bing_supply[z].x < 0)
    bing_search_proc ();
  else {
    bing_tar_move.x = (bing_supply[z].x + 0.5) * BOX_WIDTH
      - bing_abs_loc.x;
    bing_tar_move.y = (bing_supply[z].y + 0.5) * BOX_HEIGHT
      - bing_abs_loc.y;
    bing_tar_fire = bing_tar_move;
    bing_tar_grid_loc = bing_supply[z];
    bing_nav_proc ();
    bing_tar_angle = bing_angle_proc (bing_tar_move);
    sprintf (bing_message, "looking for supplies");
  }
}

/*   get out of stuck mode   */

bing_clear_stuck_proc ()
{
  int z;
  bing_stuck = 0;
  for (z = 0; z < 8; ++z)
    bing_no_go[z].x = -1;
}

/*   random number generator   */

int bing_random ()
{
  static int seed = 0;
  seed = (7 * seed + 11) % 100;
    return (seed);
}

/*   searching decision-maker   */

bing_search_proc ()
{
  if (bing_grid_loc.x == bing_search.x)
    if (bing_grid_loc.y == bing_search.y) {
      bing_search.x = GRID_WIDTH / 4
	+ bing_random () * GRID_WIDTH / 2 / 100;
      bing_search.y = GRID_HEIGHT / 4
	+ bing_random () * GRID_HEIGHT / 2 / 100;
  }
  bing_tar_move.x = (bing_search.x + 0.5) * BOX_WIDTH - bing_abs_loc.x;
  bing_tar_move.y = (bing_search.y + 0.5) * BOX_HEIGHT - bing_abs_loc.y;
  bing_tar_grid_loc = bing_search;
  bing_nav_proc ();
  bing_tar_fire = bing_tar_move;
  bing_tar_angle = bing_angle_proc (bing_tar_move);
  sprintf (bing_message, "search %d %d", bing_search.x, bing_search.y);
}

/*   navigation   */

bing_nav_proc ()
{
  Blip_info dloc;
  float dist;
  dloc.x = bing_tar_grid_loc.x - bing_grid_loc.x;
  dloc.y = bing_tar_grid_loc.y - bing_grid_loc.y;
  dist = bing_range_proc (dloc);
  if (dist < 0.5)
    bing_clear_stuck_proc ();
  else if (dist < 1.2)
    bing_one_str_proc (dloc);
  else if (dist < 1.7)
    bing_one_dia_proc (dloc);
  else if ((dloc.x == 0) || (dloc.y == 0))
    bing_guide_str_proc (dloc);
  else if (abs (dloc.x) == abs (dloc.y))
    bing_guide_dia_proc (dloc);
  else bing_guide_off_proc (dloc);
}

/*   nav - one square straight   */

bing_one_str_proc (dloc)
Blip_info dloc;
{
  int dir;
  float ang;
  ang = bing_angle_proc (dloc);
  dir = (int) ((ang + PI / 2) * 2 / PI) % 4;
  if (wall (dir, bing_grid_loc.x, bing_grid_loc.y) == 0)
    bing_clear_stuck_proc ();
  else bing_guide_str_proc (dloc);
}

/*   nav - one square diagonal   */

bing_one_dia_proc (dloc)
Blip_info dloc;
{
  int dirx, diry, blockx, blocky;
  if (dloc.x == 1)
    dirx = 1;
  else dirx = 3;
  if (dloc.y == 1)
    diry = 2;
  else diry = 0;
  blockx = wall (dirx, bing_grid_loc.x, bing_grid_loc.y)
    + wall (diry, bing_grid_loc.x + dloc.x, bing_grid_loc.y);
  blocky = wall (diry, bing_grid_loc.x, bing_grid_loc.y)
    + wall (dirx, bing_grid_loc.x, bing_grid_loc.y + dloc.y);
  if ((blockx > 0) && (blocky > 0))
    bing_guide_dia_proc (dloc);
  else if (blockx > 0) {
    dloc.x = 0;
    bing_guide_str_proc (dloc);
  }
  else if (blocky > 0) {
    dloc.y = 0;
    bing_guide_str_proc (dloc);
  }
  else bing_clear_stuck_proc ();
}

/*   guidance - straight   */

bing_guide_str_proc (dloc)
Blip_info dloc;
{
  int dir;
  if (dloc.x == 0) {
    if (dloc.y > 0)
      dir = 2;
    else dir = 0;
  }
  else if (dloc.x > 0)
    dir = 1;
  else  dir = 3;
  if (bing_obs_proc (dir) > 0) {
    if (bing_obs_proc ((dir + 3) % 4) <= 0)
      dir = (dir + 3) % 4;
    else if (bing_obs_proc ((dir + 1) % 4) <= 0)
      dir = (dir + 1) % 4;
    else if (bing_obs_proc ((dir + 2) % 4) <= 0)
      dir = (dir + 2) % 4;
    else {
      bing_panic_proc ();
      return;
    }
  }
  bing_control_proc (dir);
}

/*   guidance - diagonal   */

bing_guide_dia_proc (dloc)
Blip_info dloc;
{
  int dirx, diry, temp;
  if (dloc.x > 0)
    dirx = 1;
  else dirx = 3;
  if (dloc.y > 0)
    diry = 2;
  else diry = 0;
  if (diry == (dirx - 1)) {
    temp = dirx;
    dirx = diry;
    diry = temp;
  }
  if (bing_obs_proc (dirx) <= 0)
    bing_control_proc (dirx);
  else if (bing_obs_proc (diry) <= 0)
    bing_control_proc (diry);
  else if (bing_obs_proc ((diry + 2) % 4) <= 0)
    bing_control_proc ((diry + 2) % 4);
  else if (bing_obs_proc ((dirx + 2) % 4) <= 0)
    bing_control_proc ((dirx + 2) % 4);
  else bing_panic_proc ();
}

/*   guidance - oblique   */

bing_guide_off_proc (dloc)
Blip_info dloc;
{
  int dirx, diry, temp;
  if (dloc.x > 0)
    dirx = 1;
  else dirx = 3;
  if (dloc.y > 0)
    diry = 2;
  else diry = 0;
  if (abs (dloc.y) > abs (dloc.x)) {
    temp = dirx;
    dirx = diry;
    diry = temp;
  }
  if (bing_obs_proc (dirx) <= 0)
    bing_control_proc (dirx);
  else if (bing_obs_proc (diry) <= 0)
    bing_control_proc (diry);
  else if (bing_obs_proc ((diry + 2) % 4) <= 0)
    bing_control_proc ((diry + 2) % 4);
  else if (bing_obs_proc ((dirx + 2) % 4) <= 0)
    bing_control_proc ((dirx + 2) % 4);
  else bing_panic_proc ();
}

/*   obstruction finder   */

int bing_obs_proc (dir)
int dir;
{
  Blip_info grid;
  int blocked, z;
  if (wall (dir, bing_grid_loc.x, bing_grid_loc.y) > 0)
    return (1);
  grid = bing_grid_loc;
  if (dir == 0)
    grid.y = grid.y - 1;
  else if (dir == 1)
    grid.x = grid.x + 1;
  else if (dir == 2)
    grid.y = grid.y + 1;
  else grid.x = grid.x - 1;
  for (z = 0, blocked = -2; z < 4; ++z)
    blocked = blocked + wall (z, grid.x, grid.y);
  if (blocked > 0)
    return (1);
  if (bing_stuck == 0)
    return (0);
  for (blocked = 0, z = 0; z < 8; ++z)
    if (grid.x == bing_no_go[z].x)
      if (grid.y == bing_no_go[z].y)
	blocked = 1;
  return (blocked);
}

/*   movement control   */

bing_control_proc (dir)
int dir;
{
  if (dir == 0) {
    bing_tar_move.x = BOX_WIDTH / 2 - bing_loc.box_x;
    bing_tar_move.y = bing_grid_loc.y * BOX_HEIGHT - bing_abs_loc.y - 1;
  }
  else if (dir == 1) {
    bing_tar_move.x = (bing_grid_loc.x + 1) * BOX_WIDTH - bing_abs_loc.x;
    bing_tar_move.y = BOX_HEIGHT / 2 - bing_loc.box_y;
  }
  else if (dir == 2) {
    bing_tar_move.x = BOX_WIDTH / 2 - bing_loc.box_x;
    bing_tar_move.y = (bing_grid_loc.y + 1) * BOX_HEIGHT - bing_abs_loc.y;
  }
  else {
    bing_tar_move.x = bing_grid_loc.x * BOX_WIDTH - bing_abs_loc.x - 1;
    bing_tar_move.y = BOX_HEIGHT / 2 - bing_loc.box_y;
  }
}

/*   intialization   */

bing_init_proc ()
{
  bing_clear_stuck_proc ();
  bing_supply[1].x = -1;
  bing_supply[2].x = -1;
  bing_supply[3].x = -1;
  bing_search.x = GRID_WIDTH / 2;
  bing_search.y = GRID_HEIGHT / 2;
  get_location (&bing_loc);
  bing_grid_loc.x = bing_loc.grid_x;
  bing_grid_loc.y = bing_loc.grid_y;
  bing_abs_loc.x = bing_grid_loc.x * BOX_WIDTH + bing_loc.box_x;
  bing_abs_loc.y = bing_grid_loc.y * BOX_HEIGHT + bing_loc.box_y;
  bing_blip_on = 0;
}

/*   turn weapons off   */

bing_disarm_proc ()
{
  int z;
  for (z = 0; z < 6; ++z)
    turn_off_weapon (z);
}

/*   don't get stuck   */

bing_panic_proc ()
{
  int z, dir;
  for (z = 0, dir = 0; z < 4; ++z)
    if (wall (z, bing_grid_loc.x, bing_grid_loc.y) == 0)
      dir = z;
  bing_control_proc (dir);
}
