/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** xtanklib.h
*/

#ifndef LIB
/* Useful constants */
#define PI	3.1415926535

/* Return this if a function is passed bogus data */
#define BAD_VALUE       (-1)

/* Maximum maze coordinates */
#define GRID_WIDTH	30
#define GRID_HEIGHT	30

/* Size of maze boxes and screen */
#define BOX_WIDTH	192
#define BOX_HEIGHT	192
#define SCREEN_WIDTH	768
#define SCREEN_HEIGHT	768

/*
** Maximum values for arrays of landmarks, blips, vehicles, and bullets
** These should be used as the sizes of the landmark_info, blip_info,
** vehicle_info, and bullet_info arrays.
*/
#define MAX_BLIPS	22
#define MAX_VEHICLES	16
#define MAX_BULLETS	300
#define MAX_DISCS       16
#define MAX_LANDMARKS   50

/* Locations for weapon mounts */
#define TURRET1		0
#define TURRET2		1
#define TURRET3		2

/* What fire_weapon() returns if something went wrong */
#define FIRED           0
#define RELOADING       1
#define NO_AMMO         2
#define WEAPON_OFF      3
#define TOO_HOT         4

/* Types of weapons */
#define LMG             0
#define MG              1
#define HMG             2
#define LRIFLE          3
#define RIFLE           4
#define HRIFLE          5
#define LCANNON         6
#define CANNON          7
#define HCANNON         8
#define LROCKET         9
#define ROCKET         10
#define HROCKET        11
#define ACID           12
#define FLAME          13
#define MINE           14
#define SEEKER         15
#define SLICK          16

/* Locations for weapon mounts */
#define MOUNT_TURRET1		0
#define MOUNT_TURRET2		1
#define MOUNT_TURRET3		2
#define MOUNT_FRONT		3
#define MOUNT_BACK		4
#define MOUNT_LEFT		5
#define MOUNT_RIGHT		6

/* Sides for the armor function */
#define FRONT	0
#define BACK	1
#define	LEFT	2
#define	RIGHT	3
#define TOP	4
#define	BOTTOM	5

/* Message opcodes */
#define OP_LOCATION	0
#define OP_GOTO		1
#define OP_FOLLOW	2
#define OP_HELP		3
#define OP_ATTACK	4
#define OP_OPEN		5
#define OP_THROW	6
#define OP_CAUGHT	7
#define OP_ACK		8
#define OP_TEXT	        9
#define MAX_OPCODES	10

/* Directions to spin discs */
#define COUNTERCLOCKWISE	(-1)
#define CLOCKWISE		1
#define TOGGLE			2

/* Directions for the wall function */
#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3

/* Flags for boxes in a maze description */
#define INSIDE_MAZE	(1<<0)
#define NORTH_WALL	(1<<1)
#define WEST_WALL	(1<<2)
#define NORTH_DEST	(1<<3)
#define WEST_DEST	(1<<4)

/* Types of boxes */  
#define NORMAL		0
#define FUEL		1
#define AMMO		2
#define ARMOR		3
#define GOAL		4
#define OUTPOST		5
#define SCROLL_N	6
#define SCROLL_NE	7
#define SCROLL_E	8
#define SCROLL_SE	9
#define SCROLL_S	10
#define SCROLL_SW	11
#define SCROLL_W	12
#define SCROLL_NW	13
#define SLIP		14
#define SLOW		15

/* Types of specials */
#define CONSOLE		0
#define MAPPER		1
#define RADAR		2

/* Maximum amount of data that can fit in a message */
#define MAX_DATA_LEN    31

/* Recipient for messages sent to every vehicle */
#define RECIPIENT_ALL 255

/* Sender for messages sent by the game commentator */
#define SENDER_COM 255

/* Sender for initial set of messages on before slots have been filled */
#define SENDER_NONE 254

/* Owner of disc if it is not orbiting anyone */
#define NO_OWNER        255

/* The neutral team */
#define NEUTRAL 0

/* Boolean values */
#define FALSE 0
#define TRUE  1

/* Program ability flags */
#define PLAYS_COMBAT		(1<<0)
#define PLAYS_WAR		(1<<1)
#define PLAYS_ULTIMATE		(1<<2)
#define PLAYS_CAPTURE		(1<<3)
#define PLAYS_RACE		(1<<4)
#define DOES_SHOOT		(1<<5)
#define DOES_EXPLORE		(1<<6)
#define DOES_DODGE		(1<<7)
#define DOES_REPLENISH		(1<<8)
#define USES_TEAMS		(1<<9)
#define USES_MINES		(1<<10)
#define USES_SLICKS		(1<<11)
#define USES_SIDE_MOUNTS	(1<<12)
#define USES_MESSAGES    	(1<<13)

