/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** xtanklib.h
*/

/*
$Author: lidl $
$Id: xtanklib.h,v 2.12 1992/01/29 08:39:11 lidl Exp $

$Log: xtanklib.h,v $
 * Revision 2.12  1992/01/29  08:39:11  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.11  1992/01/26  04:56:49  stripes
 * added bounding box for vehicle at current angle
 *
 * Revision 2.10  1992/01/06  08:00:34  stripes
 * Teleport changes
 *
 * Revision 2.9  1992/01/03  23:07:01  stripes
 * Fixed IS_TURRET to handle 4 turrets.
 *
 * Revision 2.8  1991/12/10  01:21:04  lidl
 * change all occurances of "float" to "FLOAT"
 *
 * Revision 2.7  1991/12/03  20:18:51  stripes
 * placed comment about TICKSZ, cleaned up tabbing some
 *
 * Revision 2.6  1991/09/29  08:16:29  lidl
 * removed inclusion of version.h
 *
 * Revision 2.5  1991/09/26  04:12:12  lidl
 * changed TICKSZ to be 1
 *
 * Revision 2.4  1991/09/17  17:08:22  lidl
 * changed name of included file "vehicle_parts.h" to "vehicleparts.h"
 *
 * Revision 2.3  1991/02/10  13:52:16  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:36  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:46  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:11:01  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:25  aahz
 * Initial revision
 * 
*/

#ifndef _XTANKLIB_H_		/* prevent multiple inclusions */
#define _XTANKLIB_H_


#include "sysdep.h"
#include "map.h"
#include "screen.h"
#include "common.h"
#include "message.h"
#include "team.h"
#include "limits.h"
#include "vehicleparts.h"
#include "game.h"
#include "program.h"
#include "settings.h"


/* the different types of terrain boxes can have */
typedef enum {
    BAD_MAPPER = BAD_VALUE,	/* you don't have a mapper! */
    NORMAL = 0,			/* nothing unusual */
    FUEL,			/* fuel depot */
    AMMO,			/* ammunition depot */
    ARMOR,			/* armor depot */
    GOAL,			/* goal for race and ultimate games, by team */
    OUTPOST,			/* shoots at vehicles, by team */
    PEACE,			/* protection from damage, by team */
    TELEPORT,			/* teleport */
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
    START_POS,			/* vehicle starting position, by team */
    real_NUM_LANDMARK_TYPES,
    /* wall symbols, used inside Xtank only */
    NORTH_SYM,			/* indestructible */
    WEST_SYM,
    NORTH_DEST_SYM,		/* destructible */
    WEST_DEST_SYM
} LandmarkType;
#define NUM_LANDMARK_TYPES ((int) real_NUM_LANDMARK_TYPES)

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

typedef int WeaponNum;		/* index of one of the weapon instances on this
				   tank, in the range [0, MAX_WEAPONS-1].  Not
				   to be confused with WeaponType. */
/* the different conditions a weapon can be in (this should really be a set of
   bits so they can be combined...) */
typedef enum {
    BAD_WEAPON_NUM = BAD_VALUE,	/* there is no weapon with that number */
    FIRED = 0,			/* no problem, fired successfully */
    RELOADING,			/* was fired too recently (firing rate) */
    NO_AMMO,			/* no ammunition left */
    WEAPON_OFF,			/* has been turned off */
    TOO_HOT			/* vehicle is too hot */
} WeaponStatus;

typedef struct {
    WeaponType type;		/* what type of weapon it is */
    MountLocation mount;	/* where it's mounted */
    int damage;			/* damage each bullet does */
    int heat;			/* heat each firing produces */
    int range;			/* range of bullets */
    int reload;			/* how much time must pass between firings */
    int max_ammo;		/* how much ammo it can hold */
    int ammo_speed;		/* speed of bullet */
    int frames;			/* how long bullets live */
} Weapon_info;

#ifndef NO_NEW_RADAR

typedef struct {
    int   x, y;                 /* box coordinates */
    Boolean new;
    Team   team;
    int    number;
    Boolean radar;
    Boolean tactical;
    Boolean friend;
} Blip_info;

#else /* !NO_NEW_RADAR */

typedef struct {
    int	  x, y;			/* box coordinates */
} Blip_info;

#endif /* NO_NEW_RADAR */

