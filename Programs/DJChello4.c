/********************************************
* DJChello.c - my first xtank robot
* This robot isn't meant to be original.
* It's just my attempt to make something
* which works.
*
* The program is optimized to play combat in
* a sparse maze against cybornetic opponants.
* Its strategy is to explore the
* maze until interrupted.  If it sees blips on
* its radar, it will try to chase them down
* if it is healthy.  When it encounters enemy
* tanks it will move in a circle and fire on
* them, using an iterative lead algorithm.
* Anytime a new tank appears on its screen,
* it sends it a "hello" message.  When tanks 
* disappear from view they receive goodbyes.
* If the tank finds it is low on armor, fuel,
* or ammo it will, if possible, find the nearest
* replenishment sight and go there until full.
* I have tried to keep everything as simple
* as possible and still have the tank perform
* reasonably well.  The program evaluates its
* destinations using a simple but inefficient
* semi-recursive evaluation algorithm.  It
* works well in that it places higher weight to
* more accessable neighboring squares, but it
* evaluates squares up to four separate times
* each scan.
*
********************************************/

#include <stdio.h>
#include <math.h>
#include "xtanklib2.h"


/* some basic constants to be defined */
#ifndef NULL
 #define NULL 0
#endif
#ifndef TRUE
 #define TRUE 1
#endif
#ifndef FALSE
 #define FALSE 0
#endif

/* something Terry left out of the game */
#define MAX_WEAPONS              10      /* maximum number of weapons I expect to find on the tank */
#define SCREEN_BOX_RADIUS        (SCREEN_WIDTH/BOX_WIDTH - 1)/2
#define MIDDLE_OF_BOX_X          BOX_WIDTH/2
#define MIDDLE_OF_BOX_Y          BOX_HEIGHT/2
#define NUM_SIDES                 6        /*maximum side number*/
#define BLIP_X_OFFSET             2        /* add to reported blip locations (xtank error ) */
#define BLIP_Y_OFFSET             2        /* add to reported blip locations (xtank error )  */

/* scores for each landmark type */
#define OUTSIDE_MAZE_SCORE   100000      /* you don't want to go outside the maze */
#define OUTPOST_IN_BOX_SCORE    200      /* avoid outposts */
#define TANK_IN_BOX_SCORE      -200      /* get 'im (per tank) */
#define TANK_IN_BOX_SCORE2      200      /* for use when I'm trying to avoid tanks */
#define BLIP_PROXIMITY_SCORE  (-65536)    /* see local_score or blip_score for algorithm */
#define LANDMARK_PROXIMITY_SCORE (-65536) /* see local_score for algorithm */
#define DESTINATION_SCORE     (-5000)   /* if a box is my destination */
#define MY_GOAL_SCORE         (-5000)    /* the goal of my team */
#define VISITED_BOX_SCORE        75      /* per visit to the box */
#define FUEL_BOX_SCORE          (-20)    /* gets multiplied by fuel_ratio */
#define ARMOR_BOX_SCORE         (-20)    /* gets multiplied by armor ratio */
#define AMMO_BOX_SCORE          (-10)    /* gets multiplied my ammo ratio */
#define SCROLL_BOX_SCORE         20      /* arrows are a pain */
#define SLOW_BOX_SCORE           10      /* slow boxes are a waste of time */
#define SLIP_BOX_SCORE            5      /* avoid oil slick boxes */
#define WALL_SCORE               30      /* discourage robot from bumping into walls */
#define BLIP_ZERO_DISTANCE        5      /* blip score = B.P.S / (dist + B.Z.D.) */
#define LANDMARK_ZERO_DISTANCE    5      /* landmark score = L.P.S / (dist + L.Z.D.) */

/* parameters to control tank */
#define SLOWDOWN_FACTOR        (BOX_WIDTH/16)   /* slow down 16 speed units per box distance from destination */

/* define parameters for combat and survallence*/
#define NOT_HERE                (-1)
#define UNKNOWN_ID              255
#define DELTAHEADING            (.50)
#define BLIP_REFRESH_TIME       6        /* get new blip info every 6 frames */

