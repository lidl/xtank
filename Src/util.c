/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
