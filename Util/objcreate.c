/*
** objcreate.c
**
** This program uses an info file and bitmap files to create
** an xtank vehicle object file.
**
** Needs the following files to run:
**
** body.info
** body0
** body1
** body2
** body3
**
**
** Execute the following command to create body.obj
**
** objcreate body
*/

#include <stdio.h>
#include <math.h>

#define PI 3.1415926535897932384626433832795028841971
#define MAX_VIEWS 32
#define MAX_BITMAPS 32
#define MAX_TURRETS 4
#define MAX_SEGMENTS 6
#define BODY_BITMAPS 4
#define BODY_VIEWS 16
#define TRUE 1
#define FALSE 0

#define min(x,y) (((x) < (y)) ? (x) : (y))
#define max(x,y) (((x) > (y)) ? (x) : (y))
#define abs(x) (((x) > 0) ? (x) : -(x))

typedef int Boolean;

typedef struct _Coord {
    int x;
    int y;
} Coord;

typedef struct _Segment {
    int x1;
    int y1;
    int x2;
    int y2;
    int dx;			/* x2-x1 */
    int dy;			/* y2-y1 */
    float slope;		/* dy/dx */
    int intercept;		/* y1 - slope * x1 */
    int minx;			/* min(x1,x2) */
    int miny;			/* min(y1,y2) */
    int maxx;			/* max(x1,x2) */
    int maxy;			/* max(y1,y2) */
} Segment;

typedef struct _Picture {
    int width;
    int height;
    int offset_x;
    int offset_y;
} Picture;

typedef struct _Picinfo {
    Coord turret_coord[MAX_TURRETS];
    Segment segment[MAX_SEGMENTS];
} Picinfo;

typedef struct _Object {
    char type[12];		/* type of object */
    int num_pics;		/* number of picture in the object */
    Picture *pic;		/* array of pictures of the object */
    int num_turrets;		/* number of turrets in object */
    int num_segs;		/* number of segments to represent object */
    Picinfo *picinfo;		/* array of info about pictures */
} Object;

char line[81];
char name[30];
char ending[10];
char filename[50];

Picinfo picinfo[MAX_VIEWS];
Picture pic[MAX_VIEWS];
Coord turret[MAX_TURRETS];
Segment segment[MAX_SEGMENTS];