/* when to attempt refresh */
#define FUEL_CRITICAL_RATIO     5        /* when tank hits 20%, run for the nearest EXXON */
#define ARMOR_CRITICAL_RATIO    5        /* if any side reaches 20%, go for repairs */
#define AMMO_CRITICAL_RATIO    20        /* when I have only 5% of my shots left, go for more */

/* define simple "functions" */
#define max(a,b)   ( ((a) > (b)) ? (a) : (b) )
#define min(a,b)   ( ((a) < (b)) ? (a) : (b) )
#define abs(a)     ( ((a) > 0) ? (a) : -(a) )


extern int DJChello4_main();

Prog_desc DJChello4_prog =
{
  "DJChello4",    /* program name */
  "Vanguard",     /* default tank */
  "Seek out and destroy",    /* strategy */
  "Dan Connelly", /* author */
  USES_MESSAGES|PLAYS_COMBAT|PLAYS_RACE|DOES_SHOOT|DOES_EXPLORE|DOES_REPLENISH|USES_TEAMS,  /* skills */
  5,              /* skill level */
  DJChello4_main  /* main procedure */
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
  int            max_armor[NUM_SIDES];         /* maximum armor my tank has per side */
  int            armor_ratio;                  /* number representing how badly armor is needed */
  float          max_fuel;                     /* maximum fuel the tank can carry */
  int            fuel_ratio;                   /* 1 / fraction of fuel I have left */
  int            num_weapons;                  /* number of weapons on my tank */
  Weapon_info    weapon[MAX_WEAPONS];          /* info about all of my weapons */
  Weapon_info    *prime_weapon;                /* points to best turret weapon */
  int            ammo_ratio;                   /* 1/ fraction of ammo left in prime weapon */
  int            num_vehicles;                 /* number of visible vehicles */     
  Vehicle_info   vehicle[MAX_VEHICLES];        /* info about other vehicles */
  Vehicle_info   *enemy;                       /* points to nearest enemy or NULL if no enemy visible */
  ID             old_enemy;                    /* ID number of enemy last time I checked */
  struct PlayerInfo  player_info[MAX_VEHICLES+1]; /* information about other players; to say hello */
  int            num_blips;                    /* number of blips on radar screen */
  Blip_info      *blip;                        /* information about blips on screen */
  int            num_old_blips;                /* number of blips previous time I checked */
  Blip_info      *old_blip;                    /* saved information on blips ( prevent loss of info ) */
  Blip_info      blip_array1[MAX_BLIPS];       /* allocate space for the blip storage arrays */
  Blip_info      blip_array2[MAX_BLIPS];       /* allocate space for the clip storage arrays */
  int            last_blip_refresh_frame;      /* last frame on which blips were refreshed */
  int            num_landmarks;                /* number of landmarks seen on map */
  Landmark_info  landmark[MAX_LANDMARKS];      /* information about landmarks on map */
  Landmark_info  *destination_landmark;        /* Long range goal or NULL if I have none; points to landmark[] */
  struct MazeData maze_data;                   /* information I've picked up about the maze */
  Location       destination;                  /* where I want to go */
  float          time;                         /* expected time to impact; static for iteration */
};


/*************************************************************************
* DJCHELLO4_MAIN : main procedure; initializes and calls other procedures
*/

