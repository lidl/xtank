/* bullet.h - things pertaining to bullets and explosions */

#ifndef _BULLET_H_
#define _BULLET_H_

#include "object.h"
#include "vehicle.h"


typedef struct {
    Vehicle *owner;		/* pointer to vehicle that shot bullet */
    int thrower;		/* color of the guy who thre the frisbee */
    Loc  *loc;			/* pointer to location info */
    Loc  *old_loc;		/* pointer to previous location info */
    Loc   loc1;			/* 1st area for location info */
    Loc   loc2;			/* 2nd area for location info */
    float xspeed;		/* speed of travel in x direction */
    float yspeed;		/* speed of travel in y direction */
    WeaponType type;
    int   life;			/* number of frames left before bullet dies */
    Boolean hurt_owner:1;	/* whether bullet can hurt owner or not */
} Bullet;

typedef struct {
    int   number;		/* number of bullets */
    Bullet *list[MAX_BULLETS];	/* array of pointers to bullets */
    Bullet array[MAX_BULLETS];	/* array of bullets */
} Bset;

typedef struct {
    int   x, y, z;		/* coords */
    int   screen_x[MAX_TERMINALS];	/* x coord on screen */
    int   screen_y[MAX_TERMINALS];	/* y coord on screen */
    int   old_screen_x[MAX_TERMINALS];	/* previous x coord on screen */
    int   old_screen_y[MAX_TERMINALS];	/* previous y coord on screen */
    int   life;			/* # frames before explosion dies */
    Object *obj;		/* pointer to object for the explosion */
    int   color;
} Exp;

typedef struct {
    int   number;		/* number of explosions */
    Exp  *list[MAX_EXPS];	/* array of pointers to explosions */
    Exp   array[MAX_EXPS];	/* array of explosions */
} Eset;


#endif ndef _BULLET_H_
