/* object.h - structures describing graphic objects */

/*
$Author: lidl $
$Id: object.h,v 2.6 1992/08/31 01:50:45 lidl Exp $

$Log: object.h,v $
 * Revision 2.6  1992/08/31  01:50:45  lidl
 * changed to use tanktypes.h, instead of types.h
 *
 * Revision 2.5  1992/04/09  04:18:45  lidl
 * changed to use tanklimits.h and not limits.h
 *
 * Revision 2.4  1991/12/10  01:21:04  lidl
 * change all occurances of "float" to "FLOAT"
 *
 * Revision 2.3  1991/02/10  13:51:26  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:43  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:39  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:17  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:55  aahz
 * Initial revision
 * 
*/

#ifndef _OBJECT_H_
#define _OBJECT_H_


#include "tanktypes.h"
#include "xtanklib.h"
#include "tanklimits.h"


/* Indices for all the objects in the random_obj array */
#define XTANK_OBJ 0
#define TEAM_OBJ 1
#define TERP_OBJ 2
#define MAX_RANDOM_OBJS 3


typedef struct {
    int   x1;
    int   y1;
    int   x2;
    int   y2;			/* The rest of the stuff is for SPEED */
    int   dx;			/* x2-x1 */
    int   dy;			/* y2-y1 */
    FLOAT slope;		/* dy/dx */
    int   intercept;		/* y1 - slope * x1 */
    int   minx;			/* min(x1,x2) */
    int   miny;			/* min(y1,y2) */
    int   maxx;			/* max(x1,x2) */
    int   maxy;			/* max(y1,y2) */
} Segment;

typedef struct {
    Coord turret_coord[MAX_TURRETS];	/* relative to center */
    Segment segment[MAX_SEGMENTS];	/* polygon shaped to the picture */
} Picinfo;

typedef struct {
    int   width;
    int   height;
    int   offset_x;
    int   offset_y;
    int   pixmap;
} Picture;

typedef struct {
    char  type[MAX_STRING];	/* type of object */
    int   num_pics;		/* number of picture in the object */
    Picture *pic;		/* array of pictures of the object */
    int   num_turrets;		/* number of turrets in object */
    int   num_segs;		/* number of segments to represent object */
    Picinfo *picinfo;		/* array of info about pictures */
} Object;

extern int num_vehicle_objs, num_exp_objs;
extern Object *vehicle_obj[], *bullet_obj, *exp_obj[], *landmark_obj[],
    *random_obj[];

#endif ndef _OBJECT_H_