DJChello4_main()
{
  struct TankStatus status;     /* global variables */
  int old_grid_x = -1;          /* keep track of which grid I was in last time */
  int old_grid_y = -1;          /* note initialization to automatically record initial box data */
  
  set_rel_drive(9.0);           /* get moving to avoid being a sitting duck during initialization */
  initialize(&status);          /* determine statis and initialize variables */

  get_destination(&status);   /*go someplace to start off */

  while(1) {
    /* update information about surrounding each go-through */
    get_self(&status.my_info);

    do{
      /* renew information about environment and myself */
      refresh_blips(&status);                           /* refresh the blip info if it is time */
      get_vehicles(&(status.num_vehicles), status.vehicle);   /* find other vehicles */
if (status.num_vehicles != 0) fprintf(stderr, "DJC: status.num_vehicles %d\n", status.num_vehicles);
      get_location(status.my_location);                 /* update my location for proper aiming */
      search_for_enemies(&status);                      /* find enemies and say hello, goodbye */

      /* fire on nearest enemy, if possible.  Go into turn if shots were fired */
      if ( combat(&status) ){                           /* fight if necessary; return true if I was able to fire */
	set_rel_drive(9.0);                             /* full speed while in combat */
	turn_vehicle(heading() + DELTAHEADING);         /* turn if I am in a state of combat */
      }

      /* If vehicle needs supplies, dispatch to a procedure to get vehicle to supplies
	 Note they are checked in INVERSE order of importance (overwrite)*/
      compute_damage_ratios(&status);                  /* calculate need for ammo; if I need ammo I quit the fight*/
      if (status.ammo_ratio > AMMO_CRITICAL_RATIO) {
	get_replenish_sight(&status, AMMO);            /* find nearest ammunition depot */
	status.enemy = NULL;                           /* get out of here */
	}

    } while (status.enemy);

    /* check for armor and fuel after the fight is over */

    if (status.armor_ratio > ARMOR_CRITICAL_RATIO)
      get_replenish_sight(&status, ARMOR);            /* find nearest armor depot */

    if (status.fuel_ratio > FUEL_CRITICAL_RATIO)
      get_replenish_sight(&status, FUEL);             /* find nearest fuel depot */

    if ( ( status.destination_landmark ) &&
	(status.destination_landmark->x == status.my_location->grid_x) &&
	(status.destination_landmark->y == status.my_location->grid_y) ) {
      check_replenishment(&status);                   /* check to see if repairs have been completed */
    }

    /* update information about number of times I have been here*/
    if (((status.my_location)->grid_x != old_grid_x) ||
        ((status.my_location)->grid_y != old_grid_y) ) {
      get_destination(&status);                       /* after entering new square, find new destination */
      status.maze_data.num_visits[old_grid_x =        /* increment counter of number of times I have visited here */
		(status.my_location)->grid_x][old_grid_y = (status.my_location)->grid_y]++;
    }

    move_to(&status);                                 /* move to my next destination */

  }
}

/**********************************************
* INITIALIZE : initialize all the stuff that
* needs it at the start of the game
*/
initialize(stat)
struct TankStatus *stat;
{
  int i;                   /* counting variable */


  /*---Initialize player information array---*/
  
  get_self(&(stat->my_info));
  stat->my_location = &(stat->my_info.loc);
  stat->my_team     = &(stat->my_info.team);
  
  stat->player_info[0].id = stat->my_info.id;               /* I'm accounted for */
  stat->player_info[0].status = NOT_HERE;
  
  for (i=MAX_VEHICLES+1; --i; )
    {
      stat->player_info[i].status = NOT_HERE;         /* mark that nobody's here */
      stat->player_info[i].id     = UNKNOWN_ID;       /* nobody else is known */
    }

  /*---Figure out what sort of weapons I have---
     I care only about my best turret-mounted weapon */

  stat->num_weapons = num_weapons();
  stat->prime_weapon = stat->weapon;     /* start out pointing to the first weapon */
  if (stat->num_weapons > 0)
    {
      get_weapon(0, stat->weapon);         /* get info on the first weapon */

      /* Note : low-numbered weapons run out of ammo first, so I use the biggest-numbered
	 long-range weapon as my prime weapon.  This is the weapon used for aiming and
	 monitoring for ammo loss */
      for (i = stat->num_weapons; --i; )   /* count down -> favor big number weapons */
	{
	  get_weapon(0, &(stat->weapon[i]));   /* get info on another weapon */
	  if ( ( (stat->weapon[i].mount == MOUNT_TURRET1) ||
		 (stat->weapon[i].mount == MOUNT_TURRET2) ||
		 (stat->weapon[i].mount == MOUNT_TURRET3) ) &&
	       (stat->weapon[i].range > stat->prime_weapon->range) )
	    stat->prime_weapon = &(stat->weapon[i]);
	}
    }

  /*---Figure out how much armor I have available---*/
  stat->max_armor[FRONT] = max_armor(FRONT);
  stat->max_armor[BACK] = max_armor(BACK);
  stat->max_armor[LEFT] = max_armor(LEFT);
  stat->max_armor[RIGHT] = max_armor(RIGHT);
  stat->max_armor[TOP] = max_armor(TOP);
  stat->max_armor[BOTTOM] = max_armor(BOTTOM);
  stat->max_fuel = max_fuel();
  compute_damage_ratios(stat);      /* verify that I have full armor, fuel, etc */

