/*
** disgruntled.c
**
** A simple idea that works.
*/

#include "/mit/games/src/vax/xtank/Battle/xtanklib.h"
#include <math.h>

extern disgruntled_main();

 Prog_desc disgruntled_prog = {
   "disgruntled",
   "war",
   "tiger#2",
"Disgruntled is a tank which is a fairly decent, albeit unimpressive, \
fighting vehicle. It has a firing pattern which leads its target by an \
appropriate ammount, which gives it much better accuracy than spot or \
several other tanks. It also knows how to free itself from walls, although \
it is too simplistic to avoid them in the first place.",
"Terry Donahue",
2,
disgruntled_main
};

disgruntled_main()
{
  Vehicle_info vehicles[MAX_VEHICLES];
  int numvinfos;
  char temp[40];
  int id;

  while (1) {
    if (speed() == 0)
      disgruntled_get_unstuck();
    else {
      set_rel_speed(5.0);
      turn_vehicle((float) (rand()%628) / 100);
    }

    /* see if there are any vehicles around */
    get_vehicles(&numvinfos,vehicles);
    
    /* if there is another vehicle on screen, shoot at it */
    sprintf(temp,"numvinfos = %d",numvinfos);
    message(temp);
    if (numvinfos > 0) {
      id = vehicles[numvinfos - 1].id;
      disgruntled_shoot_at(id);
      sprintf(temp,"shot at id #%d",id);
      message(temp);
    }
  }
}



/*
** Gets the vehicle unstuck from an obstacle.  Call it if you think you
** ran into something.
*/
disgruntled_get_unstuck()
{
  int framenum = frame_number();
  float curangle = angle();

  /* First backup away from the obstacle */
  set_rel_speed(-3.0);

  /* Wait until 5 frames have passed, so we back up enough */
  while(frame_number() < framenum + 5)
    ;

  /* Turn around and get moving */
  turn_vehicle(curangle + PI);
  set_rel_speed(5);
}


/* 
** Moves all turrets at the vehicle whose id is vnum.
** Shoots all weapons at that vehicle, regardless of whether
** or not it is in range, or the turrets are rotated yet.
*/
disgruntled_shoot_at(vnum)
{
  int num_vehicles;               /* the number of vehicles I can see */
  Vehicle_info vehicle[MAX_VEHICLES];  /* the array of vehicles I can see */
  Vehicle_info *v;
  Location my_loc;                /* my location */
  int i;                          /* the vehicle number I'm looking at */
  int dx,dy;                      /* the distance between me and my target */
  float lead_factor;

  get_location(&my_loc);	/* find my location */
  get_vehicles(&num_vehicles,vehicle);  /* look around */

  /* go through all the vehicles I can see and look for the one I want
     to shoot at. */

  for (i = 0 ; i < num_vehicles ; i++) {
    v = &vehicle[i];
    if (v->id == vnum) {                  /* Yup, that's the one */

      /* figure out the distance between me and him (in pixels)
	 along the x axis (dx) and the y axis (dy) */

      dx = BOX_WIDTH * (v->loc.grid_x - my_loc.grid_x)
	   + v->loc.box_x - my_loc.box_x;
      dy = BOX_HEIGHT * (v->loc.grid_y - my_loc.grid_y)
           + v->loc.box_y - my_loc.box_y;

      lead_factor = sqrt((double)(dx*dx+dy*dy))/weapon_ammo_speed(0);
      dx += (int) (v->xspeed*lead_factor);
      dy += (int) (v->xspeed*lead_factor);

      aim_all_turrets(dx,dy);     /* aim my turret(s) at him */
      fire_all_weapons();            /* let him have it */
    }
  }
}
