/*-
 * Copyright (c) 1991 William T. Katz
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
     Terminator: An xtank pilot which terminates opposition without prejudice.
     Version 3.0: October 1, 1991
     William T. Katz, <wk5w@virginia.edu>
     Univ. of Virginia Medical Scientist Training Program.

     Terminator is a fairly intelligent pilot designed for semi-kamikaze
     robot tanks.  It assumes a tank with both short-range, front-mounted
     (acid/flame throwers) and long-range, turret-mounted weapons.  It will
     pursue blips in search of enemies and seek depots when resources are
     needed.  It assumes hover-craft design so no mine detection is needed.

     Capabilities:

     + Plots course through a maze to a given destination trying to keep
       course changes to a minimum (so speed can be built).

     + Seeks appropriate depots for loading resources.

     + Attempts to kill attacking outposts. A better aiming system would help.

     + If it has no goal, program randomly explores the maze.

     + If tank hits wall (i.e. stops movement for some number of frames),
       goal is reset.

     Needs:

     - Better aiming system.

     - More rigorous (i.e. based on formulas) to handling speed/movement
       taking into account tread acceleration, etc.

     - Consideration of friendly tanks when pursuing/firing on enemies.

     - Better handling of mapping so that it has memory and can function
       more effectively in the abscence of a full map.
*/

#include <stdio.h>
#include <math.h>
#include <xtanklib.h>

static void terminator_main();

static Settings_info settings; 

Prog_desc terminator_prog =
{
  "terminator",
  "Terminator",
  "Seek out and kill the enemy.  Has long-range and short-range modes.\n",
  "William Katz",
  PLAYS_COMBAT | DOES_SHOOT | DOES_REPLENISH | USES_MESSAGES,
  8,
  terminator_main
};

/*** File specific definitions. ***/

#define MAXIMUM     3   /* Status of armor, ammo, and fuel. */
#define OPERATIONAL 2
#define UNSTABLE    1
#define CRITICAL    0

#define SHORT_RANGE 0   /* Basic categories of armaments. */
#define LONG_RANGE  1

#define MaxMoves      900  /* Should be GRID_HEIGHT * GRID_WIDTH */
#define NumDirections   4  /* This should be max directional constant + 1. */
#define MaxLandmarks  200  /* This can be more than MAX_LANDMARKS. */

/*** Global variables.  Declared in struct so it is local to this file. ***/

typedef struct {

  int team_id;

  int max_armor[MAX_SIDES];
  int min_armor;       /* Least protected side armor value. */
  int armor[MAX_SIDES];
  int taking_damage;   /* If it's positive, then we have history of hits. */
  int taking_mines;   
  int critical_sides;

  int num_weapons;
  int weapon_ok[MAX_WEAPONS];
  int weapon_on[MAX_WEAPONS];
  int weapon_type[MAX_WEAPONS];
  int ammo[MAX_WEAPONS];
  Weapon_info weapons[MAX_WEAPONS];
  int max_fuel;

  float max_speed;
  float accel;
  int vheight, vwidth;

  int enemy_near, outpost_near;
  int last_enemy_sighting;
  int armor_value, ammo_value, fuel_value;
  int money;

  LandmarkType purpose;    /* Reason for moving along a path. */
  int num_landmarks;
  int num_fuel, num_armor, num_ammo;
  Landmark_info landmarks[MaxLandmarks];

  Box (*map)[GRID_HEIGHT];
  int grid_x, grid_y;      /* Starting coord of path. */
  int dgrid_x, dgrid_y;    /* Ending coord of path.   */
  int dx, dy;
  int last_dx, last_dy;
  int ddist;               /* The length in grid points of the path. */
  char dir[MaxMoves];      /* The path to a given destination.       */

} TermVars;

/*** Resource monitoring procedures. ***/

