/* vehicle_parts.h */

#ifndef _VEHICLE_PARTS_H_
#define _VEHICLE_PARTS_H_

#include "types.h"
#include "object.h"


typedef struct {
    float angle;		/* angle that the turret is pointing */
    float desired_angle;	/* angle driver wants turret to point */
    Spin  angle_flag;		/* which way the turret is rotating */
    Angle turn_rate;		/* how fast the turret can rotate */
    int   rot;			/* picture to show on the screen */
    int   old_rot;		/* picture to erase from the screen */
    Object *obj;		/* pointer to object for the turret */
} Turret;

typedef struct {
    WeaponType type;		/* weapon type (determines bullet type) */
    int   hits;			/* # hit points left in weapon */
    MountLocation mount;	/* location where weapon is mounted */
    int   reload_counter;	/* # frames until next shot can be fired */
    int   ammo;			/* number of ammo units left in weapon */
    Flag  status;		/* status of weapon (on/off,no_ammo) */
} Weapon;

typedef struct {
    int   type;
    int   side[MAX_SIDES];
    int   max_side;
} Armor;

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
    float max_speed;		/* speed limit imposed by body drag and engine
				   power */
    float engine_acc;		/* acceleration limit imposed by weight and
				   engine power (only affects speeding up) */
    float tread_acc;		/* acceleration limit imposed by the tread
				   friction (ground friction is NOT taken into
				   account here) */
    float acc;			/* the minimum of the above two (here for
				   compatibility, not very useful) */
    int   handling;
    int   weight;		/* total */
    int   space;		/* total */
    int   cost;			/* total */
} Vdesc;

typedef struct {
    char  string[MAX_STRING];
    int   value;
    int   color;
} Entry;

typedef struct {
    Entry _entry[MAX_ENTRIES];
    int   num_changes;
    int   change[MAX_ENTRIES];	/* indices of entries that changed */
} Console;

typedef struct {
    Boolean need_redisplay;
    Boolean initial_update;
    int   num_symbols;
    /* new symbols to display */
    Landmark_info symbol[(2 * NUM_BOXES + 1) * (2 * NUM_BOXES + 1)];
    Coord marker;		/* show's vehicle location */
    Coord old_marker;
    Map   map;
    int   num_landmarks;
    Landmark_info landmark[MAX_LANDMARKS];	/* locations of recently passed
					   landmarks */
} Mapper;

typedef struct {
    Byte  x;
    Byte  y;
    Byte  life;
    Byte  view;
    Byte  old_view;
    Byte  flags;
} Blip;

typedef struct {
    int   num_blips;
    Blip  blip[MAX_BLIPS];
    int   pos;			/* current rotation of sweep */
    int   start_x, start_y;	/* starting coordinate of sweep line */
    int   end_x, end_y;		/* ending coordinate of sweep line */
    int   old_start_x, old_start_y;
    int   old_end_x, old_end_y;
} Radar;

typedef struct {
    Flag  status;	/* status of the special */
    int   (*proc) ();	/* function to call for special */
    char *record;	/* pointer to special structure */
} Special;


#endif ndef _VEHICLE_PARTS_H_
