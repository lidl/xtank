/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#ifndef _MDESIGN_H_
#define _MDESIGN_H_

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

typedef struct {
	int x, y;	/* coordinates of a place in terms of boxes */
} BoxC;

typedef struct {
	int x, y;	/* coordinates of a place in terms of pixels */
} PixC;

#endif /* !_MDESIGN_H_ */
