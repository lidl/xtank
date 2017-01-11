/*-
 * Copyright (c) 1988 Terry Donahue
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
** Utility to convert 3d objects into optimized C code to do hidden line
** removal.  3d objects are in the following format:
**
** #points
** x y z
** x y z
** ...
** #lines
** p0 p1
** p0 p1
** ...
** #faces
** #edges e0 e1 e2 ...
** #edges e0 e1 e2 ...
**
** Objects must have the property that a given line is either totally
** visible or totally obscured when the object is viewed from the z=k plane.
*/

#include <stdio.h>
#include "obj3d.h"

/*
** Loads in the .3d object file given by the first argument, converts it
** into C code, saving it with a .c extension.
*/
main(argc, argv)
    int argc;
    char **argv;
{
    Obj3d obj;
    char code[MAX_CODE], name[80];

    if (argc != 2) {
	fprintf(stderr, "Usage: %s name, where name.3d is an Obj3d file", argv[0]);
	exit(1);
    }
    (void) strcpy(name, argv[1]);
    (void) strcat(name, ".3d");
    load_obj3d(&obj, name);

    optimize_obj3d(obj);

    (void) strcpy(name, argv[1]);
    (void) strcat(name, ".opt");
    save_obj3d(&obj, name);
}

/*
** Loads the object with the given name.
*/
load_obj3d(obj, filename)
    Obj3d *obj;
    char *filename;
{
    FILE *f;
    Face3d *face;
    char *buf[MAX_BUF];
    char *ptr;
    int i, j;

    f = fopen(filename, "r");

    /* Read in points */
    fgets(f, MAX_BUF, buf);
    obj->num_points = num_atoi(buf);
    obj->point = malloc(obj->num_points * sizeof(Point3d));
    for (i = 0; i < obj->num_points; i++) {
	fgets(f, MAX_BUF, buf);
	sscanf(buf, "%f %f %f", &obj->point[i].x, &obj->point[i].y, &obj->point[i].z);
    }

    /* Read in lines */
    fgets(f, MAX_BUF, buf);
    obj->num_lines = num_atoi(buf);
    obj->point = malloc(obj->num_lines * sizeof(Line3d));
    for (i = 0; i < obj->num_lines; i++) {
	fgets(f, MAX_BUF, buf);
	sscanf(buf, "%d %d", &obj->line[i].p1, &obj->line[i].p2);
    }

    /* Read in faces */
    fgets(f, MAX_BUF, buf);
    obj->num_faces = atoi(buf);
    obj->face = malloc(obj->num_faces * sizeof(Face3d));
    for (i = 0; i < obj->num_faces; i++) {
	face = &obj->face[i];
	fgets(f, MAX_BUF, buf);
	face->num_edges = atoi(buf);
	face->edge = malloc(face->num_edges * sizeof(int));
	ptr = buf;
	for (j = 0; j < face->num_edges; j++) {
	    /* Skip to next field by passing over whitespace and next number */
	    while (*ptr == ' ' || *ptr == '\t')
		ptr++;
	    while (*ptr != ' ' && *ptr != '\t')
		ptr++;
	    face->edge[j] = atoi(ptr);
	}
    }

    fclose(f);
}

/*
** Optimizes the 3d object for view from the z=0 plane.  All faces of
** the object are extended and their intersection lines with the z=0 plane
** are computed.  The intersection points of these lines are also computed.
** These lines divide the plane into various areas.  In each area, the
** lines of the object that can be seen are constant.  This also implies that
** the set of points to be transformed, the bounding polygon of the lines,
** and bounding box of the polygon are constant.  This constant information
** is precomputed for each view.  A binary tree of lines is also computed
** to efficiently compute which view is visible.
** Returns -1 if object has a face with no normal.
*/
optimize_obj3d(obj)
    Obj3d *obj;
{
    FaceVis *facevis;
    FaceVis **faceline;
    LineVis *linevis[];
    int num_facelines;

    /* Compute visibility state from z=0 plane for every face */
    facevis = (FaceVis *) malloc(obj->num_faces * sizeof(FaceVis));
    faceline = (FaceVis **) malloc(obj->num_faces * sizeof(FaceVis *));
    if (compute_facevis(obj, facevis, faceline, &num_facelines) == -1)
	return -1;

    /* * Create a table of line intersections.  If two lines do not intersect, *
       determine which is to the left of which. */
    linevis = (LineVis **) malloc(num_facelines * sizeof(LineVis *));
    compute_linevis(obj, linevis, faceline, num_facelines);

    /* * Ignore a line if the current region doesn't contain a part of the
       line. * Intersections of that line with all the boundary lines of the
       region. * This can happen in two ways: *
    
    *    1.  The line is parallel to a boundary line of the region, but *
       on the wrong side. *    2.  Every intersection of the line with another
       boundary line is *        outside the region. *
    
    * A list of active lines is kept.  Every time we move down one level * in
       the tree, two things happen: *
    
    *    1.  A line is chosen from that list. *    2.  All active lines are
       checked agains th chosen line. *        a.  Those on the wrong side of
       it are eliminated. *        b.  Those which intersect it at a point
       outside the *
    
    */

}

