/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** map.h
*/

/* Maximum maze coordinates */
#define GRID_WIDTH	30
#define GRID_HEIGHT	30

#define MAP_BORDER	-13
#define MAP_BOX_SIZE	9

#define grid2map(val) ((val) * MAP_BOX_SIZE + MAP_BORDER)
#define map2grid(val) (((val) - MAP_BORDER) / MAP_BOX_SIZE)

#define grid_equal(loc1,loc2) \
  ((loc1)->grid_x == (loc2)->grid_x && (loc1)->grid_y == (loc2)->grid_y)