static void check_armor(vars)
  TermVars *vars;
{
  int side;
  int value, damaged, mine_field;
  float critical_val, unstable_val;

  vars->armor_value = MAXIMUM;
  vars->critical_sides = 0;
  damaged = mine_field = FALSE;

  for (side = 0; side < MAX_SIDES; side++) {
    if (side == BOTTOM && vars->max_armor[BOTTOM] <= 0)  /* If we're hover. */
      continue;
    value = armor(side);
    if (value < vars->armor[side]) {
      if (side == BOTTOM)
	mine_field = TRUE;
      else
	damaged = TRUE;
    }
    vars->armor[side] = value;

    if (side == TOP && vars->enemy_near) {
      critical_val = 0.1;  /* Mainly heat seekers so allow close pursuit. */
      unstable_val = 0.1;
    }
    else if (side == BOTTOM) {
      critical_val = 0.6;  /* Don't pursue if mines. */
      unstable_val = 0.95;
    }
    else {
      critical_val = 0.35;
      unstable_val = 0.7;
    }

    if (value < vars->max_armor[side] * critical_val) {
      vars->armor_value = CRITICAL;
      vars->critical_sides++;
    }
    else if (vars->armor_value > UNSTABLE && 
	     value < vars->max_armor[side] * unstable_val)
      vars->armor_value = UNSTABLE;
    else if (vars->armor_value == MAXIMUM && value < vars->max_armor[side])
      vars->armor_value = OPERATIONAL;
  }

  if (damaged)
    vars->taking_damage = 10;   /* Ten frames to clear. */
  else if (vars->taking_damage)
    vars->taking_damage--;

  if (mine_field) {
    vars->taking_mines = 20;    /* Ten frames to clear; Shoot in place. */
    send_msg(RECIPIENT_ALL, OP_TEXT, "In mine field.");
  }
  else if (vars->taking_mines)
    vars->taking_mines--;
  if (vars->taking_mines)
    set_abs_drive(0.0);
}

static void check_ammo(vars)
  TermVars *vars;
{
  int i;
  int value;

  vars->ammo_value = MAXIMUM;
  for (i = 0; i < vars->num_weapons; i++) {
    value = weapon_ammo(i);
    vars->ammo[i] = value;
    if (value < vars->weapons[i].max_ammo * 0.25)
      vars->ammo_value = CRITICAL;
    else if (vars->ammo_value >= OPERATIONAL &&
	     value < vars->weapons[i].max_ammo * 0.6)
      vars->ammo_value = UNSTABLE;
    else if (vars->ammo_value == MAXIMUM &&
	     value < vars->weapons[i].max_ammo)
      vars->ammo_value = OPERATIONAL;
  }
}

static void check_fuel(vars)
  TermVars *vars;
{
  int value;

  value = fuel();
  vars->fuel_value = MAXIMUM;
  if (value < vars->max_fuel * 0.25)
    vars->fuel_value = CRITICAL;
  else if (value < vars->max_fuel * 0.4)
    vars->fuel_value = UNSTABLE;
  else if (value < vars->max_fuel)
    vars->fuel_value = OPERATIONAL;
}

/*** Initialization.  Find out what we have. ***/

static void terminator_init(vars)
  TermVars *vars;
{
  Vehicle_info us;
  int i;

  vars->min_armor = 9999999;
  for (i = 0; i < MAX_SIDES; i++) {
    vars->max_armor[i] = max_armor(i);
    if (i < TOP && vars->max_armor[i] < vars->min_armor)
      vars->min_armor = vars->max_armor[i];
  }
  vars->taking_damage = 0;
  vars->taking_mines = 0;
  check_armor(vars);

  vars->num_weapons = num_weapons();
  for (i = 0; i < vars->num_weapons; i++) {
    get_weapon(i, vars->weapons + i);
    vars->weapon_on[i] = FALSE;
    vars->weapon_ok[i] = TRUE;
    turn_off_weapon(i);
    if (vars->weapons[i].mount >= MOUNT_TURRET1 && 
	vars->weapons[i].mount <= MOUNT_TURRET3)
      vars->weapon_type[i] = LONG_RANGE;
    else
      vars->weapon_type[i] = SHORT_RANGE;
  }
  check_ammo(vars);
  vars->max_fuel = max_fuel();
  check_fuel(vars);

  vehicle_size(&(vars->vheight), &(vars->vwidth));

  vars->enemy_near = FALSE;
  vars->outpost_near = FALSE;
  vars->last_enemy_sighting = -1;
  vars->purpose = NORMAL;

  get_location(&us);
  vars->dgrid_x = -1;
  vars->ddist = -1;
  vars->last_dx = vars->last_dy = -1;
  vars->team_id = MAX_VEHICLES + team(us.id);

  set_abs_drive(0.0);
  vars->max_speed = max_speed();
  vars->accel = tread_acc();
}

/*** Initialize our landmark database so we don't lose any info. ***/

