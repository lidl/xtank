/* ************************************************************ */
/*								*/
/*  XTANK ENTRY:  crank tank					*/
/*								*/
/*	created by Fred Martin					*/
/*	April 23-24, 1988					*/
/*								*/
/*	for XTANK by Terry Donahue				*/
/*								*/
/* ************************************************************ */


/*  crank program  */


#include "/mit/games/src/vax/xtank/Contest/xtanklib.h"
#include <math.h>

/*  defines  */
#define CRANK_MSGS 1

#define CRANK_MAX_WEAPONS 20

/*  movement types using "tank_dir" variable  */
#define CRANK_FORW 1
#define CRANK_BACK 0
#define CRANK_TURNED 2
#define CRANK_SPEED_LIMIT 9.0

/*  major tank modes using "crank_mode"  */
#define CRANK_KILL 1
#define CRANK_WANDER 2
#define CRANK_ESCAPE 3

/*  make global values  */
static int tank_turrets;
static int tank_weapons;
static int tank_weapon_range[CRANK_MAX_WEAPONS];
static float tank_max_speed;
static char msg[80];
static Location tank_loc;
static Location last_loc;
static float tank_angle;
static Vehicle_info vehicles_array[MAX_VEHICLES];
static int num_vehicles;
static Blip_info blip_array[MAX_BLIPS];
static int num_blips;
static int tank_dir;
static int framenum;
static float hyp;
static int crank_mode;
static int crank_changed_mode;
static float last_escaping_weight;
static float last_seeking_weight;


/*  declare routines  */
void crank_init();
void crank_scheduler();
void crank_user_scheduler();
void crank_mover();
void crank_wander();
void crank_kill();
void crank_escape();
void crank_shoot_at_enemy();
float crank_conv_dir_to_rads();
float crank_compute_distance_to_position();
int crank_check_can_exit();
float crank_absolute_angle_difference();


crank_main()
{

  crank_init();

  /*  print tank status  */
  sprintf(msg, "TURRETS %d  WEAPONS %d  SPEED %f", tank_turrets,
	  tank_weapons, tank_max_speed);
  message(msg);

  while(1) {

    crank_shoot_at_enemy();

    crank_scheduler();

    crank_shoot_at_enemy();

    crank_mover();

    crank_shoot_at_enemy();

  }

}

/*  initialize global values  */
void
crank_init()
{
  int i;

  tank_turrets = num_turrets();
  tank_weapons = num_weapons();
  tank_max_speed = max_speed();
  get_location(&tank_loc);
  tank_angle = angle();

  /*  get weapon range data  */
  for (i=0; i<tank_weapons; i++)
    tank_weapon_range[i] = weapon_range(i);

  /*  compute box hypotenuse  */
  hyp = sqrt((float)(BOX_WIDTH*BOX_WIDTH + BOX_HEIGHT*BOX_HEIGHT));

  tank_dir = CRANK_FORW;
  set_rel_speed(CRANK_SPEED_LIMIT);

  /*  set initial tank mode  */
  crank_mode = CRANK_ESCAPE;
  crank_changed_mode = frame_number();

  last_escaping_weight = 0.0;
  last_seeking_weight = 0.0;

}  /* crank_init */


/*  SCHEDULER
**  chooses major tank mode
*/
void
crank_user_scheduler()

{
  char c;

  c = input();

  if (c == '\0') return;

  switch (c) {
  case 'w':
    crank_mode = CRANK_WANDER;
    break;
  case 'e':
    crank_mode = CRANK_ESCAPE;
    break;
  case 'k':
    crank_mode = CRANK_KILL;
    break;
  }

}  /* crank_user_scheduler */



/*  SCHEDULER
**  chooses major tank mode
*/
void
crank_scheduler()

{

  /*  toggle mode if over 100 frames have passed  */
  if (crank_mode == CRANK_ESCAPE &&
      frame_number() - crank_changed_mode > 100) {
    crank_mode = CRANK_KILL;
    crank_changed_mode = frame_number();
  }
  else
    if (crank_mode == CRANK_KILL &&
	frame_number() - crank_changed_mode > 200) {
      crank_mode = CRANK_ESCAPE;
      crank_changed_mode = frame_number();
    }

  /*  if vehicle in current box, escape!!  */
  if (crank_vehicle_in_my_box() == TRUE) {
    crank_mode = CRANK_ESCAPE;
    crank_changed_mode = frame_number();
    return;
  }

}  /* crank_scheduler */



/*  MOVER
**  selects type of motion 
**    depending on tank mode
*/
void
crank_mover()

{

  switch (crank_mode) {
  case CRANK_WANDER:
    crank_wander();
    break;
  case CRANK_ESCAPE:
    crank_escape();
    break;
  case CRANK_KILL:
    crank_kill();
    break;
  }

}  /* crank_mover */


