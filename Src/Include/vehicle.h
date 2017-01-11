/*-
 * Copyright (c) 1993 Josh Osborne
 * Copyright (c) 1993 Kurt Lidl
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

#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#include "tanktypes.h"
#include "message.h"
#include "vdesc.h"
#include "special.h"
#include "loc.h"


  typedef struct {
	  struct _Combatant *owner;
	  Vdesc *vdesc;				/* description of vehicle design */
	  int num_weapons;
	  Weapon weapon[MAX_WEAPONS];
	  int num_turrets;
	  Turret turret[MAX_TURRETS];
	  char *name;
	  char disp[MAX_STRING];	/* string to display under vehicle */
	  Flag flag;				/* unique flag for this vehicle */
	  Flag status;
	  Byte number;				/* serial number */
	  int team;					/* team that the vehicle is on */
	  int num_programs;			/* number of programs present */
	  Program program[MAX_PROGRAMS];
	  Program *current_prog;	/* current program being executed */
	  int next_message;			/* index to slot for next message received */
	  int new_messages;			/* number of messages received this frame */
	  Message received[MAX_MESSAGES];	/* received messages storage */
	  Message sending;			/* message to send */
	  Loc *loc;					/* pointer to location info */
	  Loc *old_loc;				/* pointer to old location info */
	  Loc loc1;					/* 1st area for location info */
	  Loc loc2;					/* 2nd area for location info */
	  Vector vector;			/* orientation info */
	  Object *obj;				/* pointer to screen object */
	  FLOAT max_fuel;			/* amount of fuel tank can hold */
	  FLOAT fuel;				/* amount of fuel currently in tank */
	  int heat;					/* internal temperature */
	  FLOAT max_turn_rate;		/* how rapidly this vehicle can rotate
								per frame) */
	  Armor armor;				/* current armor points  */
	  Special special[MAX_SPECIALS];
	  Boolean safety;			/* TRUE means turn rate is limited */
	  Boolean teleport;			/* FALSE means never teleport (for dumb robots) */
	  int num_discs;			/* number of discs owned by the vehicle */
	  int color;				/* color for vehicle and bullets it owns */
	  int death_timer;			/* countdown until resurrection */
	  int slide_count;
	  Boolean just_ported;		/* TRUE means we ported this frame */
	  int have_IFF_key[MAX_VEHICLES];	/* array of which IFF keys I have gotten*/
	  int offered_IFF_key[MAX_VEHICLES];	/* array of which IFF keys I have offered*/
	  lCoord target;			/* Saved location for a smart weapon */
/*
 * Note that by saving target in the Vehicle struct instead of the
 * display struct, multiple displays may experience unexpected
 * behavior if they are not aware this infomation is common between
 * them. Of course that might be a feature...
 */
	  FLOAT rcs;				/* current radar cross section */
	  FLOAT stealthy_rcs;		/* stealthy radar cross section */
	  FLOAT normal_rcs;			/* un-stealthy radar cross section */
#ifndef NO_CAMO

	  int time_to_camo;			/* How long it takes to get into camo (in frames) */
	  Boolean camod;			/* are we invisible now? */
	  Boolean old_camod;		/* were we invisible? */

		struct ILL {
			int gx, gy;
			int color;
		}
	  illum[MAX_VEHICLES];

	  int frame_weapon_fired;
#endif /* !NO_CAMO */
#ifndef NO_DAMAGE
	  int heat_sinks;
#endif

  }
Vehicle;

  typedef struct _Combatant {
	  char name[MAX_STRING];
	  int num_players;
	  int player[MAX_TERMINALS];
	  int num_programs;
	  int program[MAX_PROGRAMS];
	  int vdesc;
	  int number;
	  Team team;
	  int money;
	  int score;
	  int kills;
	  int deaths;
	  int keep_score;
	  int keep_kills;
	  int keep_deaths;
	  Vehicle *vehicle;
	  int mouse_speed;
  }
Combatant;

#endif /* _VEHICLE_H_ */
