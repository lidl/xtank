/* vdesc.h - part of Xtank */

#ifndef _VDESC_H_
#define _VDESC_H_

#include "tanktypes.h"
#include "object.h"
#include "vehicleparts.h"


typedef struct {
    Angle angle;		/* angle that the turret is pointing */
    Angle desired_angle;	/* angle driver wants turret to point */
    Spin  angle_flag;		/* which way the turret is rotating */
    Angle turn_rate;		/* how fast the turret can rotate */
    int   rot;			/* picture to show on the screen */
    int   old_rot;		/* picture to erase from the screen */
    Object *obj;		/* pointer to object for the turret */
#ifdef TEST_TURRETS
    Coord end;			/* show end of turret in x,y relative to mount point */
    Coord old_end;		/* erase end of turret in x,y relative to mount point */
#endif /* TEST_TURRETS */
} Turret;

typedef struct {
    WeaponType type;		/* weapon type (determines bullet type) */
    int   hits;			/* # hit points left in weapon */
    MountLocation mount;	/* location where weapon is mounted */
    int   reload_counter;	/* # frames until next shot can be fired */
    int   ammo;			/* number of ammo units left in weapon */
    Flag  status;		/* status of weapon (on/off,no_ammo) */
    int   refill_counter;	/* # frames until next shot can refilled */
} Weapon;

typedef struct {
    int   type;
    int   side[MAX_SIDES];
    int   max_side;
} Armor;

/* describes a vehicle design */
typedef struct {
    char  name[MAX_STRING];
    char  designer[MAX_STRING];
    int   body;
    int   engine;
    int   num_weapons;
    WeaponType weapon[MAX_WEAPONS];
    MountLocation mount[MAX_WEAPONS];
    Armor armor;
    Flag  specials;
    int   heat_sinks;
    int   suspension;
    int   treads;
    int   bumpers;
    /* the following values are derived from the above */
    FLOAT max_speed;		/* speed limit imposed by body drag and engine
				   power */
    FLOAT engine_acc;		/* acceleration limit imposed by weight and
				   engine power (only affects speeding up) */
    FLOAT tread_acc;		/* acceleration limit imposed by the tread
				   friction (ground friction is NOT taken into
				   account here) */
    FLOAT acc;			/* the minimum of the above two (here for
				   compatibility, not very useful) */
    int   handling;
    int   weight;		/* total */
    int   space;		/* total */
    int   cost;			/* total */
} Vdesc;


#endif ndef _VDESC_H_
