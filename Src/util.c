/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** util.c
*/

/*
$Author: lidl $
$Id: util.c,v 2.6 1991/09/15 09:24:51 lidl Exp $

$Log: util.c,v $
 * Revision 2.6  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.5  1991/05/01  18:34:26  lidl
 * added kludged up support Motorola UNIX (SysV based)
 *
 * Revision 2.4  1991/03/25  00:42:11  stripes
 * RS6K Patches (IBM is a rock sucker)
 *
 * Revision 2.3  1991/02/10  13:51:57  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:16  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:21  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:45  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:15  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "terminal.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/time.h>
#ifdef MOTOROLA
#define RANDOM     rand
#else /* MOTOROLA */
#define RANDOM     random
#endif /* MOTOROLA */
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
#if defined(MOTOROLA)
    return ((mn + (mx - mn) * (r / 32767.0)));
#else
    return ((mn + (mx - mn) * (r / 2147483647.0)));
#endif
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
    static int color_arr[256];
    static Boolean inited = FALSE;
#if !defined(_IBMR2)
    extern char *calloc();
#endif
    int i;
  
    if (! inited) {
	inited = TRUE;
	for (i = 0; i < 256; i++) {
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