typedef struct {
    int grid_x, grid_y;		/* coordinates of the box */
    int box_x, box_y;		/* coordinates within the box (in pixels) */
    int x, y;			/* absolute coordinates (in pixels) */
} Location;

typedef struct {
    LandmarkType type;
    Team team;			/* team that "owns" it */
    int x, y;			/* box coordinates */
} Landmark_info;

typedef struct {
    Flag  flags;		/* bits for walls, inside maze, etc. */
    LandmarkType  type;		/* landmark, scroll, goal, outpost, etc. */
    Team  team;			/* number of team that owns the box */
    Byte  strength;		/* strength of the scroll, outpost, etc. */
    void  *user_data;		/* robot programs can do whatever they want
				   with this */
} Box;

typedef Box Map[GRID_WIDTH][GRID_HEIGHT];

typedef struct {
    Location loc;		/* where it is */
    FLOAT xspeed, yspeed;	/* components of current velocity */
    Angle heading;		/* direction body is pointing */
    ID id;			/* unique identification number */
    Team team;			/* team number */
    int num_turrets;		/* number of turrets on the body */
    Angle turret_angle[MAX_TURRETS];	/* direction each turret is pointing */

    int body;			/* body type */
	/* New for 1.3c */
	int bwidth, bheight;	/* bounding box for vehicle at current angle */
} Vehicle_info;

typedef struct {
    Location loc;
    FLOAT xspeed, yspeed;	/* current speed (can change for Seekers) */
    WeaponType type;
    int id;					/* a unique identification number */
} Bullet_info;

typedef struct {
    Location loc;				/* where it is */
    FLOAT xspeed, yspeed;		/* current velocity */
    ID owner;					/* who has it */
    Angle angle;				/* direction from owner */
    Spin spin;					/* direction it's orbiting owner */
} Disc_info;

/* Robots now get CPU each frame, unless they use too many cycles... */
#define TICKSZ		1			/* Changing this won't change tick size! */

#define FUEL_CONSUME	0.001	/* how fast fuel gets used */

#define TURRET_WEIGHT	500

#define MAX_ACCEL		2.5		/* the amount a vehicle can accelerate under
				  				 perfect conditions */

#define NO_OWNER        255		/* Owner of disc if it is not orbiting anyone */

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
#define PLAYS_WAR			(1<<1)
#define PLAYS_ULTIMATE		(1<<2)
#define PLAYS_CAPTURE		(1<<3)
#define PLAYS_RACE			(1<<4)
#define DOES_SHOOT			(1<<5)
#define DOES_EXPLORE		(1<<6)
#define DOES_DODGE			(1<<7)
#define DOES_REPLENISH		(1<<8)
#define USES_TEAMS			(1<<9)
#define USES_MINES			(1<<10)
#define USES_SLICKS			(1<<11)
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


#define FULL_CIRCLE	(2*PI)	/* angle of a full rotation */
#define HALF_CIRCLE	(PI)	/* angle of a half rotation */
#define QUAR_CIRCLE	(PI/2)	/* angle of a quarter rotation */
#define BAD_ANGLE	999.0	/* some unreasonable angle */

/* restore an angle to the canonical [0,FULL_CIRCLE) range */
#define fixed_angle(x) ((x) - floor((x)/FULL_CIRCLE) * FULL_CIRCLE)
#define HYPOT(X,Y) hypot((double)(X),(double)(Y))
#define SQRT(X)	sqrt((double)(X))
#define SIN(X)	sin((double)(X))
#define ASIN(X)	asin((double)(X))
#define COS(X)	cos((double)(X))
#define IS_TURRET(mount)	((int)(mount) >= (int)MOUNT_TURRET1 && \
				 (int)(mount) <= (int)MOUNT_TURRET4)

#ifndef NO_NEW_RADAR
/*
* Well, IS_TURRET is here, isn't it?
*/
 
#define IS_SIDE(mount)  ((int)(mount) == (int)MOUNT_LEFT || \
                                  (int)(mount) == (int)MOUNT_RIGHT)
 
/*
* This was a simple way to grandfather all of the old usage of
* make_bullet in without changing their argument lists.
*/
 
#define make_bullet(v,loc,type,angle) make_smart_bullet(v,loc,type,angle,NULL)

#define BOOST_PHASE 5 /* How many frames it moves ballisticly forward */

#endif /* !NO_NEW_RADAR */

#include "lowlib.h"


#endif /* _XTANKLIB_H_ */
