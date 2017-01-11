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

#ifndef _VDESC_H_
#define _VDESC_H_

#include "tanktypes.h"
#include "tanklimits.h"
#include "vehicleparts.h"
#include "loc.h"
#include "object.h"

typedef struct {
	Angle angle;			/* angle that the turret is pointing */
	Angle desired_angle;		/* angle driver wants turret to point */
	Spin angle_flag;		/* which way the turret is rotating */
	Angle turn_rate;		/* how fast the turret can rotate */
	int rot;			/* picture to show on the screen */
	int old_rot;			/* picture to erase from the screen */
	Object *obj;			/* pointer to object for the turret */
#ifdef TEST_TURRETS
	Coord end;	/* show end of turret in x,y relative to mount point */
	Coord old_end;	/* erase end of turret in x,y relative to mount point */
#endif /* TEST_TURRETS */
} Turret;

typedef struct {
	WeaponType type;	/* weapon type (determines bullet type) */
	int hits;		/* # hit points left in weapon */
	MountLocation mount;	/* location where weapon is mounted */
	int reload_counter;	/* # frames until next shot can be fired */
	int ammo;		/* number of ammo units left in weapon */
	Flag status;		/* status of weapon (on/off,no_ammo) */
	int refill_counter;	/* # frames until next shot can refilled */
} Weapon;

typedef struct {
	int type;
	int side[MAX_SIDES];
	int max_side;
} Armor;

/* describes a vehicle design */
  typedef struct {
	  char name[MAX_STRING];
	  char designer[MAX_STRING];
	  int body;
	  int engine;
	  int num_weapons;
	  WeaponType weapon[MAX_WEAPONS];
	  MountLocation mount[MAX_WEAPONS];
	  Armor armor;
	  Flag specials;
	  int heat_sinks;
	  int suspension;
	  int treads;
	  int bumpers;
	  /* the following values are derived from the above */
	  FLOAT max_speed;			/* speed limit imposed by body drag and engine
				   power */
	  FLOAT engine_acc;			/* acceleration limit imposed by weight and
				   engine power (only affects speeding up) */
	  FLOAT tread_acc;			/* acceleration limit imposed by the tread
				   friction (ground friction is NOT taken into
				   account here) */
	  FLOAT acc;				/* the minimum of the above two (here for
				   compatibility, not very useful) */
	  int handling;
	  int weight;				/* total */
	  int space;				/* total */
	  int cost;					/* total */
  }
Vdesc;

#endif /* !_VDESC_H_ */
