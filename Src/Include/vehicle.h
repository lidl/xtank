/* vehicle.h - structure that describes a vehicle in play */

/*
$Author: lidl $
$Id: vehicle.h,v 2.11 1992/08/31 01:50:45 lidl Exp $

$Log: vehicle.h,v $
 * Revision 2.11  1992/08/31  01:50:45  lidl
 * changed to use tanktypes.h, instead of types.h
 *
 * Revision 2.10  1992/05/19  22:56:08  lidl
 * post chris moore patches
 *
 * Revision 2.9  1992/03/31  21:49:23  lidl
 * Post Aaron-3d patches, camo patches, march patches & misc PIX stuff
 *
 * Revision 2.8  1992/03/31  04:05:52  lidl
 * pre-aaron patches, post 1.3d release (ie mailing list patches)
 *
 * Revision 2.7  1992/01/29  08:39:11  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.6  1992/01/06  08:00:34  stripes
 * Teleport changes
 *
 * Revision 2.5  1991/12/10  01:21:04  lidl
 * change all occurances of "float" to "FLOAT"
 *
 * Revision 2.4  1991/12/03  20:19:47  stripes
 * more of the new skid code changes (KJL)
 *
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


#include "tanktypes.h"
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
    FLOAT max_fuel;		/* amount of fuel tank can hold */
    FLOAT fuel;			/* amount of fuel currently in tank */
    int   heat;			/* internal temperature */
    FLOAT max_turn_rate;	/* how rapidly this vehicle can rotate
								per frame) */
    Armor armor;		/* current armor points  */
    Special special[MAX_SPECIALS];
    Boolean safety;		/* TRUE means turn rate is limited */
    Boolean teleport;		/* FALSE means never teleport (for dumb robots) */
    Boolean mouse_speed;	/* FALSE means only use keyboard to set speed */
    int   num_discs;		/* number of discs owned by the vehicle */
    int   color;		/* color for vehicle and bullets it owns */
    int   death_timer;		/* countdown until resurrection */
    int   slide_count;
    Boolean just_ported;	/* TRUE means we ported this frame */
    int   have_IFF_key[MAX_VEHICLES]; /* array of which IFF keys I have gotten*/
    int   offered_IFF_key[MAX_VEHICLES]; /* array of which IFF keys I have offered*/
    lCoord target;		/* Saved location for a smart weapon */
/*
 * Note that by saving target in the Vehicle struct instead of the
 * display struct, multiple displays may experience unexpected 
 * behavior if they are not aware this infomation is common between
 * them. Of course that might be a feature...
 */
     float rcs;			/* current radar cross section */
     float stealthy_rcs;	/* stealthy radar cross section */
     float normal_rcs;		/* un-stealthy radar cross section */
#ifndef NO_CAMO

    int time_to_camo;   /* How long it takes to get into camo (in frames) */
    Boolean camod;              /* are we invisible now? */
    Boolean old_camod;              /* were we invisible? */

    struct ILL {
        int gx, gy;
	int color;
    } illum[MAX_VEHICLES];

#endif /* !NO_CAMO */

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
    int   keep_score;
    int   keep_kills;
    int   keep_deaths;
    Vehicle *vehicle;
    int   mouse_speed;
} Combatant;

#endif /* _VEHICLE_H_ */