/*  WANDER
**  by moving in clear direction 
*/
void
crank_wander()
{

  int dx, dy;
  float ang;

#ifdef CRANK_MSGS
  message("Wandering...");
#endif

  /*  update location  */
  get_location(&tank_loc);
  tank_angle = angle();

  if (crank_stuck_subs() == TRUE) return;

  /*  if tank can exit w/o changing dir, then ok.
      otherwise, turn.  */
  if (crank_check_can_exit() == FALSE)
    crank_turn_to_nearest_exit();
  else {
    if (random() % 2 == 0)
      crank_turn_to_nearest_exit();
  }
  
  set_rel_speed(CRANK_SPEED_LIMIT);
  tank_dir = CRANK_FORW;

}  /* crank_wander */


/*  KILL
**  by moving in clear direction 
**  or towards a tank
*/
void
crank_kill()
{

  int dx, dy;
  float ang;

#ifdef CRANK_MSGS
  message("Meet your maker");
#endif

  /*  update location  */
  get_location(&tank_loc);
  tank_angle = angle();

  /*  update vehicle info  */
  get_vehicles(&num_vehicles, vehicles_array);

  /*  update blip info  */
  get_blips(&num_blips, blip_array);

  if (crank_stuck_subs() == TRUE) return;

  /*  if vehicle accessible, turn to its box  */
  if (crank_weighted_seek() == TRUE) {
#ifdef CRANK_MSGS
    message("Here I come");
#endif
    set_rel_speed(CRANK_SPEED_LIMIT);
    tank_dir = CRANK_FORW;
    return;
  }


  /*  else do standard exit wander  */
  if (crank_check_can_exit() == FALSE)
    crank_turn_to_nearest_exit();
  else {
    if (random() % 2 == 0)
      crank_turn_to_nearest_exit();
  }
  
  set_rel_speed(CRANK_SPEED_LIMIT);
  tank_dir = CRANK_FORW;

}  /* crank_kill */


/*  ESCAPE
**  by moving in clear direction 
**  or away from a tank
*/
void
crank_escape()
{

  int dx, dy;
  float ang;

#ifdef CRANK_MSGS
  message("We'd be outa here now");
#endif

  /*  update location  */
  get_location(&tank_loc);
  tank_angle = angle();

  /*  update vehicle info  */
  get_vehicles(&num_vehicles, vehicles_array);

  /*  update blip info  */
  get_blips(&num_blips, blip_array);

  if (crank_stuck_subs() == TRUE) return;

  /*  if vehicle accessible, turn to its box  */
  if (crank_weighted_escape() == TRUE) {
#ifdef CRANK_MSGS
    message("Here I go");
#endif
    set_rel_speed(CRANK_SPEED_LIMIT);
    tank_dir = CRANK_FORW;
    return;
  }


  /*  else do standard exit wander  */
  if (crank_check_can_exit() == FALSE)
    crank_turn_to_nearest_exit();
  else {
    if (random() % 2 == 0)
      crank_turn_to_nearest_exit();
  }
  
  set_rel_speed(CRANK_SPEED_LIMIT);
  tank_dir = CRANK_FORW;

}  /* crank_escape */

crank_stuck_subs()
{

  if (speed() == 0 && tank_dir == CRANK_FORW) {
#ifdef CRANK_MSGS
    message("Getting unstuck.");
#endif
    crank_start_unstuck();
    return TRUE;
  }

  if (tank_dir == CRANK_BACK) {
#ifdef CRANK_MSGS
    message("Unsticking.");
#endif
    crank_during_unstuck();
    return TRUE;
  }

  if (speed() == 0 && tank_dir == CRANK_BACK) {
#ifdef CRANK_MSGS
    message("Reversing direction.");
#endif
    set_rel_speed(CRANK_SPEED_LIMIT);
    tank_dir = CRANK_FORW;
  }

  return FALSE;

}  /* crank_stuck_subs */


crank_vehicle_in_box(loc_x, loc_y)

     int loc_x, loc_y;

{

  int i;

  /*  triviality  */
  if (num_vehicles == 0) return FALSE;

  /*  loop and check  */
  for (i=0; i<num_vehicles; i++)
    if (vehicles_array[i].loc.grid_x == loc_x &&
	vehicles_array[i].loc.grid_y == loc_y)
      return TRUE;

  return FALSE;

}  /* crank_vehicle_in_box */


crank_vehicle_in_my_box()
{ return crank_vehicle_in_box(tank_loc.grid_x, tank_loc.grid_y); }