static void init_landmarks(vars)
  TermVars *vars;
{
  LandmarkType type;
  int x, y;
  int num;

  vars->map = map_get();   /* This should be replaced to get some memory. */

  vars->num_fuel = vars->num_armor = vars->num_ammo = 0;
  num = 0;
  for (x = 0; x < GRID_WIDTH; x++)
    for (y = 0; y < GRID_HEIGHT; y++)
      if ((type = map_landmark(vars->map, x, y)) != NORMAL) {
	vars->landmarks[num].type = type;
	switch (type) {
	case FUEL:
	  vars->num_fuel++;
	  break;
	case ARMOR:
	  vars->num_armor++;
	  break;
	case AMMO:
	  vars->num_ammo++;
	  break;
	}
	vars->landmarks[num].x = x;
	vars->landmarks[num].y = y;
	num++;
      }
  vars->num_landmarks = num;
}

/*** Weapon arming and firing procedures. ***/

static int discharge_weapon(vars, i, dist)
  TermVars *vars;
  int i, dist;
{
  int range, fired;

  range = vars->weapons[i].range;
  range *= range;
  fired = FALSE;
  if (dist <= range) {
    if (!(vars->weapon_on[i])) {
      turn_on_weapon(i);
      vars->weapon_on[i] = TRUE;
    }
    if (fire_weapon(i) == FIRED) {
      vars->ammo[i]--;
      if (vars->ammo_value != CRITICAL && 
	  vars->ammo[i] < vars->weapons[i].max_ammo * 0.25)
	vars->ammo_value = CRITICAL;
      else if (vars->ammo_value == OPERATIONAL &&
	       vars->ammo[i] < vars->weapons[i].max_ammo * 0.6)
	vars->ammo_value = UNSTABLE;
      else if (vars->ammo_value == MAXIMUM &&
	       vars->ammo[i] < vars->weapons[i].max_ammo)
	vars->ammo_value = OPERATIONAL;
      fired = TRUE;
    }
  }
  return(fired);
}

/*** Toggle the armament to maximize reloading while readying for enemy. ***/

static void toggle_ammo(vars)
  TermVars *vars;
{
  int i;
  int max_ammo[2], max_num[2];

  if (vars->enemy_near) {         /* If enemy, keep some guns on. */
    max_ammo[0] = max_ammo[1] = -1;
    for (i = 0; i < vars->num_weapons; i++)
      if (vars->ammo[i] > max_ammo[vars->weapon_type[i]]) {
	max_ammo[vars->weapon_type[i]] = vars->ammo[i];
	max_num[vars->weapon_type[i]] = i;
      }
    for (i = 0; i < vars->num_weapons; i++)
      if (i == max_num[vars->weapon_type[i]])
	vars->weapon_ok[i] = TRUE;
      else {
	if (vars->weapon_on[i]) {
	  vars->weapon_on[i] = FALSE;
	  turn_off_weapon(i);
	}
	vars->weapon_ok[i] = FALSE;
      }
  }
  else                            /* If no enemy, can afford all guns off. */
    for (i = 0; i < vars->num_weapons; i++) {
      if (vars->weapon_on[i]) {
	vars->weapon_on[i] = FALSE;
	turn_off_weapon(i);
      }
      vars->weapon_ok[i] = FALSE;
    }
}

/*** Plot a course to a given destination. Finds good result in minimal
     time by visiting each grid box only once.  Grows borders from our
     current position until we reach the destination or no more growing
     can occur.  Then we simply backtrack through best path.               ***/

#define N_DIR    0
#define S_DIR    1
#define W_DIR    2
#define E_DIR    3
#define I_DIR    4

