/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** xtanklib.h
*/

#ifndef _XTANKLIB_H_		/* prevent multiple #inclusions */
#define _XTANKLIB_H_


#include "sysdep.h"
#include "map.h"
#include "screen.h"
#include "common.h"
#include "message.h"


typedef unsigned char ID;	/* vehicle identification */
typedef unsigned char Team;	/* team number */
typedef float Angle;
typedef int WeaponNum;		/* index of one of the weapon instances on this
				   tank, in the range [0, MAX_WEAPONS-1].  Not
				   to be confused with WeaponType. */

/* the different games that can be played */
typedef enum {
    COMBAT_GAME, WAR_GAME, ULTIMATE_GAME, CAPTURE_GAME, RACE_GAME
} Game;

/* the different types of terrain boxes can have */
typedef enum {
    BAD_MAPPER = -1,		/* you don't have a mapper! */
    NORMAL = 0,			/* nothing unusual */
    FUEL,			/* fuel depot */
    AMMO,			/* ammunition depot */
    ARMOR,			/* armor depot */
    GOAL,			/* goal for race and ultimate games */
    OUTPOST,			/* outpost shoots at you */
    PEACE,
    SCROLL_N,			/* north scrolling */
    SCROLL_NE,			/* northeast scrolling */
    SCROLL_E,			/* east scrolling */
    SCROLL_SE,			/* southeast scrolling */
    SCROLL_S,			/* south scrolling */
    SCROLL_SW,			/* southwest scrolling */
    SCROLL_W,			/* west scrolling */
    SCROLL_NW,			/* northwest scrolling */
    SLIP,			/* slippery */
    SLOW,			/* slowdown */
    START_POS,			/* vehicle starting position */
    /* wall symbols, used inside Xtank only */
    NORTH_SYM,			/* indestructible */
    WEST_SYM,
    NORTH_DEST_SYM,		/* destructible */
    WEST_DEST_SYM
} LandmarkType;
#define NUM_LANDMARK_TYPES 18	/* how many of them there are */

/* the different sides a tank has (places armor can be put) */
typedef enum {
    FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM
} Side;
#define MAX_SIDES 6		/* how many of them there are (not including
				   the _SYM ones) */

/* the different turrets */
typedef enum {
    TURRET1, TURRET2, TURRET3
} TurretNum;

/* the different weapon mounting locations on a vehicle */
typedef enum {
    /* keep these turrets consistent with TurretNum */
    MOUNT_TURRET1 = TURRET1,
    MOUNT_TURRET2 = TURRET2,
    MOUNT_TURRET3 = TURRET3,
    MOUNT_FRONT, MOUNT_BACK, MOUNT_LEFT, MOUNT_RIGHT
} MountLocation;
#define NUM_MOUNTS 7		/* how many weapon mounts there are */

/* the different types of specials */
typedef enum {
    /* these should be in the same order as the initialization data in
       vdesign.h */
    CONSOLE, MAPPER, RADAR, REPAIR, RAMPLATE, DEEPRADAR, STEALTH, NAVIGATION
} SpecialType;

/* the different types of weapons */
typedef enum {
    /* these should be in the same order as the initialization data in
       vdesign.h */
    LMG,			/* light machine gun */
    MG,				/* machine gun */
    HMG,			/* heavy machine gun */
    LRIFLE,			/* light pulse rifle */
    RIFLE,			/* pulse rifle */
    HRIFLE,			/* heavy pulse rifle */
    LCANNON,			/* light autocannon */
    CANNON,			/* autocannon */
    HCANNON,			/* heavy autocannon */
    LROCKET,			/* light rocket launcher */
    ROCKET,			/* rocket launcher */
    HROCKET,			/* heavy rocket launcher */
    ACID,			/* acid sprayer */
    FLAME,			/* flame thrower */
    MINE,			/* mine layer */
    SEEKER,			/* heat seeker */
    SLICK,			/* oil slick */
    DISC			/* disk shooter */
} WeaponType;
#define VMAX_WEAPONS	18	/* how many weapon types there are */

/* the different conditions a weapon can be in (this should really be a set of
   bits so they can be combined...) */
typedef enum {
    BAD_WEAPON_NUM = -1,	/* there is no weapon with that number */
    FIRED = 0,			/* no problem, fired successfully */
    RELOADING,			/* was fired too recently (firing rate) */
    NO_AMMO,			/* no ammunition left */
    WEAPON_OFF,			/* has been turned off */
    TOO_HOT			/* vehicle is too hot */
} WeaponStatus;

/* the different directions discs can be spun */
typedef enum {
    COUNTERCLOCKWISE = -1, NO_SPIN = 0, CLOCKWISE = 1, TOGGLE = 2
} Spin;

/* the different walls that a box can have */
typedef enum {
    NO_DIR = -1, NORTH, EAST, SOUTH, WEST
} WallSide;

/* the different kinds of walls */
typedef enum {
    MAP_NONE = 0,		/* no wall at all */
    MAP_WALL,			/* normal (indestructible) wall */
    MAP_DEST			/* destructible wall */
} WallType;

/* game settings that are available to robot players */
typedef struct {
    Game game;			/* combat, war, ultimate, capture, race */
    Boolean ricochet;		/* whether bullets bounce off walls */
    Boolean rel_shoot;		/* whether shooter's speed added to bullet's */
    Boolean no_wear;		/* whether vehicles take damage & lose fuel */
    Boolean restart;		/* whether vehicles restart after death */
    Boolean full_map;		/* whether vehicles start out with full map */
    Boolean pay_to_play;	/* whether vehicles have to "pay to play" */
	int shocker_walls;      /* whether walls cause shocks(extra damage) */
    int winning_score;		/* score needed to win the game */
    int takeover_time;		/* how long you have to be in a square in order
				   to capture it (in War mode) */
    int outpost_strength;	/* firepower of outposts (0 - 10) */
    float scroll_speed;		/* speed of scroll boxes (0 - 10) */
    float slip_friction;	/* friction in slip boxes (0 - 1) */
    float normal_friction;	/* friction in all other boxes (0 - 1) */
    float disc_friction;	/* friction factor applied to disc (0 - 1) */
    float box_slowdown;		/* slowdown caused by slow boxes (0 - 1) */
    float owner_slowdown;	/* how much to slow down disc owner (0 - 1) */
} Settings_info;

