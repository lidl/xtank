/*************************************************\
*                                                 *
* program to read in an xtank object file, and    *
* trim each bitmap in to to the minimum size      *
*                                                 *
\*************************************************/


#include <stdio.h>
#include <math.h>
#define FALSE 0
#define TRUE 1
#define MAX_VIEWS 32

typedef struct {
    int height, width, x_offset, y_offset, shorts_per_line;
    unsigned short *data;
} Bitmap;


unsigned short mask[16] = {0x0001, 0x0002, 0x0004, 0x0008,
    0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800,
    0x1000, 0x2000, 0x4000, 0x8000
};

Bitmap bitmap[MAX_VIEWS];
int num_bitmaps;


main(argc, argv)
    int argc;
    char *argv[];
{
    char *malloc(), filename[256], newfilename[256], line[256];
    int i;
    Bitmap *b;

    /* Parse command line for bitmap filename */
    if (argc > 1)
	strcpy(filename, argv[1]);
    strcat(filename, ".obj");
    strcpy(newfilename, filename);
    strcat(filename, "_save");

    sprintf(line, "rm -f %s", filename);
    system(line);
    sprintf(line, "mv %s %s", newfilename, filename);
    system(line);

    read_object_file(filename);

    for (i = 0; i < num_bitmaps; i++) {	/* for each picture in the object */
	b = &bitmap[i];
	while (top_row_zero(b))
	    remove_top_row(b);
	while (bottom_row_zero(b))
	    remove_bottom_row(b);
	while (left_column_zero(b))
	    remove_left_column(b);
	while (right_column_zero(b))
	    remove_right_column(b);
    }
    printf("%d bitmaps processed\n", num_bitmaps);
    write_object_file(filename, newfilename);
}


print_data(b)
    Bitmap *b;
{
    int i, j;

    for (j = 0; j < b->height; j++) {
	for (i = 0; i < b->shorts_per_line; i++)
	    printf("%d ", b->data[i + j * b->shorts_per_line]);
	printf("\n");
    }
}


top_row_zero(b)
    Bitmap *b;
{
    return (row_zero(b, 0));
}

bottom_row_zero(b)
    Bitmap *b;
{
    return (row_zero(b, b->height - 1));
}

left_column_zero(b)
    Bitmap *b;
{
    int j;

    for (j = 0; j < b->height; j++)
	if (pixel_on(b, 0, j))
	    return (FALSE);
    return (TRUE);
}

right_column_zero(b)
    Bitmap *b;
{
    int j;

    for (j = 0; j < b->height; j++)
	if (pixel_on(b, b->width - 1, j))
	    return (FALSE);
    return (TRUE);
}

remove_top_row(b)
    Bitmap *b;
{
    int i, j;

    for (j = 1; j < b->height; j++)
	for (i = 0; i < b->shorts_per_line; i++)
	    b->data[(j - 1) * b->shorts_per_line + i] = b->data[j * b->shorts_per_line + i];
    b->height--;
    b->y_offset--;
}

remove_bottom_row(b)
    Bitmap *b;
{
    b->height--;
}

remove_left_column(b)
    Bitmap *b;
{
    Bitmap temp;
    int x, y, i, num_shorts;

    temp.height = b->height;
    temp.width = b->width - 1;
    temp.shorts_per_line = (temp.width + 15) / 16;
    num_shorts = temp.shorts_per_line * temp.height;
    temp.data = (unsigned short *) malloc(2 * num_shorts);
    if (temp.data == NULL) {
	perror("Could not allocate memory for temporary bitmap.\n");
	exit(1);
    }
    for (i = 0; i < num_shorts; i++)
	temp.data[i] = 0;

    for (x = 1; x < b->width; x++)
	for (y = 0; y < b->height; y++)
	    if (pixel_on(b, x, y)) {
		set_pixel(&temp, x - 1, y);
		if (!(pixel_on(&temp, x - 1, y)))
		    printf("Not setting %d, %d\n", x - 1, y);
	    }
    b->width = temp.width;
    b->shorts_per_line = temp.shorts_per_line;
    free(b->data);
    b->data = temp.data;
    b->x_offset--;
}


remove_right_column(b)
    Bitmap *b;
{
    Bitmap temp;
    int x, y, i, num_shorts;

    temp.height = b->height;
    temp.width = b->width - 1;
    temp.shorts_per_line = (temp.width + 15) / 16;
    num_shorts = temp.shorts_per_line * temp.height;
    temp.data = (unsigned short *) malloc(2 * num_shorts);
    if (temp.data == NULL) {
	perror("Could not allocate memory for temporary bitmap.\n");
	exit(1);
    }
    for (i = 0; i < num_shorts; i++)
	temp.data[i] = 0;

    for (x = 0; x < b->width - 1; x++)
	for (y = 0; y < b->height; y++)
	    if (pixel_on(b, x, y)) {
		set_pixel(&temp, x, y);
		if (!(pixel_on(&temp, x, y)))
		    printf("Not Setting %d, %d\n", x, y);
	    }
    b->width = temp.width;
    b->shorts_per_line = temp.shorts_per_line;
    free(b->data);
    b->data = temp.data;
}


