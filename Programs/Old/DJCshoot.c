/********************************************
* DJCshoot.c - my first xtank robot
* This robot isn't meant to be original.
* It's just my attempt to make something
* which works.
********************************************/

#include <stdio.h>
#include <math.h>
#include "/mit/games/src/vax/xtank/Src/xtanklib.h"

#define NOT_HERE (-1)
#define UNKNOWN_ID 255
#define MAX_WEAPONS 6
#define NULL 0

extern int DJCshoot_main();

Prog_desc DJCshoot_prog =
{
  "DJCshoot",     /* program name */
  "Vanguard",     /* default tank */
  "martyrdom",    /* strategy */
  "Dan Connelly", /* author */
  USES_MESSAGES|PLAYS_COMBAT|DOES_SHOOT,  /* skills */
  1,              /* skill level */
  DJCshoot_main   /* main procedure */
};




/*************************************************************************
* DJCSHOOT_MAIN : main procedure; initializes and calls other procedures
*/

DJCshoot_main()
{

  Vehicle_info	my_info;               /* info about me */
  Location      *my_location;           /* my tank's location */
  Vehicle_info  vehicle[MAX_VEHICLES];  /* info about all visible vehicles */
  int		num_vehicles;           /* number of vehicles seen */
  Weapon_info   weapon[MAX_WEAPONS];    /* weapons mounted on my tank */    
  Weapon_info   *prime_weapon;          /* weapon which I plan strategy with */
  int           numweapons;             /* number of weapons on my tank */
  Vehicle_info  *enemy;                 /* nearest enemy */
  ID            old_enemy;              /* ID of nearest enemy last frame */
  int           dx,dy,range;            /* distance to other tank */
  int           min_range;              /* dist^2 to other tank */
  int           i,j;                    /* counting variables */
  int           num;                    /* temporary storage variable */
  float         t;                      /* time to ammo impact */
  float         dxenemy, dyenemy;       /* distance to enemy in pixels */
  Angle         aim_angle=.8, new_angle=.8; /* angle to aim weapons (iterative improvement) */


  /*---Initialize player information array---*/

  my_location= &(my_info.loc);
  get_self(&my_info);

  /*---Figure out what sort of weapons I have---
     I care only about my best turret-mounted weapon */


  numweapons = num_weapons();
  prime_weapon = &weapon[0];     /* start out pointing to the first weapon */
  if (numweapons > 0)
    {
      get_weapon(0, weapon);         /* get info on the first weapon */

      for (i=1; (i < numweapons) && (i < MAX_WEAPONS ); i++)
	{
	  get_weapon(0, &weapon[i]);   /* get info on another weapon */
	  if ( ( (weapon[i].mount == MOUNT_TURRET1) ||
		 (weapon[i].mount == MOUNT_TURRET2) ||
		 (weapon[i].mount == MOUNT_TURRET3) ) &&
	       (weapon[i].range > prime_weapon->range) )
	    prime_weapon = &weapon[i];
	}
    }



  while(1) {
    get_vehicles(&num_vehicles, vehicle);
    get_location(my_location);
  
    /* say hello to new friends */
    enemy = NULL;     /* set enemy to the nul pointer */
    for (i=0; i<num_vehicles; i++)
      {
	send(RECIPIENT_ALL,OP_TEXT,"Vehicle seen");

	if (vehicle[i].team != my_info.team)       /* he's an enemy; check range */
	  {
	    send(RECIPIENT_ALL, OP_TEXT, "Enemy detected");
	    dx = (vehicle[i].loc.x - my_location->x);
	    dy = (vehicle[i].loc.y - my_location->y);
	    range = dx*dx + dy*dy;
	    if ( (!enemy) || (range < min_range))
	      {
		min_range = range;
		enemy = &(vehicle[i]);
	      }
	  }
      }
    

    /* check for a nearby enemy */
    if ((!enemy) || (numweapons == 0))
      {
	old_enemy = my_info.id;    /* mark myself as my enemy so I will recognize a new enemy */
      }
    else {
      send(RECIPIENT_ALL, OP_TEXT, "You are my enemy");
      /* find spot to aim at */
      dxenemy = (float)(enemy->loc.x - my_location->x);
      dyenemy = (float)(enemy->loc.y - my_location->y);
      do {
	aim_angle= new_angle;
	t = dxenemy / ( (prime_weapon->ammo_speed) * cos(aim_angle) - enemy->xspeed );
	new_angle = atan2( dyenemy + (enemy->yspeed)*t, dxenemy + (enemy->xspeed)*t );
      } while (abs(new_angle - aim_angle) > .05);
      aim_angle= new_angle;
      
      /*fire on the spot*/
      turn_all_turrets(aim_angle);
      fire_all_weapons();

      /*taunt the enemy when I pick him*/
      if (old_enemy != enemy->id)
	{
	  old_enemy = enemy->id;
	  send (old_enemy, OP_TEXT, "I'm gonna get YOU!");
	}
    }

    done();
  }
}
