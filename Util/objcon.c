/*
** objcon.c
**
**
** This program accepts an X10 format object file from standard
** input, and writes out an X11 format object file to standard output.
*/

#include <stdio.h>
#include <math.h>
#include <strings.h>
#define FALSE 0
#define TRUE 1
#define MAX_VIEWS 32

typedef struct
{
  int height, width, x_offset, y_offset, shorts_per_line;
  unsigned short *data;
} Bitmap;

typedef struct
{
  int height, width, x_offset, y_offset, bytes_per_line;
  unsigned char *data;
} X11_Bitmap;

Bitmap bitmap[MAX_VIEWS];
X11_Bitmap x11_bitmap[MAX_VIEWS];
int num_bitmaps;


main(argc,argv)
     int argc;
     char *argv[];
{
  char *malloc(), filename[256], newfilename[256],line[256];
  int i;
  Bitmap *b;

  /* Parse command line for bitmap filename */
  if(argc > 1) strcpy(filename,argv[1]);
  strcat(filename, ".obj");
  strcpy(newfilename, "/mit/games/src/vax/xtank/Src/Objects_x11/");
  strcat(newfilename, filename);
  
  /* delete any current version of the file to prevent file open errors */
  sprintf(line, "rm -f %s", newfilename);
  system(line);

  read_object_file_X10(filename);

  for (i=0; i<num_bitmaps; i++)
    convert_X10_to_X11(&bitmap[i], &x11_bitmap[i]);

  write_object_file_X11(filename, newfilename);
}


convert_X10_to_X11(x10_bitmap, x11_bitmap)
     Bitmap *x10_bitmap;
     X11_Bitmap *x11_bitmap;
{
  int i, j, x10_elements, bytes_wide, x11_elements;

  x10_elements = ((x10_bitmap->width + 15)/16) * x10_bitmap->height;
  bytes_wide = (x10_bitmap->width + 7)/8;
  x11_elements = bytes_wide * x10_bitmap->height;

  /* copy over data that remains the same */
  x11_bitmap->width = x10_bitmap->width;
  x11_bitmap->height = x10_bitmap->height;
  x11_bitmap->x_offset = x10_bitmap->x_offset;
  x11_bitmap->y_offset = x10_bitmap->y_offset;
  x11_bitmap->bytes_per_line = bytes_wide;
  
  /* allocate space for the bitmap data */
  x11_bitmap->data = (unsigned char *) malloc(x11_elements);
  if(x11_bitmap->data == NULL) {
    fprintf(stderr,"Could not allocate memory for bitmap.\n");
    return;
  }
  
  /* Create the x11_value array from the x10_value array */
  j = 0;
  for(i = 0 ; i < x10_elements ; i++) {
    x11_bitmap->data[j++] = x10_bitmap->data[i];
    if(j % bytes_wide) x11_bitmap->data[j++] = x10_bitmap->data[i]>>8;
  }

}


