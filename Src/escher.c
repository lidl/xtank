/*-
 * Copyright (c) 1989 Christopher Alex North-Keys
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

/*
** escher.c version started Thu d.07.12.1989
*/

#include "sysdep.h"
#include "graphics.h"
#include "bullet.h"
#include "vehicle.h"
#include "terminal.h"
#include "proto.h"

/*
 * corners (corner one is near the visual origin)
 *  2   1
 *  3   0
 */

/* M.C.Escher impossible frame data */
int ex[4][4] =
{
	{0, 0, 1, 2},
	{0, 1, 2, 1},
	{0, 0, -1, -2},
	{0, -1, -2, -1}};
int ey[4][4] =
{
	{0, 1, 2, 1},
	{0, 0, -1, -2},
	{0, -1, -2, -1},
	{0, 0, 1, 2}};

void
menu_frame(int win, int x, int y, int w, int h, int func, int color, int frame)
{
	int c;						/* corner */
	int d;						/* delta inside of frame */
	int px[4], py[4];

	draw_rect(win, x, y, w, h, func, color);

	if (frame < 4)
		return;

	d = frame >> 1;

	px[0] = x + w;
	py[0] = y + h;
	px[1] = x + w;
	py[1] = y;
	px[2] = x;
	py[2] = y;
	px[3] = x;
	py[3] = y + h;

	for (c = 0; c < 4; c++) {	/* connect each corner to itself and next
								   corner */
		int n;					/* next corner */

		n = (c == 3 ? 0 : c + 1);

		/* 2c to 3c self-connect */
		draw_line(win,
				  px[c] + (d * ex[c][2]), py[c] + (d * ey[c][2]),
				  px[c] + (d * ex[c][3]), py[c] + (d * ey[c][3]),
				  func, color);
		/* 1c to 0n */
		draw_line(win,
				  px[c] + (d * ex[c][1]), py[c] + (d * ey[c][1]),
				  px[n] + (d * ex[n][0]), py[n] + (d * ey[n][0]),
				  func, color);
		/* 2c to 1n */
		draw_line(win,
				  px[c] + (d * ex[c][2]), py[c] + (d * ey[c][2]),
				  px[n] + (d * ex[n][1]), py[n] + (d * ey[n][1]),
				  func, color);
		/* 3c to 2n */
		draw_line(win,
				  px[c] + (d * ex[c][3]), py[c] + (d * ey[c][3]),
				  px[n] + (d * ex[n][2]), py[n] + (d * ey[n][2]),
				  func, color);
	}
	return;
}
