/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
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