pixel_on(b, x, y)
    Bitmap *b;
    int x, y;
{
    if (x >= 0 && x < b->width && y >= 0 && y < b->height)
	if (b->data[y * b->shorts_per_line + x / 16] & mask[x % 16])
	    return (1);
    return (0);
}


set_pixel(b, x, y)
    Bitmap *b;
    int x, y;
{
    b->data[y * b->shorts_per_line + x / 16] ^= mask[x % 16];
}


row_zero(b, row)
    Bitmap *b;
    int row;
{
    int i;

    for (i = 0; i < b->shorts_per_line; i++)
	if (b->data[row * b->shorts_per_line + i])
	    return (0);
    return (1);
}

print_pixels(b)
    Bitmap *b;
{
    int i, j;

    for (j = 0; j < b->height; j++) {
	for (i = 0; i < b->width; i++)
	    if (pixel_on(b, i, j))
		printf("*");
	    else
		printf(" ");
	printf("\n");
    }
}


read_object_file(filename)
    char *filename;
{
    FILE *datafile;
    char line[256];
    int tank = FALSE;
    int num_shorts, i, j, num_views;

    datafile = fopen(filename, "r");
    if (datafile == 0) {
	perror("error opening file.\n");
	exit(1);
    }
    /* reads until first define statment */

    while (1) {
	fgets(line, 256, datafile);
	if (!(strncmp(line, "#define", 7)))
	    break;
    }

    sscanf(line, "#define %*s %d", &num_views);
    if (num_views > MAX_VIEWS) {
	perror("Too many bitmaps.\n");
	exit(1);
    }
    /* search for either picinfo or picture */

    while (1) {
	fgets(line, 256, datafile);
	if (!(strncmp(line, "Picture", 7)))
	    break;
	if (!(strncmp(line, "Picinfo", 7)))
	    tank = TRUE;
    }

    /* if it's a tank, read 4 lines of after picture, else read views lines */
    if (tank)
	num_bitmaps = 4;
    else
	num_bitmaps = num_views;

    for (i = 0; i < num_bitmaps; i++) {
	fscanf(datafile, "      { %d, %d, %d, %d },", &bitmap[i].width,
	       &bitmap[i].height, &bitmap[i].x_offset, &bitmap[i].y_offset);
    }

    for (i = 0; i < num_bitmaps; i++) {
	/* search for "static" */
	while (1) {
	    fgets(line, 256, datafile);
	    if (!(strncmp(line, "static", 6)))
		break;
	    if (!(strncmp(line, "short", 5)))
		break;
	}

	bitmap[i].shorts_per_line = (bitmap[i].width + 15) / 16;
	num_shorts = bitmap[i].shorts_per_line * bitmap[i].height;
	bitmap[i].data = (unsigned short *) malloc(2 * num_shorts);
	if (bitmap[i].data == NULL) {
	    perror("Could not allocate memory for original bitmap.\n");
	    exit(1);
	}
	for (j = 0; j < num_shorts; j++)
	    fscanf(datafile, "%*2s%4x%*s", &bitmap[i].data[j]);
    }
    fclose(datafile);
}


write_object_file(filename, newfilename)
    char *filename, *newfilename;
{
    FILE *datafile, *outfile;
    char line[256];
    int i, j, num_shorts;

    datafile = fopen(filename, "r");
    if (datafile == 0) {
	perror("error opening file.\n");
	exit(1);
    }
    outfile = fopen(newfilename, "w");
    if (datafile == 0) {
	perror("error opening file.\n");
	exit(1);
    }
    /* search for picture saving all lines along the way */
    while (1) {
	fgets(line, 256, datafile);
	fputs(line, outfile);
	if (!(strncmp(line, "Picture", 7)))
	    break;
    }

    for (i = 0; i < num_bitmaps; i++) {
	fgets(line, 256, datafile);
	fprintf(outfile, "      { %d, %d, %d, %d }%c\n", bitmap[i].width,
		bitmap[i].height, bitmap[i].x_offset, bitmap[i].y_offset,
		i == num_bitmaps - 1 ? ' ' : ',');
    }

    /* search for "static" or "short", saving all lines along the way */
    while (1) {
	fgets(line, 256, datafile);
	if (!(strncmp(line, "static", 6)))
	    break;
	if (!(strncmp(line, "short", 5)))
	    break;
	fputs(line, outfile);
    }

    for (i = 0; i < num_bitmaps; i++) {
	fputs(line, outfile);
	printf("Writing bitmap %d.\n", i);
	num_shorts = bitmap[i].shorts_per_line * bitmap[i].height;
	for (j = 0; j < num_shorts; j++) {
	    if (j % 8 == 0)
		fprintf(outfile, "%s    ", j == 0 ? "" : "\n");
	    if (j != num_shorts - 1)
		fprintf(outfile, "0x%04x, ", bitmap[i].data[j]);
	    else
		fprintf(outfile, "0x%04x};\n", bitmap[i].data[j]);
	}

	fprintf(outfile, "\n");

	while (1) {
	    fgets(line, 256, datafile);
	    if (!(strncmp(line, "static", 6)))
		break;
	    if (!(strncmp(line, "short", 5)))
		break;
	}
    }

    while (!(feof(datafile))) {
	fputs(line, outfile);
	fgets(line, 256, datafile);
    }

    fclose(datafile);
    fclose(outfile);
}
