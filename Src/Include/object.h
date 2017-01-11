/*-
 * Copyright (c) 1993 Josh Osborne
 * Copyright (c) 1993 Kurt Lidl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
** Comment: structures describing graphic objects
*/

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "tanktypes.h"
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
