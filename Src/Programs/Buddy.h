/*
** Buddy.h
**
** Header file for Buddy.c, an ultimate robot
**
** This file is Copyright 1990 by Daniel Schmidt; All rights reserved.
**
** You are free to modify this file as long as you make clear
** what code is yours and what is mine.
**          
** Previous Log:  Buddy.h,v
 * Revision 1.4  90/08/19  15:17:35  dschmidt
 * Better throwing code, including throwing in a range.
 * Throw at the whole side of a goal, rather than a point on that side.
 * Clear the disc.
 * Run after discs at full speed.
 * Keep track of disc spin internally.
 * 
 * Revision 1.3  90/08/17  20:28:15  dschmidt
 * Only throw to a teammate if he's closer to the goal than I am
 * 
 * Revision 1.2  90/08/15  21:24:45  dschmidt
 * Rip out Buddy_move_carefully and use Buddy_wander instead.
 * 
 * Revision 1.1  90/08/15  19:31:52  dschmidt
 * Initial revision
 * 
**
*/

/*
$Author: lidl $
$Id: Buddy.h,v 2.5 1992/09/06 23:45:12 lidl Exp $

$Log: Buddy.h,v $
 * Revision 2.5  1992/09/06  23:45:12  lidl
 * made to compile with gcc -ansi (blah) on a vax running ultrix 4.2
 *
 * Revision 2.4  1991/09/19  05:26:04  lidl
 * fixed a small __STDC__/__GNUC__ difference in opinions
 *
 * Revision 2.3  1991/02/10  13:49:55  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:05  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:10:32  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:08:48  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:01:39  aahz
 * Initial revision
 * 
*/

#ifdef __GNUC__
# ifdef __STDC__
#  define Inline __inline
# else
#  define Inline inline
# endif
#else
#define Inline
#endif

/* States: currently only two */
#define GOT_DISC      0 	/* I have the disc */
#define CHAOS         1 	/* I don't have the disc */

#define DISC_FRAMES 20		/* How many frames of lookahead when chasing
                                   a disc */

#define THROW_THRESH .2         /* A magic number which determines how near
                                   the disc has to be to the "perfect"
                                   angle at which we'd like to throw it */

#define VEHICLE_TOO_CLOSE 150   /* To know when to play keepaway */

#define MAX_DISC_SPEED 25.0	/* Maximum speed at which I can throw
				   a disc */

#define DISC_CLOSE_SQ 800	/* If disc is this close to me, it's too close
                                   to be in orbit */
#define DISC_ORBIT_SQ 1600.0

#define HALF_BOX_WIDTH (BOX_WIDTH/2)
#define HALF_BOX_HEIGHT (BOX_HEIGHT/2)

#define MAX_FILL_DEPTH 12	/* Default depth for shortest-path searches */

/* Functions/Macros */
#define sqr(x)        ((x)*(x))
#define sqrtint(x)    (int) (sqrt((double)(x)))
#define fix_angle(x)  (x -= (2*PI) * floor(x/(2*PI)))
#define asinint(x)    (Angle) asin((double)(x))
#define acosint(x)    (Angle) acos((double)(x))

#define Buddy_spin_discs(x) (spin_discs(x), bi->spin = (x))
#define Buddy_wall_east(bi,x,y) map_west_result(((bi)->map)[(x)+1][y].flags)
#define Buddy_wall_west(bi,x,y) map_west_result(((bi)->map)[x][y].flags)
#define Buddy_wall_north(bi,x,y) map_north_result(((bi)->map)[x][y].flags)
#define Buddy_wall_south(bi,x,y) map_north_result(((bi)->map)[x][(y)+1].flags)

/* Structs */

typedef struct {		/* Information about goals */
  Location loc;			/* Location of goal  */
  Team team;			/* Team the goal is on */
} Goal_info;

typedef struct {		/* Info for calculating shortest paths */
  char nextx,nexty;		/* The next box I should travel to */
  Boolean seen;			/* If I know how to get from here to my
				   destination */
  char dist;			/* Distance from here to the destination */
} Mazebox;

/* Binfo contains all data which would otherwise be global.  We need
   to keep it in one structure and pass it around from function to
   function so that multiple copies of Buddy don't use the same data. */

typedef struct {
   Angle desireddir;		/* what angle I'd like to go at */
   float maxspeed;		/* max possible speed */

   /* Here I store everything I know about the world */
   Vehicle_info vinfo[MAX_VEHICLES];
   Blip_info blinfo[MAX_BLIPS];
   Disc_info dinfo[MAX_DISCS];
   Goal_info ginfo[MAX_LANDMARKS];
   Landmark_info linfo[MAX_LANDMARKS];
   Settings_info settings;

   int numvehicles;
   int numblips;
   int numdiscs;
   int numgoals;

   Message msg;			/* Last received message */

   /* Map containing information about walls and landmarks */
   Box (*map)[GRID_HEIGHT];

   /* Maps containing information about how to get from one place
      to another.  `maze' is for calculating paths to the enemy goal;
      `scrapmaze' is for calculating all other paths. */
   Mazebox maze[GRID_WIDTH][GRID_HEIGHT];
   Mazebox scrapmaze[GRID_WIDTH][GRID_HEIGHT];

   /* Global space for arrays used in Buddy_compute_path */
   Coord cutting_edge[100], cutting_edge2[100];

   /* The last destination I tried to find a path to */
   Coord lastfillorigin;

   /* Array of locations I should throw the disc at when I
      want to throw it in a goal. */
   int num_goal_locs;
   Location goal_locs_1[4];
   Location goal_locs_2[4];

   Vehicle_info me;		/* What I am is what I am */

   int frame;			/* Current frame of the game */
   int next_frame;		/* Frame after this */

   int discowned;		/* How many discs I own */
   Spin spin;			/* What way I'm spinning discs */
   int mode;			/* What state I'm in */
   int men_on;			/* Enemy vehicles near me */
   int closestgoal;		/* Closest goal to me */
   int enemy_goal;		/* Number of enemy goal */
   Boolean throwing;		/* If I'm in the middle of throwing,
                                   don't be distracted */
} Binfo;

static void main();		/* the main routine */
static Buddy_sulk();
static Buddy_deal_with_messages();
static Buddy_find_goals();
static Buddy_set_open_goal_locs();
static Buddy_panic();
static Buddy_deal_with_disc();
static Buddy_throw_at_goal();
static Buddy_go_to_goal();
static Buddy_maybe_pass();
static Buddy_play_keep_away();
static Buddy_throw_at_loc();
static Buddy_throw_in_range();
static Buddy_find_the_action();
static Buddy_move_into_box();
static Buddy_go_get_the_disc();
static Buddy_go_to_box();
static Buddy_compute_path_to_square();
static Buddy_compute_path();
static Buddy_wander();
static Buddy_clear_the_disc();
static Buddy_get_clear_range();

Prog_desc Buddy_prog = {
   "Buddy",
   "Ultimate7",
   "Buddy is for playing ultimate and being a general nuisance. \
Use him only in mazes where there is only one enemy goal. \
Make sure to set Full Map on in the Settings menu.\
Give him a tank with a mapper and radar.  I recommend Ultimate7.",
   "Dan Schmidt",
   PLAYS_ULTIMATE | USES_TEAMS | USES_MESSAGES,
   8,
   main
  };				/* The data about the Buddy program */
