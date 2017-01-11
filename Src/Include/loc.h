/*-
 * Copyright (c) 1988 Terry Donahue
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

#ifndef _LOC_H_
#define _LOC_H_

  typedef struct {
	  FLOAT x, y, z;			/* absolute coordinates */
	  FLOAT box_x, box_y;		/* coordinates relative to box */
	  int grid_x, grid_y;		/* coordinates of the box in the grid */
	  int screen_x[MAX_TERMINALS];	/* screen coords for each terminal */
	  int screen_y[MAX_TERMINALS];
  }
Loc;

  typedef struct {
	  int x, y;
	  int grid_x, grid_y;
  }
Intloc;

  typedef struct {
	  /* current velocity W.R.T the ground */
	  FLOAT speed;				/* magnitude */
	  Angle angle;				/* direction */
	  FLOAT xspeed;				/* x component */
	  FLOAT yspeed;				/* y component */
	  /* desired velocity */
	  FLOAT drive;				/* magnitude */
	  Angle desired_heading;	/* direction */
	  /* misc */
	  Angle heading;			/* direction front of vehicle is pointing */
	  Angle old_heading;
	  Spin heading_flag;		/* indicates rotation direction */
	  int rot;					/* index of picture (based on heading) */
	  int old_rot;
  }
Vector;


#ifdef UNIX

#include "screen.h"

/*
** Makes loc = old_loc + (dx,dy)
*/

#define update_loc(old_loc,loc,dx,dy) \
  {  \
    (loc)->z = (old_loc)->z;  \
    (loc)->x = (old_loc)->x + dx;  \
    (loc)->y = (old_loc)->y + dy;  \
 \
    (loc)->box_x = (old_loc)->box_x + dx;  \
    (loc)->box_y = (old_loc)->box_y + dy;  \
 \
    if((loc)->box_x >= BOX_WIDTH) {  \
      (loc)->box_x -= BOX_WIDTH;  \
      (loc)->grid_x = (old_loc)->grid_x + 1;  \
    }  \
    else if((loc)->box_x < 0) {  \
      (loc)->box_x += BOX_WIDTH;  \
      (loc)->grid_x = (old_loc)->grid_x - 1;  \
    }  \
    else (loc)->grid_x = (old_loc)->grid_x;  \
 \
    if((loc)->box_y >= BOX_HEIGHT) {  \
      (loc)->box_y -= BOX_HEIGHT;  \
      (loc)->grid_y = (old_loc)->grid_y + 1;  \
    }  \
    else if((loc)->box_y < 0) {  \
      (loc)->box_y += BOX_HEIGHT; \
      (loc)->grid_y = (old_loc)->grid_y - 1;  \
    }  \
    else (loc)->grid_y = (old_loc)->grid_y;  \
  }

#endif /* def UNIX */


#endif /* ndef _LOC_H_ */
