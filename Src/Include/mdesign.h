/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** mdesign.h
*/

/*
$Author: rpotter $
$Id: mdesign.h,v 2.3 1991/02/10 13:51:18 rpotter Exp $

$Log: mdesign.h,v $
 * Revision 2.3  1991/02/10  13:51:18  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:35  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:29  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:09  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:52  aahz
 * Initial revision
 * 
*/

/* Padding on each side of the maze in boxes */
#define PAD 2

/* Height and width of a box in pixels */
#define DES_BOX_WIDTH  24
#define DES_BOX_HEIGHT 24

/* Offsets of maze from upper left of window */
#define X_OFFSET (-40)
#define Y_OFFSET (-40)

/* How close the mouse must be to line to toggle */
#define THRESHOLD 5

/* Actions used when dealing with walls */
#define MAKE_NORM_WALL  0
#define MAKE_DEST_WALL  1
#define DESTROY_WALL    2
#define CONTINUE        3

/* Interface locations and fonts */
#define INFO_Y   43
#define INPUT_Y  46
#define MD_FONT  M_FONT


typedef unsigned int Wall;

typedef struct
{
    int   x, y;	/* coordinates of a place in terms of boxes */
} BoxC;

typedef struct
{
    int   x, y;	/* coordinates of a place in terms of pixels */
} PixC;
