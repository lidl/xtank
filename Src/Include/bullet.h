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

/*
** Comment: things pertaining to bullets and explosions
*/

#ifndef _BULLET_H_
#define _BULLET_H_

#include "vehicleparts.h"
#include "object.h"
#include "vehicle.h"
#include "tanktypes.h"

/* The defines for all of the options we support */

/* Other options */
#define F_OUTP		0x0F	/* mask.  lower 4 bits specify which levels of */
				/* outposts can fire this weapon.  0 means the */
				/* outpost will never fire this weapon. Otherwise */
				/* should be set to 1..10 */

#define F_CHO		(1<<4)	/* the bullet can hurt owner after first 10 frames of it's life */

/* Creation options */
#define F_CR3		(1<<0)	/* create 3 in a fan (oil slicks) */
#define F_MAP		(1<<1)	/* bullet is fired by clicking on map window */
#define F_BOTH		(1<<2)	/* bullet is fired from map or anim window */
#define F_NREL		(1<<3)	/* bullet is never fired w/relative velocity */

/* Display options */
#define F_BL		1	/* blue hud-line (default (0) is grey) */
#define F_RE		2	/* red */
#define F_OR		3	/* orange */
#define F_YE		4	/* yellow */
#define F_GR		5	/* green */
#define F_VI		6	/* violet */
#define F_CM		7	/* mask for above colors */
#define F_NOHD		(1<<3)	/* don't draw a hud-line for this weapon */
#define F_TELE		(1<<4)	/* shooter can swich to bullet view */
#define F_ROT		(1<<5)	/* bitmap is a "movie", like mine */
#define F_TRL		(1<<6)	/* bullet needs an exaust trail */
#define F_BEAM		(1<<7)	/* bullet is drawn as a line segment (laser) */
#define F_NOPT		(1<<8)	/* don't display this as a point */
#define F_TAC		(1<<9)	/* show the bullet on tac-link */

/* Movement options */
#define F_KEYB		(1<<0)	/* steer w/keyboard (i.e. tow missles) */
#define F_MINE		(1<<1)	/* move 5 frames, then stop */
#define F_DET		(1<<2)	/* (for area weapons) explode at end of life */

/* Hit options */
#define AREA		(1<<0)	/* area explosion (damages all sides) */
#define F_NOHIT		(1<<1)	/* the bullet won't hit vehicles (e.g. nukes) */
#define F_SLICK		(1<<2)	/* oil slicks */
#define F_HOVER		(1<<3)	/* (for height=-1 bullets) random chance to hit hovers */

/* one of these are passed to the special hit function */
#define HIT_VEH		1
#define HIT_OUTP	2
#define HIT_WALL	3

/* end-o-defines */

/* returns an index (0..numv-1) based on angle
 * (HAK 4/93)
 */

#define find_rot(angle,numv) ((int)(((angle) + PI/(numv) + PI*2)*((numv) / (PI*2))))%(numv)


  typedef struct {
	  Vehicle *owner;	/* pointer to vehicle that shot bullet */
	  int thrower;		/* color of the guy who thre the frisbee */
	  Loc *loc;		/* pointer to location info */
	  Loc *old_loc;		/* pointer to previous location info */
	  Loc loc1;		/* 1st area for location info */
	  Loc loc2;		/* 2nd area for location info */
	  FLOAT xspeed;		/* speed of travel in x direction */
	  FLOAT yspeed;		/* speed of travel in y direction */
	  WeaponType type;
	  int life;		/* number of frames left before bullet dies */
	  Boolean hurt_owner;	/* whether bullet can hurt owner or not */
	  lCoord target;	/* last target update for a smart weapon */
	  int state;		/* what a smart bullet is up to */
	  int mode;

	/* Look out! Something wicked this way comes. */
	/* HAK and MEL mods 3/24/93 */

	  int rot;			/* which bitmap to use */
	  int old_rot;			/* so it knows which view to undraw */
	  Loc old_old_loc;		/* used for lasers */
	  long stuff;			/* used for anything the bullet wants... for its eyes only */

          int safety;			/* Number of frames before a bullet becomes active */
	  int num_views;		/* Number of bitmaps the bullet has */

          unsigned long other_flgs;	/* All sorts of flag type things (Hints for the menus etc..) */
          unsigned long creat_flgs;	/* Used for bullet creation */
	  unsigned long disp_flgs;	/* Type of display code (In case of special animations etc...) */
	  unsigned long move_flgs;	/* Movement type (normal, mine, anti-rad etc... ) */
	  unsigned long hit_flgs;	/* Damage type (normal, high, low, blast, area etc... ) */

	/* Pointer to special creation code */
	  void (*creat_func)(Weapon *v, Loc *bloc, Angle angle);

	  void (*disp_func)();		/* Pointer to special display code */

	  void (*upd_func)(void *b);	/* Pointer to special update code */

	/* Pointer to special damage code */
	  void (*hit_func)(int whatHit, void *b, int dx, int dy,
				void *parm1, void *parm2, void *parm3);

	/* End-o-big mods */

  }
Bullet;

  typedef struct {
	  int number;			/* number of bullets */
	  Bullet *list[MAX_BULLETS];	/* array of pointers to bullets */
	  Bullet array[MAX_BULLETS];	/* array of bullets */
  }
Bset;

  typedef struct {
	  int x, y, z;			/* coords */
	  int screen_x[MAX_TERMINALS];	/* x coord on screen */
	  int screen_y[MAX_TERMINALS];	/* y coord on screen */
	  int old_screen_x[MAX_TERMINALS];	/* previous x coord on screen */
	  int old_screen_y[MAX_TERMINALS];	/* previous y coord on screen */
	  int life;			/* # frames before explosion dies */
	  Object *obj;			/* pointer to object for the explosion */
	  int color;
  }
Exp;

  typedef struct {
	  int number;			/* number of explosions */
	  Exp *list[MAX_EXPS];		/* array of pointers to explosions */
	  Exp array[MAX_EXPS];		/* array of explosions */
  }
Eset;


#endif /* ndef _BULLET_H_ */