/*
**  will turn tank in direction of nearest exit  **
**  using minimum turn angle                     **
**  ignores other obstacles			 **
*/
crank_turn_to_nearest_exit()
{

  int i;
  int start_x, start_y, end_x, end_y, dx, dy;
  float ang;
  float hyp;
  int best_dir, best_dx, best_dy;
  float min_ang, ang_diff;
  float tank_angle_corr, ang_corr;


  get_location(&tank_loc);
  tank_angle = angle();
  if (tank_angle > PI)
    tank_angle_corr = tank_angle - 2.0 * PI;
  else
    tank_angle_corr = tank_angle;

  /*  compute our global x, y  */
  start_x = tank_loc.grid_x * BOX_WIDTH + tank_loc.box_x;
  start_y = tank_loc.grid_y * BOX_HEIGHT + tank_loc.box_y;

  /*  no best dir  */
  best_dir = -1;

  /*  search for nearest exit direction  */
  for (i=0; i<4; i++)

    if (wall(i, tank_loc.grid_x, tank_loc.grid_y) == FALSE) {

      /*  compute middle of that wall  */
      switch(i) {
      case NORTH:
	end_x = tank_loc.grid_x * BOX_WIDTH + 0.5 * BOX_WIDTH;
	end_y = tank_loc.grid_y * BOX_HEIGHT;
	break;
      case EAST:
	end_x = (1 + tank_loc.grid_x) * BOX_WIDTH;
	end_y = tank_loc.grid_y * BOX_HEIGHT + 0.5 * BOX_HEIGHT;
	break;
      case SOUTH:
	end_x = tank_loc.grid_x * BOX_WIDTH + 0.5 * BOX_WIDTH;
	end_y = (1 + tank_loc.grid_y) * BOX_HEIGHT;
	break;
      case WEST:
	end_x = tank_loc.grid_x * BOX_WIDTH;
	end_y = tank_loc.grid_y * BOX_HEIGHT + 0.5 * BOX_HEIGHT;
	break;
      }

      dx = end_x - start_x;
      dy = end_y - start_y;

      ang = (float) atan2((float) dy,(float) dx);
      if (ang > PI)
	ang_corr = ang - 2.0 * PI;
      else
	ang_corr = ang;

      ang_diff = fabs(tank_angle_corr - ang_corr);
      
      /*  new best  */
      if (best_dir == -1 || ang_diff < min_ang) {

	best_dir = i;
	min_ang = ang_diff;
	best_dx = dx;
	best_dy = dy;

      }  /* best */

    }  /* wall clear */

  if (best_dir == -1)
    crank_turn_randomly();

#ifdef CRANK_MSGS
  switch(best_dir) {
  case NORTH:
    sprintf(msg, "Turning north.");
    break;
  case EAST:
    sprintf(msg, "Turning east.");
    break;
  case SOUTH:
    sprintf(msg, "Turning south.");
    break;
  case WEST:
    sprintf(msg, "Turning west.");
    break;
  }
  message(msg);
#endif

  /* head towards the edge of the target box */
  ang = (float) atan2((float) best_dy,(float) best_dx);

  /*  slow down if large angle  */
  if (min_ang > PI / 2) {
    set_rel_speed(-3.);
#ifdef CRANK_MSGS
  message("Hard turn!");
#endif
    framenum = frame_number();
    while (frame_number() - framenum < 5) 
      {crank_shoot_at_enemy();}  /* do something useful */
    turn_vehicle(ang);
    set_rel_speed(CRANK_SPEED_LIMIT);
    return;
  }
  else turn_vehicle(ang);
  
}  /* crank_turn_to_nearest_exit */


