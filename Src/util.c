/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** util.c
*/

#include "common.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/time.h>

#define RANDOM     random
#define SEEDRANDOM (void) srandom
extern long random();
extern int srandom();
#endif

#ifdef AMIGA
#include <time.h>

#define RANDOM     rand
#define SEEDRANDOM srand
#endif

#include "xtank.h"
#include "gr.h"

/*
** Initializes random number generator.
*/
init_random()
{
  extern long time();

  SEEDRANDOM((int) time((long *) NULL));
}

/*
** Returns a random integer from 0 to mx-1.
*/
rnd(mx)
     int mx;
{
  if(mx > 0) return (RANDOM() % mx);
  else return 0;
}

/*
** Returns a random float from mn to mx.
*/
float rnd_interval(mn,mx)
     float mn, mx;
{
  long int r;

  r = RANDOM();
  return((mn + (mx - mn) * ((float) r / 2147483647.0)));
}

/*
** Displays a string in a window at the beginning of the specified row.
*/
display_mesg(w,string,row,font)
     int w;
     char *string;
     int row;
     int font;
{
  draw_text_rc(w,0,row,string,font,WHITE);
}

/*
** Displays a string in a window starting at the specified row and column.
*/
display_mesg2(w,string,column,row,font)
     int w;
     char *string;
     int column;
     int row;
     int font;
{
  draw_text_rc(w,column,row,string,font,WHITE);
}

/*
** Sets the loc structure to the center of the given box coordinates.
*/
make_loc(loc,grid_x,grid_y)
     Loc *loc;
     int grid_x,grid_y;
{
  loc->grid_x = grid_x;
  loc->grid_y = grid_y;
  loc->box_x = BOX_WIDTH/2;
  loc->box_y = BOX_HEIGHT/2;
  loc->x = loc->grid_x * BOX_WIDTH + loc->box_x;
  loc->y = loc->grid_y * BOX_HEIGHT + loc->box_y;
}

/*
** Frees all allocated storage in preparation for exiting program.
*/
free_everything()
{
  extern int num_terminals;
  extern Terminal *terminal[];
  int i;

  /* Free the storage for all the terminals */
  for(i = 0 ; i < num_terminals ; i++)
    close_terminal(terminal[i]);

  /* Close the graphics system */
  close_graphics();
}