main(argc, argv)
    int argc;
    char *argv[];
{
    FILE *info_file, *object_file, *bitmap_file;
    double rot_angle;
    Segment seg;
    Boolean body;
    char yn;
    int num_bitmaps, num_views;
    int num_turrets, num_segs;
    int x, y, x1, y1, x2, y2;
    int i, j, k;

    /* Parse command line for object name */
    if (argc > 1)
	strncpy(name, argv[1], 29);
    else {
	fprintf(stderr, "Object filename required.\n");
	exit(1);
    }

    /* Find out what kind of object we are dealing with */
    printf("Is this object a body? (y/n): ");
    scanf("%c", &yn);

    if (yn == 'y') {
	body = TRUE;
	num_bitmaps = BODY_BITMAPS;
	num_views = BODY_VIEWS;

	/* Object is a vehicle, so read info from info file */
	strcpy(filename, name);
	strcat(filename, ".info");
	if ((info_file = fopen(filename, "r")) == NULL) {
	    fprintf(stderr, "Could not open info file %s.\n", filename);
	    exit(1);
	}
	fgets(line, 80, info_file);
	sscanf(line, "%d", &num_turrets);
	if (num_turrets > 0)
	    for (i = 0; i < num_turrets; i++) {
		fgets(line, 80, info_file);
		sscanf(line, "%d %d", &turret[i].x, &turret[i].y);
	    }

	fgets(line, 80, info_file);
	sscanf(line, "%d", &num_segs);
	if (num_segs > 0)
	    for (i = 0; i < num_segs; i++) {
		fgets(line, 80, info_file);
		sscanf(line, "%d %d", &segment[i].x1, &segment[i].y1);
	    }

	fclose(info_file);
    } else {
	body = FALSE;

	/* Object is not a body, so ask how many bitmaps there are */
	printf("Enter number of bitmaps in object: ");
	scanf("%d", &num_bitmaps);
	if (num_bitmaps <= 0 || num_bitmaps > MAX_BITMAPS) {
	    fprintf(stderr, "Illegal number of bitmaps.\n");
	    exit(1);
	}
	num_views = num_bitmaps;
    }

    /* Create object file */
    strcpy(filename, name);
    strcat(filename, ".obj");
    if ((object_file = fopen(filename, "w")) == NULL) {
	fprintf(stderr, "Could not open object file %s.\n", filename);
	exit(1);
    }
    /* Add header and # views to object file */
    fprintf(object_file, "/*\n");
    fprintf(object_file, "** Information about the %s object\n", name);
    fprintf(object_file, "*/\n");
    fprintf(object_file, "\n");
    fprintf(object_file, "#define %s_views %d\n", name, num_views);
    fprintf(object_file, "\n");

    /* If object is a body, add picinfo structure to object file */
    if (body) {
	fprintf(object_file, "Picinfo %s_picinfo[%s_views] = {\n", name, name);
	for (i = 0; i < num_views; i++) {
	    fprintf(object_file, "\t{ { ");

	    /* put in dummy value if there are no turrets */
	    if (num_turrets == 0) {
		fprintf(object_file, "{0,0}");
	    } else {
		for (j = 0; j < num_turrets; j++) {
		    rotate_point(turret[j].x, turret[j].y, &x, &y, (double) (2 * PI * i) / num_views);
		    fprintf(object_file, "{%d,%d}", x, y);
		    if (j != num_turrets - 1)
			fprintf(object_file, ", ");
		}
	    }
	    fprintf(object_file, " },\n");

	    fprintf(object_file, "\t  { ");
	    for (j = 0; j < num_segs; j++) {
		rotate_point(segment[j].x1, segment[j].y1, &x1, &y1, 2 * PI * i / num_views);
		k = (j + 1) % num_segs;
		rotate_point(segment[k].x1, segment[k].y1, &x2, &y2, 2 * PI * i / num_views);
		make_segment(&seg, x1, y1, x2, y2);
		fprintf(object_file, "{%d,%d,%d,%d,%d,%d,%f,%d,%d,%d,%d,%d}",
		   seg.x1, seg.y1, seg.x2, seg.y2, seg.dx, seg.dy, seg.slope,
		      seg.intercept, seg.minx, seg.miny, seg.maxx, seg.maxy);
		if (j != num_segs - 1)
		    fprintf(object_file, ",\n\t    ");
	    }
	    fprintf(object_file, " }");
	    if (i != num_views - 1)
		fprintf(object_file, " },\n");
	    else
		fprintf(object_file, " }\n");
	}
	fprintf(object_file, "};\n\n");
    }
    /* Get bitmap sizes from the bitmaps */
    for (i = 0; i < num_bitmaps; i++) {
	strcpy(filename, name);
	sprintf(ending, "%d", i);
	strcat(filename, ending);
	if ((bitmap_file = fopen(filename, "r")) == NULL) {
	    fprintf("Could not open bitmap file %s.\n", filename);
	    exit(1);
	}
	fgets(line, 80, bitmap_file);	/* width line */
	sscanf(line, "#define %*s %d", &pic[i].width);
	pic[i].offset_x = pic[i].width / 2;

	fgets(line, 80, bitmap_file);	/* height line */
	sscanf(line, "#define %*s %d", &pic[i].height);
	pic[i].offset_y = pic[i].height / 2;

	fclose(bitmap_file);
    }

    /* Add picture structure to object file */
    fprintf(object_file, "Picture %s_pic[%s_views] = {\n", name, name);
    for (i = 0; i < num_bitmaps; i++) {
	fprintf(object_file, "\t{ %d, %d, %d, %d }", pic[i].width, pic[i].height,
		pic[i].offset_x, pic[i].offset_y);
	if (i != num_bitmaps - 1)
	    fprintf(object_file, ",\n");
	else
	    fprintf(object_file, "\n");
    }
    fprintf(object_file, "};\n\n");

    /* Add object structure to object file */
    fprintf(object_file, "Object %s_obj = {\n", name);
    fprintf(object_file, "\t%c%s%c,\n", '"', name, '"');
    fprintf(object_file, "\t%s_views,\n", name);
    fprintf(object_file, "\t%s_pic%s\n", name, body ? "," : "");
    if (body) {
	fprintf(object_file, "\t%d,\t\t\t/* num_turrets */\n", num_turrets);
	fprintf(object_file, "\t%d,\t\t\t/* num_segs */\n", num_segs);
	fprintf(object_file, "\t%s_picinfo\n", name);
    }
    fprintf(object_file, "};\n\n");

    /* Add all bitmaps from bitmap files to object file */
    for (i = 0; i < num_bitmaps; i++) {
	strcpy(filename, name);
	sprintf(ending, "%d", i);
	strcat(filename, ending);
	if ((bitmap_file = fopen(filename, "r")) == NULL) {
	    fprintf("Could not open bitmap file %s.\n", filename);
	    exit(1);
	}
	fgets(line, 80, bitmap_file);	/* width line */
	fgets(line, 80, bitmap_file);	/* height line */
	fgets(line, 80, bitmap_file);	/* static unsigned char decl. line */

	/* Replace declaration with something that makes sense */
	/* fprintf(object_file,"static short %s%d_bits[] = {\n",name,i); */
	fprintf(object_file, "static unsigned char %s%d_bits[] = {\n", name, i);

	/* Copy the shorts quickly */
	while (fgets(line, 80, bitmap_file) != NULL)
	    fputs(line, object_file);
	fputs("\n", object_file);

	fclose(bitmap_file);
    }

    /* Add bitmap ordering to object file */
    fprintf(object_file, "unsigned char *%s_bitmap[%s_views] = {\n", name, name);
    for (i = 0; i < num_bitmaps; i++) {
	sprintf(line, "\t%s%d_bits", name, i);
	if (i < num_bitmaps - 1)
	    strcat(line, ",\n");
	else
	    strcat(line, "\n");
	fputs(line, object_file);
    }
    fprintf(object_file, "};\n");

    fclose(object_file);
}


