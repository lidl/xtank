/*
** Roger.h
**
** Header file for roger.c, a robot for XTANK 0.9+ which plays ultimate
**
** ver 0.010: dachurch + dschmidt, July 13, 1988
**     moose.h comes into being with structures from old bisfree code
** ver 0.011: dachurch, July 14, 1988
**     roger.h formed, to split ultimate and disc-tag applications
**
*/

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

#define TRUE   1		/* Just in Case */
#define FALSE  0

#define NO_CHANGE   (4*PI)	/* moose_turn returning go_ahead */

#define DISC_FRAMES 20		/* So we can chase discs */

#define STRAIGHT_THRESHOLD .1	/* how close we need to be to straight N
				   S E or W (in radians) to be considered
				   going in that direction */

#define NORTH_WALL (1<<0)	/* To navigate the maze */
#define WEST_WALL  (1<<1)
#define SEEN	   (1<<2)

/* Functions/Macros */
#define abs(x)        ((x)>0?(x):(-x)) /* Function macros */
#define sqr(x)        ((x)*(x))
#define min(x,y)      ((x)<(y)?(x):(y))
#define max(x,y)      ((x)>(y)?(x):(y))
#define fix_angle(x)  (x -= (2*PI) * floor(x/(2*PI)))
#define sqrtint(x)    (int) sqrt((double)(x))
#define atanint(y,x)  (Angle) atan2((double)(y),(double)(x))
#define asinint(x)    (Angle) asin((double)(x))
#define acosint(x)    (Angle) acos((double)(x))
#define doturnstuff(a1,p1,pm1,a2,p2,pm2)                                   \
do {                                                                       \
  foo = (a1); bar = (a2);                                                  \
  min_ang = min(min_ang,(p1)+(pm1)*foo);                                   \
  max_ang = max(max_ang,(p2)+(pm2)*bar);                                   \
} while (0)

#define get_gx        (bi->loc.grid_x)
#define get_gy        (bi->loc.grid_y)

#define get_bx        (bi->loc.box_x)
#define get_by        (bi->loc.box_y)

/* Structs */
typedef struct {		/* Will time various world checks */
   int bullet_check;
   int vehicle_check;
   int wall_check;
   int disc_check;
} Timing;			/* Concept stolen from Mike Zehr */

typedef int Maze[GRID_WIDTH][GRID_HEIGHT]; /* pretty obvious */

typedef struct {		/* Coordinate in the world */
   int x,y;
} Coord;

/* And then the structure which ate new york.... */
typedef struct {
   Location loc;		/* Our location and such */
   Angle dir;			/* Our heading */
   Angle desired_dir;		/* What we wish our heading was */
   float myspeed;
   float maxspeed;
   int xspeed, yspeed;
   Coord absloc;

   Bullet_info binfo[MAX_BULLETS]; /* The info for lib calls */
   Vehicle_info vinfo[MAX_VEHICLES];
   Blip_info blinfo[MAX_BLIPS];
   Disc_info dinfo[MAX_DISCS];
   Landmark_info linfo[MAX_LANDMARKS];

   int numbullets;		/* for passing to lib calls */
   int numvehicles;
   int numblips;
   int numdiscs;
   int numlandmarks;
   
   int discowned;		/* How many discs I got */

   int mode;			/* data about playing */
   int midstride;		/* Am I in some plan at the moment....*/
				/* t fields will be initialized if midstride */

   Maze maze;			/* internal maze representing */

   int frame;			/* state data */
   int next_frame;
   Timing timing;
} Binfo;

extern roger_main();		/* the main routine */
Angle roger_turn();

Prog_desc roger_prog = {
   "roger",
   "ultimate",
   "Moose",
   "Roger is for chasing discs around the board and keeping them from you",
   "Dan Schmidt and <slink> Doug Church <lurk lurk slink shhhh>",
   5,
   roger_main
  };				/* The data about the roger program */
