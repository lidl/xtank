/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "xtank.h"
#include "proto.h"

extern Settings settings;
extern Map real_map;

#define CPY(s) (strcpy(malloc((unsigned)strlen(s)+1),s))

/*
** Puts a maze into the box array.  If there is a maze name, it uses
** that maze.  Otherwise a random one is created.
*/
void
setup_maze(void)
{
	extern Maze maze;
	extern int num_terminals;
	FLOAT base_prob;
	FLOAT prob[SLOW + 1], sum;
	int i;

	if (settings.mdesc != (Mdesc *) NULL) {
		/* Make the maze from the maze description */
		make_maze(settings.mdesc);
	} else {
		/* No maze description so make a random maze of the proper
	   density */
		make_random_maze_walls();
		remove_walls(100 - settings.maze_density);
		make_dest_walls(20);

		/* Base prob of a flag increases with number of players */
		base_prob = .007 + .003 * num_terminals;

		/* Set the probs for box flags */
		prob[(int) FUEL] = base_prob;
		prob[(int) AMMO] = base_prob;
		prob[(int) ARMOR] = base_prob;
		prob[(int) PEACE] = base_prob;
		prob[(int) GOAL] = (settings.si.game == COMBAT_GAME ||
							settings.si.game == WAR_GAME) ?
		  0 : base_prob;
		prob[(int) OUTPOST] = base_prob;
		for (i = (int) SCROLL_N; i <= (int) SCROLL_NW; i++)
			prob[i] = base_prob / 2;
		prob[(int) SLIP] = 2 * base_prob;
		prob[(int) SLOW] = base_prob;

		/* Compute the sum of the probabilities so far */
		for (sum = 0.0, i = (int) FUEL; i <= (int) SLOW; i++)
			sum += prob[i];

		prob[(int) NORMAL] = 1 - sum;

		set_box_types((int) SLOW + 1, prob);


		/* Clear the starting location array */
		for (i = 0; i < MAX_TEAMS; i++)
			maze.num_starts[i] = 0;
	}

	/* Set strengths of outposts and find starting locations */
	process_maze();
}

/*
** Makes a maze structure from the specified maze description.
*/
void
make_maze(Mdesc *d)
{
	static Box empty_box =
	{0, NORMAL, 0};
	Box *b;
	Byte flags, *dptr;
	int num_empties;
	int i, j;

	/* For each box there is a byte that contains 8 flags.  If EMPTY_BOXES
       is set, the remaining 7 bits give the number of empty boxes
       (excluding this one) to make before reading the next byte.
       Otherwise, If TYPE_EXISTS is set, the next byte is the box type.
       Otherwise, the type is 0. If TEAM_EXISTS is set, the next byte is
       the box team.  Otherwise, the team is NEUTRAL. */
	num_empties = 0;
	dptr = d->data;
	for (i = 0; i < GRID_WIDTH; i++)
		for (j = 0; j < GRID_HEIGHT; j++) {
			b = &real_map[i][j];

			/* Are we making empty boxes? */
			if (num_empties > 0) {
				*b = empty_box;
				num_empties--;
				continue;
			}
			flags = *(dptr++);

			/* Check for empty box flag */
			if (flags & EMPTY_BOXES) {
				*b = empty_box;
				num_empties = (int) (flags & ~EMPTY_BOXES);
			} else {
				/* Get the box values */
				b->flags = flags & MAZE_FLAGS;
				b->type = (flags & TYPE_EXISTS) ? (LandmarkType) *(dptr++) :
				  NORMAL;

				b->teleport_code = (flags & TYPE_EXISTS &&
								  b->type == TELEPORT) ? (Byte) * (dptr++) :
				  NORMAL;

				b->team = (flags & TEAM_EXISTS) ? *(dptr++) : NEUTRAL;
			}
		}
}

