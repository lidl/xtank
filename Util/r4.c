/*
** r4.c
**
** This program reads in an X10 format bitmap file and
** writes bitmap files for 4 rotations of the object.
** It does not handle hotspots and will die if the file has been changed
** from the standard X10 bitmap output.
*/

#include <stdio.h>
#include <math.h>

#define ROTATIONS 16
#define PI 3.14159265358979

int num_shorts_per_line, num_shorts;
int width, height;
int mask[16] = {0x0001, 0x0002, 0x0004, 0x0008,
    0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800,
    0x1000, 0x2000, 0x4000, 0x8000
};

main(argc, argv)
    int argc;
    char *argv[];
{
    char line[80], *malloc();
    unsigned short *rotation[ROTATIONS];
    double r, rot_angle, angle;
    double dj, dk, dx, dy;
    int x, y, x_center, y_center;
    int i, j, k;
    FILE *bitmap_file;
    char arg[50];
    char ending[10];
    char filename[50];

    /* Parse command line for bitmap filename */
    if (argc > 1)
	strcpy(arg, argv[1]);
    strcpy(filename, arg);
    if ((bitmap_file = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "Could not find bitmap file %s.\n", filename);
	exit();
    }
    /* Read info about bitmap from standard input */
    fgets(line, 80, bitmap_file);
    sscanf(line, "#define %*s %d", &width);
    fgets(line, 80, bitmap_file);
    sscanf(line, "#define %*s %d", &height);
    fgets(line, 80, bitmap_file);
    sscanf(line, "static short %*s = {");

    num_shorts_per_line = (width + 15) / 16;
    num_shorts = num_shorts_per_line * height;
    rotation[0] = (unsigned short *) malloc(2 * num_shorts);
    if (rotation[0] == NULL) {
	fprintf(stderr, "Could not allocate memory for original bitmap.\n");
	return;
    }
    for (i = 0; i < num_shorts; i++)
	fscanf(bitmap_file, "%*2s%4x%*s", &rotation[0][i]);
    fclose(bitmap_file);

    /* Compute rotations of the bitmap */
    x_center = width / 2;
    y_center = height / 2;

    for (i = 1; i < 5; i++) {
	/* Allocate memory for this rotation */
	rotation[i] = (unsigned short *) malloc(2 * num_shorts);
	if (rotation[i] == NULL) {
	    fprintf(stderr, "Could not allocate memory for rotation %d.\n", i);
	    return;
	}
	/* Calculate angle of rotation */
	rot_angle = (-(i - 1) * 2 * PI) / ROTATIONS - PI / 2;

	/* Draw rotated image onto blank rotation bitmap */
	for (j = 0; j < width; j++)
	    for (k = 0; k < height; k++) {
		dj = (double) (j - x_center);
		dk = (double) (k - y_center);
		r = hypot(dj, dk);
		angle = atan2(dk, dj) + rot_angle;
		dx = r * cos(angle);
		dy = r * sin(angle);
		x = (int) (dx + (float) x_center + .5);
		y = (int) (dy + (float) y_center + .5);
		if (pixel_on(rotation[0], x, y))
		    set_pixel(rotation[i], j, k);
	    }

	/* Write the rotated bitmap to standard output */
	strcpy(filename, arg);
	sprintf(ending, "%d", i - 1);
	strcat(filename, ending);
	if ((bitmap_file = fopen(filename, "w")) == NULL) {
	    printf("Could not open bitmap file %s.\n", filename);
	    exit;
	}
	fprintf(bitmap_file, "#define %s%d_width %d", filename, i - 1, width);
	fprintf(bitmap_file, "#define %s%d_height %d", filename, i - 1, height);
	fprintf(bitmap_file, "static short %s%d_bits[] = {", filename, i - 1);
	for (j = 0; j < num_shorts; j++) {
	    if (j % 8 == 0)
		fprintf(bitmap_file, "\n    ");
	    if (j != num_shorts - 1)
		fprintf(bitmap_file, "0x%04x, ", rotation[i][j]);
	    else
		fprintf(bitmap_file, "0x%04x};\n", rotation[i][j]);
	}
    }
}


pixel_on(value, x, y)
    unsigned short *value;
    int x, y;
{
    if (x >= 0 && x < width && y >= 0 && y < height)
	if (value[y * num_shorts_per_line + x / 16] & mask[x % 16])
	    return (1);
    return (0);
}


set_pixel(value, x, y)
    unsigned short *value;
    int x, y;
{
    if (x >= 0 && x < width && y >= 0 && y < height)
	value[y * num_shorts_per_line + x / 16] |= mask[x % 16];
}
