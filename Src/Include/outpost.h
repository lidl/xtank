/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

/* How large an outpost is */
#define OUTPOST_WIDTH  30
#define OUTPOST_HEIGHT 30

/* these two numbers should be powers of 2 for the get_outpost_coord() macro to
   work): */
#define OUTPOST_PATS   4		/* number of outpost motion patterns */
#define OUTPOST_FRAMES 32		/* how long it takes each pattern to cycle */
#define get_outpost_coord(b,fr) \
  (&outpost_coord[((b)->strength) & (OUTPOST_PATS-1)] \
                 [(fr) & (OUTPOST_FRAMES-1)])

extern Coord outpost_coord[OUTPOST_PATS][OUTPOST_FRAMES];
