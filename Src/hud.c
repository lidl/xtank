/*****************************************************************************\
* hud.c - an Xtank assistant by Robert Potter (rpotter@grip.cis.upenn.edu)    *
*                                                                             *
* Draws "helpful" stuff on the user's X display.  At the moment, this is just *
* lines indicating the direction of range of the weapons.  Many other things  *
* could be added without much difficulty.                                     *
*                                                                             *
* BUGS:                                                                       *
*     Leaves trash on the sceen when the window gets exposed, 'cause it       *
*     doesn't notice expose events.                                           *
*                                                                             *
* If you modify this to use the (incomplete, though it once worked) circle    *
* code, put the following yucky stuff in your Makefile (because Xtank doesn't *
* have XDrawArc() compiled in):                                               *
*                                                                             *
* XDrArc.o: /lib/libX11.a                                                     *
*     ar x /lib/libX11.a XDrArc.o                                             *
*                                                                             *
* hud-temp.o: hud.c handy.h                                                   *
*     cc -O4 -c hud.c -o hud-temp.o -I../Src                                  *
*                                                                             *
* hud.o: hud-temp.o XDrArc.o                                                  *
*     ld -r -o hud.o hud-temp.o XDrArc.o                                      *
\*****************************************************************************/

#include "stdio.h"
#include "xtank.h"
#include "xtanklib.h"
#include "graphics.h"
#include "gr.h"
#include "vehicle.h"
#include "terminal.h"

extern int thud_main();

Prog_desc hud_prog = {
	"thud",
	"BASE",
	"Heads Up Display.  Put it on your tank, and it draws some supposedly useful things on your X display.",
	"Robert Potter (potter@cs.rochester.edu, rpotter@grip.cis.upenn.edu)",
	0,
	0,
	thud_main
};


extern char *malloc();
extern double sin(), cos();


#define HALF_CIRCLE    (PI)		/* angle of a half rotation */
#define QUAR_CIRCLE    (PI/2)	/* angle of a quarter rotation */


/* description of a line on the "animation" window */
typedef struct
{
	int x1, y1, x2, y2;
	int color;
} LineGraphic;

/* description of a line on the "animation" window */
typedef struct
{
	int x, y, radius;
	int color;
} CircleGraphic;

/* what type a graphical object is */
typedef enum
{
	LineType, CircleType
} GraphicType;

/* description of a graphical object */
typedef struct s_Graphic
{
    struct s_Graphic *next;	/* pointer to next in a linked list */
	GraphicType type;
	union
	{
		LineGraphic line;
		CircleGraphic circle;
	} it;						/* I want anonymous unions! */
	int last_frame_drew;
} Graphic;

/* everything I know, packaged into a structure so that it can be easily
   passed to subroutines (all this to avoid global data) */
typedef struct
{
	int max_range[NUM_MOUNTS];	/* the range of the longest-range weapon on
								   each mount */
	Weapon_info winfo[NUM_MOUNTS];
	Graphic *glist;				/* linked list of graphic objects that are
								   drawn on the display and that we must
								   erase next turn */
	Video *vid;
} All;


/* erase all the graphic objects we have drawn since the previous erase */

