/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** screen.h
*/

#ifdef S1024x864
/* Window geometry information */
#define ANIM_WIN_X	0
#define ANIM_WIN_Y	0
#define ANIM_WIN_WIDTH	768
#define ANIM_WIN_HEIGHT	768

#define GAME_WIN_X	768
#define GAME_WIN_Y	0
#define GAME_WIN_WIDTH	250
#define GAME_WIN_HEIGHT	192

#define CONS_WIN_X	768
#define CONS_WIN_Y	296
#define CONS_WIN_WIDTH	250
#define CONS_WIN_HEIGHT	210

#define MAP_WIN_X	768
#define MAP_WIN_Y	512
#define MAP_WIN_WIDTH	250
#define MAP_WIN_HEIGHT	250

#define HELP_WIN_X	0
#define HELP_WIN_Y	768
#define HELP_WIN_WIDTH  1018
#define HELP_WIN_HEIGHT	90

#define MSG_WIN_X	768
#define MSG_WIN_Y	198
#define MSG_WIN_WIDTH   250
#define MSG_WIN_HEIGHT	92

/* Battle windows.  Not always mapped */
#define STAT_WIN_X	768
#define STAT_WIN_Y	0
#define STAT_WIN_WIDTH	252
#define STAT_WIN_HEIGHT	97

#define BORDER          3
#define STAT_BORDER	2

#define BOX_WIDTH	192
#define BOX_HEIGHT	192
#define NUM_BOXES	4
#endif

#ifdef S640x400
/* Window geometry information */
#define ANIM_WIN_X	0
#define ANIM_WIN_Y	0
#define ANIM_WIN_WIDTH	400
#define ANIM_WIN_HEIGHT	400

#define GAME_WIN_X	400
#define GAME_WIN_Y	0
#define GAME_WIN_WIDTH	238
#define GAME_WIN_HEIGHT	192

#define CONS_WIN_X	400
#define CONS_WIN_Y	0
#define CONS_WIN_WIDTH	238
#define CONS_WIN_HEIGHT	158

#define MAP_WIN_X	400
#define MAP_WIN_Y	160
#define MAP_WIN_WIDTH	238
#define MAP_WIN_HEIGHT	238

#define HELP_WIN_X	0
#define HELP_WIN_Y	310
#define HELP_WIN_WIDTH  638
#define HELP_WIN_HEIGHT	88

#define MSG_WIN_X	0
#define MSG_WIN_Y	0
#define MSG_WIN_WIDTH   238
#define MSG_WIN_HEIGHT	88

/* Battle windows.  Not always mapped */
#define STAT_WIN_X	400
#define STAT_WIN_Y	0
#define STAT_WIN_WIDTH	238
#define STAT_WIN_HEIGHT	60

#define BORDER          1
#define STAT_BORDER	1

#define BOX_WIDTH	100
#define BOX_HEIGHT	100
#define NUM_BOXES	4
#endif
