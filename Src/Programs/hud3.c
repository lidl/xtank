/*
** Xtank
**
** Copyright 1992 by Robert Potter
**
** hud3.c
*/

/*
$Author: lidl $
$Id: hud3.c,v 1.1.1.1 1995/02/01 00:25:47 lidl Exp $
*/

/*****************************************************************************\
* hud3.c - an Xtank assistant by Robert Potter (rpotter@grip.cis.upenn.edu)   *
* 									      *
* Draws "helpful" stuff on the user's X display.  At the moment, this is just *
* lines indicating the direction of range of the weapons.  Many other things  *
* could be added without much difficulty.				      *
* 									      *
* Handles relative shooting and weapon on/off/ammoless.			      *
* 									      *
* Handles window refresh.						      *
* 									      *
* Color-codes weapons by type.						      *
* 									      *
* NOTE: This uses headers from the xtank/Src directory.  In particular, it    *
* doesn't want the xtanklib.h that's in xtank/Programs.			      *
\*****************************************************************************/

#include "stdio.h"
#include "xtank.h"
#include "xtanklib.h"
#include "graphics.h"
#include "gr.h"
#include "vehicle.h"
#include "terminal.h"

static void main();	/* forward reference */

Prog_desc hud3_prog = {
    "hud3",
    "anything",
    "Heads Up Display.  Put it on your tank, and it draws useful stuff on your display.",
    "Robert Potter (rpotter@grip.cis.upenn.edu)",
    0,
    0,
    main
};


/* description of a graphical object (already drawn on the screen, this info
   stored for later erasure) */
typedef struct _Graphic {
    struct _Graphic *next;	/* pointer to next in a linked list */
    int x1, y1, x2, y2;		/* line endpoints */
    int color;
    int frame;			/* frame when drawn (needed to handle window
				   refresh) */
} Graphic;

/* everything I know, packaged into a structure so that it can be easily
   passed to subroutines (all this to avoid global data) */
typedef struct {
    int frame;			/* current game clock (saves repeated calls to
				   frame_number()) */
    Vehicle_info self;		/* information about our vehicle */

    Weapon_info mntmax[NUM_MOUNTS];	/* info on the longest-range usable
					   weapon on each mount */

    Graphic *glist;		/* linked list of graphic objects that are
				   drawn on the display and that we must erase
				   next turn */
    Video *vid;			/* video of this tank's human owner */
} All;


static Settings_info si;	/* can be global because it can be shared */


/* erase all the graphic objects we have drawn since the previous erase */

static void erase_graphics(allp)
    All *allp;
{
    Graphic *gp;

    while (allp->glist != NULL) {
	gp = allp->glist;
	allp->glist = allp->glist->next;

	/* only erase it if the window hasn't been refreshed since we drew it
	   (which would have erased it for us already) */
	if (gp->frame > allp->vid->last_expose_frame) {
	    /* erase it */
	    XDrawLine(allp->vid->dpy, allp->vid->win[ANIM_WIN].id,
		      allp->vid->graph_gc[DRAW_XOR][gp->color],
		      gp->x1, gp->y1, gp->x2, gp->y2);
	}
	free((char *) gp);
    }
}


/* draw a line on the user's X window */

static void draw_screen_line(allp, x1, y1, x2, y2, color)
    All *allp;
    int x1, y1, x2, y2;
    int color;
{
    Graphic *gp = (Graphic *) malloc(sizeof(Graphic));

    XDrawLine(allp->vid->dpy, allp->vid->win[ANIM_WIN].id,
	      allp->vid->graph_gc[DRAW_XOR][color],
	      x1, y1, x2, y2);

    /* link into list, for later erasure */
    gp->x1 = x1;
    gp->y1 = y1;
    gp->x2 = x2;
    gp->y2 = y2;
    gp->color = color;
    gp->frame = allp->frame;
    gp->next = allp->glist;
    allp->glist = gp;
}



/* figures out which Video this tank is attached to */