  /*---initialize blip arrays---*/
  stat->blip = stat->blip_array1;
  stat->old_blip = stat->blip_array2;

  /*--- misc initializations ---*/
  stat->time = 0;                        /* set the time to impact to zero for the first initialization */
  stat->num_weapons = num_weapons();     /* number of weapons mounted on my tank */
  stat->num_weapons = min(stat->num_weapons, MAX_WEAPONS);   /* limit to array size */
  stat->max_speed = max_speed();         /* how fast my tank can go on normal turf */
  stat->last_blip_refresh_frame = -BLIP_REFRESH_TIME; /* set for immediate refresh */
  stat->num_blips = 0;
  stat->num_old_blips = 0;
  stat->destination_landmark = NULL;     /* long-range destination, for refuelling etc */
}

/******************************************************
* COMPUTE_DAMAGE_RATIOS : calculate a score representing
* how badly armor is needed.
*/
compute_damage_ratios(stat)
struct TankStatus *stat;
{
  /*--- armor score ---
    The calculation for armor uses a harmonic sum; when any single side gets low on armor the
    flag will go up to get new armor.  Note sides which start with zero armor will not contribute
    to the sum */

  stat->armor_ratio  = stat->max_armor[FRONT] / (armor(FRONT) + 1) +
                       stat->max_armor[BACK] / (armor(BACK) + 1) +
		       stat->max_armor[LEFT] / (armor(LEFT) + 1) +
		       stat->max_armor[RIGHT] / (armor(RIGHT) + 1) +
		       stat->max_armor[TOP] / (armor(TOP) + 1) +
		       stat->max_armor[BOTTOM] / (armor(BOTTOM) + 1);

  /*--- fuel score ---
    When the fuel gets low, the fuel score will go like 1/fuel */

  stat->fuel_ratio = stat->max_fuel / (fuel() + .1);

  /*--- ammo score ---
    Note: the following uses the ammo in the prime weapon as a guage of the need for ammo.
    It uses address subtraction to calculate the index of the weapon to use.  This assumes
    that for structures &(a[m]) - &(a[n]) = m-n. */

  stat->ammo_ratio = (stat->prime_weapon)->max_ammo / (weapon_ammo(stat->prime_weapon - stat->weapon) + 1);

  /*============DEBUG INFORMATION==============*/
  {
    char data[128];
    sprintf(data,"A : %d; W : %d; F : %d",stat->armor_ratio, stat->ammo_ratio, stat->fuel_ratio);
    send(stat->my_info.id, OP_TEXT, data);
    if (stat->destination_landmark) {
      sprintf(data, "destination @ %d,%d", (stat->destination_landmark)->x, (stat->destination_landmark)->y);
      send(stat->my_info.id, OP_TEXT, data);
    }
    else
      send(stat->my_info.id, OP_TEXT, "no destination now");
  }
}
  

/***********************************************
* REFRESH_BLIPS : update the known information
* about blips, if necessary
*/
refresh_blips(stat)
struct TankStatus *stat;
{
  if (( frame_number() - stat->last_blip_refresh_frame ) >= BLIP_REFRESH_TIME) {
   Blip_info *temp;
   temp = stat->old_blip;
   stat->old_blip = stat->blip;
   stat->blip = temp;
   stat->num_old_blips = stat->num_blips;
   get_blips(&(stat->num_blips), stat->blip);
   stat->last_blip_refresh_frame = frame_number();

 }
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
int combat(stat)
struct TankStatus *stat;
{
  Vehicle_info   *enemy;                 /* to prevent lots of references to stat */
  int            dx,dy;                  /* relative position of enemy */
  int            my_x, my_y;             /* my location ( shorthand )*/
  Location       target_spot;            /* spot to aim bullets */
  int            i;                      /* counting variable */

  enemy = stat->enemy;

