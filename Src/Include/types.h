/* types.h - simple typedefs that are used in many places */

/*
$Author: lidl $
$Id: types.h,v 2.5 1992/01/29 08:39:11 lidl Exp $

$Log: types.h,v $
 * Revision 2.5  1992/01/29  08:39:11  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.4  1991/12/10  01:21:04  lidl
 * change all occurances of "float" to "FLOAT"
 *
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
typedef FLOAT Angle;

typedef struct {
    short x, y;
} Coord;

#ifndef NO_NEW_RADAR
typedef struct {
    long x, y;
} lCoord;
#endif /* !NO_NEW_RADAR */

/* directions of rotation */
typedef enum {
    COUNTERCLOCKWISE = -1, NO_SPIN = 0, CLOCKWISE = 1, TOGGLE = 2
} Spin;


#endif ndef _TYPES_H_
