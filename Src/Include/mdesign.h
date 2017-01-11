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

#ifndef _MDESIGN_H_
#define _MDESIGN_H_

/* Padding on each side of the maze in boxes */
#define PAD 2

/* Height and width of a box in pixels */
#define DES_BOX_WIDTH  24
#define DES_BOX_HEIGHT 24

/* Offsets of maze from upper left of window */
#define X_OFFSET (-40)
#define Y_OFFSET (-40)

/* How close the mouse must be to line to toggle */
#define THRESHOLD 5

/* Actions used when dealing with walls */
#define MAKE_NORM_WALL  0
#define MAKE_DEST_WALL  1
#define DESTROY_WALL    2
#define CONTINUE        3

/* Interface locations and fonts */
#define INFO_Y   43
#define INPUT_Y  46
#define MD_FONT  M_FONT

typedef unsigned int Wall;

typedef struct {
	int x, y;	/* coordinates of a place in terms of boxes */
} BoxC;

typedef struct {
	int x, y;	/* coordinates of a place in terms of pixels */
} PixC;

#endif /* !_MDESIGN_H_ */