  /* check for a nearby enemy */
  if ((!enemy) || (stat->num_weapons == 0)) {
      stat->old_enemy = stat->my_info.id;    /* mark myself as my enemy so I will recognize a new enemy */
      return(FALSE);
    }
  else {
    /* update my position for best marksmanship */
    get_location(stat->my_location);

    /* find spot to aim at - iterate solution*/
    dx = (enemy->loc.x - (my_x = (stat->my_location)->x) + (int)((stat->time)*(enemy->xspeed)));
    dy = (enemy->loc.y - (my_y = (stat->my_location)->y) + (int)((stat->time)*(enemy->yspeed)));
    stat->time = (float)sqrt((double)(dx*dx + dy*dy))/(float)((stat->prime_weapon)->ammo_speed);
    dx = (target_spot.x = (enemy->loc.x + (int)((stat->time)*(enemy->xspeed)))) - my_x;
    dy = (target_spot.y = (enemy->loc.y + (int)((stat->time)*(enemy->yspeed)))) - my_y;

    /* check for clear path */
    target_spot.grid_x = target_spot.x / BOX_WIDTH;
    target_spot.grid_y = target_spot.y / BOX_HEIGHT;
    target_spot.box_x  = target_spot.x % BOX_WIDTH;
    target_spot.box_y  = target_spot.y % BOX_HEIGHT;

    if (! clear_path(stat->my_location, &target_spot))
      if (clear_path(stat->my_location,&(enemy->loc))) {
	aim_all_turrets(enemy->loc.x - my_x, enemy->loc.y - my_y);
	if ( ! weapon_on(stat->prime_weapon - stat->weapon) )   /* if my prime weapon is off then ...*/
	  for (i=stat->num_weapons; i--; )                      /* turn on all weapons */
	    turn_on_weapon(i);                                  /* note: I need check only the prime weapon */
	fire_all_weapons();                                     /* missiles away! */
      }
      else{
	aim_all_turrets(dx,dy);
	stat->enemy = NULL;   /* reset to move to destination */
	return(FALSE);
      }
    else {
      aim_all_turrets(dx,dy);
      if ( ! weapon_on(stat->prime_weapon - stat->weapon) )   /* if my prime weapon is off then ...*/
	for (i=stat->num_weapons; i--; )                      /* turn on all weapons */
	  turn_on_weapon(i);                                  /* note: I need check only the prime weapon */
      fire_all_weapons();                                     /* don't wait for the whites of their eyes! */
    }

    /*taunt the enemy when I pick him*/
    if (stat->old_enemy != enemy->id)
      {
	stat->old_enemy = enemy->id;
	send(stat->old_enemy, OP_TEXT, "Will you be my friend?");	   
	set_rel_drive(9.0);       /* move to stay away from enemy shots*/
      }
  }

  return(TRUE);

}

/***************************************************
* GET_REPLENISH_SIGHT: Find the nearest landmark for
* replenishment.  It is assumed the location
* of the landmark is the middle of the box in
* which it is found.  Note ths routine will not
* disturb the global destination if it is unable
* to find a good destination here.  Thus, if I am
* low on ammo and fuel and a good ammo sight is found,
* it won't be tossed because no fuel sight could be found
* even though fuel is more important and a fuel destination
* would ordinarily overwrite an ammo destination.
*/
get_replenish_sight(stat, dest_type)
struct TankStatus *stat;
LandmarkType dest_type;
{
  Landmark_info  *destination_landmark;        /* where I plan to go */
  int            my_x, my_y;                   /* my location */
  int            dx, dy;                       /* relative distance of a given landmark */
  int            range, min_range;             /* distance^2 to landmark */
  int            i;                            /* counter */

  /* get information about environment */
  get_landmarks(&(stat->num_landmarks), stat->landmark);    /* get landmark info from mapper */
  my_x = (stat->my_location)->grid_x;
  my_y = (stat->my_location)->grid_y;

  /* find location of nearest replenish place */
  destination_landmark= NULL;                  /* no suitable places available yet */
  for (i= stat->num_landmarks; i--; )      /* check each reported landmark */
    if (stat->landmark[i].type == dest_type) {
      dx = stat->landmark[i].x - my_x;
      dy = stat->landmark[i].y - my_y;
      range = dx*dx + dy*dy;
      if ((! destination_landmark) || (range < min_range)) {
	min_range = range;
	destination_landmark = &(stat->landmark[i]);
      }    
    };
  if (destination_landmark)
    stat->destination_landmark = destination_landmark;
}

/**********************************************
* CHECK_REPLENISHMENT : check whether the repairs
* have been completed.  If they have, reset the
* long-range destination to null.
*/
check_replenishment(stat)
struct TankStatus *stat;
{
  int i;       /* counting variable */