typedef struct {
  char *name;			/* name of program */
  char *vehicle;		/* name of default vehicle */
  char *strategy;		/* description of strategy */
  char *author;			/* name of author */
  unsigned int abilities;	/* things the program does */
  int skill;			/* skill at doing these things */
  int (*func)();		/* main procedure of program */
  char *code;			/* pointer to code memory */
} Prog_desc;

typedef struct {
  unsigned int flags;		/* bits for walls, inside maze */
  char type;			/* landmark, scroll, goal, outpost, etc. */
  char team;			/* number of team that owns the box */
  char strength;		/* strength of the scroll, outpost, etc. */
} Box;

typedef Box Map[GRID_WIDTH][GRID_HEIGHT];

typedef unsigned char Boolean;
#endif

typedef unsigned char ID;
typedef unsigned char Team;
typedef unsigned char Game;
typedef float Angle;
typedef int WeaponNum;
typedef int MountLocation;
typedef int SpecialType;
typedef int WallSide;
typedef int Side;
typedef int DiscSpin;
typedef int LandmarkType;
typedef int Opcode;
typedef int TurretNum;
typedef int WeaponError;
typedef int WeaponType;

#ifndef LIB
typedef struct {
  ID sender;			/* vehicle number of sender */
  Team sender_team;		/* team number of sender */
  ID recipient;			/* vehicle number of recipient */
  unsigned char opcode;			/* type of message */
  unsigned char data[MAX_DATA_LEN];	/* data of message */
} Message;
#endif

typedef struct {
  int grid_x;
  int grid_y;
  int box_x;
  int box_y;
  int x;
  int y;
} Location;
 
typedef struct {
  int type;
  int team;
  int x;
  int y;
} Landmark_info;

typedef struct {
  int x;
  int y;
} Blip_info;

typedef struct {
  int type;
  MountLocation mount;
  int damage;
  int heat;
  int range;
  int reload;
  int max_ammo;
  int ammo_speed;
} Weapon_info;

typedef struct {
  Location loc;
  float xspeed;
  float yspeed;
  float heading;
  ID id;
  Team team;
  int body;
} Vehicle_info;

typedef struct {
  Location loc;
  float xspeed;
  float yspeed;
  WeaponType type;
} Bullet_info;

typedef struct {
  Location loc;
  float xspeed;
  float yspeed;
  ID owner;
  Angle angle;
  DiscSpin spin;
} Disc_info;

typedef struct {
  Boolean ricochet;
  Boolean no_wear;
  Boolean full_map;
  Game game;
  int winning_score;
  int outpost_strength;
  float scroll_speed;
  float box_slowdown;
  float disc_friction;
  float owner_slowdown;
} Settings_info;

Angle heading(),turn_rate(),turret_angle(),turret_turn_rate();
float max_speed(),speed(),acc(),fuel(),max_fuel();
Box **map_get();

/* Map macros for fast map information */
#define MAP_NONE 0
#define MAP_WALL 1
#define MAP_DEST 2

#define map_off_grid(x,y) \
  ((x) < 0 || (x) >= GRID_WIDTH-1 || (y) < 0 || (y) >= GRID_HEIGHT-1)

#define map_north_result(fl) \
  (((fl)&NORTH_WALL) ? (((fl)&NORTH_DEST) ? MAP_DEST : MAP_WALL) : MAP_NONE)

#define map_west_result(fl) \
  (((fl)&WEST_WALL) ? (((fl)&WEST_DEST) ? MAP_DEST : MAP_WALL) : MAP_NONE)

#define map_wall(map,dir,x,y) \
  (map_off_grid(x,y) ? MAP_NONE : \
   (((dir) == NORTH) ? (map_north_result((map)[x][y].flags)) : \
    (((dir) == EAST) ? (map_west_result((map)[(x)+1][y].flags)) : \
     (((dir) == SOUTH) ? (map_north_result((map)[x][(y)+1].flags)) : \
      (((dir) == WEST) ? (map_west_result((map)[x][y].flags)))))))

#define map_landmark(map,x,y) (map_off_grid(x,y) ? 0 : ((map)[x][y].type))
#define map_team(map,x,y)     (map_off_grid(x,y) ? 0 : ((map)[x][y].team))
#define map_strength(map,x,y) (map_off_grid(x,y) ? 0 : ((map)[x][y].strength))
