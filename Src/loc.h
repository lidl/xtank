/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** loc.h
*/

#include "config.h"

#ifdef UNIX

#include "screen.h"


/*
** Makes loc = old_loc + (dx,dy)
*/
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
#endif
