#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** util.c
*/

#include "config.h"
#include "xtank.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "terminal.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/time.h>
#define RANDOM     random
#define SEEDRANDOM (void) srandom
extern long random();
extern int srandom();

#endif	/* def UNIX */

#ifdef AMIGA
#include <time.h>
#define RANDOM     rand
#define SEEDRANDOM srand
#endif


extern Terminal *term;


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
	if (mx > 0)
		return (RANDOM() % mx);
	else
		return 0;
}

/*
** Returns a random float from mn to mx.
*/
float rnd_interval(mn, mx)
float mn, mx;
{
    long r;

	r = RANDOM();
    return ((mn + (mx - mn) * (r / 2147483647.0)));
}

/*
** Displays a string in a window at the beginning of the specified row.
*/
display_mesg(w, string, row, font)
int w;
char *string;
int row;
int font;
{
	static int *color_arr = NULL;
	extern char *calloc();
	int i;

	if (color_arr == NULL)
	{
		color_arr = (int *) calloc(256, sizeof(int));
		assert(color_arr != NULL);
		for (i = 0; i < 256; i++)
		{
			switch (i)
			{
				case ('R'):
					color_arr[i] = RED;
					break;
				case ('O'):
					color_arr[i] = ORANGE;
					break;
				case ('Y'):
					color_arr[i] = YELLOW;
					break;
				case ('G'):
					color_arr[i] = GREEN;
					break;
				case ('B'):
					color_arr[i] = BLUE;
					break;
				case ('V'):
					color_arr[i] = VIOLET;
					break;
				case ('N'):
					color_arr[i] = GREY;
					break;
				case ('C'):
					color_arr[i] = CUR_COLOR;
					break;		/* COM-> */
				default:
					color_arr[i] = WHITE;
					break;
			}
		}
	}
	i = (w != MSG_WIN) ? WHITE : color_arr[string[0]];
	draw_text_rc(w, 0, row, string, font, i);
}

/*
** Displays a string in a window starting at the specified row and column.
*/
display_mesg2(w, string, column, row, font)
int w;
char *string;
int column;
int row;
int font;
{
	draw_text_rc(w, column, row, string, font, WHITE);
}

display_mesg1(w, string, column, row, font, color)
int w;
char *string;
int column;
int row;
int font;
int color;
{
	draw_text_rc(w, column, row, string, font, color);
}


/*
** Sets the loc structure to the center of the given box coordinates.
*/
make_loc(loc, grid_x, grid_y)
Loc *loc;
int grid_x, grid_y;
{
	loc->grid_x = grid_x;
	loc->grid_y = grid_y;
	loc->box_x = BOX_WIDTH / 2;
	loc->box_y = BOX_HEIGHT / 2;
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
	for (i = 0; i < num_terminals; i++)
		close_terminal(terminal[i]);

	/* Close the graphics system */
	close_graphics();
}
