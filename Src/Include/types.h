/* types.h - simple typedefs that are used in many places */

/*
$Author: rpotter $
$Id: types.h,v 2.3 1991/02/10 13:51:52 rpotter Exp $

$Log: types.h,v $
 * Revision 2.3  1991/02/10  13:51:52  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:11  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:15  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:40  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:12  aahz
 * Initial revision
 * 
*/

#ifndef _TYPES_H_
#define _TYPES_H_


typedef enum {False = 0, True = 1} Boolean;
#define FALSE ((int)False)	/* lint likes these better :-) */
#define TRUE ((int)True)
typedef unsigned char Byte;
typedef unsigned int Flag;	/* a set of bits */
typedef float Angle;

typedef struct {
    short x, y;
} Coord;

/* directions of rotation */
typedef enum {
    COUNTERCLOCKWISE = -1, NO_SPIN = 0, CLOCKWISE = 1, TOGGLE = 2
} Spin;


#endif ndef _TYPES_H_
