/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** map.h
*/

/*
$Author: lidl $
$Id: map.h,v 1.1.1.1 1995/02/01 00:25:41 lidl Exp $
*/

/* Maximum maze coordinates */
#define GRID_WIDTH	30
#define GRID_HEIGHT	30

/* for the little map display window */
#define MAP_BORDER	-13
#define MAP_BOX_SIZE	9

#define grid2map(val) ((val) * MAP_BOX_SIZE + MAP_BORDER)
#define map2grid(val) (((val) - MAP_BORDER) / MAP_BOX_SIZE)

#define grid_equal(loc1,loc2) \
  ((loc1)->grid_x == (loc2)->grid_x && (loc1)->grid_y == (loc2)->grid_y)

#define xy2map(val) ((val) * MAP_BOX_SIZE + MAP_BORDER)