static found_path(vars)
  TermVars *vars;
{
  Location cur;
  unsigned char px[MaxMoves], py[MaxMoves];
  int best_score[GRID_WIDTH][GRID_HEIGHT];
  char gradient[GRID_WIDTH][GRID_HEIGHT];
  int x, y;
  int bottom, top, i;
  int last_dir;
  char dir_flag[5];  /* This is to prevent constant course changing. */

  dir_flag[N_DIR] = 0x01;
  dir_flag[S_DIR] = 0x02;
  dir_flag[W_DIR] = 0x04;
  dir_flag[E_DIR] = 0x08;
  dir_flag[I_DIR] = 0;

  for (x = 0; x < GRID_WIDTH; x++)
    for (y = 0; y < GRID_HEIGHT; y++) {
      best_score[x][y] = -1;
      gradient[x][y] = 0;
    }

  get_location(&cur);
  vars->grid_x = x = cur.grid_x;
  vars->grid_y = y = cur.grid_y;
  vars->ddist = ABS(vars->dgrid_x - x) + ABS(vars->dgrid_y - y);
  if (vars->ddist == 0) {
    vars->ddist = 0;
    return(TRUE);
  }

  best_score[x][y] = 0;
  top = 1;
  bottom = 0;
  px[0] = x;
  py[0] = y;
  do {
    /* Pull one off the queue. */

    x = px[bottom];
    y = py[bottom];
    bottom++;
    if (landmark(x,y) >= SCROLL_N && landmark(x,y) <= SCROLL_NW)
      continue;   /* Avoid the scroll landmarks. */

    /* Visit it's neighbors which have not been already queued. */

    if (!map_wall(vars->map, NORTH, x, y)) {
      if (best_score[x][y-1] == -1) {
	px[top] = x;
	py[top] = y-1;
	best_score[x][y-1] = best_score[x][y] + 1;
	gradient[x][y-1] |= dir_flag[S_DIR];
	top++;
      }
      else if (best_score[x][y-1] == best_score[x][y] + 1)
	gradient[x][y-1] |= dir_flag[S_DIR];
    }
    if (!map_wall(vars->map, SOUTH, x, y)) {
      if (best_score[x][y+1] == -1) {
	px[top] = x;
	py[top] = y+1;
	best_score[x][y+1] = best_score[x][y] + 1;	
	gradient[x][y+1] |= dir_flag[N_DIR];
	top++;
      }
      else if (best_score[x][y+1] == best_score[x][y] + 1)
	gradient[x][y+1] |= dir_flag[N_DIR];
    }
    if (!map_wall(vars->map, WEST, x, y)) {
      if (best_score[x-1][y] == -1) {
	px[top] = x-1;
	py[top] = y;
	best_score[x-1][y] = best_score[x][y] + 1;	
	gradient[x-1][y] |= dir_flag[E_DIR];
	top++;
      }
      else if (best_score[x-1][y] == best_score[x][y] + 1)
	gradient[x-1][y] |= dir_flag[E_DIR];
    }
    if (!map_wall(vars->map, EAST, x, y)) {
      if (best_score[x+1][y] == -1) {
	px[top] = x+1;
	py[top] = y;
	best_score[x+1][y] = best_score[x][y] + 1;	
	gradient[x+1][y] |= dir_flag[W_DIR];
	top++;
      }
      else if (best_score[x+1][y] == best_score[x][y] + 1)
	gradient[x+1][y] |= dir_flag[W_DIR];
    }
  } while (bottom < top && (x != vars->dgrid_x || y != vars->dgrid_y));

  if (best_score[vars->dgrid_x][vars->dgrid_y] == -1) {
    vars->ddist = -1;
    return(FALSE);
  }

    /* Should protect against destination being a scrollbar as might happen in
       the case of enemy pursuit. */

  x = vars->dgrid_x;
  y = vars->dgrid_y;
  vars->ddist = best_score[x][y];
  last_dir = I_DIR;
  
  do {
    i = best_score[x][y] - 1;
    if (gradient[x][y] & dir_flag[last_dir])
      vars->dir[i] = last_dir;
    else if (gradient[x][y] & dir_flag[N_DIR])
      vars->dir[i] = N_DIR;
    else if (gradient[x][y] & dir_flag[S_DIR])
      vars->dir[i] = S_DIR;
    else if (gradient[x][y] & dir_flag[W_DIR])
      vars->dir[i] = W_DIR;
    else if (gradient[x][y] & dir_flag[E_DIR])
      vars->dir[i] = E_DIR;
    last_dir = vars->dir[i];
    
    switch (vars->dir[i]) {
    case N_DIR:
      vars->dir[i] = SOUTH;
      y--;
      break;
    case S_DIR:
      vars->dir[i] = NORTH;
      y++;
      break;
    case W_DIR:
      vars->dir[i] = EAST;
      x--;
      break;
    case E_DIR:
      vars->dir[i] = WEST;
      x++;
      break;
    }
  } while (x != cur.grid_x || y != cur.grid_y);

  return(TRUE);
}

/*** Deal with the enemy. ***/

