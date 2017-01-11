/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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

/* Types of landmarks */  
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

typedef struct {
  char *name;			/* name of program */
  char *type;			/* type of program */
  char *vehicle;		/* name of default vehicle */
  char *strategy;		/* description of strategy */
  char *author;			/* name of author */
  int skill;			/* skill at doing what it does */
  int (*func)();		/* main procedure of program */
  char *code;			/* pointer to code memory */
} Prog_desc;

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
