/* object.h - structures describing graphic objects */

#ifndef _OBJECT_H_
#define _OBJECT_H_


#include "types.h"
#include "limits.h"


typedef struct {
    int   x1;
    int   y1;
    int   x2;
    int   y2;			/* The rest of the stuff is for SPEED */
    int   dx;			/* x2-x1 */
    int   dy;			/* y2-y1 */
    float slope;		/* dy/dx */
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


#endif ndef _OBJECT_H_