/*
**  weighted seek,
**  optimizing for exit condition,
**	minimum turn angle,
**	vehicles visible,
**	blips visible.
*/
crank_weighted_seek()
{

  int i, j;
  int start_x, start_y, end_x, end_y, dxx[4], dyy[4], dx, dy;
  float angles[4], weights[4];
  float hyp;
  int veh_x, veh_y;
  int best_dir;
  float best_weight;
  float ang_diff, ang;
  float tank_angle_corr, ang_corr, new_ang_corr;

  float exit_weight = 100.;
  float angle_weight = 10.;
  float vehicle_weight = 40.;
  float blip_weight = 20.;
  
  for (i=0; i<4; i++) weights[i] = 0.0;

  get_location(&tank_loc);
  tank_angle = angle();
  if (tank_angle > PI)
    tank_angle_corr = tank_angle - 2.0 * PI;
  else
    tank_angle_corr = tank_angle;

  /*  compute our global x, y  */
  start_x = tank_loc.grid_x * BOX_WIDTH + tank_loc.box_x;
  start_y = tank_loc.grid_y * BOX_HEIGHT + tank_loc.box_y;

  /*  search over exit directions  */
  for (i=0; i<4; i++) {

    /*** EXIT WEIGHT ***/
    if (wall(i, tank_loc.grid_x, tank_loc.grid_y) == FALSE)
      weights[i] += exit_weight;

    /*** ANGLE TURN WEIGHT ***/
    /*  compute middle of the wall and its grid coords  */
    switch(i) {
    case NORTH:
      end_x = tank_loc.grid_x * BOX_WIDTH + 0.5 * BOX_WIDTH;
      end_y = tank_loc.grid_y * BOX_HEIGHT;
      break;
    case EAST:
      end_x = (1 + tank_loc.grid_x) * BOX_WIDTH;
      end_y = tank_loc.grid_y * BOX_HEIGHT + 0.5 * BOX_HEIGHT;
      break;
    case SOUTH:
      end_x = tank_loc.grid_x * BOX_WIDTH + 0.5 * BOX_WIDTH;
      end_y = (1 + tank_loc.grid_y) * BOX_HEIGHT;
      break;
    case WEST:
      end_x = tank_loc.grid_x * BOX_WIDTH;
      end_y = tank_loc.grid_y * BOX_HEIGHT + 0.5 * BOX_HEIGHT;
      break;
    }

    dxx[i] = end_x - start_x;
    dyy[i] = end_y - start_y;
    
    ang = (float) atan2((float) dyy[i],(float) dxx[i]);

    if (ang > PI)
      ang_corr = ang - 2.0 * PI;
    else
      ang_corr = ang;
    
    ang_diff = fabs(tank_angle_corr - ang_corr);
    angles[i] = ang_diff;

    weights[i] += (PI - ang_diff) / PI * angle_weight;

    /*** VEHICLE WEIGHT ***/
    if (num_vehicles != 0)
      for (j=0; j<num_vehicles; j++) {
	
	veh_x = vehicles_array[j].loc.grid_x * BOX_WIDTH +
	  vehicles_array[j].loc.box_x;
	
	veh_y = vehicles_array[j].loc.grid_y * BOX_HEIGHT +
	  vehicles_array[j].loc.box_y;
	
	dx = veh_x - start_x;
	dy = veh_y - start_y;
	
	new_ang_corr = (float) atan2((float) dy,(float) dx);
	if (new_ang_corr > PI)
	  new_ang_corr = new_ang_corr - 2.0 * PI;
	
	ang_diff = fabs(new_ang_corr - ang_corr);
	
	if (ang_diff <= PI / 4.0)
	  weights[i] += vehicle_weight / num_vehicles;
	
      }  /* vehicle for */
      
    /*** BLIP WEIGHT ***/
    if (num_blips != 0)
      for (j=0; j<num_blips; j++) {
	
	veh_x = blip_array[j].x * BOX_WIDTH;
	
	veh_y = blip_array[j].y * BOX_HEIGHT;
	
	dx = veh_x - start_x;
	dy = veh_y - start_y;
	
	new_ang_corr = (float) atan2((float) dy,(float) dx);
	if (new_ang_corr > PI)
	  new_ang_corr = new_ang_corr - 2.0 * PI;
	
	ang_diff = fabs(new_ang_corr - ang_corr);
	
	if (ang_diff <= PI / 4.0)
	  weights[i] += blip_weight / num_blips;
	
      }  /* blip for */

    /*  SHOOT!!  */
    crank_shoot_at_enemy();
    
  }  /* directions for */

  /*  find best turn  */
  best_dir = -1;
  for (i=0; i<4; i++)
    if ((best_dir == -1) || (weights[i] > best_weight)) {
      best_dir = i;
      best_weight = weights[i];
    }

  if (best_weight < exit_weight) {
#ifdef CRANK_MSGS
    message("No good exit!!!");
#endif
    last_seeking_weight = 0.0;
    return FALSE;
  }

  last_seeking_weight = best_weight;

#ifdef CRANK_MSGS
  switch(best_dir) {
  case NORTH:
    sprintf(msg, "Chasing north at %f", best_weight);
    break;
  case EAST:
    sprintf(msg, "Chasing east at %f", best_weight);
    break;
  case SOUTH:
    sprintf(msg, "Chasing south at %f.", best_weight);
    break;
  case WEST:
    sprintf(msg, "Chasing west at %f.", best_weight);
    break;
  }
  message(msg);
#endif

  /* head towards the edge of the target box */
  ang = (float) atan2((float) dyy[best_dir], (float)dxx[best_dir]);

  /*  slow down if large angle  */
  if (angles[best_dir] > PI / 2) {
    set_rel_speed(-3.);
#ifdef CRANK_MSGS
  message("Hard turn!");
#endif
    framenum = frame_number();
    while (frame_number() - framenum < 5) 
      {crank_shoot_at_enemy();}  /* do something useful */
    turn_vehicle(ang);
    set_rel_speed(CRANK_SPEED_LIMIT);
    return TRUE;
  }
  else turn_vehicle(ang);

  return TRUE;
  
}  /* crank_weighted_seek */


