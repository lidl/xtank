/*
** Pryn.h
**
** Header file for pryn.c, a robot for XTANK 0.9+ which plays ultimate
**
** ver 0.010: dachurch + dschmidt, July 13, 1988
**     moose.h comes into being with structures from old bisfree code
** ver 0.011: dachurch, July 14, 1988
**     chaser.h formed, to split ultimate and disc-tag applications
**
*/

#define NULL 0

/* Used in pryn_turn_carefully */

#define SE 0
#define S  1
#define SW 2
#define W  3
#define NW 4
#define N  5
#define NE 6
#define E  7

#define GOT_DISC      10	/* We have the disc, look for receiver */
#define CHAOS         50	/* The disc is out there, we don't know why */

#define NO_CHANGE   (4*PI)	/* moose_turn returning go_ahead */

#define DISC_FRAMES 20		/* How many frames of lookahead when chasing
                                   a disc */

#define TOO_CLOSE 8            /* How close to a wall before we turn away */

#define THROW_THRESHOLD .8
#define VEHICLE_TOO_CLOSE 150

#define NORTH_WALL (1<<0)	/* To navigate the maze */
#define WEST_WALL  (1<<1)
#define SEEN	   (1<<2)

/* Functions/Macros */
#define abs(x)        ((x)>0?(x):(-x)) /* Function macros */
#define sqr(x)        ((x)*(x))
#define sqrtint(x)    (int) (sqrt((double)(x)))
#define min(x,y)      ((x)<(y)?(x):(y))
#define max(x,y)      ((x)>(y)?(x):(y))
#define fix_angle(x)  (x -= (2*PI) * floor(x/(2*PI)))
#define atanint(y,x)  (Angle) atan2((double)(y),(double)(x))
#define asinint(x)    (Angle) asin((double)(x))
#define acosint(x)    (Angle) acos((double)(x))
/* Don't ask */
#define doturnstuff(a1,p1,pm1,a2,p2,pm2)                                   \
do {                                                                       \
  foo = (a1); bar = (a2);                                                  \
  min_ang = min(min_ang,(p1)+(pm1)*foo);                                   \
  max_ang = max(max_ang,(p2)+(pm2)*bar);                                   \
} while (0)

/* Structs */
typedef struct {		/* Will time various world checks */
   int bullet_check;
   int vehicle_check;
   int wall_check;
   int disc_check;
} Timing;			/* Concept stolen from Mike Zehr - tada */

typedef int Maze[GRID_WIDTH][GRID_HEIGHT]; /* pretty obvious */

typedef struct {		/* Coordinate in the world */
   int x,y;
} Coord;

typedef struct {
  int x,y;
  Team team;
  Boolean enemy;
  int distance;
} Goal_info;

/* And then the structure which ate new york.... */
typedef struct {
   Angle desireddir;
   float myspeed;
   float maxspeed;
   int xspeed, yspeed;

   Bullet_info binfo[MAX_BULLETS]; /* The info for lib calls */
   Vehicle_info vinfo[MAX_VEHICLES];
   Blip_info blinfo[MAX_BLIPS];
   Disc_info dinfo[MAX_DISCS];
   Landmark_info linfo[MAX_LANDMARKS];
   Goal_info ginfo[MAX_LANDMARKS];
   Settings_info settings;

   Vehicle_info me;

   int cur_tc;			/* what too_close is at this speed */
  
   int numbullets;		/* for passing to lib calls */
   int numvehicles;
   int numblips;
   int numdiscs;
   int numlandmarks;
   int numgoals;

   int lastmessages;
   
   int lastnumlandmarks;

   int discowned;		/* How many discs I got */

   int mode;			/* data about playing */
   Boolean midstride;		/* Am I in some plan at the moment....*/
				/* t fields will be initialized if midstride */

   int menon;			/* how many enemies near me */
   Boolean knowmenon;		/* do I know how many enemies near me? */

   Maze maze;			/* internal maze representing */

   int frame;			/* state data */
   int next_frame;
   Timing timing;
} Binfo;

extern pryn_main();		/* the main routine */
Angle pryn_turn();
Angle pryn_turn_carefully();

Prog_desc pryn_prog = {
   "pryn",
   "ultimate",
   "Ultimate7",
   "Pryn is for playing ultimate and being a general nuisance",
   "Dan Schmidt and <slink> Doug Church <lurk lurk slink shhhh>",
   6,
   pryn_main
  };				/* The data about the pryn program */
