/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** xtank.h
*/

#ifndef _XTANK_H_
#define _XTANK_H_

#include <stdio.h>
#include <strings.h>
#include <math.h>
#include "common.h"
#include "screen.h"

#ifdef AMIGA
#include "amiga.h"
#endif

#define VERSION "1.0"

/* Maze geometry information */
#define GRID_HEIGHT	30
#define GRID_WIDTH	30
#define MAZE_HEIGHT	26
#define MAZE_WIDTH	26
#define MAZE_TOP	2
#define MAZE_BOTTOM	27
#define MAZE_LEFT	2
#define MAZE_RIGHT	27

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

/* Additional flags used in maze */
#define BOX_CHANGED	(1<<5)
#define VEHICLE_0	(1<<8)
#define ANY_VEHICLE	0x0fffff00

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
#define START_POS	16

/* Weapon mount locations */
#define MOUNT_TURRET1		0
#define MOUNT_TURRET2		1
#define MOUNT_TURRET3		2
#define MOUNT_FRONT		3
#define MOUNT_BACK		4
#define MOUNT_LEFT		5
#define MOUNT_RIGHT		6

/* General max values */
#define MAX_STRING	24
#define MAX_VIEWS	32
#define MAX_BULLETS	300
#define MAX_EXPS	100
#define MAX_WEAPONS     6
#define MAX_SPECIALS	3
#define MAX_PROGRAMS    3
#define MAX_VEHICLES	16
#define MAX_LANDMARKS   50
#define MAX_BLIPS	22
#define MAX_TURRETS	3
#define MAX_SEGMENTS	6
#define MAX_ENTRIES	44
#define MAX_SPEED	25
#define MAX_WEAPON_STATS 18
#define MAX_SIDES	6
#define MAX_TEAMS	7
#define MAX_GAME_SPEED  30

/* Maximum number of lines drawn on screen in 3d mode */
#define MAX_LINES       256

/* Description max values */
#define MAX_VDESCS	30
#define MAX_MDESCS	30
#define MAX_PDESCS	30
#define MAX_SDESCS	30

/* Messages max values */
#define MAX_MESSAGES    8
#define MAX_DATA_LEN    31

#ifdef UNIX
#define MAX_TERMINALS	10
#endif
#ifdef AMIGA
#define MAX_TERMINALS	1
#endif

/* Flags for display routines */
#define OFF	        0
#define ON	        1
#define REDISPLAY       2

/* Playing modes */
#define SINGLE_MODE	0
#define MULTI_MODE	1
#define DEMO_MODE	2
#define BATTLE_MODE	3
#define MENU_MODE	4

/* Games that can be played */
#define COMBAT_GAME	0
#define WAR_GAME	1
#define ULTIMATE_GAME	2
#define CAPTURE_GAME	3
#define RACE_GAME       4

/* Return values for animation routine */
#define GAME_FAILED	       (-1)
#define GAME_RUNNING		0
#define GAME_OVER		1
#define GAME_QUIT		2
#define GAME_RESET		3

/* Return values for description loading */
#define DESC_LOADED     0
#define DESC_SAVED	1
#define DESC_NOT_FOUND  2
#define DESC_BAD_FORMAT 3
#define DESC_NO_ROOM    4

/* Sides for the armor structure */
#define FRONT	0
#define BACK	1
#define	LEFT	2
#define	RIGHT	3
#define TOP	4
#define	BOTTOM	5

/* Directions of travel */
#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3

/* Flags and stati for specials */
#define CONSOLE		0
#define MAPPER		1
#define RADAR		2

#define SP_update	0
#define SP_activate	1
#define SP_deactivate	2
#define SP_toggle	3
#define SP_draw		4
#define SP_erase	5
#define SP_redisplay	6
#define SP_break	7
#define SP_repair	8

#define SP_nonexistent	0
#define SP_off		1
#define SP_on		2
#define SP_broken	3

/* States for an angle */
#define NO_ROTATION		0
#define CLOCKWISE		1
#define COUNTERCLOCKWISE	(-1)
#define TOGGLE			2

/* States for a speed */
#define NO_ACCELERATION		0
#define ACCELERATING		1
#define DECELERATING		(-1)

/* Vehicle, weapon, and program status masks */
#define VS_functioning		(1<<0)
#define VS_is_alive		(1<<1)
#define VS_was_alive		(1<<2)
#define VS_disc_spin		(1<<3)
#define VS_rel_turret		(1<<4)
#define VS_sliding		(1<<5)

#define WS_on			(1<<0)
#define WS_func			(1<<1)
#define WS_no_ammo		(1<<2)

#define TS_3d			(1<<0)
#define TS_wide 		(1<<1)
#define TS_long 		(1<<2)
#define TS_extend 		(1<<3)
#define TS_clip 		(1<<4)

#define PROG_on                 (1<<0)

/* Return this if a function is passed bogus data */
#define BAD_VALUE      (-1)

/* Turret locations */
#define TURRET1         0
#define TURRET2         1
#define TURRET3         2

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
#define DISC           17

/* Types of explosions */
#define EXP_TANK      0
#define EXP_GLEAM     1
#define EXP_DAM0      2
#define EXP_DAM1      3
#define EXP_DAM2      4
#define EXP_DAM3      5
#define EXP_DAM4      6
#define EXP_EXHAUST   7

/* Cosell, the ultimate commentator (in the wrong sense) opcodes */
#define COS_INIT_MOUTH    0
#define COS_OWNER_CHANGE  1
#define COS_SLICK_DROPPED 2
#define COS_BIG_SMASH     3
#define COS_GOAL_SCORED   4
#define COS_BEEN_SLICKED  5

#define COS_IGNORE        0
#define COS_WALL_HIT      1

/* Types of descriptions */
#define VDESC 0
#define MDESC 1
#define SDESC 2

/* 3d elevation information */
#define WALLTOP_Z        (BOX_WIDTH/4)
#define WALLBOTTOM_Z     (-BOX_WIDTH/4)
#define TURRET_MOUNT_Z   (BOX_WIDTH/8)
#define SIDE_MOUNT_Z     (-BOX_WIDTH/8)

#include "structs.h"

extern Settings settings;
extern Map box;
extern int frame;

#endif _XTANK_H_