/*
**  weighted escape:
**  optimizing for exit condition,
**	minimum turn angle,
**	vehicles visible,
**	blips visible.
*/
crank_weighted_escape()
{

  int i, j;
  int start_x, start_y, end_x, end_y, dxx[4], dyy[4], dx, dy;
  float angles[4], weights[4];
  float hyp;
  int veh_x, veh_y;
  int best_dir;
  float best_weight;
  float ang_diff, ang;
  float tank_angle_corr, ang_corr, new_ang_corr;

  float exit_weight = 100.;
  float angle_weight = 10.;
  float vehicle_weight = 40.;
  float blip_weight = 10.;
  
  for (i=0; i<4; i++) weights[i] = 0.0;

  get_location(&tank_loc);
  tank_angle = angle();
  if (tank_angle > PI)
    tank_angle_corr = tank_angle - 2.0 * PI;
  else
    tank_angle_corr = tank_angle;

  /*  compute our global x, y  */
  start_x = tank_loc.grid_x * BOX_WIDTH + tank_loc.box_x;
  start_y = tank_loc.grid_y * BOX_HEIGHT + tank_loc.box_y;

  /*  search over exit directions  */
  for (i=0; i<4; i++) {

    /*** EXIT WEIGHT ***/
    if (wall(i, tank_loc.grid_x, tank_loc.grid_y) == FALSE)
      weights[i] += exit_weight;

    /*** ANGLE TURN WEIGHT ***/
    /*  compute middle of the wall and its grid coords  */
    switch(i) {
    case NORTH:
      end_x = tank_loc.grid_x * BOX_WIDTH + 0.5 * BOX_WIDTH;
      end_y = tank_loc.grid_y * BOX_HEIGHT;
      break;
    case EAST:
      end_x = (1 + tank_loc.grid_x) * BOX_WIDTH;
      end_y = tank_loc.grid_y * BOX_HEIGHT + 0.5 * BOX_HEIGHT;
      break;
    case SOUTH:
      end_x = tank_loc.grid_x * BOX_WIDTH + 0.5 * BOX_WIDTH;
      end_y = (1 + tank_loc.grid_y) * BOX_HEIGHT;
      break;
    case WEST:
      end_x = tank_loc.grid_x * BOX_WIDTH;
      end_y = tank_loc.grid_y * BOX_HEIGHT + 0.5 * BOX_HEIGHT;
      break;
    }

    dxx[i] = end_x - start_x;
    dyy[i] = end_y - start_y;
    
    ang = (float) atan2((float) dyy[i],(float) dxx[i]);

    if (ang > PI)
      ang_corr = ang - 2.0 * PI;
    else
      ang_corr = ang;
    
    ang_diff = fabs(tank_angle_corr - ang_corr);
    angles[i] = ang_diff;

    weights[i] += (PI - ang_diff) / PI * angle_weight;

    /*** VEHICLE WEIGHT ***/
    if (num_vehicles != 0)
      for (j=0; j<num_vehicles; j++) {
	
	veh_x = vehicles_array[j].loc.grid_x * BOX_WIDTH +
	  vehicles_array[j].loc.box_x;
	
	veh_y = vehicles_array[j].loc.grid_y * BOX_HEIGHT +
	  vehicles_array[j].loc.box_y;
	
	dx = veh_x - start_x;
	dy = veh_y - start_y;
	
	new_ang_corr = (float) atan2((float) dy,(float) dx);
	if (new_ang_corr > PI)
	  new_ang_corr = new_ang_corr - 2.0 * PI;
	
	ang_diff = fabs(new_ang_corr - ang_corr);
	
	if (ang_diff > PI / 4.0)
	  weights[i] += vehicle_weight / num_vehicles;
	
      }  /* vehicle for */
      
    /*** BLIP WEIGHT ***/
    if (num_blips != 0)
      for (j=0; j<num_blips; j++) {
	
	veh_x = blip_array[j].x * BOX_WIDTH;
	
	veh_y = blip_array[j].y * BOX_HEIGHT;
	
	dx = veh_x - start_x;
	dy = veh_y - start_y;
	
	new_ang_corr = (float) atan2((float) dy,(float) dx);
	if (new_ang_corr > PI)
	  new_ang_corr = new_ang_corr - 2.0 * PI;
	
	ang_diff = fabs(new_ang_corr - ang_corr);
	
	if (ang_diff > PI / 4.0)
	  weights[i] += blip_weight / num_blips;
	
      }  /* blip for */
    
    /*  SHOOT!!  */
    crank_shoot_at_enemy();
    
  }  /* directions for */

  /*  find best turn  */
  best_dir = -1;
  for (i=0; i<4; i++)
    if ((best_dir == -1) || (weights[i] > best_weight)) {
      best_dir = i;
      best_weight = weights[i];
    }

  if (best_weight < exit_weight) {
#ifdef CRANK_MSGS
    message("No good exit!!!");
#endif
    last_escaping_weight = 0.0;
    return FALSE;
  }

  last_escaping_weight = best_weight;

#ifdef CRANK_MSGS
  switch(best_dir) {
  case NORTH:
    sprintf(msg, "Escaping north at %f", best_weight);
    break;
  case EAST:
    sprintf(msg, "Escaping east at %f", best_weight);
    break;
  case SOUTH:
    sprintf(msg, "Escaping south at %f.", best_weight);
    break;
  case WEST:
    sprintf(msg, "Escaping west at %f.", best_weight);
    break;
  }
  message(msg);
#endif

  /* head towards the edge of the target box */
  ang = (float) atan2((float) dyy[best_dir], (float)dxx[best_dir]);

  /*  slow down if large angle  */
  if (angles[best_dir] > PI / 2) {
    set_rel_speed(-3.);
#ifdef CRANK_MSGS
  message("Hard turn!");
#endif
    framenum = frame_number();
    while (frame_number() - framenum < 5) 
      {crank_shoot_at_enemy();}  /* do something useful */
    turn_vehicle(ang);
    set_rel_speed(CRANK_SPEED_LIMIT);
    return TRUE;
  }
  else turn_vehicle(ang);

  return TRUE;
  
}  /* crank_weighted_escape */


