/*
** r16.c
**
** This program reads in X10 formatted bitmap files and
** writes out bitmap files for 16 of rotations of the object.
** It does not handle hotspots and will die if the file has been changed
** from the standard X10 bitmap output.
*/

#include <stdio.h>
#include <math.h>

#define ROTATIONS 16
#define PI 3.14159265358979

int num_shorts_per_line,num_shorts;
int width,height;
int mask[16] = { 0x0001, 0x0002, 0x0004, 0x0008,
		 0x0010, 0x0020, 0x0040, 0x0080,
		 0x0100, 0x0200, 0x0400, 0x0800,
		 0x1000, 0x2000, 0x4000, 0x8000
		 };

main(argc,argv)
     int argc;
     char *argv[];
{
  char line[80],bits_name[30],*malloc();
  unsigned short *rotation[ROTATIONS+1];
  FILE *bitmap_file;
  char arg[30];
  char ending[10];
  char filename[50];
  double rot_angle;
  int i,j;

  /* Parse command line for bitmap filename */
  if(argc > 1) strcpy(arg,argv[1]);
  else {
    fprintf(stderr,"Bitmap filename required.\n");
    exit(1);
  }

  /* Read info about bitmap from standard input */
  strcpy(filename,arg);
  strcat(filename,"0");
  if((bitmap_file = fopen(filename,"r")) == NULL) {
    fprintf(stderr,"Could not find bitmap file %s.\n",filename);
    exit(1);
  }

  fgets(line,80,bitmap_file);
  sscanf(line,"#define %*s %d",&width);
  fgets(line,80,bitmap_file);
  sscanf(line,"#define %*s %d",&height);
  fgets(line,80,bitmap_file);
  fclose(bitmap_file);

  num_shorts_per_line = (width + 15) / 16;
  num_shorts = num_shorts_per_line * height;
  for(i = 0 ; i < ROTATIONS + 1 ; i++) {
    rotation[i] = (unsigned short *) malloc(2 * num_shorts);
    if(rotation[i] == NULL) {
      fprintf(stderr,"Could not allocate memory for rotation %d.\n",i);
      return;
    }
  }

  /* Read all three bitmaps into rotation array */
  for(i = 0 ; i < 3 ; i++) {
    strcpy(filename,arg);
    sprintf(ending,"%d",i);
    strcat(filename,ending);
    if((bitmap_file = fopen(filename,"r")) == NULL) {
      fprintf("Could not find bitmap file %s.\n",filename);
      exit(1);
    }
    fgets(line,80,bitmap_file);
    sscanf(line,"#define %*s %d",&width);
    fgets(line,80,bitmap_file);
    sscanf(line,"#define %*s %d",&height);
    fgets(line,80,bitmap_file);
    sscanf(line,"static short %s = {",bits_name);
    for(j = 0 ; j < num_shorts ; j++)
      fscanf(bitmap_file,"%*2s%4x%*s",&rotation[i][j]);
    fclose(bitmap_file);
  }

  /* Flip rotation 1 on vertical axis.  Store in extra array slot */
  flip_bitmap(rotation[1],rotation[ROTATIONS]);

  /* Compute rotations 3-15 of the bitmap */
  for(i = 3 ; i < ROTATIONS ; i++) {
    rot_angle = PI/2*(i/4);

    /* Draw rotated image onto blank rotation bitmap */
    if(i == 3)
      rotate_bitmap(rotation[ROTATIONS],rotation[3],PI/2);
    else
      rotate_bitmap(rotation[i%4],rotation[i],rot_angle);

    /* Write the rotated bitmap to standard output */
    strcpy(filename,arg);
    sprintf(ending,"%d",i);
    strcat(filename,ending);
    if((bitmap_file = fopen(filename,"w")) == NULL) {
      printf("Could not open bitmap file %s.\n",filename);
      exit();
    }
    fprintf(bitmap_file,"#define %s_width %d\n",filename,width);
    fprintf(bitmap_file,"#define %s_height %d\n",filename,height);
    fprintf(bitmap_file,"static short %s_bits[] = {",filename);
    for(j = 0 ; j < num_shorts ; j++) {
      if(j%8 == 0) fprintf(bitmap_file,"\n    ");
      if(j != num_shorts - 1) fprintf(bitmap_file,"0x%04x, ",rotation[i][j]);
      else fprintf(bitmap_file,"0x%04x};\n",rotation[i][j]);
    }
    fclose(bitmap_file);
  }
}


pixel_on(value,x,y)
     unsigned short *value;
     int x,y;
{
  if(x >= 0 && x < width && y >= 0 && y < height)
    if(value[y * num_shorts_per_line + x/16] & mask[x%16])
      return(1);
  return(0);
}


set_pixel(value,x,y)
     unsigned short *value;
     int x,y;
{
  if(x >= 0 && x < width && y >= 0 && y < height)
    value[y * num_shorts_per_line + x/16] |= mask[x%16];
}


rotate_bitmap(source,dest,rot_angle)
     unsigned short *source,*dest;
     double rot_angle;
{
  int j,k;
  double dj,dk,dx,dy,r,angle;
  int x,y,x_center,y_center;

  x_center = width/2;
  y_center = height/2;

  for(j = 0 ; j < width ; j++)
    for(k = 0 ; k < height ; k++) {
      dj = (double) (j - x_center);
      dk = (double) (k - y_center);
      r = hypot(dj,dk);
      angle = atan2(dk,dj) - rot_angle;
      dx = r*cos(angle);
      dy = r*sin(angle);
      x = (int) (dx + (float) x_center + .5);
      y = (int) (dy + (float) y_center + .5);
      if(pixel_on(source,x,y))
	set_pixel(dest,j,k);
    }
}
  
       
flip_bitmap(source,dest)
     unsigned short *source,*dest;
{
  int j,k;

  for(j = 0 ; j < width ; j++)
    for(k = 0 ; k < height ; k++)
      if(pixel_on(source,(width-1) - j,k))
	set_pixel(dest,j,k);
}
