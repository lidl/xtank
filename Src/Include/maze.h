/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** $Id$
*/

#ifndef _MAZE_H_
#define _MAZE_H_

#include "game.h"
#include "tanktypes.h"

  typedef struct {
	  Game type;
	  char *name;
	  char *designer;
	  char *desc;
	  Byte *data;
  }
Mdesc;

  typedef struct {
	  int num_starts[MAX_TEAMS];/* number of start locs for each team */
	  Coord start[MAX_TEAMS][MAX_VEHICLES];	/* coordinates of starting
						   locations */
  }
Maze;


#endif /* ndef _MAZE_H_ */
