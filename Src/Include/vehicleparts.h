/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** $Id$
*/

#ifndef _VEHICLEPARTS_H_
#define _VEHICLEPARTS_H_

/* the different sides a tank has (places armor can be put) */
  typedef enum {
	  FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM,
	  real_MAX_SIDES
  } Side;

#define MAX_SIDES ((int)real_MAX_SIDES)	/* how many there are */

/* the different turrets */
  typedef enum {
	  TURRET1, TURRET2, TURRET3, TURRET4,
	  real_MAX_TURRETS
  } TurretNum;

#define MAX_TURRETS ((int)real_MAX_TURRETS)	/* how many there are */

/* the different weapon mounting locations on a vehicle */
  typedef enum {
	  /* keep these turrets consistent with TurretNum */
	  MOUNT_TURRET1 = TURRET1,
	  MOUNT_TURRET2 = TURRET2,
	  MOUNT_TURRET3 = TURRET3,
	  MOUNT_TURRET4 = TURRET4,
	  MOUNT_FRONT, MOUNT_BACK, MOUNT_LEFT, MOUNT_RIGHT,
	  real_NUM_MOUNTS
  } MountLocation;

#define NUM_MOUNTS ((int)real_NUM_MOUNTS)	/* how many there are */

/* the different types of specials */
  typedef enum {
#define QQ(sym,type,cost) sym,
#include "special-defs.h"		/* read this file for an explanation */
#undef  QQ
	  real_MAX_SPECIALS
  } SpecialType;

#define MAX_SPECIALS	((int)real_MAX_SPECIALS)	/* how many there are */
/* the different types of weapons */
  typedef enum {

#define QQ(sym,type,dam,rng,ammo,tm,spd,wgt,spc,mspc,fr,ht,ac,cost,refl,safe,hgt) sym,

#define RR(n_views,mnt,o_flgs,creat_flgs,disp_flgs,move_flgs,hit_flgs,cr_f,di_f,upd_f,hit_f) /* as nothing */

#include "weapon-defs.h"		/* read this file for an explanation */
#undef QQ
#undef RR
	  real_VMAX_WEAPONS
  } WeaponType;

#define VMAX_WEAPONS	((int)real_VMAX_WEAPONS)	/* how many there are */

/* the differnt types of treads */
  typedef enum {
#define QQ(sym,type,fric,cost) sym,
#include "tread-defs.h"			/* read this file for an explanation */
#undef QQ
	  real_MAX_TREADS
  } TreadType;

#define MAX_TREADS ((int)real_MAX_TREADS)	/* how many there are */


#endif /* ndef _VEHICLEPARTS_H_ */
