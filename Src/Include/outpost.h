/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** outpost.h
*/

/*
$Author: lidl $
$Id: outpost.h,v 1.1.1.1 1995/02/01 00:25:43 lidl Exp $
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
