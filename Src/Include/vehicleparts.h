/* vehicleparts.h - part of Xtank */

/*
$Author: lidl $
$Id: vehicleparts.h,v 2.6 1992/03/31 21:49:23 lidl Exp $

$Log: vehicleparts.h,v $
 * Revision 2.6  1992/03/31  21:49:23  lidl
 * Post Aaron-3d patches, camo patches, march patches & misc PIX stuff
 *
 * Revision 2.5  1991/12/03  20:22:08  senft
 * Updated enums for the fourth turrent.
 *
 * Revision 2.4  1991/09/17  17:07:03  lidl
 * changed name from vehicle_parts.h to vehicleparts.h
 * a little SysVRcrippled support/compatibility
 *
 * Revision 2.3  1991/02/10  13:52:05  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:24  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:31  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:51  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:19  aahz
 * Initial revision
 * 
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
#include "special-defs.h"	/* read this file for an explanation */
#undef  QQ
    real_MAX_SPECIALS
} SpecialType;
#define MAX_SPECIALS	((int)real_MAX_SPECIALS) /* how many there are */

#ifdef NO_TIMED_REFILL

/* the different types of weapons */
typedef enum {
#define QQ(sym,type,dam,rng,ammo,tm,spd,wgt,spc,fr,ht,ac,cost) sym,
#include "weapon-defs.h"	/* read this file for an explanation */
#undef  QQ
    real_VMAX_WEAPONS
} WeaponType;
#define VMAX_WEAPONS	((int)real_VMAX_WEAPONS) /* how many there are */

#else /* NO_TIMED_REFILL */
/* the different types of weapons */
typedef enum {
#define QQ(sym,type,dam,rng,ammo,tm,spd,wgt,spc,fr,ht,ac,cost,refill) sym,
#include "weapon-defs.h"	/* read this file for an explanation */
#undef  QQ
    real_VMAX_WEAPONS
} WeaponType;
#define VMAX_WEAPONS	((int)real_VMAX_WEAPONS) /* how many there are */

#endif /* NO_TIMED_REFILL */

/* the differnt types of treads */
typedef enum {
#define QQ(sym,type,fric,cost) sym,
#include "tread-defs.h"		/* read this file for an explanation */
#undef QQ
    real_MAX_TREADS
} TreadType;
#define MAX_TREADS ((int)real_MAX_TREADS)	/* how many there are */


#endif ndef _VEHICLEPARTS_H_
