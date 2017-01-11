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

#ifndef _TERMINAL_H_
#define _TERMINAL_H_

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

#endif /* _TERMINAL_H_ */
