/*-
 * Copyright (c) 1990 Josh Osborne
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
#include "graphics.h"
#include "scroll.h"
#include "gr.h"
#include "proto.h"

void
draw_scrollbar(scrollbar *sbar)
{
	FLOAT fspan, fpos;
	int pspan, ppos;

	draw_rect(sbar->win, sbar->x, sbar->y, sbar->w - 1, sbar->h - 1,
			  DRAW_COPY, GREY);
	draw_filled_rect(sbar->win, sbar->x + 1, sbar->y + 1, sbar->w - 2, sbar->h - 2,
					 DRAW_COPY, BLACK);
	if (sbar->span >= sbar->total) {
		/* entire list is displayed */
		draw_filled_rect(sbar->win, sbar->x + 1, sbar->y + 1, sbar->w - 2, sbar->h - 2,
						 DRAW_COPY, BLUE);
		return;
	}
	fspan = (0.0 + sbar->span) / (0.0 + sbar->total);
	pspan = sbar->h * fspan;
	if (sbar->pos + sbar->span > sbar->total) {
		sbar->pos = sbar->total - sbar->span;
	}
	fpos = (0.0 + sbar->pos) / (0.0 + sbar->total);
	ppos = sbar->h * fpos;
	if (ppos + 1 + pspan > sbar->h) {
		ppos = sbar->h - pspan - 1;
	}
	ppos += sbar->y;
	draw_filled_rect(sbar->win, sbar->x + 1, ppos + 1, sbar->w - 2, pspan,
					 DRAW_COPY, BLUE);
}

/* Update scrollbar for mouse click, move bar, return TRUE if contents of menu
   need to be re-drawn, else FALSE */
int
drag_scrollbar(scrollbar *sbar, int mx, int my, unsigned int button)
{
	FLOAT fpos;
	int new_pos;

	if (my < sbar->y || my > sbar->y + sbar->h) {
		rorre("This sucks\n");
	}
	if (mx < sbar->x || mx > sbar->x + sbar->w) {
		rorre("This sucks wind\n");
	}
	switch (button) {
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

	if (new_pos > (sbar->total - sbar->span)) {
		new_pos = sbar->total - sbar->span;
	}
	if (new_pos < 0) {
		new_pos = 0;
	}
	if (new_pos == sbar->pos) {
		return (FALSE);
	}
	sbar->pos = new_pos;
	draw_scrollbar(sbar);
	return (TRUE);
}
