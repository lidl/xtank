/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** shooter.c
*/

#include "xtanklib.h"
#include <math.h>

extern int shooter_main();

Prog_desc shooter_prog = {
  "shooter",
  "Qwiky",
  "Sits in one place and fires at the nearest enemy.",
  "Terry Donahue",
  PLAYS_COMBAT|DOES_SHOOT,
  1,
  shooter_main
};

#define ENEMY_NONE   0
#define ENEMY_RADAR  1
#define ENEMY_SCREEN 2

shooter_main()
{
  Vehicle_info enemy;
  Location myloc;
  Weapon_info winfo;
  int enemy_flag;

  /* Find out info about first weapon */
  if(num_weapons() > 0) 
    get_weapon(0,&winfo);
  else
    winfo.ammo_speed = 0;

  for(;;) {
    /* figure out where we are */
    get_location(&myloc);

    /* find the nearest enemy */
    enemy_flag = shooter_find(&myloc,&enemy);

    /* try to shoot at enemy if on screen and we have a weapon */
    if(enemy_flag == ENEMY_SCREEN && winfo.ammo_speed != 0)
      shooter_shoot(&myloc,&enemy,winfo.ammo_speed);

    /* Give up remaining cpu time to improve speed of game */
    done();
  }
}

/*
** Looks for vehicles on the screen.  Puts closest vehicle with
** a clear path to it into t and returns ENEMY_SCREEN.  If no
** such vehicle, returns ENEMY_NONE.
*/
shooter_find(myloc,t)
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
shooter_shoot(myloc,t,ammo_speed)
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