  if (! stat->destination_landmark) {                          /* check for an error; thisroutine should only */
    send (stat->my_info.id, OP_TEXT, "check_repl called w/o dest"); /* be called if destination_landmark is defined */
    return;
  }

  switch((stat->destination_landmark)->type) {
    case AMMO:
      if (weapon_ammo(stat->prime_weapon - stat->weapon) ==  (stat->prime_weapon)->max_ammo) {
	stat->destination_landmark = NULL;
	for (i=stat->num_weapons; i--; )                      /* turn on all weapons */
	  turn_on_weapon(i);
      }
      else
	if ( weapon_on(stat->prime_weapon - stat->weapon) )   /* if my prime weapon is on then ...*/
	  for (i=stat->num_weapons; i--; )                    /* turn off all weapons */
	    turn_off_weapon(i);                               /* note: I need check only the prime weapon */
      get_destination(stat);                                  /* figure out where to go next */
      break;

    case ARMOR:
      if (stat->armor_ratio == 0)
	stat->destination_landmark = NULL;
      get_destination(stat);                                  /* figure out where to go next */
      break;

    case FUEL:
      if ( (fuel() / stat->max_fuel) > .95)
	stat->destination_landmark = NULL;
      get_destination(stat);                                  /* figure out where to go next */
      break;
    }

}

/**********************************************
* MOVE_TO : move to the destination as fast as
* possible, slowing down if you get there
*/
int move_to(stat)
struct TankStatus *stat;
{
  Angle aim_angle;       /* angle to which I want to aim */
  float desired_speed;   /* speed I want to be going */
  int   dx,dy;           /* distances to destination */

/*  if ( ! clear_path(&(stat->my_location),&(stat->destination)))
    return(FALSE);       /* I can't get there from here or I'll hit a wall, crash, and burn */

  aim_angle = atan2((double)(dy = (stat->destination.y - (stat->my_location)->y)) ,
		    (double)(dx = (stat->destination.x - (stat->my_location)->x)));

  turn_vehicle(aim_angle);

  /* I want to go the maximum speed unless I am close to my destination, in which case I proportionally slow down */
  /* Note that I go fast if I want to go far OR I am going slower than I want to be */
  set_abs_drive( (desired_speed = (float)(abs(dx)+abs(dy))/SLOWDOWN_FACTOR) > stat->max_speed ?
		stat->max_speed : desired_speed );

