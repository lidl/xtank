/*
** Xtank
**
** Copyright 1990 by Jost Osborne
**
** scroll.c
*/

/*
$Author: rpotter $
$Id: scroll.c,v 2.3 1991/02/10 13:51:35 rpotter Exp $

$Log: scroll.c,v $
 * Revision 2.3  1991/02/10  13:51:35  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:53  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:52  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:26  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:02  aahz
 * Initial revision
 * 
*/

#include "xtank.h"
#include "graphics.h"
#include "scroll.h"
#include "gr.h"


draw_scrollbar(sbar)
scrollbar *sbar;
{
	float fspan, fpos;
	int pspan, ppos;

	draw_rect(sbar->win, sbar->x, sbar->y, sbar->w - 1, sbar->h - 1,
			  DRAW_COPY, GREY);
	draw_filled_rect(sbar->win, sbar->x + 1, sbar->y + 1, sbar->w - 2, sbar->h - 2,
					 DRAW_COPY, BLACK);
	if (sbar->span >= sbar->total)
	{
		/* entire list is displayed */
		draw_filled_rect(sbar->win, sbar->x + 1, sbar->y + 1, sbar->w - 2, sbar->h - 2,
						 DRAW_COPY, BLUE);
		return;
	}
	fspan = (0.0 + sbar->span) / (0.0 + sbar->total);
	pspan = sbar->h * fspan;
	if (sbar->pos + sbar->span > sbar->total)
	{
		sbar->pos = sbar->total - sbar->span;
	}
	fpos = (0.0 + sbar->pos) / (0.0 + sbar->total);
	ppos = sbar->h * fpos;
	if (ppos + 1 + pspan > sbar->h)
	{
		ppos = sbar->h - pspan - 1;
	}
	ppos += sbar->y;
	draw_filled_rect(sbar->win, sbar->x + 1, ppos + 1, sbar->w - 2, pspan,
					 DRAW_COPY, BLUE);
}

/* Update scrollbar for mouse click, move bar, return TRUE if contents of menu
   need to be re-drawn, else FALSE */
int drag_scrollbar(sbar, mx, my, button)
scrollbar *sbar;
int mx, my;
unsigned int button;
{
	float fpos;
	int new_pos;

	if (my < sbar->y || my > sbar->y + sbar->h)
	{
		rorre("This sucks\n");
	}
	if (mx < sbar->x || mx > sbar->x + sbar->w)
	{
		rorre("This sucks wind\n");
	}

	switch (button)
	{
		case EVENT_MBUTTON:
			fpos = (my - sbar->y + 0.0) / (0.0 + sbar->h);
			new_pos = fpos * sbar->total;
			break;

		case EVENT_LBUTTON:
			new_pos = sbar->pos + (sbar->span >> 1);
			break;

		case EVENT_RBUTTON:
			new_pos = sbar->pos - (sbar->span >> 1);
			break;
	}

	if (new_pos > (sbar->total - sbar->span))
	{
		new_pos = sbar->total - sbar->span;
	}
	if (new_pos < 0)
	{
		new_pos = 0;
	}
	if (new_pos == sbar->pos)
	{
		return (FALSE);
	}
	sbar->pos = new_pos;
	draw_scrollbar(sbar);
	return (TRUE);
}
