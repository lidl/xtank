/********************************************
* DJChello.c - my first xtank robot
* This robot isn't meant to be original.
* It's just my attempt to make something
* which works.
********************************************/

#include <stdio.h>
#include <math.h>
#include "/mit/games/src/vax/xtank/Programs/xtanklibnew.h"

#define NOT_HERE (-1)
#define UNKNOWN_ID 255
#define MAX_WEAPONS 6
#define NULL 0
#define DELTAHEADING (.50)

extern int DJChello3_main();

Prog_desc DJChello3_prog =
{
  "DJChello3",    /* program name */
  "Vanguard",     /* default tank */
  "martyrdom",    /* strategy */
  "Dan Connelly", /* author */
  USES_MESSAGES|PLAYS_COMBAT|DOES_SHOOT,  /* skills */
  3,              /* skill level */
  DJChello3_main  /* main procedure */
};

struct PlayerInfo
{
  ID  id;                           /* player identification */
  int status;                       /* number in array of visible vehicles or NOT_HERE */
};

static struct MazeData {
  int            num_visits[GRID_WIDTH][GRID_HEIGHT];    /* number of times I have visited a box */
};

static struct TankStatus {
  Vehicle_info   my_info;                      /* vehicle information about me */
  Location       *my_location;                 /* vehicle location ( points into my_info )*/
  Team           *my_team;                      /* vehicle team (points to my_info) */
  float          max_speed;                    /* maximum speed my vehicle is capable of */
  int            num_weapons;                  /* number of weapons on my tank */
  Weapon_info    weapon[MAX_WEAPONS];          /* info about all of my weapons */
  Weapon_info    *prime_weapon;                /* points to best turret weapon */
  int            num_vehicles;                 /* number of visible vehicles */     
  Vehicle_info   vehicle[MAX_VEHICLES];        /* info about other vehicles */
  Vehicle_info   *enemy;                       /* points to nearest enemy or NULL if no enemy visible */
  ID             old_enemy;                    /* ID number of enemy last time I checked */
  struct PlayerInfo  player_info[MAX_VEHICLES+1]; /* information about other players; to say hello */
  int            num_blips;                    /* number of blips on radar screen */
  Blip_info      blip[MAX_BLIPS];              /* information about blips on screen */
  struct MazeData maze_data;                   /* information I've picked up about the maze */
  Location       destination;                  /* where I want to go */  
  float          time;                         /* expected time to impact; static for iteration */
};


/*************************************************************************
* DJCHELLO3_MAIN : main procedure; initializes and calls other procedures
*/

DJChello3_main()
{
  struct TankStatus status;

  initialize(&status);

  while(1) {
    get_vehicles(&(status.num_vehicles), status.vehicle);
    get_location(status.my_location);

    search_for_enemies(&status);

    combat(&status);

    done();
  }
}

/**********************************************
* INITIALIZE : initialize all the stuff that
* needs it.
*/
initialize(stat)
struct TankStatus *stat;
{
  int i;          /* counting variable */

  /*---Initialize player information array---*/
  
  get_self(&(stat->my_info));
  stat->my_location = &(stat->my_info.loc);
  stat->my_team     = &(stat->my_info.team);
  
  stat->player_info[0].id = stat->my_info.id;               /* I'm accounted for */
  stat->player_info[0].status = NOT_HERE;
  
  for (i=1; i<=MAX_VEHICLES; i++)
    {
      stat->player_info[i].status = NOT_HERE;              /* mark that nobody's here */
      stat->player_info[i].id     = UNKNOWN_ID;       /* nobody else is known */
    }

  /*---Figure out what sort of weapons I have---
     I care only about my best turret-mounted weapon */

  stat->num_weapons = num_weapons();
  stat->prime_weapon = stat->weapon;     /* start out pointing to the first weapon */
  if (stat->num_weapons > 0)
    {
      get_weapon(0, stat->weapon);         /* get info on the first weapon */

      for (i=1; (i < stat->num_weapons) && (i < MAX_WEAPONS ); i++)
	{
	  get_weapon(0, &(stat->weapon[i]));   /* get info on another weapon */
	  if ( ( (stat->weapon[i].mount == MOUNT_TURRET1) ||
		 (stat->weapon[i].mount == MOUNT_TURRET2) ||
		 (stat->weapon[i].mount == MOUNT_TURRET3) ) &&
	       (stat->weapon[i].range > stat->prime_weapon->range) )
	    stat->prime_weapon = &(stat->weapon[i]);
	}
    }