static void fire_control(vars, move, turn)
  TermVars *vars;
  int move, turn;
{
  Vehicle_info target, us;
  Location tloc;
  Angle to_enemy, away_enemy, angle;
  int tx, ty;          /* anticipated location of target */
  int x, y, dx, dy, dist;
  double lead;
  Side side, best;
  int damage, least_damage, most_damage;
  int i, short_ok, long_ok, frame;

  /* Check for enemies. */

  get_self(&us);

  if (!get_closest_enemy(&target)) {
    vars->enemy_near = FALSE;
    if (speed() < 0.0)
      set_abs_drive(0.0);
  }
  else {
    vars->enemy_near = TRUE;
    vars->last_enemy_sighting = frame_number();
  }

  check_armor(vars);
  if (vars->taking_mines) {
    move = FALSE;
    turn = TRUE;
  }

  if (!vars->enemy_near) {
    if (vars->taking_damage) {     /* Check for an outpost. */
      init_landmarks(vars);
      for (dx = -2; dx <= 2; dx++)
	for (dy = -2; dy <= 2; dy++)
	  if (landmark(us.loc.grid_x+dx, us.loc.grid_y+dy) == OUTPOST) {
	    x = y = 0;
	    for (i = 0; i < 33; i += 4) {
	      get_outpost_loc(us.loc.grid_x + dx, us.loc.grid_y + dy, 
			      frame_number() + i, &tx, &ty);
	      x += tx;
	      y += ty;
	    }
	    vars->outpost_near = TRUE;
	    tx = x / 9;
	    ty = y / 9;
	    tloc.x = tx;
	    tloc.y = ty;
	    tloc.grid_x = us.loc.grid_x + dx;
	    tloc.grid_y = us.loc.grid_y + dy;
	    if (clear_path(&(us.loc), &tloc))
	      goto kill_outpost;
	  }
    }
    return;
  }
  
  vars->outpost_near = FALSE;

  /* Get locations. */

  dx = target.loc.x - us.loc.x;
  dy = target.loc.y - us.loc.y;
  lead = ABS(5.0 - (double) ABS(dy) / 100.0);
  if (lead < 0.0)
    lead = 0.0;
  tx = target.loc.x + lead * target.xspeed;
  lead = ABS(5.0 - (double) ABS(dx) / 100.0);
  if (lead < 0.0)
    lead = 0.0;
  ty = target.loc.y + lead * target.yspeed;

kill_outpost:

  dx = tx - us.loc.x - us.yspeed;
  dy = ty - us.loc.y - us.xspeed;
  if (vars->outpost_near)
    dist = (dx * dx + dy * dy) / 2;    
  else
    dist = dx * dx + dy * dy;
  to_enemy = ATAN2(dy, dx);
  if (to_enemy < 0.0)
    to_enemy += 2.0 * PI;
  turn_all_turrets(to_enemy);

  /* Turn to prevent damages. */

  most_damage = 1000;
  least_damage = -1;
  best = FRONT;
  for (side = FRONT; side < TOP; side++) {
    damage = vars->armor[side];
    if (damage < most_damage)
      most_damage = damage;
    if (damage > least_damage) {
      least_damage = damage;
      best = side; 
    }
  }
  short_ok = long_ok = TRUE;
  if (turn) {
    if (most_damage < (vars->min_armor / 2) && vars->money >= 100) {
      short_ok = FALSE;
      if (best == FRONT)
	angle = 0.0;
      else if (best == LEFT)
	angle = - PI / 2.0;
      else if (best == RIGHT)
	angle = PI / 2.0;
      else
	angle = PI;
      away_enemy = to_enemy - angle;
      if (away_enemy < 0.0)
	away_enemy += 2.0 * PI;
      turn_vehicle(away_enemy);
    }
  }
  else
    short_ok = FALSE;

  /* Bring short and/or long-range armaments to bear. */

  if (short_ok) {
    turn_vehicle(to_enemy);
    if (move && speed() != vars->max_speed)
      set_abs_drive(vars->max_speed);
    for (i = 0; i < vars->num_weapons; i++)
      if (vars->weapon_ok[i] && vars->weapon_type[i] == SHORT_RANGE)
	discharge_weapon(vars, i, dist);
  }

  angle = PI * (1000.0 * 1000.0 - dist) / (4.0 * 1000.0 * 1000.0);
  if (angle < 0.0)
    angle = 0.0;
  if (long_ok)
    for (i = 0; i < vars->num_weapons; i++)
      if (vars->weapon_ok[i] && vars->weapon_type[i]==LONG_RANGE
	  && ((vars->weapons[i].mount == TURRET1 && 
	       ABS(turret_angle(0) - to_enemy) < 0.025 + angle) ||
	      (vars->weapons[i].mount==TURRET2 && 
	       ABS(turret_angle(1) - to_enemy) < 0.025 + angle) ||
	      (vars->weapons[i].mount==TURRET3 && 
	       ABS(turret_angle(2) - to_enemy) < 0.025 + angle)))
	discharge_weapon(vars, i, dist);
}

/*** Spot the nearest landmark of a given type. ***/