/*
** Computes visibility states from z=0 plane for every face in the given
** object.  Puts facevis structures in facevis, and creates a list of
** the facevis structures with lines in faceline.
** Returns -1 if a face has no normal.
*/
compute_facevis(obj, facevis, faceline, num_facelines)
    Obj3d *obj;
    FaceVis *facevis;
    FaceVis **faceline;
    int *num_facelines;
{
    int i;
    int p1, p2, p3;
    float x, y, z;
    float dx1, dx2, dy1, dy2, dz1, dz2;

    *num_facelines = 0;
    for (i = 0; i < obj->num_faces; i++) {
	face = &obj->face[i];

	/* Get 3 non-colinear points on the face */
	p1 = face->edge[0].p1;
	p2 = face->edge[0].p2;
	if (p1 == p2)
	    return -1;

	p3 = face->edge[1].p1;
	if (p3 == p2 || p3 == p1)
	    p3 = face->edge[1].p2;
	if (p3 == p2 || p3 == p1)
	    return -1;

	/* Compute normal to the face pointing towards visible side */
	x = obj->point[p1].x;
	y = obj->point[p1].y;
	z = obj->point[p1].z;
	dx1 = obj->point[p2].x - x;
	dx2 = obj->point[p3].x - x;
	dy1 = obj->point[p2].y - y;
	dy2 = obj->point[p3].y - y;
	dz1 = obj->point[p2].z - z;
	dz2 = obj->point[p3].z - z;
	nx = dy1 * dz2 - dy2 * dz1;
	ny = dz1 * dx2 - dz2 * dx1;
	nz = dx1 * dy2 - dx2 * dy1;

	if (nx == 0.0 && ny == 0.0) {
	    /* Perpendicular to z=0 plane; Either always or never visible */
	    facevis[i].vis = ((z * nz < 0) ? ALWAYS : NEVER);
	} else {
	    /* Intersects z=0 plane; viewable when ax + by > c */
	    facevis[i].a = nx;
	    facevis[i].b = ny;
	    facevis[i].c = nx * x + ny * y + nz * z;
	    facevis[i].vis = SOMETIMES;

	    /* Save index of this facevis in the faceline array */
	    faceline[(*num_facelines)++] = &facevis[i];
	}
    }
}

/*
** Puts a table of line intersection points in linevis with a width
** of num_facelines.  Each entry says whether a given pair of lines
** intersect, or are parallel.  If they intersect, the intersection
** point is given.  If they are parallel, their ordering is determined.
*/
compute_linevis(obj, linevis, num_facelines)
    Obj3d *obj;
    LineVis *linevis;
    FaceVis **faceline;
    int num_facelines;
{
    int i, j;
    float a1, b1, c1, a2, b2, c2, det;

    for (i = 0; i < num_facelines; i++) {
	linevis[i] = (LineVis *) malloc(num_facelines * sizeof(LineVis));
	a1 = faceline[i]->a;
	b1 = faceline[i]->b;
	c1 = faceline[i]->c;

	/* A line visible from itself */
	linevis[i][i].vis = NEVER;

	/* Fill in the upper right and lower left triangles of the table */
	for (j = 0; j < i; j++) {
	    a2 = faceline[j]->a;
	    b2 = faceline[j]->b;
	    c2 = faceline[j]->c;

	    det = a1 * b2 - a2 * b1;
	    if (det == 0.0) {
		/* Lines are parallel, so determine which side each is on */
		linevis[i][j].vis = ((c2 > c1) ? ALWAYS : NEVER);
		linevis[j][i].vis = ((c1 > c2) ? ALWAYS : NEVER);
	    } else {
		/* Lines intersect, so determine point of intersection */
		linevis[i][j].x = (c1 * b2 - c2 * b1) / det;
		linevis[i][j].y = (c1 * a2 - c2 * a1) / det;
		linevis[i][j].vis = SOMETIMES;

		linevis[j][i] = linevis[i][j];
	    }
	}
    }
}

/*
** Saves the 3d object into a file.
*/
save_obj3d(obj, filename)
    Obj3d *obj;
    char *filename;
{
}
