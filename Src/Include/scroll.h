/* scroll.h - part of XTank */

#define MAXSPAN 25

/*
$Author: rpotter $
$Id: scroll.h,v 2.3 1991/02/10 13:51:36 rpotter Exp $

$Log: scroll.h,v $
 * Revision 2.3  1991/02/10  13:51:36  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:55  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:54  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:27  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:03  aahz
 * Initial revision
 * 
*/

typedef struct {
	int win;		/* xtank window id */
	int x, y;		/* x & y pos of scroll bar */
	int w, h;		/* Width & Height of bar */
	int pos;		/* Index of item at top of bar */
	int span;		/* #of items that fit in the onscreen menu */
	int total;		/* #of items that fit in the virtual menu */
} scrollbar;