static found_landmark(vars, type, minimize)
  TermVars *vars;
  LandmarkType type;
  int minimize;    /* Get to close or far depot. */
{
  Location our_loc;
  int i, dx, dy;
  int best, dist;
  char unused[MaxLandmarks];
  int msign;

  if (minimize)
    msign = 1;
  else
    msign = -1;

  /* Get positions. */

  get_location(&our_loc);

  /* Initialize landmark list to all unused. */

  for (i = 0; i < vars->num_landmarks; i++)
    unused[i] = TRUE;

  do {

    /* See if there is any of the given type. */
    
    best = -1;
    dist = msign * 999999999;
    for (i = 0; i < vars->num_landmarks; i++) {
      if (unused[i] && vars->landmarks[i].type == type) {
	dx = vars->landmarks[i].x - our_loc.grid_x;
	dy = vars->landmarks[i].y - our_loc.grid_y;
	if (msign * dist > msign * dx * dx + dy * dy) {
	  dist = dx * dx + dy * dy;
	  best = i;
	}
      }
    }

    if (best == -1) {
      vars->ddist = -1;
      return(FALSE);
    }

    vars->dgrid_x = vars->landmarks[best].x;
    vars->dgrid_y = vars->landmarks[best].y;
    vars->dx = vars->dgrid_x * BOX_WIDTH + BOX_WIDTH / 2;
    vars->dy = vars->dgrid_y * BOX_HEIGHT + BOX_HEIGHT / 2;

    if (found_path(vars))
      break;

    unused[best] = FALSE;

  } while (TRUE);

  return(TRUE);
}

/*** Determine the absolute drive speed at this position. ***/

static float determine_drive(vars, pos)
  TermVars *vars;
  int pos;
{
  int i, dist;
  float cur_speed, ideal, decel, pivot;

  cur_speed = speed();

  decel = 2.0 * vars->accel * vars->accel;
  pivot = 5.0 + 2.0 * decel;
  if (pos == 0)
    return(vars->max_speed);
  dist = vars->ddist - pos;
  if (dist == 0) {
    if (cur_speed > pivot)
      cur_speed = vars->max_speed - cur_speed / decel;
    else
      cur_speed = pivot;
  }
  else if (dist > 4) { 
    for (i = pos; i <= pos + 3; i++)
      if (vars->dir[i] != vars->dir[pos-1])
	break;
    dist = i - pos;
  }
  if (dist <= 4) {
    ideal = pivot + decel * (float) dist;
    if (cur_speed > ideal)
      cur_speed = ideal - cur_speed / decel;
    else
      cur_speed = ideal;
  }
  else
    cur_speed = vars->max_speed;

  if (cur_speed > vars->max_speed)
    cur_speed = vars->max_speed;
  else if (cur_speed < -vars->max_speed)
    cur_speed = -vars->max_speed;

  return(cur_speed);
}

/*** Return true if we are moving backwards. ***/

static int moving_backwards(us)
  Vehicle_info *us;
{
  float direction;

  direction = cos((double) (ATAN2(us->yspeed, us->xspeed) - us->heading));
  if (direction < 0.0)
    return(TRUE);
  else
    return(FALSE);
}

/*** Move to a landmark. ***/