  /*--- misc initializations ---*/
  stat->time = 0;      /* set the time to impact to zero for the first initialization */
}
/***********************************************
* SEARCH_FOR_ENEMIES : look for enemies.  Say
* hello and goodbye to those who come and go.
*/
search_for_enemies(stat)
struct TankStatus *stat;
{
  int i,j;        /* counting variables */
  int num;        /* a temporary variable */
  int dx,dy;      /* deltas to enemy ( in pixels ) */
  int range;      /* dist^2 to find nearest enemy */
  int min_range;  /* dist^2 to nearest enemy */

  /* say hello to new friends */
  stat->enemy = NULL;     /* set enemy to the nul pointer; assume nobody's there first  */
  for (i=0; i < stat->num_vehicles; i++)
    {
      for (j=0; (stat->player_info[j].id != stat->vehicle[i].id) &&
	   (stat->player_info[j].id != UNKNOWN_ID); j++);
      if (stat->player_info[j].status == NOT_HERE)
	{
	  send(stat->vehicle[i].id, OP_TEXT, "Hello!");  /* send a hearty hello */
	  /* send(stat->my_info.id, OP_TEXT, "hello sent");  /* Debug line */
	  stat->player_info[j].id = stat->vehicle[i].id;      /* remember who he is */
	}
      stat->player_info[j].status = i;                  /* remember that he is here */
      if ( !(*(stat->my_team)) || (stat->vehicle[i].team != *(stat->my_team)) )  /* he's an enemy; check range */
	{
	  dx = (stat->vehicle[i].loc.x - (stat->my_location)->x);
	  dy = (stat->vehicle[i].loc.y - (stat->my_location)->y);
	  range = dx*dx + dy*dy;
	  if ((stat->enemy == NULL) || (range < min_range))
	    {
	      min_range = range;
	      stat->enemy = &(stat->vehicle[i]);
	    }
	}
    }
    
  /* say goodbye to departed loved ones */
  for (i=0; stat->player_info[i].id != UNKNOWN_ID; i++)
    {
      if ( (stat->player_info[i].id != stat->my_info.id) && (num = stat->player_info[i].status) != NOT_HERE &&
	  ( (num >= stat->num_vehicles) || stat->vehicle[num].id != stat->player_info[i].id ))
	{
	  send(stat->player_info[i].id, OP_TEXT, "Goodbye!");     /* wave goodbye */
	  /* send(stat->my_info.id, OP_TEXT, "goodbye sent"); /* Debug line */
	  stat->player_info[i].status = NOT_HERE;  /* remember that he's gone */
	}
    }
}

/****************************************************************
* COMBAT : if enemies are near,go into a tight circle and shoot
*   a lot
*/
combat(stat)
struct TankStatus *stat;
{
  Vehicle_info   *enemy;                 /* to prevent lots of references to stat */
  int            dx,dy;                  /* relative position of enemy */

  enemy = stat->enemy;

  /* check for a nearby enemy */
  if ((!enemy) || (stat->num_weapons == 0))
    {
      stat->old_enemy = stat->my_info.id;    /* mark myself as my enemy so I will recognize a new enemy */
      set_rel_drive(0.0);             /* stop rotating if no enemies are around */
    }
  else {
    /* find spot to aim at - iterate solution*/
    dx = (enemy->loc.x - (stat->my_location)->x + (int)((stat->time)*(enemy->xspeed)));
    dy = (enemy->loc.y - (stat->my_location)->y + (int)((stat->time)*(enemy->yspeed)));
    stat->time = (float)sqrt((double)(dx*dx + dy*dy))/(float)((stat->prime_weapon)->ammo_speed);
    dx = (enemy->loc.x - (stat->my_location)->x + (int)((stat->time)*(enemy->xspeed)));
    dy = (enemy->loc.y - (stat->my_location)->y + (int)((stat->time)*(enemy->yspeed)));
    
    /*fire on the spot*/
    aim_all_turrets(dx,dy);
    fire_all_weapons();
    
    /*taunt the enemy when I pick him*/
    if (stat->old_enemy != enemy->id)
      {
	stat->old_enemy = enemy->id;
	send(stat->old_enemy, OP_TEXT, "Will you be my friend?");	   
	set_rel_drive(9.0);       /* move to stay away from enemy shots*/
      }
  }
  
  /*turn to avoid overexposure of one side*/
  turn_vehicle(heading() + DELTAHEADING);
}
