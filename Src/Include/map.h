/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** map.h
*/

/*
$Author: rpotter $
$Id: map.h,v 2.3 1991/02/10 13:51:10 rpotter Exp $

$Log: map.h,v $
 * Revision 2.3  1991/02/10  13:51:10  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:27  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:17  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:01  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:45  aahz
 * Initial revision
 * 
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
