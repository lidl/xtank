/*
 * Copyright (c) 1989 Christoph. Alex. North-Keys
 */

#ifndef lint
static char Version[] = "@(#)escher.c Thu d.07.12.1989 C. North-Keys";

#endif

/*
 * escher.c version started Thu d.07.12.1989
 */

#include "graphics.h"

/*
 * corners (corner one is near the visual origin)
 *  2   1
 *  3   0
 */

/* M.C.Escher impossible frame data */
int ex[4][4] = {{0, 0, 1, 2}, {0, 1, 2, 1}, {0, 0, -1, -2}, {0, -1, -2, -1}};
int ey[4][4] = {{0, 1, 2, 1}, {0, 0, -1, -2}, {0, -1, -2, -1}, {0, 0, 1, 2}};

menu_frame(win, x, y, w, h, func, color, frame)
int win, x, y, w, h, func, color, frame;
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

	for (c = 0; c < 4; c++)		/* connect each corner to itself and next
								   corner */
	{
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