  return(TRUE);
}

/**********************************************
* LOCAL_SCORE : determine an undesirability score
* for a grid box based on its contents
*/
int local_score(stat,grid_x,grid_y)
struct TankStatus *stat;
int grid_x, grid_y;         
{
  int    score=0;           /* undesirability score set by this square */
  int    i;               /* counter */
  int    dx,dy;           /* distance to a box */

  /*--- figure out what landmarks are here ---*/
  switch(landmark(grid_x, grid_y)) {
  case NORMAL:
    break;
  case FUEL:
    score += FUEL_BOX_SCORE * stat->fuel_ratio;
    if ((stat->destination_landmark) && ((stat->destination_landmark)->type == FUEL))
      score += DESTINATION_SCORE;
    break;
  case AMMO:
    score += AMMO_BOX_SCORE * stat->ammo_ratio;
    if ((stat->destination_landmark) && ((stat->destination_landmark)->type == AMMO))
      score += DESTINATION_SCORE;
    break;
  case ARMOR:
    score += ARMOR_BOX_SCORE * stat->armor_ratio;
    if ((stat->destination_landmark) && ((stat->destination_landmark)->type == ARMOR))
      score += DESTINATION_SCORE;
    break;
  case GOAL:
    score += MY_GOAL_SCORE; 
    break;
  case OUTPOST:
    score += OUTPOST_IN_BOX_SCORE;
    break;
  case SCROLL_N:
  case SCROLL_NE:
  case SCROLL_E:
  case SCROLL_SE:
  case SCROLL_S:
  case SCROLL_SW:
  case SCROLL_W:
  case SCROLL_NW:
    score += SCROLL_BOX_SCORE;
    break;
  case SLIP:
    score += SLIP_BOX_SCORE;
    break;
  case SLOW:
    score += SLOW_BOX_SCORE;
    break;
  }

  /*--- check the box for vehicles ---
     If the box is close, use my visual display.  If it is far, I need to rely on radar */
  dx = grid_x - (stat->my_location)->grid_x;
  dy = grid_y - (stat->my_location)->grid_y;
  if ( max(dx,dy) <= SCREEN_BOX_RADIUS )
    /* check each visible vehicle for location and team */
    for (i=0; i < stat->num_vehicles; i++)
      if ( (stat->vehicle[i].team != stat->my_info.team) &&
	  (stat->vehicle[i].loc.grid_x == grid_x) &&
	  (stat->vehicle[i].loc.grid_y == grid_y) )
	if (stat->destination_landmark)
	  score += TANK_IN_BOX_SCORE2; 
	else
	  score += TANK_IN_BOX_SCORE;

  /*--use maze information to judge box ---*/
  score += VISITED_BOX_SCORE * stat->maze_data.num_visits[grid_x][grid_y];  /* go to mazes to which I haven't been */

  return(score);
}

/*************************************************************
* DESTINATION_SCORE : calculates a score for a square based on
* its proximity to blips on the radar or replenishment sight.
* This routine is
* evaluated only once per meta-neighbor call,for the 
* primary target square.
*/
destination_score(stat,grid_x,grid_y)
struct TankStatus *stat;
int grid_x, grid_y;
{
  int score=0;      /* adjustment to score for blips */
  int dx,dy;        /* relative position of blip */
  int i;            /* counter */


  /*--- target information ---
     If I am low on supplies, the landmark destination pointer will have been defined to
     point to the appropriate destination.  That is my primary objective.  If the landmark
     is not defined, I want to hunt out enemies by chasing after blips.
     */
  /*If I have a destination landmark I'm going for, use it */
  if ( stat->destination_landmark ) {
      dx = grid_x - (stat->destination_landmark)->x;
      dy = grid_y - (stat->destination_landmark)->y;

      /*==============DEBUG=============*/
      {
	char data[128];
	sprintf(data,"dx:%d dy:%d dlx:%d dly:%d",dx,dy,(stat->destination_landmark)->x, (stat->destination_landmark)->y);
	send(stat->my_info.id, OP_TEXT, data);
      }

      score += LANDMARK_PROXIMITY_SCORE / (abs(dx) + abs(dy) + LANDMARK_ZERO_DISTANCE);
    }

  else {
    /* Check new blips */
    for (i=0; i < stat->num_blips; i++)
      {
	dx = grid_x - stat->blip[i].x - BLIP_X_OFFSET;
	dy = grid_y - stat->blip[i].y - BLIP_Y_OFFSET;
	
	score += BLIP_PROXIMITY_SCORE / (abs(dx) + abs(dy) + BLIP_ZERO_DISTANCE);
      }

    /* Check old blips, too, so I don't lose track of faded blips before I see them again */
    for (i=0; i < stat->num_old_blips; i++)
      {
	dx = grid_x - stat->old_blip[i].x - BLIP_X_OFFSET;
	dy = grid_y - stat->old_blip[i].y - BLIP_Y_OFFSET;

	score += BLIP_PROXIMITY_SCORE / (abs(dx) + abs(dy) + BLIP_ZERO_DISTANCE);
      }
  }

  /*===============DEBUG=====================*/
  {
    char data[128];
    sprintf(data,"x:%d y:%d dscore:%d",grid_x,grid_y,score);
    send(stat->my_info.id,OP_TEXT,data);
  }

  return(score);
}


/**********************************************************
* NEIGHBOR_SCORES : This routine sums the scores assigned
* to all squares adjacent to a given square if they are 
* not blocked by a wall, to double the score of the given
* square.
*/
int neighbor_scores(stat,grid_x,grid_y)
struct TankStatus *stat;
int grid_x, grid_y;
{
  int score = 3*local_score(stat,grid_x, grid_y);
  if (!wall(NORTH,grid_x,grid_y))
    score += local_score(stat,grid_x, grid_y-1);
  else
    score += WALL_SCORE;
  if (!wall(EAST,grid_x,grid_y))
    score += local_score(stat,grid_x+1, grid_y);
  else
    score += WALL_SCORE;
   if (!wall(SOUTH,grid_x,grid_y))
    score += local_score(stat,grid_x, grid_y+1);
  else
    score += WALL_SCORE;
   if (!wall(WEST,grid_x,grid_y))
    score += local_score(stat,grid_x-1, grid_y);
  else
    score += WALL_SCORE;
   
  return(score);
}

/********************************************************
* META_NEIGHBOR_SCORES : This routine adds ( with weighting)
* the scores of all neighbors and all neighbors of neighbors
* of a given square with the score of the original square
*/
int meta_neighbor_scores(stat,grid_x,grid_y)
struct TankStatus *stat;
 int grid_x, grid_y;
{
  int score= 4*( local_score(stat,grid_x,grid_y) + destination_score(stat,grid_x,grid_y));
  if (!wall(NORTH,grid_x,grid_y))
    score += neighbor_scores(stat,grid_x,grid_y-1);
  else
    score += WALL_SCORE;
  if (!wall(EAST,grid_x,grid_y))
    score += neighbor_scores(stat,grid_x+1,grid_y);
  else
    score += WALL_SCORE;
  if (!wall(SOUTH,grid_x,grid_y))
    score += neighbor_scores(stat,grid_x,grid_y+1);
  else
    score += WALL_SCORE;
  if (!wall(WEST,grid_x,grid_y))
    score += neighbor_scores(stat,grid_x-1,grid_y);
  else
    score += WALL_SCORE;

  /*===============DEBUG=====================*/
  {
    char data[128];
    sprintf(data,"x:%d y:%d score:%d",grid_x,grid_y,score);
    send(stat->my_info.id,OP_TEXT,data);
  }

  return(score);
}

/*********************************************************
* GET_DESTINATION : by determining which adjacent square
* has the lowest meta-neighbor score, this routine choses
* where the tank should go next.
*/    
get_destination(stat)
struct TankStatus *stat;
{
  int score, min_score;    /* scores */
  int dx=0;                /* displacement to destination */
  int dy=0;                /* displacement to destination */
  int grid_x, grid_y;      /* present box in maze */

