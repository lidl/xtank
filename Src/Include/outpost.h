/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** outpost.h
*/

/*
$Author: rpotter $
$Id: outpost.h,v 2.3 1991/02/10 13:51:28 rpotter Exp $

$Log: outpost.h,v $
 * Revision 2.3  1991/02/10  13:51:28  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:46  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:44  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:19  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:01:57  aahz
 * Initial revision
 * 
*/

/* How large an outpost is */
#define OUTPOST_WIDTH  30
#define OUTPOST_HEIGHT 30

/* these two numbers should be powers of 2 for the get_outpost_coord() macro to
   work): */
#define OUTPOST_PATS   4	/* number of outpost motion patterns */
#define OUTPOST_FRAMES 32	/* how long it takes each pattern to cycle */
#define get_outpost_coord(b,fr) \
  (&outpost_coord[((b)->strength) & (OUTPOST_PATS-1)] \
                 [(fr) & (OUTPOST_FRAMES-1)])

extern Coord outpost_coord[OUTPOST_PATS][OUTPOST_FRAMES];