/* 
**  check if we can exit current box
**  by moving in current direction   */
int
crank_check_can_exit()
{

  Location next_loc;
  int glob_x, glob_y, dx, dy;
  

  tank_angle = angle();
  get_location(&tank_loc);

  /*  compute our global pos  */
  glob_x = tank_loc.grid_x * BOX_WIDTH + tank_loc.box_x;
  glob_y = tank_loc.grid_y * BOX_HEIGHT + tank_loc.box_y;

  /*  add in to next box  */
  glob_x = glob_x + (int)(hyp * cos(tank_angle));
  glob_y = glob_y + (int)(hyp * sin(tank_angle));

  /*  cvt back to Location  */
  next_loc.grid_x = glob_x / BOX_WIDTH;
  next_loc.grid_y = glob_y / BOX_HEIGHT;
  
  next_loc.box_x = glob_x - next_loc.grid_x * BOX_WIDTH;
  next_loc.box_y = glob_y - next_loc.grid_y * BOX_HEIGHT;

  return crank_clear_path(&tank_loc, &next_loc);

}  /* crank_check_can_exit */


float
crank_absolute_angle_difference(ang1, ang2)

     float ang1, ang2;

{

  float ang1_corr, ang2_corr;

  /*  fix angle 1  */
  if (ang1 > PI)
    ang1_corr = ang1 - 2.0 * PI;
  else
    ang1_corr = ang1;

  /*  fix angle 2  */
  if (ang2 > PI)
    ang2_corr = ang2 - 2.0 * PI;
  else
    ang2_corr = ang2;

  return fabs(ang1_corr - ang2_corr);

}  /* crank_absolute_angle_difference */

crank_turn_randomly()

{

#ifdef CRANK_MSGS
  message("Turning randomly.");
#endif

  if ((random() % 2) == 0)
    turn_vehicle(tank_angle + PI * ((random() % 100) / 100));
  else
    turn_vehicle(tank_angle - PI * ((random() % 100) / 100));

}  /* crank_turn_randomly */
  


/*
** Gets the vehicle unstuck from an obstacle.  Call it if you think you
** ran into something.
*/
crank_start_unstuck()
{

  /*  save framenum  */
  framenum = frame_number();

  /* First change dir away from the obstacle */
  set_rel_speed(-3.0);
  tank_dir = CRANK_BACK;

#ifdef CRANK_MSGS
  message("Backing up.");
#endif

}  /* crank_start_unstuck */


crank_during_unstuck()
{

  float curangle = angle();

  if (speed() == 0) {
    set_rel_speed(CRANK_SPEED_LIMIT);
    tank_dir = CRANK_FORW;
  }

  /*  backed up for 7 ticks?  */
  if (frame_number() - framenum <= 7) return;

  /* Turn around and get moving */
  if ((random() % 3) == 0) 
    crank_turn_randomly();
  else
    crank_turn_to_nearest_exit();

  set_rel_speed(CRANK_SPEED_LIMIT);
  tank_dir = CRANK_FORW;

}  /* crank_during_unstuck */


float
crank_conv_dir_to_rads(dir)
     int dir;
{
  return (dir - 1) * PI / 2;
}


void
crank_shoot_at_enemy()
{

  int i;
  Vehicle_info *v;
  Location *veh_loc, veh_vel_loc;
  float dist, min_dist;
  int found;

  /*  get vehicle info  */
  get_vehicles(&num_vehicles, vehicles_array);

  if (num_vehicles == 0) return;

  /*  no vehicle selected yet  */
  found = FALSE;

  /*  select closest vehicle w/clear path  */

  for (i=0; i<num_vehicles; i++) {

    veh_loc = &(vehicles_array[i].loc);
    dist = crank_compute_distance_to_position(veh_loc);

    if ((dist < min_dist) || (found == FALSE)) {

      /* check if straight-line to hit */
      veh_vel_loc = vehicles_array[i].loc;
      
      if (crank_clear_path(&veh_vel_loc, &tank_loc) == TRUE) {

	/*  select vehicle  */
	min_dist = dist;
	v = &(vehicles_array[i]);
	found = TRUE;
      }

    }

  }  /* vehicle for */
    
  /*  found a vehicle?  */
  if (found == FALSE) return;
    
  /*  destroy it  */
  crank_shoot_it_smartly(v);

}  /* crank_shoot_at_enemy */


float
crank_compute_distance_to_position(loc)

     Location *loc;

{

  int my_x, my_y;
  int its_x, its_y;

  /*  compute a bunch of global coordinates  */
  my_x = tank_loc.grid_x * GRID_WIDTH + 
    tank_loc.box_x;

  my_y = tank_loc.grid_y * GRID_HEIGHT +
    tank_loc.box_y;

  its_x = loc->grid_x * GRID_WIDTH + loc->box_x;

  its_y = loc->grid_y * GRID_WIDTH + loc->box_y;

  return (my_x - its_x) * (my_x - its_x) +
    (my_y - its_y) * (my_y - its_y);

}  /* crank_compute_distance_to_position */
  
  

/* 
** Returns the lowest armor value of front, back, left, and right sides.
*/
crank_lowest_armor()
{
  int low_so_far = 100;   /* lowest armor value found so far */
  int side;               /* current side I'm looking at */

  /* go through all the sides and check if the armor on that side is
     less than the lowest we've found so far.  If it is, I set low_so_far
     to that value. */

  for (side = 0 ; side < 4 ; ++side)
    if (low_so_far>armor(side)) low_so_far = armor(side);

  return (low_so_far);
}
  

