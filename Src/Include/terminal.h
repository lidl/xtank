/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** $Id$
*/

#include "vehicle.h"
#include "loc.h"

  typedef struct {
	  /* This should duplicate the definition for an XSegment */
	  short x1, y1, x2, y2;
  }
Line;

  typedef struct {
	  int num;
	  char player_name[MAX_STRING];
	  int vdesc;
	  Vehicle *vehicle;	/* can be NULL */
	  int status;
	  Intloc loc;		/* coordinates of ulc of screen in maze */
	  Intloc old_loc;
	  char *video;		/* video info specific to machine */
			/* Rest is for 3d mode */
	  FLOAT heading;	/* direction of view */
	  FLOAT view_angle;	/* the angle of view width */
	  FLOAT aspect;		/* the aspect ratio of the view */
	  int view_dist;	/* the range of sight in pixels */
	  int num_lines;	/* number of lines drawn */
	  Line line[MAX_LINES];	/* lines drawn on the screen */
	  Boolean observer;	/* True if this terminal is not controlling a
				   vehicle */
	  int mouse_speed;	/* True if this terminal sets vehicle speed */
	  /* by mouse clicks */
	  int mouse_drive;      /* Non Zero if this terminal sets vehicle */
				/* direction via mouse motion */
	  Boolean mouse_drive_active; 
	  int mouse_heat;        /* Non Zero means that this terminal wants */
	                         /* weapons that generate more than */
 				 /* mouse_heat units of heat to be fired */
 				 /* from the Left Mouse Button Only. */
	  char *keymap;          /* string of pairs of chars for keyboard */
				 /* mapping */
	  Boolean teleview;	/* True only when someone is looking from the 
				   camera of a teleguided missile */
	  int rplay_fd;		/* rplay sound file descriptor */
  }
Terminal;


extern Terminal *terminal[];
extern int num_terminals;
