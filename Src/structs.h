/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** structs.h
*/

#ifndef _STRUCTS_H_
#define _STRUCTS_H_


#include "xtanklib.h"
#include "limits.h"
#include "vehicle_parts.h"


typedef struct {
    Game  type;
    char *name;
    char *designer;
    char *desc;
    Byte *data;
} Mdesc;

typedef struct {
    int num_starts[MAX_TEAMS];	/* number of start locs for each team */
    Coord start[MAX_TEAMS][MAX_VEHICLES];	/* coordinates of starting
						   locations */
} Maze;

typedef struct
{
    float x;	/* absolute coordinates */
    float y;
    float z;
    float box_x;	/* coordinates relative to box */
    float box_y;
    int   grid_x;	/* coordinates of the box in the grid */
    int   grid_y;
    int   screen_x[MAX_TERMINALS];	/* screen coords for each terminal */
    int   screen_y[MAX_TERMINALS];
} Loc;

typedef struct
{
    int   x;
    int   y;
    int   grid_x;
    int   grid_y;
} Intloc;

typedef struct {
    float speed;	/* actual ground speed */
    Angle angle;	/* actual ground angle */
    Angle heading;	/* direction vehicle is pointing */
    float drive;	/* speed the wheels are turning */
    float xspeed;	/* x component of speed and angle */
    float yspeed;	/* y component of speed and angle */
    float old_heading;
    float desired_heading;
    Spin heading_flag;	/* indicates rotation direction */
    int   rot;		/* index of picture (based on heading) */
    int   old_rot;
} Vector;

typedef struct
{
    Prog_desc *desc;	/* description of program */
    int   status;	/* status of program */
    int   next_message;	/* index of next message for program to read */
    int   total_time;	/* execution time used by prog (in usec) */
    char *thread;	/* pointer to thread of execution */
} Program;

typedef struct
{
    int   x;
    int   y;
    int   len;
    char *str;
}     Word;

typedef struct
{
    int   mode;			/* single, multi, demo, battle */
    int   game_speed;		/* max. number of frames per second */
    Mdesc *mdesc;		/* pointer to maze description */
    int   maze_density;		/* % of walls kept in random maze (0 - 100) */
    Boolean point_bullets;	/* whether bullets are points or pictures */
    Boolean commentator;	/* whether commentator will comment on game */
    Boolean robots_dont_win;	/* whether robots can win a game */
    Boolean max_armor_scale;	/* scales armor to max instead of per side */
    int   difficulty;		/* difficulty of enemy robots (0 - 10) */
    Settings_info si;		/* lots of other info that is made available to
				   robot players */
} Settings;

typedef struct
{
    char  vehicle_name[MAX_STRING];
    Byte  player;
    Byte  program;
    Byte  team;
} Comb;

typedef struct
{
    char  name[MAX_STRING];	/* name of the setup */
    char  maze_name[MAX_STRING];	/* name of the maze used in the setup
					   */
    Comb  comb[MAX_VEHICLES];	/* short descriptions of each combatant */
    Settings settings;		/* the settings for the game */
} Sdesc;


#endif ndef _STRUCTS_H_
