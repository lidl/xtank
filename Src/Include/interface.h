/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** interface.h
*/

/*
$Author: lidl $
$Id: interface.h,v 1.1.1.1 1995/02/01 00:25:40 lidl Exp $
*/

#ifdef S1024x864
/* Menu coordinates */
#define LEV0_Y          150
#define LEV0_X          15
#define LEV1_X          190
#define LEV2_X          380
#define LEV3_X			570

#define DONE_X		30  /*HAK*/
#define DONE_Y		30
#define COMBATANTS_X    30
#define COMBATANTS_Y    600
#define GRID_X          110
#define GRID_Y          30
#define PLAYERS_X       110  /*HAK*/
#define PLAYERS_Y       90
#define PROGRAMS_X      170
#define PROGRAMS_Y      90
#define VEHICLES_X      290
#define VEHICLES_Y      90
#define TEAMS_X         450
#define TEAMS_Y         90
#define MAZES_X         420
#define MAZES_Y         90
#define SETUPS_X        580
#define SETUPS_Y        90

/* Height of top teir of menus */
#define TIER1_HEIGHT    260

/* Number of fields in the grid */
#define MAX_GRIDS       4

/* Number of character width of each field of the combatants grid */
#define MAX_COMB_WID    17

/* Initial value for all the grid_val members */
#define UNDEFINED	255

/* Asking questions coordinates in rows and columns */
#define ASK_X		39
#define ASK_Y		10

/* The standard font for the interface */
#define INT_FONT	M_FONT

#define DISP_X 3
#define DISP_Y 3
#define DISP_WIDTH  80
#define DISP_HEIGHT 46
#endif

#ifdef S640x400
#define INT_FONT	S_FONT

#define DISP_X 		2
#define DISP_Y 		2
#define DISP_WIDTH	45
#define DISP_HEIGHT	45
#endif

#define iprint(str,x,y) \
  (display_mesg2(ANIM_WIN,str,x,y,INT_FONT))

/* Changed to include args (HAK) 2/93 */
#define clear_ask(x,y) \
  (clear_text_rc(ANIM_WIN,(x),(y),50,4,INT_FONT))

/* Why not use an enum here? */
#define MAIN_MENU	0

#define MACHINE_MENU	(MAIN_MENU+1)
#define PLAY_MENU	(MACHINE_MENU+1)
#define SETTINGS_MENU	(PLAY_MENU+1)
#define DONE_MENU       (SETTINGS_MENU+1)  /*HAK*/
#define COMBATANTS_MENU	(DONE_MENU+1)
#define VIEW_MENU	(COMBATANTS_MENU+1)
#define LOAD_MENU	(VIEW_MENU+1)
#define DESIGN_MENU	(LOAD_MENU+1)
#define HELP_MENU	(DESIGN_MENU+1)

#define MAZES_MENU	(HELP_MENU+1)
#define SETUPS_MENU	(MAZES_MENU+1)
#define GAMES_MENU	(SETUPS_MENU+1)
#define PLAYERS_MENU	(GAMES_MENU+1)	/* must be sequential */
#define PROGRAMS_MENU	(PLAYERS_MENU+1)	/* must be sequential */
#define VEHICLES_MENU	(PROGRAMS_MENU+1)	/* must be sequential */
#define TEAMS_MENU	(VEHICLES_MENU+1)	/* must be sequential */
#define NUM_MENU	(TEAMS_MENU+1)
#define FLAGS_MENU	(NUM_MENU+1)
#define FORCE_MENU  (FLAGS_MENU+1)

#define MAX_MENUS	(FORCE_MENU+1)

#define SET_DENSITY	0
#define SET_DIFFICULTY  1
#define SET_OUTPOST     2
#define SET_SCROLL      3
#define SET_BOX_SLOW    4
#define SET_DISC_FRIC   5
#define SET_DISC_SLOW   6
#define SET_SHOCKERWALL 7
#define SET_DISC_DAMAGE	8
#define SET_DISC_HEAT	9
#define SET_DISC_SPEED	10
#define SET_MAD_DISKS	11
