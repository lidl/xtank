/*
** Buddy.h
**
** Header file for buddy.c, a robot for XTANK 0.9+ which plays ultimate
**
** ver 0.010: dachurch + dschmidt, July 13, 1988
**     moose.h comes into being with structures from old bisfree code
** ver 0.011: dachurch, July 14, 1988
**     chaser.h formed, to split ultimate and disc-tag applications
**
*/

#define NULL 0

#define SE 0
#define S  1
#define SW 2
#define W  3
#define NW 4
#define N  5
#define NE 6
#define E  7

#define GOT_DISC      10	/* We have the disc, look for receiver */
#define CHAOS         50	/* The disc is out there, we dont know why */

#define NO_CHANGE   (4*PI)	/* moose_turn returning go_ahead */

#define DISC_FRAMES 20		/* So we can chase discs */

#define TOO_CLOSE 100            /* How close to a wall before we turn away */

#define ROD 50			/* Radius o' destruction */

#define THROW_THRESHOLD .3
#define VEHICLE_TOO_CLOSE 150

#define BUDDY_NORTH_WALL (1<<0)	/* To navigate the maze */
#define BUDDY_WEST_WALL  (1<<1)
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
  int distance;
} Goal_info;

/* And then the structure which ate new york.... */
typedef struct {
   Location loc;		/* Our location and such */
   Angle dir;
   float myspeed;
   float maxspeed;
   int xspeed, yspeed;
   Coord absloc;

   Bullet_info binfo[MAX_BULLETS]; /* The info for lib calls */
   Vehicle_info vinfo[MAX_VEHICLES];
   Blip_info blinfo[MAX_BLIPS];
   Disc_info dinfo[MAX_DISCS];
   Landmark_info linfo[MAX_LANDMARKS];
   Goal_info goalinfo[MAX_LANDMARKS];

   Vehicle_info me;

   int numbullets;		/* for passing to lib calls */
   int numvehicles;
   int numblips;
   int numdiscs;
   int numlandmarks;
   int numgoals;
   
   int lastnumlandmarks;

   int discowned;		/* How many discs I got */

   int mode;			/* data about playing */
   int midstride;		/* Am I in some plan at the moment....*/
				/* t fields will be initialized if midstride */

   Maze maze;			/* internal maze representing */

   int frame;			/* state data */
   int next_frame;
   Timing timing;
} Binfo;

extern buddy_main();		/* the main routine */
Angle buddy_turn();
Angle buddy_manuever();

Prog_desc buddy_prog = {
   "buddy",
   "Moose",
   "Buddy is for playing ultimate and being a general nuisance",
   "Dan Schmidt and <slink> Doug Church <lurk lurk slink shhhh>",
   PLAYS_ULTIMATE|DOES_EXPLORE|USES_TEAMS,
   6,
   buddy_main
  };				/* The data about the buddy program */