/*
** Moves vehicle in a straight line towards the center of a box
** Does not attempt to avoid walls.
** Will not return until the vehicle has arrived at the destination.
** Uses atan2, so #include <math.h>
*/
crank_move_to_box(x,y)
     int x,y;
{
  Location loc;
  int dx,dy;
  float ang;
  float spd;
  int dist;

  do {
    /* find out where we are */
    get_location(&loc);

    /* compute dx and dy to center of target box */
    dx = (x - loc.grid_x) * BOX_WIDTH + (BOX_WIDTH/2 - loc.box_x);
    dy = (y - loc.grid_y) * BOX_HEIGHT + (BOX_HEIGHT/2 - loc.box_y);

    /* head towards the center of the target box */
    ang = (float) atan2((float) dy,(float) dx);
    turn_vehicle(ang);

    /* move faster if we are farther away */
    dist = dx*dx + dy*dy;
    spd = (float) dist * 9.0 / ((float) (SCREEN_WIDTH * SCREEN_WIDTH) / 40);
    set_rel_speed(spd);
  } while(dist > 25*25);	/* keep going until we are within 25 pixels */

  /* we are there, so stop */
  set_rel_speed(0.0);
}


/* 
** Moves all turrets at the vehicle whose id is vnum.
** Shoots each weapon at that vehicle, if it is in range.
** Neglects turret turn position.
*/
crank_shoot_it_smartly(v)

     Vehicle_info *v;
{
  int i;
  int dx,dy;                      /* the distance between me and my target */
  float dist;
  float lead_angle;
  int weapon_speed = 22;
  

  dx = BOX_WIDTH * (v->loc.grid_x - tank_loc.grid_x)
    + v->loc.box_x - tank_loc.box_x;
  dy = BOX_HEIGHT * (v->loc.grid_y - tank_loc.grid_y)
    + v->loc.box_y - tank_loc.box_y;

  dist = sqrt((float)(dx*dx + dy*dy));

  /*  compute lead angle  */
  lead_angle = asin((dx * v->yspeed + dy * v->xspeed)
		    / (weapon_speed * dist));
		   

  /* turn my turret toward him */
  turn_turret(0, atan2((float)dy, (float)dx) + lead_angle);

  /*  cycle through weapons  */
  for (i=0; i<tank_weapons; i++)
    if (tank_weapon_range[i] >= dist) fire_weapon(i);

}


/*
** Returns 1 if there are no walls blocking the path from start to finish,
** otherwise returns 0.  Keep in mind that the path has 0 width, so
** a bullet would make it through, but a vehicle might not.
**
** This code is optimized, but I'm sure someone could improve on it.
*/
crank_clear_path(start,finish)
     Location *start, *finish;
{
  int start_x,start_y,finish_x,finish_y;
  int dx,dy,lattice_dx,lattice_dy;
  int tgrid_x,tgrid_y,fgrid_x,fgrid_y;

  /* Compute absolute x coordinate in maze */
  start_x = start->grid_x * BOX_WIDTH + start->box_x;
  start_y = start->grid_y * BOX_HEIGHT + start->box_y;
  finish_x = finish->grid_x * BOX_WIDTH + finish->box_x;
  finish_y = finish->grid_y * BOX_HEIGHT + finish->box_y;

  /* Computed x and y differences from start to finish */
  dx = finish_x - start_x;
  dy = finish_y - start_y;

  /* Set up temporary and final box coordinates */
  tgrid_x = start->grid_x;
  tgrid_y = start->grid_y;
  fgrid_x = finish->grid_x;
  fgrid_y = finish->grid_y;

  /* Figure out the general direction that the line is travelling in
  ** so that we can write specific code for each case.
  **
  ** In the NE, SE, NW, and SW cases, 
  ** lattice_dx and lattice_dy are the deltas from the starting
  ** location to the lattice point that the path is heading towards.
  ** The slope of the line is compared to the slope to the lattice point
  ** This determines which wall the path intersects.
  ** Instead of comparing dx/dy with lattice_dx/lattice_dy, I multiply
  ** both sides by dy * lattice_dy, which lets me do 2 multiplies instead
  ** of 2 divides.
  */
  if(fgrid_x > tgrid_x)
    if(fgrid_y > tgrid_y) {	/* Southeast */
      lattice_dx = (tgrid_x + 1)*BOX_WIDTH - start_x;
      lattice_dy = (tgrid_y + 1)*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx < dy * lattice_dx) {
	  if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y++;
	  lattice_dy += BOX_HEIGHT;
	}
	else {
	  if(wall(EAST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x++;
	  lattice_dx += BOX_WIDTH;
	}
      }
    }
    else if(fgrid_y < tgrid_y) { /* Northeast */
      lattice_dx = (tgrid_x + 1)*BOX_WIDTH - start_x;
      lattice_dy = tgrid_y*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx > dy * lattice_dx) {
	  if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y--;
	  lattice_dy -= BOX_HEIGHT;
	}
	else {
	  if(wall(EAST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x++;
	  lattice_dx += BOX_WIDTH;
	}
      }
    }
    else {			/* East */
      for(; tgrid_x < fgrid_x ; tgrid_x++)
	if(wall(EAST,tgrid_x,tgrid_y)) return(0);
    }

  else if(fgrid_x < tgrid_x)
    if(fgrid_y > tgrid_y) {	/* Southwest */
      lattice_dx = tgrid_x*BOX_WIDTH - start_x;
      lattice_dy = (tgrid_y + 1)*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx > dy * lattice_dx) {
	  if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y++;
	  lattice_dy += BOX_HEIGHT;
	}
	else {
	  if(wall(WEST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x--;
	  lattice_dx -= BOX_WIDTH;
	}
      }
    }
    else if(fgrid_y < tgrid_y) { /* Northwest */
      lattice_dx = tgrid_x*BOX_WIDTH - start_x;
      lattice_dy = tgrid_y*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx < dy * lattice_dx) {
	  if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y--;
	  lattice_dy -= BOX_HEIGHT;
	}
	else {
	  if(wall(WEST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x--;
	  lattice_dx -= BOX_WIDTH;
	  }
      }
    }
    else {			/* West */
      for(; tgrid_x > fgrid_x ; tgrid_x--)
	if(wall(WEST,tgrid_x,tgrid_y)) return(0);
    }

  else
    if(fgrid_y > tgrid_y) {	/* South */
      for(; tgrid_y < fgrid_y ; tgrid_y++)
	if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
    }
    else if(fgrid_y < tgrid_y) { /* North */
      for(; tgrid_y > fgrid_y ; tgrid_y--)
	if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
    }
  return(1);
}


