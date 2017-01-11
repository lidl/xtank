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

#ifdef S1024x864
/* Window geometry information */
#define ANIM_WIN_X        0
#define ANIM_WIN_Y        0
#define ANIM_WIN_WIDTH    768
#define ANIM_WIN_HEIGHT   768

#define HELP_WIN_X        0
#define HELP_WIN_Y        768
#define HELP_WIN_WIDTH    (1015-MAP_WIN_WIDTH)
#define HELP_WIN_HEIGHT   90

#define MAP_WIN_X         768
#define MAP_WIN_Y         0
#define MAP_WIN_WIDTH     250
#define MAP_WIN_HEIGHT    250

#define CONS_WIN_X        768
#define CONS_WIN_Y        (MAP_WIN_Y + MAP_WIN_HEIGHT + 6)
#define CONS_WIN_WIDTH    250
#define CONS_WIN_HEIGHT   306

#define MSG_WIN_X         768
#define MSG_WIN_Y         (CONS_WIN_Y + CONS_WIN_HEIGHT + 6)
#define MSG_WIN_WIDTH     250
#define MSG_WIN_HEIGHT    92

#define GAME_WIN_X        768
#define GAME_WIN_Y        (MSG_WIN_Y + MSG_WIN_HEIGHT + 6)
#define GAME_WIN_WIDTH    250
#define GAME_WIN_HEIGHT   192

/* Battle windows.  Not always mapped */
#define STAT_WIN_X	768
#define STAT_WIN_Y	0
#define STAT_WIN_WIDTH	252
#define STAT_WIN_HEIGHT	97

#define BORDER          3
#define STAT_BORDER	2
#endif

#ifdef S640x400
/* Window geometry information */
#define ANIM_WIN_X        0
#define ANIM_WIN_Y        0
#define ANIM_WIN_WIDTH    400
#define ANIM_WIN_HEIGHT   400

#define HELP_WIN_X        0
#define HELP_WIN_Y        310
#define HELP_WIN_WIDTH    638
#define HELP_WIN_HEIGHT   88

#define GAME_WIN_X        400
#define GAME_WIN_Y        0
#define GAME_WIN_WIDTH    238
#define GAME_WIN_HEIGHT   192

#define CONS_WIN_X        400
#define CONS_WIN_Y        0
#define CONS_WIN_WIDTH    238
#define CONS_WIN_HEIGHT   158

#define MAP_WIN_X         400
#define MAP_WIN_Y         160
#define MAP_WIN_WIDTH     238
#define MAP_WIN_HEIGHT    238

#define MSG_WIN_X         0
#define MSG_WIN_Y         0
#define MSG_WIN_WIDTH     238
#define MSG_WIN_HEIGHT    88

/* Battle windows.  Not always mapped */
#define STAT_WIN_X	400
#define STAT_WIN_Y	0
#define STAT_WIN_WIDTH	238
#define STAT_WIN_HEIGHT	60

#define BORDER          1
#define STAT_BORDER	1
#endif


#define SCREEN_WIDTH	ANIM_WIN_WIDTH	/* for robot interface */
#define SCREEN_HEIGHT	ANIM_WIN_HEIGHT

#define BOX_WIDTH	(ANIM_WIN_WIDTH/NUM_BOXES)
#define BOX_HEIGHT	(ANIM_WIN_HEIGHT/NUM_BOXES)