/*
** Makes a mdesc structure from the current maze.
*/
void
build_mdesc(Mdesc *d, Game type, char *name, char *designer, char *desc)
{
	Byte temp[GRID_WIDTH * GRID_HEIGHT * 4];
	Box *b;
	Byte flags, *dptr;
	int num_empties, num_bytes;
	int i, j;

	/* Go through the boxes, counting empties if they occur, and adding a byte
       to temp when they do not.  Once all the boxes have been coded, malloc
       the proper amount of space and copy * the data bytes over. */
	num_empties = 0;
	dptr = temp;
	for (i = 0; i < GRID_WIDTH; i++)
		for (j = 0; j < GRID_HEIGHT; j++) {
			b = &real_map[i][j];

			flags = b->flags |
			  ((b->type != NORMAL) ? TYPE_EXISTS : 0) |
			  ((b->team != NEUTRAL) ? TEAM_EXISTS : 0);

			/* Is this an empty box? */
			if (flags == 0) {
				if (num_empties == 0) {
					/* Start an empty box byte and increment number of
					   empties */
					*dptr = EMPTY_BOXES;
					num_empties++;
				} else if (num_empties == 128) {
					/* Last empty box byte is full, so start a new one */
					*(++dptr) = EMPTY_BOXES;
					num_empties = 1;
				} else {
					/* Add another empty to the current empty box byte */
					(*dptr)++;
					num_empties++;
				}
			} else {
				if (num_empties > 0) {
					/* Move on to the next byte and clear num_empties */
					dptr++;
					num_empties = 0;
				}
				*(dptr++) = flags;
				if (flags & TYPE_EXISTS) {
					*(dptr++) = (Byte) b->type;
					if (b->type == TELEPORT)
						*(dptr++) = b->teleport_code;
				}
				if (flags & TEAM_EXISTS)
					*(dptr++) = b->team;
			}
		}

	/* If we are in the middle of an empty box byte, finish it */
	if (num_empties > 0)
		dptr++;

	/* Copy the data, placing a final '\0' at the end */
	num_bytes = (int) (dptr - temp);
	d->data = (Byte *) malloc((unsigned) num_bytes + 1);
	for (i = 0; i < num_bytes; i++)
		d->data[i] = temp[i];
	d->data[num_bytes] = '\0';

	/* Copy the rest of the fields */
	d->type = type;
	d->name = CPY(name);
	d->designer = CPY(designer);
	d->desc = CPY(desc);
}

#define nullify(ptr) \
    (ptr != NULL && (free((char *) ptr), \
		     ptr = NULL))

/*
** Clears a maze desc by freeing the strings and setting type to COMBAT_GAME.
*/
void
clear_mdesc(Mdesc *d)
{
	d->type = COMBAT_GAME;
	nullify(d->name);
	nullify(d->designer);
	nullify(d->desc);
	nullify(d->data);
}

/*
** Finds the starting locations in the maze, setting them to type NORMAL,
** and putting their coordinates into the appropriate starting location
** array.  If oupost strength is set to 0, outposts are removed from the
** maze.  Otherwise, their strenghts are determined.
*/
void
process_maze(void)
{
	extern Maze maze;
	Box *b;
	Coord *c;
	int *num;
	int strength4;
	int i, j;

	/* Clear the starting location array */
	for (i = 0; i < MAX_TEAMS; i++)
		maze.num_starts[i] = 0;

	strength4 = settings.si.outpost_strength << 2;

	for (i = 0; i < GRID_WIDTH; i++)
		for (j = 0; j < GRID_HEIGHT; j++) {
			b = &real_map[i][j];

			if (b->type == START_POS) {
				/* Add starting location to team array if there is room */
				num = &maze.num_starts[b->team];
				if (*num < MAX_TEAMS - 1) {
					c = &maze.start[b->team][(*num)++];
					c->x = i;
					c->y = j;
				}
				b->type = NORMAL;
			} else if (b->type == OUTPOST) {
				if (strength4 == 0)
					b->type = NORMAL;
				else
					b->strength = 1 + rnd(strength4);
			}
		}
}

