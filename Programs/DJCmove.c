/********************************************************
* DJCmove.c - a program which moves around in search of
*  ?
******************************************************/

#include <math.h>
#include "xtanklibnew.h"
/* #include "/mit/games/src/vax/xtank/Src/xtanklib.h" */

/* some basic constants to be defined */
#define NULL    0

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

/* scores for each landmark type */
#define OUTSIDE_MAZE_SCORE    10000      /* you don't want to go outside the maze */
#define OUTPOST_IN_BOX_SCORE    200      /* avoid outposts */
#define TANK_IN_BOX_SCORE       100      /* per enemy tank*/
#define VISITED_BOX_SCORE        75      /* per visit to the box */
#define SCROLL_BOX_SCORE         20      /* arrows are a pain */
#define SLOW_BOX_SCORE           10      /* slow boxes are a waste of time */
#define SLIP_BOX_SCORE            5      /* avoid oil slick boxes */
#define WALL_SCORE               10      /* discourage robot from bumping into walls */

/* parameters to control tank */
#define SLOWDOWN_FACTOR        (BOX_WIDTH/20)   /* slow down 20 speed units per box distance from destination */

/* define simple "functions" */
#define max(a,b)   ( ((a) > (b)) ? (a) : (b) )
#define min(a,b)   ( ((a) < (b)) ? (a) : (b) )
#define abs(a)     ( ((a) > 0) ? (a) : -(a) )

extern int DJCmove_main();

Prog_desc DJCmove_prog =
{
  "DJCmove",     /* program name */
  "Vanguard",     /* default tank */
  "martyrdom",    /* strategy */
  "Dan Connelly", /* author */
  USES_MESSAGES|PLAYS_COMBAT|DOES_EXPLORE,  /* skills */
  3,              /* skill level */
  DJCmove_main   /* main procedure */
};

static struct MazeData {
  int            num_visits[GRID_WIDTH][GRID_HEIGHT];    /* number of times I have visited a box */
};

static struct TankStatus {
  Vehicle_info   my_info;                           /* vehicle information about me */
  Location       *my_location;                      /* vehicle location ( points into my_info )*/
  float          max_speed;                         /* maximum speed my vehicle is capable of */
  int            num_weapons;                       /* number of weapons on my tank */
  Weapon_info    weapon[MAX_WEAPONS];               /* info about all of my weapons */
  int            num_vehicles;                      /* number of visible vehicles */     
  Vehicle_info   vehicle[MAX_VEHICLES];             /* info about other vehicles */
  int            num_blips;                         /* number of blips on radar screen */
  Blip_info      blip[MAX_BLIPS];                   /* information about blips on screen */
  struct MazeData maze_data;                        /* information I've picked up about the maze */
  Location       destination;                       /* where I want to go */
};

DJCmove_main()
{
  struct TankStatus status;                    /* "global" information about this tank */
  int i;                                       /* counter */
  int old_grid_x = -1;                         /* keep track of which grid I was in last time */
  int old_grid_y = -1;                         /* note initialization to automatically record initial box data */

  get_self(&status.my_info);                    /* determine my stats */
  status.my_location = &status.my_info.loc;     /* set my location pointer */

  status.num_weapons = num_weapons() ;           /* number of weapons on my tank */
  status.num_weapons = min(status.num_weapons, MAX_WEAPONS);   /* limit to array size */

  status.max_speed = max_speed();

  for (i=0; i < status.num_weapons; i++)
    get_weapon(i,&status.weapon[i]);

  while(1) {                                     /* loop till I drop */
    /* get needed data of surroundings */
    get_vehicles(&status.num_vehicles, status.vehicle);
    get_blips(&status.num_blips, status.blip);
    get_self(&status.my_info);

    /* determine destination */
    get_destination(&status);

    {
      char    data[128];
      sprintf(data,"%d,%d : %d,%d: %d %d",status.destination.grid_x, status.destination.grid_y,
	      status.destination.box_x, status.destination.box_y, status.destination.x, status.destination.y);
      send (RECIPIENT_ALL,OP_TEXT,data);
    }

    /* move to destination */
    move_to(&status);       /* ignore the possibility of failure due to line-of-sight blockage for now */

    /* update information about number of times I have been here*/
    if (((status.my_location)->grid_x != old_grid_x) ||
        ((status.my_location)->grid_y != old_grid_y) )
      status.maze_data.num_visits[old_grid_x = (status.my_location)->grid_x][old_grid_y = (status.my_location)->grid_y]++;

    done();

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
  set_abs_drive( (desired_speed = (float)(abs(dx)+abs(dy))/SLOWDOWN_FACTOR - speed()) > stat->max_speed ?
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
  int    score=0;         /* undesirability score set by this square */
  int    i;               /* counter */
  int    dx,dy;           /* distance to a box */

  /*--- figure out what landmarks are here ---*/
  switch(landmark(grid_x, grid_y)) {
  case NORMAL:
    break;
  case FUEL:
    break;
  case AMMO:
    break;
  case ARMOR:
    break;
  case GOAL:
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
      score += TANK_IN_BOX_SCORE * ( (stat->vehicle[i].team != stat->my_info.team) &&
				    (stat->vehicle[i].loc.grid_x == grid_x) &&
				    (stat->vehicle[i].loc.grid_y == grid_y) );
  else {
    /* check each blip on the screen for location only ( team not available ) */
    for (i=0; i < stat->num_blips; i++) 
      score += TANK_IN_BOX_SCORE * ( (stat->blip[i].x == grid_x) &&
				    (stat->blip[i].y == grid_x) );
  }


  /*--use maze information to judge box ---*/
  score += VISITED_BOX_SCORE * stat->maze_data.num_visits[grid_x][grid_y];  /* go to mazes to which I haven't been */

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
  int score = 4*local_score(stat,grid_x,grid_y);
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

  (stat->destination).grid_x = grid_x + dx;
  (stat->destination).grid_y = grid_y + dy;
  (stat->destination).box_x  = MIDDLE_OF_BOX_X;
  (stat->destination).box_y  = MIDDLE_OF_BOX_Y;
  (stat->destination).x      = BOX_WIDTH  * (stat->destination).grid_x + MIDDLE_OF_BOX_X;
  (stat->destination).y      = BOX_HEIGHT * (stat->destination).grid_y + MIDDLE_OF_BOX_Y;

}
