/* vehicle.h - structure that describes a vehicle in play */

/*
$Author: rpotter $
$Id: vehicle.h,v 2.3 1991/02/10 13:52:04 rpotter Exp $

$Log: vehicle.h,v $
 * Revision 2.3  1991/02/10  13:52:04  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:22  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:29  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:50  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:18  aahz
 * Initial revision
 * 
*/

#ifndef _VEHICLE_H_
#define _VEHICLE_H_


#include "types.h"
#include "message.h"
#include "vdesc.h"
#include "special.h"
#include "loc.h"


typedef struct {
    struct _Combatant *owner;
    Vdesc *vdesc;		/* description of vehicle design */
    int   num_weapons;
    Weapon weapon[MAX_WEAPONS];
    int   num_turrets;
    Turret turret[MAX_TURRETS];
    char *name;
    char  disp[MAX_STRING];	/* string to display under vehicle */
    Flag  flag;			/* unique flag for this vehicle */
    Flag  status;
    Byte  number;		/* serial number */
    int   team;			/* team that the vehicle is on */
    int   num_programs;		/* number of programs present */
    Program program[MAX_PROGRAMS];
    Program *current_prog;	/* current program being executed */
    int   next_message;		/* index to slot for next message received */
    int   new_messages;		/* number of messages received this frame */
    Message received[MAX_MESSAGES]; /* received messages storage */
    Message sending;		/* message to send */
    Loc  *loc;			/* pointer to location info */
    Loc  *old_loc;		/* pointer to old location info */
    Loc   loc1;			/* 1st area for location info */
    Loc   loc2;			/* 2nd area for location info */
    Vector vector;		/* orientation info */
    Object *obj;		/* pointer to screen object */
    float max_fuel;		/* amount of fuel tank can hold */
    float fuel;			/* amount of fuel currently in tank */
    int   heat;			/* internal temperature */
    float turn_rate[MAX_SPEED + 1]; /* safe turning rate for each speed */
    Armor armor;		/* current armor points  */
    Special special[MAX_SPECIALS];
    Boolean safety;		/* TRUE means turn rate is limited */
    int   num_discs;		/* number of discs owned by the vehicle */
    int   color;		/* color for vehicle and bullets it owns */
    int   death_timer;		/* countdown until resurrection */
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
    Team  team;
    int   money;
    int   score;
    int   kills;
    int   deaths;
    Vehicle *vehicle;
} Combatant;


#endif ndef _VEHICLE_H_