static Video *find_our_vid()
{
    extern int num_terminals;
    extern Terminal *terminal[];
    extern Vehicle *cv;		/* xtank's current vehicle */
    int tn;

    /* find a terminal attached to this vehicle */
    for (tn = 0; tn < num_terminals; ++tn) {
	if (terminal[tn]->vehicle == cv)
	    break;
    }

    if (tn == num_terminals)
	return NULL;		/* failure */

    return (Video *) terminal[tn]->video;
}


static void main()
{
    All actual_all;
    All *allp = &actual_all;	/* for uniform reference syntax */
    int i;


    bzero((char *) allp, sizeof(*allp));	/* clear all fields to zero */

    if ((allp->vid = find_our_vid()) == NULL) {	/* none? */
	while (1) done();	/* give up */
    }

    get_settings(&si);

    done();			/* skip the first turn.  For some reason if we
				   don't do this we sometimes leave trash on
				   the screen. */

    while (1) {
	erase_graphics(allp);

	/* mark each mount as void */
	for (i = 0; i < NUM_MOUNTS; ++i)
	    allp->mntmax[i].range = 0;

	allp->frame = frame_number();
	get_self(&(allp->self));

	/* look at our weapons */
	for (i = num_weapons() - 1; i >= 0; --i) {
	    /* only look at weapons that are usable */
	    if (weapon_on(i) && weapon_ammo(i) > 0) {
		Weapon_info wi;
		
		get_weapon(i, &wi);
		/* keep track of the weapon with the greatest range on each of
		   the mounts */
		if (wi.range > allp->mntmax[(int)wi.mount].range) {
		    allp->mntmax[(int)wi.mount] = wi;
		}
	    }
	}

	for (i = 0; i < NUM_MOUNTS; ++i) {
	    if (allp->mntmax[i].range != 0) {
		int x1, y1, x2, y2;	/* line ends */
		int color;
		Angle a;	/* angle of this mount */

		x1 = (SCREEN_WIDTH / 2);	/* location of tank center */
		y1 = (SCREEN_HEIGHT / 2);

		switch (i) {
		  case MOUNT_TURRET1:
		  case MOUNT_TURRET2:
		  case MOUNT_TURRET3:
		  case MOUNT_TURRET4:
		    {
			int tdx, tdy;

			a = turret_angle((TurretNum) i);
			turret_position((TurretNum) i, &tdx, &tdy);
			x1 += tdx;
			y1 += tdy;
		    }
		    break;
		  case MOUNT_FRONT:	a = heading();			break;
		  case MOUNT_BACK:	a = heading() + HALF_CIRCLE;	break;
		  case MOUNT_LEFT:	a = heading() - QUAR_CIRCLE;	break;
		  case MOUNT_RIGHT:	a = heading() + QUAR_CIRCLE;	break;
		}

		switch (allp->mntmax[i].type) {
		  case LMG: case MG: case HMG:
		    color = BLUE;
		    break;
/* HAK
		  case LRIFLE: case RIFLE: case HRIFLE:
		    color = RED;
		    break;
*/
		  case LCANNON: case CANNON: case HCANNON:
		    color = ORANGE;
		    break;
		  case LROCKET: case ROCKET: case HROCKET:
		    color = YELLOW;
		    break;
		  case ACID: case FLAME:
		    color = GREEN;
		    break;
		  case SEEKER:
		    color = VIOLET;
		    break;
		  default:
		    color = GREY;
		    break;
		}

		x2 = x1 + (int) (allp->mntmax[i].range * COS(a));
		y2 = y1 + (int) (allp->mntmax[i].range * SIN(a));
		if (si.rel_shoot) {
		    x2 += (int) allp->self.xspeed * allp->mntmax[i].frames;
		    y2 += (int) allp->self.yspeed * allp->mntmax[i].frames;
		}
		draw_screen_line(allp, x1, y1, x2, y2, color);
	    }
	}

	done();			/* wait for next turn */
    }
}
