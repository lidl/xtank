/* vehicle.h - structure that describes a vehicle in play */

#ifndef _VEHICLE_H_
#define _VEHICLE_H_


#include "types.h"
#include "message.h"
#include "structs.h"


typedef struct {
    struct _Combatant *owner;	/* owner of vehicle */
    char *name;			/* name of the vehicle */
    char  disp[MAX_STRING];	/* string to display under vehicle */
    Flag  flag;			/* unique flag for this vehicle */
    Flag  status;		/* status of vehicle */
    Byte  number;		/* number of vehicle (0 to MAX_VEHICLES-1) */
    int   team;			/* team that the vehicle is on */
    int   num_programs;		/* number of programs present in vehicle */
    Program program[MAX_PROGRAMS]; /* program to control vehicle */
    Program *current_prog;	/* current program being executed */
    int   next_message;		/* index to slot for next message received */
    int   new_messages;		/* number of messages received this frame */
    Message received[MAX_MESSAGES]; /* received messages storage */
    Message sending;		/* message to send */
    Loc  *loc;			/* pointer to location info */
    Loc  *old_loc;		/* pointer to old location info */
    Loc   loc1;			/* 1st area for location info */
    Loc   loc2;			/* 2nd area for location info */
    Vector vector;		/* orientation info about vehicle */
    int   num_turrets;		/* number of turrets on tank */
    Turret *turret;		/* pointer to beginning of turret array */
    Object *obj;		/* pointer to screen object for the vehicle */
    Vdesc *vdesc;		/* description of vehicle */
    float max_fuel;		/* amount of fuel tank can hold */
    float fuel;			/* amount of fuel in tank */
    int   heat;			/* amount of heat in vehicle */
    float turn_rate[MAX_SPEED + 1]; /* safe turning rate for each speed */
    Armor armor;		/* present armor points on vehicle */
    int   num_weapons;		/* number of weapons on vehicle */
    Weapon weapon[MAX_WEAPONS];	/* array of weapons */
    Special special[MAX_SPECIALS]; /* array of specials */
    Boolean safety;		/* TRUE means turn rate is limited */
    int   num_discs;		/* number of discs owned by the vehicle */
    int   color;		/* color for vehicle and bullets it owns */
    int   death_timer;
    int   is_dead;		/* redundant info ... here for efficiency */
    int   slide_count;
} Vehicle;

typedef struct _Combatant {
    char  name[MAX_STRING];
    int   num_players;
    int   player[MAX_TERMINALS];
    int   num_programs;
    int   program[MAX_PROGRAMS];
    int   vdesc;
    int   number;
    int   team;
    int   money;
    int   score;
    int   kills;
    int   deaths;
	int   flesh;			/* whether or not a HUMAN is playing this one */
    Vehicle *vehicle;
} Combatant;


#endif ndef _VEHICLE_H_