#define UNTOUCHED	0
#define NEXT_TO_MAZE	1
#define PART_OF_MAZE	2
#define OFF_LIMITS	3

/*
** Creates a random maze of walls into the box array.  Also clears
** the types and teams of all boxes to 0, and determines insideness
** for each box.
*/
void
make_random_maze_walls(void)
{
	static Box empty_box =
	{0, NORMAL, 0};
	static Box north_box =
	{NORTH_WALL, NORMAL, 0};
	static Box west_box =
	{WEST_WALL, NORMAL, 0};
	static Box both_box =
	{NORTH_WALL | WEST_WALL, NORMAL, 0};
	static int dx[4] =
	{0, 1, 0, -1};
	static int dy[4] =
	{-1, 0, 1, 0};
	Box *b;
	Byte box_state[GRID_WIDTH][GRID_HEIGHT], *bs;
	Coord adjacent[MAZE_WIDTH * MAZE_HEIGHT];
	Coord cell;
	short int x, y, nx, ny;
	short int north, south, west, east;
	short int choice, chosen;
	int i, j;
	int indx;
	int size;

	/* initialize boxes and box states */
	for (i = 0; i < GRID_WIDTH; i++)
		for (j = 0; j < GRID_HEIGHT; j++) {
			b = &real_map[i][j];
			bs = &box_state[i][j];

			if (i >= MAZE_LEFT && i <= MAZE_RIGHT && j >= MAZE_TOP &&
				j <= MAZE_BOTTOM) {
				*b = both_box;
				*bs = UNTOUCHED;
			} else {
				if (i == MAZE_RIGHT + 1 && j >= MAZE_TOP && j <= MAZE_BOTTOM)
					*b = west_box;
				else if (j == MAZE_BOTTOM + 1 && i >= MAZE_LEFT &&
						 i <= MAZE_RIGHT)
					*b = north_box;
				else
					*b = empty_box;
				*bs = OFF_LIMITS;
			}
		}

	/* Pick a random cell to start in */
	cell.x = (x = rnd(MAZE_WIDTH) + MAZE_LEFT);
	cell.y = (y = rnd(MAZE_HEIGHT) + MAZE_TOP);

	/* Make it part of the maze, and add its neighbors to the adjacent list */
	size = 0;
	box_state[x][y] = PART_OF_MAZE;
	real_map[x][y].flags |= INSIDE_MAZE;

	for (i = 0; i < 4; i++) {
		nx = x + dx[i];
		ny = y + dy[i];

		bs = &box_state[nx][ny];
		if (*bs == UNTOUCHED) {
			cell.x = nx;
			cell.y = ny;
			adjacent[++size] = cell;
			*bs = NEXT_TO_MAZE;
		}
	}

	/* Now connect the entire maze together using the following algorithm:
       Take a random cell off the adjacent list, Make the cell part of the maze
       in the box_state array Add each of its untouched neighbors to the queue
       Pick a random neighbor that is part of the maze remove the wall there Do
       it all again until the adjacent list is empty All box_statees are either
       part of maze or off limits so we're done */
	while (size) {
		indx = rnd(size) + 1;	/* pick random cell on the adjacent list */
		cell = adjacent[indx];
		adjacent[indx] = adjacent[size--];	/* delete cell from adjacent
						   list */
		x = cell.x;				/* get the x and y coords of the cell */
		y = cell.y;
		box_state[x][y] = PART_OF_MAZE;	/* make the cell part of the maze */
		real_map[x][y].flags |= INSIDE_MAZE;
		north = box_state[x][y - 1];	/* get the status of the four neighbors
					   */
		south = box_state[x][y + 1];
		west = box_state[x - 1][y];
		east = box_state[x + 1][y];

		/* Find the number of neighbors that are part of the maze and choose
	   one */
		chosen = rnd((north == PART_OF_MAZE) + (south == PART_OF_MAZE) +
					 (west == PART_OF_MAZE) + (east == PART_OF_MAZE)) + 1;
		choice = 0;

		switch (north) {
		  case PART_OF_MAZE:
			  if (chosen == ++choice)	/* if the chosen direction, remove
				       the wall */
				  real_map[x][y].flags &= ~NORTH_WALL;
			  break;
		  case UNTOUCHED:		/* if untouched, put on adjacent list */
			  box_state[x][y - 1] = NEXT_TO_MAZE;
			  cell.x = x;
			  cell.y = y - 1;
			  adjacent[++size] = cell;
		}

		switch (south) {
		  case PART_OF_MAZE:
			  if (chosen == ++choice)
				  real_map[x][y + 1].flags &= ~NORTH_WALL;
			  break;
		  case UNTOUCHED:
			  box_state[x][y + 1] = NEXT_TO_MAZE;
			  cell.x = x;
			  cell.y = y + 1;
			  adjacent[++size] = cell;
		}

		switch (west) {
		  case PART_OF_MAZE:
			  if (chosen == ++choice)
				  real_map[x][y].flags &= ~WEST_WALL;
			  break;
		  case UNTOUCHED:
			  box_state[x - 1][y] = NEXT_TO_MAZE;
			  cell.x = x - 1;
			  cell.y = y;
			  adjacent[++size] = cell;
		}

		switch (east) {
		  case PART_OF_MAZE:
			  if (chosen == ++choice)
				  real_map[x + 1][y].flags &= ~WEST_WALL;
			  break;
		  case UNTOUCHED:
			  box_state[x + 1][y] = NEXT_TO_MAZE;
			  cell.x = x + 1;
			  cell.y = y;
			  adjacent[++size] = cell;
		}
	}
}