read_object_file_X10(filename)
  char *filename;
{
  FILE *datafile;
  char line[256];
  int tank=FALSE;
  int num_shorts,i,j, num_views;

  datafile = fopen(filename, "r");
  if(datafile == 0)
    {
      perror("error opening file.\n");
      exit(1);
    }

  /* read until first define statment */
  while(1)
    {
      fgets(line, 256, datafile);
      if(!(strncmp(line, "#define", 7))) break;
    }

  /* find out how many views are in the object file */
  sscanf(line, "#define %*s %d", &num_views);
  if(num_views > MAX_VIEWS) {
    perror("Too many bitmaps.\n");
    exit(1);
  }

  /* 
  ** Search for "Picture".
  ** If "Picinfo" occurs before "Picture" then it's a tank.
  */
  while(1)
    {
      fgets(line, 256, datafile);
      if(!(strncmp(line, "Picture", 7)) || 
	 !(strncmp(line, "static Picture", 14))) break;
      if(!(strncmp(line, "Picinfo", 7)) ||
	 !(strncmp(line, "static Picinfo", 14))) tank = TRUE;
    }
	
  /* 
  ** Tanks have only 4 bitmaps, even though they have 16 views.
  ** Other objects have as many bitmaps as views.
  */
  if (tank) num_bitmaps = 4;
  else num_bitmaps = num_views;

  for(i=0;i<num_bitmaps;i++)
    {
      fscanf(datafile, "      { %d, %d, %d, %d },", &bitmap[i].width, 
	     &bitmap[i].height, &bitmap[i].x_offset, &bitmap[i].y_offset);
    }

  for(i=0;i<num_bitmaps;i++)
    {
      /* search for "static" */
      while (1)
	{
	  fgets(line, 256, datafile);
	  if (!(strncmp(line, "static", 6))) break;
	  if (!(strncmp(line, "short", 5))) break;
	}
      
      bitmap[i].shorts_per_line = (bitmap[i].width + 15) / 16;
      num_shorts = bitmap[i].shorts_per_line * bitmap[i].height;
      bitmap[i].data = (unsigned short *) malloc(2 * num_shorts);
      if(bitmap[i].data == NULL) 
	{
	  perror("Could not allocate memory for original bitmap.\n");
	  exit(1);
	}
      for(j = 0 ; j < num_shorts ; j++)
	fscanf(datafile,"%*2s%4x%*s",&bitmap[i].data[j]);
    }
  fclose(datafile);
}


write_object_file_X10(filename, newfilename)
     char *filename, *newfilename;
{
  FILE *datafile, *outfile;
  char line[256];
  int i,j,num_shorts;

  datafile = fopen(filename, "r");
  if(datafile == 0)
    {
      perror("error opening file.\n");
      exit(1);
    }

  outfile = fopen(newfilename, "w");
  if(datafile == 0)
    {
      perror("error opening file.\n");
      exit(1);
    }

  /* search for picture saving all lines along the way */
  while(1)
    {
      fgets(line, 256, datafile);
      fputs(line, outfile);
      if(!(strncmp(line, "Picture", 7))) break;
    }

  for(i=0;i<num_bitmaps;i++)
    {
      fgets(line, 256, datafile);
      fprintf(outfile, "      { %d, %d, %d, %d }%c\n", bitmap[i].width,
	      bitmap[i].height, bitmap[i].x_offset, bitmap[i].y_offset,
	      i==num_bitmaps-1 ? ' ' : ',');
    }

  /* search for "static" or "short", saving all lines along the way */
  while(1)
    {
      fgets(line, 256, datafile);
      if(!(strncmp(line, "static", 6))) break;
      if(!(strncmp(line, "short", 5))) break;
      fputs(line, outfile);
    }

  for(i=0;i<num_bitmaps;i++)
    {
      fputs(line, outfile);
      printf("Writing bitmap %d.\n",i);
      num_shorts = bitmap[i].shorts_per_line * bitmap[i].height;
      for(j = 0 ; j < num_shorts; j++) 
	{
	  if(j%8 == 0) fprintf(outfile,"%s    ",j==0 ? "" : "\n");
	  if(j != num_shorts - 1) 
	    fprintf(outfile,"0x%04x, ",bitmap[i].data[j]);
	  else 
	    fprintf(outfile,"0x%04x};\n",bitmap[i].data[j]);
	}

      fprintf(outfile, "\n");

      while(1)
	{
	  fgets(line, 256, datafile);
	  if(!(strncmp(line, "static", 6))) break;
	  if(!(strncmp(line, "short", 5))) break;
	}
    }

  while(!(feof(datafile)))
    {
      fputs(line, outfile);
      fgets(line,256,datafile);
    }

  fclose(datafile);
  fclose(outfile);
}


