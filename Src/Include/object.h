/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** Comment: structures describing graphic objects
**
** object.h
*/

/*
$Author: lidl $
$Id: object.h,v 1.1.1.1 1995/02/01 00:25:43 lidl Exp $
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
	  int x1;
	  int y1;
	  int x2;
	  int y2;					/* The rest of the stuff is for SPEED */
	  int dx;					/* x2-x1 */
	  int dy;					/* y2-y1 */
	  FLOAT slope;				/* dy/dx */
	  int intercept;			/* y1 - slope * x1 */
	  int minx;					/* min(x1,x2) */
	  int miny;					/* min(y1,y2) */
	  int maxx;					/* max(x1,x2) */
	  int maxy;					/* max(y1,y2) */
  }
Segment;

  typedef struct {
	  Coord turret_coord[MAX_TURRETS];	/* relative to center */
	  Segment segment[MAX_SEGMENTS];	/* polygon shaped to the picture */
  }
Picinfo;

  typedef struct {
	  int width;
	  int height;
	  int offset_x;
	  int offset_y;
	  int pixmap;
  }
Picture;

  typedef struct {
	  char type[MAX_STRING];	/* type of object */
	  int num_pics;				/* number of picture in the object */
	  Picture *pic;				/* array of pictures of the object */
	  int num_turrets;			/* number of turrets in object */
	  int num_segs;				/* number of segments to represent object */
	  Picinfo *picinfo;			/* array of info about pictures */
  }
Object;

extern int num_vehicle_objs, num_exp_objs, num_bullet_objs;
extern Object *vehicle_obj[], *bullet_obj[], *exp_obj[], *landmark_obj[], *random_obj[];

#endif /* ndef _OBJECT_H_ */
