/* maze.h - part of Xtank */

#ifndef _MAZE_H_
#define _MAZE_H_

#include "game.h"
#include "types.h"

typedef struct {
    Game  type;
    char *name;
    char *designer;
    char *desc;
    Byte *data;
} Mdesc;

typedef struct {
    int num_starts[MAX_TEAMS];	/* number of start locs for each team */
    Coord start[MAX_TEAMS][MAX_VEHICLES];	/* coordinates of starting
						   locations */
} Maze;


#endif ndef _MAZE_H_