/*
** Decreases the number of walls in the maze by the specified percentage.
*/
void
remove_walls(int percent)
{
	int i, j;

	for (i = MAZE_LEFT; i <= MAZE_RIGHT; i++)
		for (j = MAZE_TOP; j <= MAZE_BOTTOM; j++) {
			if ((rnd(100) < percent) && (j != MAZE_TOP))
				real_map[i][j].flags &= ~NORTH_WALL;
			if ((rnd(100) < percent) && (i != MAZE_LEFT))
				real_map[i][j].flags &= ~WEST_WALL;
		}
}

/*
** Make destructible walls at the specified percentage.
*/
void
make_dest_walls(int percent)
{
	Box *b;
	int i, j;

	for (i = MAZE_LEFT; i <= MAZE_RIGHT; i++)
		for (j = MAZE_TOP; j <= MAZE_BOTTOM; j++) {
			b = &real_map[i][j];
			if ((rnd(100) < percent) && (j != MAZE_TOP))
				if (b->flags & NORTH_WALL)
					b->flags |= NORTH_DEST;
			if ((rnd(100) < percent) && (i != MAZE_LEFT))
				if (b->flags & WEST_WALL)
					b->flags |= WEST_DEST;
		}
}

/*
** Gives boxes different types, according to the given probability array.
*/
void
set_box_types(int num_prob, FLOAT prob[])
{
	FLOAT number, pick;
	LandmarkType type;
	Box *b;
	int x, y;

	/* Assumes that the sum of the probabilities equals 1 */
	for (x = 0; x < GRID_WIDTH; x++)
		for (y = 0; y < GRID_HEIGHT; y++) {
			b = &real_map[x][y];

			if (b->flags & INSIDE_MAZE) {
				pick = rnd_interval(0.0, 1.0);

				/* Figure out which type is associated with the random pick */
				for (type = NORMAL, number = prob[(int) type];
					 number < pick && (int) type < num_prob;
					 number += prob[++type]) ;
				b->type = type;
			}
		}
}
