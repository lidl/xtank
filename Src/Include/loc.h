/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** loc.h
*/

/*
$Author: lidl $
$Id: loc.h,v 2.6 1992/01/29 08:39:11 lidl Exp $

$Log: loc.h,v $
 * Revision 2.6  1992/01/29  08:39:11  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.5  1991/12/10  01:21:04  lidl
 * change all occurances of "float" to "FLOAT"
 *
 * Revision 2.4  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.3  1991/02/10  13:51:01  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:16  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:06  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:52  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:41  aahz
 * Initial revision
 * 
*/


#ifndef _LOC_H_
#define _LOC_H_

typedef struct {
    FLOAT x, y, z;	/* absolute coordinates */
    FLOAT box_x, box_y;	/* coordinates relative to box */
    int   grid_x, grid_y;	/* coordinates of the box in the grid */
    int   screen_x[MAX_TERMINALS];	/* screen coords for each terminal */
    int   screen_y[MAX_TERMINALS];
} Loc;

typedef struct {
    int   x, y;
    int   grid_x, grid_y;
} Intloc;

typedef struct {
    /* current velocity W.R.T the ground */
    FLOAT speed;	/* magnitude */
    Angle angle;	/* direction */
    FLOAT xspeed;	/* x component */
    FLOAT yspeed;	/* y component */
    /* desired velocity */
    FLOAT drive;	/* magnitude */
    Angle desired_heading;	/* direction */
    /* misc */
    Angle heading;	/* direction front of vehicle is pointing */
    Angle old_heading;
    Spin heading_flag;	/* indicates rotation direction */
    int   rot;		/* index of picture (based on heading) */
    int   old_rot;
} Vector;


#ifdef UNIX

#include "screen.h"

/*
** Makes loc = old_loc + (dx,dy)
*/

#ifndef NO_NEW_RADAR

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

#else /* !NO_NEW_RADAR */

#define update_loc(old_loc,loc,dx,dy) \
  {  \
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

#endif /* !NO_NEW_RADAR */

#endif def UNIX


#endif ndef _LOC_H_
