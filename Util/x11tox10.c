/*
** x11tox10.c
**
** This program accepts an X11 format bitmap file from standard
** input, and writes out an X10 format bitmap file to standard output.
** It does not handle hotspots and will die if the file has been changed
** from the standard X11 bitmap output.
*/

#include <stdio.h>

main()
{
    char line[80], width_line[80], height_line[80], bits_name[30], *malloc();
    int width, height, i, j;
    int bytes_wide, shorts_wide, x10_max, x11_max;
    unsigned short *x10_value;
    unsigned char *x11_value;

    /* Read info about x11 bitmap from standard input */
    gets(width_line);
    sscanf(width_line, "#define %*s %d", &width);
    gets(height_line);
    sscanf(height_line, "#define %*s %d", &height);
    gets(line);
    sscanf(line, "static char %s = {", bits_name);

    bytes_wide = (width + 7) / 8;
    x11_max = bytes_wide * height;

    x11_value = (unsigned char *) malloc(x11_max);
    if (x11_value == NULL) {
	fprintf(stderr, "Could not allocate memory for bitmap.\n");
	return;
    }
    for (i = 0; i < x11_max; i++)
	scanf("%*2s%2x%*s", &x11_value[i]);

    /* Create the x10_value array from the x11_value array */
    shorts_wide = (width + 15) / 16;
    x10_max = shorts_wide * height;
    x10_value = (unsigned short *) malloc(2 * x10_max);
    if (x10_value == NULL) {
	fprintf(stderr, "Could not allocate memory for bitmap.\n");
	return;
    }
    for (i = 0, j = 0; i < x11_max; j++) {
	x10_value[j] = x11_value[i++];
	if (i % bytes_wide)
	    x10_value[j] += x11_value[i++] << 8;
    }

    /* Write x11 formatted bitmap to standard output */

    printf("%s\n", width_line);
    printf("%s\n", height_line);
    printf("static short %s = {", bits_name);
    for (i = 0; i < x10_max; i++) {
	if (i % 8 == 0)
	    printf("\n    ");
	if (i != x10_max - 1)
	    printf("0x%04x, ", x10_value[i]);
	else
	    printf("0x%04x};\n", x10_value[i]);
    }
}