typedef struct {
    WeaponType type;
    MountLocation mount;
    int	  damage;
    int	  heat;
    int	  range;
    int	  reload;
    int	  max_ammo;
    int	  ammo_speed;
	int   frames;
} Weapon_info;

typedef struct {
    int	  x;
    int	  y;
} Blip_info;

typedef struct {
    int   grid_x;
    int   grid_y;
    int   box_x;
    int   box_y;
    int   x;
    int   y;
} Location;

typedef struct {
    LandmarkType type;
    Team team;
    int   x;
    int   y;
} Landmark_info;

typedef struct {
    Flag  flags;		/* bits for walls, inside maze */
    LandmarkType  type;		/* landmark, scroll, goal, outpost, etc. */
    Team  team;			/* number of team that owns the box */
    Byte  strength;		/* strength of the scroll, outpost, etc. */
    void  *user_data;		/* robot programs can do whatever they want
				   with this */
} Box;

typedef Box Map[GRID_WIDTH][GRID_HEIGHT];

typedef struct {
    Location loc;		/* where it is */
    float xspeed, yspeed;	/* components of its velocity */
    Angle heading;		/* direction body is pointing */
    ID	  id;			/* unique identification number */
    Team  team;			/* team number */
    int	  body;			/* body number */
    int   num_turrets;		/* number of turrets on the body */
    Angle turret_angle[MAX_TURRETS];	/* direction each turret is pointing */
} Vehicle_info;

typedef struct {
    Location loc;
    float xspeed;
    float yspeed;
    WeaponType type;
    int id;			/* a unique identification number */
} Bullet_info;

typedef struct {
    Location loc;
    float xspeed;
    float yspeed;
    ID    owner;
    Angle angle;
    Spin spin;
} Disc_info;

typedef struct {
    char *name;	/* name of program */
    char *vehicle;	/* name of default vehicle */
    char *strategy;	/* description of strategy */
    char *author;	/* name of author */
    unsigned int abilities;	/* things the program does */
    int	  skill;		/* skill at doing these things (0-10) */
    int   (*func) ();	/* main procedure of program */
    char *code;	/* pointer to code memory */
} Prog_desc;

#define TICKSZ		2	/* number of frames that go by before a robot
				   program gets control again (unless they ues
				   too much time, of course) */

#define FUEL_CONSUME	0.001	/* how fast fuel gets used */

#define TURRET_WEIGHT	500

#define MAX_ACCEL 2.5		/* the amount a vehicle can accelerate under
				   perfect conditions */

#define NO_OWNER        255	/* Owner of disc if it is not orbiting anyone
				   */

#define NEUTRAL 0		/* The neutral team */

/* How large a landmark is */
#define LANDMARK_WIDTH 50
#define LANDMARK_HEIGHT 50

/* Flags for boxes in a maze description */
#define INSIDE_MAZE	(1<<0)
#define NORTH_WALL	(1<<1)
#define WEST_WALL	(1<<2)
#define NORTH_DEST	(1<<3)
#define WEST_DEST	(1<<4)
#define TYPE_EXISTS	(1<<5)
#define TEAM_EXISTS	(1<<6)
#define EMPTY_BOXES	(1<<7)
#define MAZE_FLAGS	(INSIDE_MAZE|NORTH_WALL|WEST_WALL|NORTH_DEST|WEST_DEST)

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
#define MAX_PROG_ABILITIES      14

/* Map macros for fast map information */
#define map_off_grid(x,y) \
  ((x) < 0 || (x) >= GRID_WIDTH-1 || (y) < 0 || (y) >= GRID_HEIGHT-1)

#define map_north_result(fl) \
  (((fl)&NORTH_WALL) ? (((fl)&NORTH_DEST) ? MAP_DEST : MAP_WALL) : MAP_NONE)

#define map_west_result(fl) \
  (((fl)&WEST_WALL) ? (((fl)&WEST_DEST) ? MAP_DEST : MAP_WALL) : MAP_NONE)

#define map_wall(map,dir,x,y) \
  (map_off_grid(x,y) ? MAP_NONE : \
   ((dir) == NORTH ? map_north_result((map)[x][y].flags) : \
    ((dir) == EAST ? map_west_result((map)[(x)+1][y].flags) : \
     ((dir) == SOUTH ? map_north_result((map)[x][(y)+1].flags) : \
      ((dir) == WEST ? map_west_result((map)[x][y].flags) : MAP_NONE)))))

#define map_landmark(map,x,y) (map_off_grid(x,y) ? 0 : ((map)[x][y].type))
#define map_team(map,x,y)     (map_off_grid(x,y) ? 0 : ((map)[x][y].team))
#define map_strength(map,x,y) (map_off_grid(x,y) ? 0 : ((map)[x][y].strength))


WeaponStatus fire_weapon();
Angle heading(), turn_rate(), turret_angle(), turret_turn_rate(), aim_turret();
float max_speed(), speed(), fuel(), max_fuel();
float tread_acc(), engine_acc(), acc();
Box (*map_get())[GRID_HEIGHT];
Team team();
void turret_position(), throw_discs(), spin_discs();
WallType wall();

#endif ndef _XTANKLIB_H_