write_object_file_X11(filename, newfilename)
     char *filename, *newfilename;
{
  FILE *datafile, *outfile;
  char line[256], temp[256];
  int i,j,num_bytes;

  datafile = fopen(filename, "r");
  if(datafile == 0)
    {
      perror("error opening file.\n");
      exit(1);
    }

  outfile = fopen(newfilename, "w");
  if(datafile == 0)
    {
      perror("error opening file.\n");
      exit(1);
    }

  /* search for "Picture" saving all lines along the way and prepending
  ** "static " before lines beginning with "Picture" or "Picinfo" 
  */
  while(1)
    {
      fgets(line, 256, datafile);
      if(!(strncmp(line, "Picinfo", 7))) 
	{
	  strcpy(temp, line);
	  strcpy(line, "static ");
	  strcat(line, temp);
	}
      if(!(strncmp(line, "Object", 6))) 
	{
	  strcpy(temp, line);
	  strcpy(line, "static ");
	  strcat(line, temp);
	}
      if(!(strncmp(line, "Picture", 7))) 
	{
	  strcpy(temp, line);
	  strcpy(line, "static ");
	  strcat(line, temp);
	  break;
	}
      if(!(strncmp(line, "static Picture", 14))) break; 
      fputs(line, outfile);
    }
  fputs(line, outfile);

  for(i=0;i<num_bitmaps;i++)
    {
      fgets(line, 256, datafile);
      fprintf(outfile, "      { %d, %d, %d, %d }%c\n", x11_bitmap[i].width,
	      x11_bitmap[i].height, x11_bitmap[i].x_offset, 
	      x11_bitmap[i].y_offset, i==num_bitmaps-1 ? ' ' : ',');
    }

  /* search for "static short" or "short", saving all lines along the way */
  while(1)
    {
      fgets(line, 256, datafile);
      if(!(strncmp(line, "static short", 11))) break;
      if(!(strncmp(line, "Object", 6))) 
	{
	  strcpy(temp, line);
	  strcpy(line, "static ");
	  strcat(line, temp);
	}
      if(!(strncmp(line, "short", 5))) break;
      fputs(line, outfile);
    }

  for(i=0;i<num_bitmaps;i++)
    {
      replace_short_w_char(line);
      fputs(line, outfile);
      printf("Writing bitmap %d.\n",i);
      /* Write x11 formatted bitmap to standard output */
      num_bytes = x11_bitmap[i].bytes_per_line * x11_bitmap[i].height;
      for(j = 0 ; j < num_bytes; j++) 
	{
	  if(j%8 == 0) fprintf(outfile,"%s    ",j==0 ? "" : "\n");
	  if(j != num_bytes - 1) 
	    fprintf(outfile,"0x%02x, ",x11_bitmap[i].data[j]);
	  else 
	    fprintf(outfile,"0x%02x};\n",x11_bitmap[i].data[j]);
	}
      
      fprintf(outfile, "\n");

      while(1)
	{
	  fgets(line, 256, datafile);
	  if(!(strncmp(line, "static", 6))) break;
	  if(!(strncmp(line, "short", 5))) break;
	}
    }

  replace_short_w_char(line);

  while(!(feof(datafile)))
    {
      fputs(line, outfile);
      fgets(line,256,datafile);
    }

  fclose(datafile);
  fclose(outfile);
}

/*
** Replaces first occurence of "short" within string with
** "unsigned char". 
** If "short" is at the start of string, it also prepends "static "
** to the string.
*/
replace_short_w_char(string)
     char *string;
{
  char temp[256], *substring, rest_of_line[256];
  int length, start, position;
  
  strcpy(temp, string);
  
  length = strlen(temp);
  start = 0;
  
  while(start < length)
    {
      substring = index(&temp[start], 's');
      if ((int) substring == 0) return;
      position = (int) substring - (int) temp;
      if(!(strncmp(substring, "short", 5))) break;
      start = position + 1;
    }
  
  strcpy(rest_of_line, &temp[position+5]);
  if (position == 0) 
    strcpy(string, "static ");
  else {
    strncpy(string, temp, position);
    string[position] = '\0';
  }
  strcat(string, "unsigned char");
  strcat(string, rest_of_line);
}