static void erase_graphics(allp)
All *allp;
{
	Graphic *gp;

	while (allp->glist != NULL)
	{
		gp = allp->glist;
		allp->glist = allp->glist->next;

		if (gp->last_frame_drew > allp->vid->last_expose_frame)
		{
			switch (gp->type)
			{
				case LineType:
					XDrawLine(allp->vid->dpy, allp->vid->win[ANIM_WIN].id,
						   allp->vid->graph_gc[DRAW_XOR][gp->it.line.color],
							  gp->it.line.x1, gp->it.line.y1,
							  gp->it.line.x2, gp->it.line.y2);
					break;
				case CircleType:

#ifdef ARCS
					XDrawArc(allp->vid->dpy, allp->vid->win[ANIM_WIN].id,
						 allp->vid->graph_gc[DRAW_XOR][gp->it.circle.color],
							 gp->it.circle.x, gp->it.circle.y,
						 gp->it.circle.radius * 2, gp->it.circle.radius * 2,
							 0, 360 * 64);
#endif

					break;
			}
		}
        free((char *)gp);
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
	gp->last_frame_drew = frame_number();
	gp->type = LineType;
	gp->it.line.x1 = x1;
	gp->it.line.y1 = y1;
	gp->it.line.x2 = x2;
	gp->it.line.y2 = y2;
	gp->it.line.color = color;
	gp->next = allp->glist;
	allp->glist = gp;
}



/* figures out which Video this tank is attached to */

static Video *find_our_vid()
{
	extern int num_terminals;
	extern Terminal *terminal[];
	extern Vehicle *cv;			/* xtank's current vehicle */
	int tn;

	/* find a terminal attached to this vehicle */
	for (tn = 0; tn < num_terminals; ++tn)
	{
		if (terminal[tn]->vehicle == cv)
			break;
	}

	if (tn == num_terminals)
		return NULL;			/* failure */

	return (Video *) terminal[tn]->video;
}


thud_main()
{
	All actual_all;
	All *allp = &actual_all;	/* for uniform reference syntax */
	Settings_info si;
	int i;


	bzero((char *)allp, sizeof(*allp));	/* clear all fields to zero */
	allp->vid = find_our_vid();
	if (allp->vid == NULL)
	{							/* none? */
		while (1)
			done();				/* give up */
	}

	get_settings(&si);

	done();						/* skip the first turn.  For some reason if
								   we don't do this we sometimes leave trash
								   on the screen. */

	while (1)
	{
		erase_graphics(allp);

		/* look at our weapons */
		for (i = num_weapons() - 1; i >= 0; --i)
		  {
		    Weapon_info wi;
		    
		    get_weapon(i, &wi);
		    if ((wi.range > allp->max_range[(int)wi.mount]) 
				&& (cv->weapon[i].status&WS_on))
		    {
				allp->max_range[(int)wi.mount] = wi.range;
				memcpy((char *)&allp->winfo[(int)wi.mount],
					(char *)&wi, sizeof(wi));
		    }
		  }
		
		for (i = 0; i < NUM_MOUNTS; ++i)
		{
			int x1, y1, x2, y2;
			Angle a;

#ifdef SLOWER_CODE
			x1 = (SCREEN_WIDTH / 2);	/* location of tank center */
			y1 = (SCREEN_HEIGHT / 2);
#else
			x1 = (SCREEN_WIDTH >> 1);	/* location of tank center */
			y1 = (SCREEN_HEIGHT >> 1);
#endif

			switch (i)
			{
				case MOUNT_TURRET1:
				case MOUNT_TURRET2:
				case MOUNT_TURRET3:
					{
						int tdx, tdy;

                        a = turret_angle((TurretNum)i);
                        turret_position((TurretNum)i, &tdx, &tdy);
						x1 += tdx;
						y1 += tdy;
					}
					break;
				case MOUNT_FRONT:
					a = heading();
					break;
				case MOUNT_BACK:
					a = heading() + HALF_CIRCLE;
					break;
				case MOUNT_LEFT:
					a = heading() - QUAR_CIRCLE;
					break;
				case MOUNT_RIGHT:
					a = heading() + QUAR_CIRCLE;
					break;
			}

			x2 = x1 + (int) (allp->max_range[i] * cos(a));
			y2 = y1 + (int) (allp->max_range[i] * sin(a));
			if (si.rel_shoot)
			{
				Vehicle_info vinfo;

				get_self(&vinfo);

				x2 += (int) vinfo.xspeed * allp->winfo[i].frames;
				y2 += (int) vinfo.yspeed * allp->winfo[i].frames;
			}
			if (allp->max_range[i])
			  draw_screen_line(allp, x1, y1, x2, y2, (i%3)?((i+1)%3?GREEN:YELLOW):RED);
			allp->max_range[i]=0;
		}


		done();					/* wait for start of next turn */
	}
}
