/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** vstructs.h
*/

/*
$Author: rpotter $
$Id: vstructs.h,v 2.3 1991/02/10 13:52:07 rpotter Exp $

$Log: vstructs.h,v $
 * Revision 2.3  1991/02/10  13:52:07  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:26  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:34  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:52  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:20  aahz
 * Initial revision
 * 
*/

#define MIN_ARMOR        0
#define MAX_ARMOR      999

#define OVER_WEIGHT      (1<<0)
#define OVER_SPACE       (1<<1)
#define BAD_MOUNT        (1<<2)

typedef struct
{
    char *type;
    int   defense;
    int   weight;
    int   space;
    int   cost;
} Armor_stat;

typedef struct
{
    char *type;
    int   damage;
    int   range;
    int   max_ammo;
    int   reload_time;
    int   ammo_speed;
    int   weight;
    int   space;
    int   frames;
    int   heat;
    int   ammo_cost;
    int   cost;
} Weapon_stat;

typedef struct
{
    char *type;
    int   power;
    int   weight;
    int   space;
    int   fuel_cost;
    int   fuel_limit;
    int   cost;
} Engine_stat;

typedef struct
{
    char *type;
    int   size;
    int   weight;
    int   weight_limit;
    int   space;
    float drag;
    int   handling_base;
    int   turrets;
    int   cost;
} Body_stat;

typedef struct
{
    char *type;
    int   handling_adj;
    int   cost;
} Suspension_stat;

typedef struct
{
    char *type;
    float friction;
    int   cost;
} Tread_stat;

typedef struct
{
    char *type;
    float elasticity;
    int   cost;
} Bumper_stat;

typedef struct
{
    char *type;
    int   cost;
} Special_stat;

typedef struct
{
    int   weight;
    int   space;
    int   cost;
} Heat_sink_stat;



extern Heat_sink_stat heat_sink_stat;
extern Weapon_stat weapon_stat[];
extern Armor_stat armor_stat[];
extern Engine_stat engine_stat[];
extern Body_stat body_stat[];
extern Suspension_stat suspension_stat[];
extern Tread_stat tread_stat[];
extern Bumper_stat bumper_stat[];
extern Special_stat special_stat[];