rotate_point(x1, y1, x2, y2, rot_angle)
    int x1, y1, *x2, *y2;
    double rot_angle;
{
    double r, angle, x, y;

    r = hypot((double) x1, (double) y1);
    angle = atan2((double) y1, (double) x1) + rot_angle;

    x = r * cos(angle);
    if (x > 0)
	*x2 = (int) (x + .5);
    else
	*x2 = (int) (x - .5);

    y = r * sin(angle);
    if (y > 0)
	*y2 = (int) (y + .5);
    else
	*y2 = (int) (y - .5);
}




/* This function makes a full-blown segment data structure out of */
/* two coordinates. */

make_segment(seg, x1, y1, x2, y2)
    Segment *seg;
    int x1, y1, x2, y2;
{
    seg->x1 = x1;
    seg->x2 = x2;
    seg->y1 = y1;
    seg->y2 = y2;

    seg->dx = x2 - x1;
    seg->dy = y2 - y1;

    if (seg->dx != 0)
	seg->slope = (float) seg->dy / (float) seg->dx;
    else {
	if (seg->dy > 0)
	    seg->slope = 9999999.0;
	else if (seg->dy < 0)
	    seg->slope = -9999999.0;
	else
	    seg->slope = 0;
    }

    seg->intercept = y1 - seg->slope * x1;

    seg->minx = min(x1, x2);
    seg->miny = min(y1, y2);
    seg->maxx = max(x1, x2);
    seg->maxy = max(y1, y2);
}