  grid_x= (stat->my_location)->grid_x;
  grid_y= (stat->my_location)->grid_y;

  /* If I am at my destination landmark then stop in the center of the box */
  /* else I need to find a new destination */
  if ( ! stat->destination_landmark )
    send(stat->my_info.id,OP_TEXT,"No destination");
  else
    send(stat->my_info.id,OP_TEXT,"I have a destination");

  if ( (! stat->destination_landmark) ||
      (grid_x != (stat->destination_landmark)->x) ||
      (grid_y != (stat->destination_landmark)->y) ) { 
       
    min_score= OUTSIDE_MAZE_SCORE;

    if (! wall(NORTH,grid_x,grid_y)){
      if ( (score = meta_neighbor_scores(stat,grid_x, grid_y-1)) < min_score ) {
	dx = 0; dy = -1;
	min_score = score;
      }
    }
    if (! wall(EAST,grid_x,grid_y)){
      if ( (score = meta_neighbor_scores(stat,grid_x+1, grid_y)) < min_score ) {
	dx = 1; dy = 0;
	min_score = score;
      }
    }
    if (! wall(SOUTH,grid_x,grid_y)){
      if ( (score = meta_neighbor_scores(stat,grid_x, grid_y+1)) < min_score ) {
	dx = 0; dy = 1;
	min_score = score;
      }
    }
    if (! wall(WEST,grid_x,grid_y)){
      if ( (score = meta_neighbor_scores(stat,grid_x-1, grid_y)) < min_score ) {
	dx = -1; dy = 0;
	min_score = score;
      }
    }

 
  }
    
  (stat->destination).grid_x = grid_x + dx;
  (stat->destination).grid_y = grid_y + dy;
  (stat->destination).box_x  = MIDDLE_OF_BOX_X;
  (stat->destination).box_y  = MIDDLE_OF_BOX_Y;
  (stat->destination).x      = BOX_WIDTH  * (stat->destination).grid_x + MIDDLE_OF_BOX_X;
  (stat->destination).y      = BOX_HEIGHT * (stat->destination).grid_y + MIDDLE_OF_BOX_Y;

}