static void move_to_dest(vars)
  TermVars *vars;
{
  Vehicle_info us;
  Location cur;
  Angle angle;
  int dx, dy, dgrid_x, dgrid_y;
  int pos, finished, i, pushed_around;
  int last_move_frame;   /* Last frame number with non-zero speed. */
  int cur_frame;

  char text[81], *dest_str;

  dgrid_x = vars->grid_x;
  dgrid_y = vars->grid_y;
  dx = dgrid_x * BOX_WIDTH + BOX_WIDTH / 2;
  dy = dgrid_y * BOX_HEIGHT + BOX_HEIGHT / 2;
  pos = 0;

 move_to_landmark:
  
  /* Tell where we are going. */

  if (vars->purpose == ARMOR)
    dest_str = "armor";
  else if (vars->purpose == AMMO)
    dest_str = "ammo";
  else if (vars->purpose == FUEL)
    dest_str = "fuel";
  else
    dest_str = "enemy";
  sprintf(text, "Seeking %s @ (%d,%d)", dest_str, vars->dx, vars->dy);
  send_msg(RECIPIENT_ALL, OP_TEXT, text);
  last_move_frame = frame_number();

  /* Get to the landmark by following the precomputed path. */

  do {
    cur_frame = frame_number();
    get_self(&us);
    if (ABS(us.xspeed) + ABS(us.yspeed) >= 2.0)
      last_move_frame = cur_frame;
    else if (cur_frame - last_move_frame >= 30) {    /* Assume we hit wall. */
      send_msg(RECIPIENT_ALL, OP_TEXT, "Hit wall.  Adjusting.");
      vars->ddist = -1;
      vars->last_dx = vars->last_dy = -1;
      return;
    }
    if (dgrid_x == us.loc.grid_x && dgrid_y == us.loc.grid_y) {
      if (dgrid_x == vars->dgrid_x && dgrid_y == vars->dgrid_y) {
	if (ABS(us.loc.x - vars->dx) < LANDMARK_WIDTH / 2 &&
	    ABS(us.loc.y - vars->dy) < LANDMARK_HEIGHT / 2)
	  break;
      }
      else {
	switch(vars->dir[pos]) {
	case NORTH:
	  dgrid_y--;
	  break;
	case EAST:
	  dgrid_x++;
	  break;
	case SOUTH:
	  dgrid_y++;
	  break;
	case WEST:
	  dgrid_x--;
	  break;
	}
	dx = dgrid_x * BOX_WIDTH + BOX_WIDTH / 2;
	dy = dgrid_y * BOX_HEIGHT + BOX_HEIGHT / 2;
	pos++;
      }
    }

    pushed_around = moving_backwards(&us);
    if (pushed_around)
      set_abs_drive(vars->max_speed);
    else
      set_abs_drive(determine_drive(vars, pos));
    if (pushed_around && vars->enemy_near) {
      fire_control(vars, TRUE, TRUE);
      if (vars->purpose == NORMAL)
	return;
    }
    else {
      angle = ATAN2(dy - us.loc.y, dx - us.loc.x);
      if (us.heading != angle) {
	turn_vehicle(angle);
	if (!vars->enemy_near && !vars->outpost_near)
	  turn_all_turrets(angle);
      }
      fire_control(vars, FALSE, FALSE);
      if (vars->enemy_near) {
	if (vars->purpose == NORMAL)
	  return;
	if (vars->critical_sides >= 2 && vars->taking_damage &&
	    pos >= vars->ddist - 8 &&
	    (vars->last_dx != vars->dx || vars->last_dy != vars->dy)) {
	  send_msg(RECIPIENT_ALL, OP_TEXT, "Changing destination.");
	  vars->last_dx = vars->dx;
	  vars->last_dy = vars->dy;
	  vars->ddist = -1;
	  return;
	}
      }
    }
  } while (TRUE);

  /* We have arrived. */

  set_abs_drive(0.0);
  if (vars->purpose == NORMAL) {      /* This is where enemy was. */
    vars->ddist = -1;
    vars->last_dx = vars->last_dy = -1;
    return;
  }
  
  finished = FALSE;
  check_armor(vars);
  do {
    check_ammo(vars);
    check_fuel(vars);
    switch (vars->purpose) {
    case ARMOR:
      if (vars->armor_value >= OPERATIONAL) {
	if (vars->ammo_value == CRITICAL)
	  vars->purpose = AMMO;
	else if (vars->fuel_value == CRITICAL)
	  vars->purpose = FUEL;
	if (vars->purpose != ARMOR || vars->armor_value == MAXIMUM)
	  finished = TRUE;
      }
      break;
    case AMMO:
      if (vars->ammo_value != CRITICAL) {
	if (vars->armor_value == CRITICAL)
	  vars->purpose = ARMOR;
	else if (vars->fuel_value == CRITICAL)
	  vars->purpose = FUEL;
	if (vars->purpose != AMMO || vars->ammo_value == MAXIMUM)
	  finished = TRUE;
      }
      toggle_ammo(vars);
      break;
    case FUEL:
      if (vars->fuel_value >= OPERATIONAL) {
	if (vars->ammo_value == CRITICAL && vars->armor_value >= OPERATIONAL) {
	  vars->purpose = AMMO;
	  finished = TRUE;
	}
	if (vars->purpose != FUEL || vars->fuel_value == MAXIMUM)
	  finished = TRUE;
      }
      break;
    }
    vars->money = get_money();
    if (vars->money < 100) {
      vars->purpose = NORMAL;
      finished = TRUE;
    }

    fire_control(vars, FALSE, TRUE);
    if (!vars->taking_damage || !vars->enemy_near) {
      get_location(&cur);
      if (ABS(cur.x - vars->dx) >= LANDMARK_WIDTH / 2 || 
	  ABS(cur.y - vars->dy) >= LANDMARK_HEIGHT / 2)
	goto move_to_landmark;
    }
    else if (vars->critical_sides >= 2 && vars->taking_damage)
      finished = TRUE;    /* Find another depot. */

    if (!finished)
      done();

  } while (!finished && vars->purpose != NORMAL);

  vars->ddist = -1;
  vars->last_dx = vars->last_dy = -1;
  for (i = 0; i < vars->num_weapons; i++)
    vars->weapon_ok[i] = TRUE;
}

