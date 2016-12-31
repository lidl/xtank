/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "xtank.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "terminal.h"
#include "proto.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>		/* for random(), calloc() */

#ifdef MOTOROLA
#define RANDOM     rand
#else /* MOTOROLA */
#define RANDOM     random
#endif /* MOTOROLA */
#define SEEDRANDOM srandom
#endif /* UNIX */

#ifdef AMIGA
#include <time.h>
#define RANDOM     rand
#define SEEDRANDOM srand
#endif

extern Terminal *term;

/*
** Initializes random number generator.
*/
void
init_random(void)
{
	SEEDRANDOM((int) time((long *) NULL));
}

/*
** Returns a random integer from 0 to mx-1.
*/
int
rnd(int mx)
{
	if (mx > 0)
		return (RANDOM() % mx);
	else
		return 0;
}

/*
** Returns a random FLOAT from mn to mx.
*/
FLOAT
rnd_interval(FLOAT mn, FLOAT mx)
{
	long r;

	r = RANDOM();
#if defined(MOTOROLA)
	return ((mn + (mx - mn) * (r / 32767.0)));
#else
	return ((mn + (mx - mn) * (r / 2147483647.0)));
#endif
}

/*
** Displays a string in a window at the beginning of the specified row.
*/
void
display_mesg(int w, char *string, int row, int font)
{
	static int color_arr[256];
	static Boolean inited = FALSE;

	int i;

	if (!inited) {
		inited = TRUE;
		for (i = 0; i < 256; i++) {
			switch (i) {
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
int
display_mesg2(int w, char *string, int column, int row, int font)
{
	draw_text_rc(w, column, row, string, font, WHITE);
}

int
display_mesg1(int w, char *string, int column, int row, int font, int color)
{
	draw_text_rc(w, column, row, string, font, color);
}


/*
** Frees all allocated storage in preparation for exiting program.
*/
void
free_everything(void)
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

/*
 * Adapted from "Graphics Gems"
 * gives approximate distance from loc1 to loc2
 * with only overestimations, and then never by more
 * than (9/8) + one bit uncertainty. -ane
 */

long
idist(long x1, long y1, long x2, long y2)
{
	if ((x2 -= x1) < 0)
		x2 = -x2;
	if ((y2 -= y1) < 0)
		y2 = -y2;
	return (x2 + y2 - (((x2 > y2) ? y2 : x2) >> 1));
}