/*
** Puts coordinates of nearest landmark of the type given in lm->type into
** lm->x and lm->y.  A coordinate of (-1,-1) means none was found.
*/
crank_nearest_landmark(lm)
     Landmark_info *lm;
{
  Location myloc;
  int num_lmarks;
  Landmark_info lmark[MAX_LANDMARKS];
  int curdist;
  int mindist = 9999;		/* distance to nearest landmark so far */
  int dx,dy;
  int i;

  /* find out where the vehicle is */
  get_location(&myloc);

  /* find out where all the landmarks are */
  get_landmarks(&num_lmarks,lmark);

  /* set lm coordinates to (-1,-1) in case we find no landmarks */
  lm->x = -1;
  lm->y = -1;

  /* look through the array of landmarks, looking for the specified
     type, and remembering the closest one */
  for (i = 0 ; i < num_lmarks ; ++i)
    /* Check for landmark of the right type */
    if (lmark[i].type == lm->type) {
      /* find out how far away it is */
      dx = lmark[i].x - myloc.grid_x;
      dy = lmark[i].y - myloc.grid_y;
      curdist = dx*dx + dy*dy;

      /* Check to see if it is closer than the closest one so far */
      if (curdist < mindist) {
	/* copy the coordinates and the distance */
	lm->x = lmark[i].x;
	lm->y = lmark[i].y;
	mindist = curdist;
      }
    }
}


/*
** Returns 1 if any weapon is out of ammo, otherwise returns 0
*/
crank_any_out_of_ammo()
{
  int i;
  int answer = 0;

  for (i=0;i<num_weapons();++i) 
    if (weapon_ammo(i)==0) answer = 1;
  
  return(answer);
}


/*
** Returns 1 if all weapons are out of ammo, otherwise returns 0
*/
crank_all_out_of_ammo()
{
  int i;
  int answer = 1;
  
  for (i=0;i<num_weapons();++i)
    if (weapon_ammo(i)>0) answer = 0;

  return(answer);
}


/*
** Turns off all weapons.  Useful when arriving at an ammo dump.
*/
crank_turn_off_all_weapons()
{
  int i;
  for (i=0;i<num_weapons();++i)
    turn_off_weapon(i);
}


/*
** Turns on all weapons.  Useful when leaving an ammo dump.
*/
crank_turn_on_all_weapons()
{
  int i;
  for (i=0;i<num_weapons();++i)
    turn_on_weapon(i);
}


/*
** Executes a careful turn to the desired angle.
** Slows down to speed 3.0, then turns, then speeds up to original speed.
*/
crank_careful_turn(desired_angle)
     float desired_angle;
{
  float myspeed = speed();

  /* Slow down to speed 3.0 so we can turn well */
  set_abs_speed(3.0);
  while (speed() > 3.0)
    ;

  /* Turn to desired angle */
  turn_vehicle(desired_angle);

  /* Speed up to original speed */
  set_abs_speed(myspeed);
}


/*
** Causes your vehicle to stop, spin and fire forward for the rest of
** the battle.  Not very useful, but fun to watch...
*/
crank_go_nuts()
{
  int i;

  set_abs_speed(0);
  while (!crank_all_out_of_ammo()) {
    turn_vehicle(angle()+.3);
    for (i=0;i<num_weapons();++i) {
      turn_turret(weapon_mount(i),angle());
      fire_all_weapons();
    }
  }
}
