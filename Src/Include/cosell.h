/* cosell.h - stuff for the commentator */

/*
$Author: rpotter $
$Id: cosell.h,v 2.3 1991/02/10 13:50:19 rpotter Exp $

$Log: cosell.h,v $
 * Revision 2.3  1991/02/10  13:50:19  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:33  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:10  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:16  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:16  aahz
 * Initial revision
 * 
*/

/* Cosell, the ultimate commentator (in the wrong sense) opcodes */
#define COS_INIT_MOUTH    0
#define COS_OWNER_CHANGE  1
#define COS_SLICK_DROPPED 2
#define COS_BIG_SMASH     3
#define COS_GOAL_SCORED   4
#define COS_BEEN_SLICKED  5

#define COS_IGNORE        0
#define COS_WALL_HIT      1
