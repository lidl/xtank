/*
** Moose.h
**
** Header file for moose.c, a robot for XTANK 0.9+ which plays ultimate
**
** ver 0.010: dachurch + dschmidt, July 13, 1988
**     moose.h comes into being with structures from old bisfree code
**
*/

/* Constants */
#define SE 0			/* Compass directions for making turns */
#define S  1
#define SW 2
#define W  3
#define NW 4
#define N  5
#define NE 6
#define E  7

#define JUST_NOODLIN  0		/* Various Modes for Ultimate: NIL mode */
#define GOT_DISC      10	/* We have the disc, look for receiver */
#define THROWIN       11	/* In the process of making a throw */
#define CUTTIN        20	/* cutting, trying to get open */
#define CHASIN	      21	/* chasing after a pass */
#define POINTIN       30	/* defense, pointing the discman */
#define COVERIN       40	/* covering open receiver */
#define CHAOS         50	/* The disc is out there, we dont know why */

#define TRUE   1		/* Just in Case */
#define FALSE  0

#define NO_CHANGE   (4*PI)	/* moose_turn returning go_ahead */

#define DISC_FRAMES 20		/* So we can chase discs */

#define NORTH_WALL (1<<0)	/* To navigate the maze */
#define WEST_WALL  (1<<1)
#define SEEN	   (1<<2)

/* Functions/Macros */
#define abs(x)        ((x)>0?(x):(-x)) /* Function macros */
#define sqr(x)        ((x)*(x))
#define min(x,y)      ((x)<(y)?(x):(y))
#define fix_angle(x)  (x -= (2*PI) * floor(x/(2*PI)))
#define atanint(y,x)  (Angle) atan2((double)(y),(double)(x))

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
} Timing;			/* Concept stolen from Mike Zehr - tada */

typedef int Maze[GRID_WIDTH][GRID_HEIGHT]; /* pretty obvious */

typedef struct {		/* Coordinate in the world */
   int x,y;
} Coord;

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

   int numbullets;		/* for passing to lib calls */
   int numvehicles;
   int numblips;
   int numdiscs;
   int numlandmarks;
   
   int discowned;		/* How many discs I got */

   int mode;			/* data about playing */
   ID locked;			/* who we're covering */
   Team us;			/* our team */
   int midstride;		/* Am I in some plan at the moment....*/
				/* t fields will be initialized if midstride */
   int tox, toy;		/* the x,y we want to be at */
   int thead;			/* The heading we want to be at */
   int tdata;			/* For holding miscellaneous data */

   Maze maze;			/* internal maze representing */

   int frame;			/* state data */
   int next_frame;
   Timing timing;
} Binfo;

extern moose_main();		/* the main routine */
/* Boolean moose_wall();		/* Is it a wall */
double moose_turn();
double moose_new_turn();

Prog_desc moose_prog = {
   "moose",
   "ultimate",
   "Moose",
   "Moose plays a hot game o' ultimate to you buddy",
   "Dan Schmidt and <slink> Doug Church <lurk lurk slink shhhh>",
   1,
   moose_main
  };				/* The data about the moose program */