/*** Move towards the enemy. ***/

static void move_towards_enemy(vars)
  TermVars *vars;
{
  Vehicle_info us;
  int num_blips;
  Blip_info blips[MAX_BLIPS];
  int i, dx, dy, dist;
  int min, min_dist;

  get_blips(&num_blips, blips);
  get_self(&us);

 find_enemy:

  min_dist = 9999999;
  min = -1;
  for (i = 0; i < num_blips; i++)
    if (blips[i].x != -1) {
      dx = blips[i].x - us.loc.grid_x;
      dy = blips[i].y - us.loc.grid_y;
      dist = dx * dx + dy * dy;
      if (min_dist > dist) {
	min_dist = dist;
	min = i;
      }
    }

  if (min != -1) {      /* Found a blip. */
    vars->dgrid_x = blips[min].x;
    vars->dgrid_y = blips[min].y;
    vars->dx = vars->dgrid_x * BOX_WIDTH + BOX_WIDTH / 2;
    vars->dy = vars->dgrid_y * BOX_HEIGHT + BOX_HEIGHT / 2;
    vars->purpose = NORMAL;
    if (found_path(vars))
      move_to_dest(vars);
    else {
      blips[min].x = -1;    /* Try another blip. */
      goto find_enemy;
    }
  }
  else {                /* Seek out random explorations. */
    vars->dgrid_x = rand() % GRID_WIDTH;
    vars->dgrid_y = rand() % GRID_HEIGHT;
    vars->dx = vars->dgrid_x * BOX_WIDTH + BOX_WIDTH / 2;
    vars->dy = vars->dgrid_y * BOX_HEIGHT + BOX_HEIGHT / 2;
    vars->purpose = NORMAL;
    if (found_path(vars))
      move_to_dest(vars);
  }
}

/*** The main control procedure. ***/

static void main_control(vars)
  TermVars *vars;
{
  int i, j;
  int direction_change;
  float angle;
  Vehicle_info us;

  /* Hunt and kill and rest and ... */

  init_landmarks(vars);
  direction_change = 0;

  while (TRUE) {

    vars->purpose = NORMAL;

    vars->money = get_money();
    if (vars->money >= 100)  {
      
      /* Monitor armor, ammo, and fuel. */
      
      check_armor(vars);
      if (vars->num_armor > 0) {
	if (vars->armor_value == CRITICAL ||
	    (vars->armor_value == UNSTABLE && !vars->enemy_near)) {
	  vars->purpose = ARMOR;
	  goto move_vehicle;
	}
      }
      
      if (vars->num_ammo > 0 && vars->ammo_value == CRITICAL) {
	vars->purpose = AMMO;
	goto move_vehicle;
      }
      
      if (vars->num_fuel > 0) {
	check_fuel(vars);
	if (vars->fuel_value == CRITICAL) {
	  vars->purpose = FUEL;
	  goto move_vehicle;
	}
      }
    }

    if (!vars->enemy_near)
      move_towards_enemy(vars);
    fire_control(vars, TRUE, TRUE);

    done();
    continue;
      
    /* Move the vehicle to a destination. */

  move_vehicle:

    fire_control(vars, FALSE, FALSE);
    init_landmarks(vars);
    switch (vars->purpose) {
    case ARMOR:
      if (!found_landmark(vars, ARMOR, !vars->enemy_near))
	vars->purpose = NORMAL;
      break;
    case AMMO:
      if (!found_landmark(vars, AMMO, !vars->enemy_near))
	vars->purpose = NORMAL;
      break;
    case FUEL:
      if (!found_landmark(vars, FUEL, !vars->enemy_near))
	vars->purpose = NORMAL;
      break;
    }

    if (vars->purpose != NORMAL && vars->ddist >= 0)
      move_to_dest(vars);
  }
}

/*** The main loop accessable to the xtank program. ***/

static void terminator_main()
{
  TermVars vars;

  /* Initialize our variables. */

  get_settings(&settings);
  terminator_init(&vars);

  /* Pass control to main loop. */

  main_control(&vars);
}  

