/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** drone.c
**
** This is an xtank program designed to act intelligently without using
** up much cpu time.
*/

#include "xtanklib.h"
#include <math.h>

extern int drone_main();

Prog_desc drone_prog = {
  "drone",
  "Vanguard",
  "Wanders towards enemies and attacks them.  This program is designed to \
use a small amount of processing time, improving speed of games with many \
robots.",
  "Terry Donahue",
  PLAYS_COMBAT|DOES_SHOOT|DOES_EXPLORE,
  2,
  drone_main
};

#define ENEMY_NONE   0
#define ENEMY_RADAR  1
#define ENEMY_SCREEN 2

drone_main()
{
  Vehicle_info enemy;
  Location myloc;
  Weapon_info winfo;
  int find_count,move_count;
  int find_thresh,move_thresh;
  int enemy_flag;

  /* Initialize counters */
  find_count = 0;
  find_thresh = 4;
  move_count = 0;
  move_thresh = 10;

  /* Get us going at the beginning */
  set_rel_drive(5.0);

  /* Find out info about first weapon */
  if(num_weapons() > 0) 
    get_weapon(0,&winfo);
  else
    winfo.ammo_speed = 0;

  for(;;) {
    /* Figure out where we are */
    get_location(&myloc);

    /* Find the nearest enemy */
    if(find_count++ > find_thresh) {
      enemy_flag = drone_find(&myloc,&enemy);
      find_count = 0;
    }

    /* Try to shoot at enemy if on screen and we have a weapon */
    if(enemy_flag == ENEMY_SCREEN && winfo.ammo_speed != 0)
      drone_shoot(&myloc,&enemy,winfo.ammo_speed);

    /* Every MOVE_THRESH iterations, move towards the enemy */
    if(move_count++ > move_thresh) {
      switch(enemy_flag) {
        case ENEMY_NONE:
	  drone_move_to_box(&myloc,rnd(GRID_WIDTH),rnd(GRID_HEIGHT));
	  break;
        case ENEMY_RADAR:
        case ENEMY_SCREEN:
	  drone_move_to_box(&myloc,enemy.loc.grid_x,enemy.loc.grid_y);
	  break;
	}
      move_count = 0;
    }

    /* Give up remaining cpu time to improve speed of game */
    done();
  }
}

/*
** Looks for vehicles on the screen.  Puts closest vehicle with
** a clear path to it into t and returns ENEMY_SCREEN.  If no
** such vehicle, returns ENEMY_NONE.
*/
drone_find(myloc,t)
     Location *myloc;
     Vehicle_info *t;
{
  Vehicle_info vinfo[MAX_VEHICLES];
  int num_vinfos;
  Vehicle_info *v;
  int dx,dy,range,min_range;
  int i;

  get_vehicles(&num_vinfos,vinfo);
  
  /*
  ** Find the closest vehicle with a clear path to it.
  */
  min_range = 99999999;
  for(i = 0 ; i < num_vinfos ; i++) {
    v = &vinfo[i];

    /* ignore vehicles that have no clear path to them */
    if(!clear_path(myloc,&v->loc)) continue;

    dx = v->loc.x - myloc->x;
    dy = v->loc.y - myloc->y;
    range = dx*dx + dy*dy;
    if(range < min_range) {
      min_range = range;
      *t = *v;
    }
  }

  if(min_range == 99999999)
    return ENEMY_NONE;
  else
    return ENEMY_SCREEN;
}

/*
** Shoots at vehicle t with all weapons.  Uses quick leading fan algorithm.
*/
drone_shoot(myloc,t,ammo_speed)
     Location *myloc;
     Vehicle_info *t;
     int ammo_speed;
{
  int dx,dy;
  float lead_factor;

  dx = t->loc.x - myloc->x;
  dy = t->loc.y - myloc->y;

  /* Lead the target approximately, shoot fanning */
  lead_factor = 2 * sqrt((double) (dx*dx + dy*dy)) /
    (float) ammo_speed;
  dx += (int) (t->xspeed * lead_factor * (float) rnd(20) / 19.0);
  dy += (int) (t->yspeed * lead_factor * (float) rnd(20) / 19.0);

  /* Point all the turrets towards where he is going to be */
  aim_all_turrets(dx,dy);

  /* Shoot all weapons */
  fire_all_weapons();
}

/*
** Wanders towards the box with coordinates (targ_x,targ_y).
*/
drone_move_to_box(myloc,targ_x,targ_y)
     Location *myloc;
     int targ_x,targ_y;
{
  static box_x[4] = { BOX_WIDTH/2, BOX_WIDTH, BOX_WIDTH/2, 0 };
  static box_y[4] = { 0, BOX_HEIGHT/2, BOX_HEIGHT, BOX_HEIGHT/2 };
  int grid_x,grid_y;
  int i,choices,dir,pick;
  float spd,ang;
  int dx,dy;
  int odds[4];

  /* If we've stopped, move forwards or backwards without turning */
  if(speed() == 0) {
    spd = rnd(10) ? -3.0 : 5.0;
  }
  else {
    grid_x = myloc->grid_x;
    grid_y = myloc->grid_y;
      
    /* Increase the odds of going towards the target */
    if(targ_x <= grid_x) odds[WEST] = 4;
    else odds[WEST] = 1;
    if(targ_x >= grid_x) odds[EAST] = 4;
    else odds[EAST] = 1;
    if(targ_y <= grid_y) odds[NORTH] = 4;
    else odds[NORTH] = 1;
    if(targ_y >= grid_y) odds[SOUTH] = 4;
    else odds[SOUTH] = 1;
    
    /* If a dir is blocked by a wall, make the odds of going that way 0 */
    choices = 0;
    for(i = 0 ; i < 4 ; i++) {
      if(wall(grid_x,grid_y,i)) odds[i] = 0;
      choices += odds[i];
    }
      
    /* Pick a random choice and determine the direction */
    pick = rnd(choices);
    for(dir = 0, i = odds[dir] ; i < pick ; i += odds[++dir])
      ;

    /* Move towards middle of edge towards selected direction */
    dx = box_x[dir] - myloc->box_x;
    dy = box_y[dir] - myloc->box_y;
    ang = atan2((float) dy,(float) dx);

    turn_vehicle(ang);

    /* Move at a random speed from 2 to 6 */
    spd = (float) (2 + rnd(5));
  }

  set_rel_drive(spd);
}
