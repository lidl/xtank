/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** interface.h
*/

#ifdef S1024x864
/* Menu coordinates */
#define LEV0_Y          120
#define LEV0_X          15
#define LEV1_X          170
#define LEV2_X          355
#define LEV3_X		550

#define COMBATANTS_X    1
#define COMBATANTS_Y    1
#define GRID_X          100
#define GRID_Y          10
#define PLAYERS_X       100
#define PLAYERS_Y       50
#define PROGRAMS_X      200
#define PROGRAMS_Y      50
#define VEHICLES_X      300
#define VEHICLES_Y      50
#define TEAMS_X         450
#define TEAMS_Y         50
#define MAZES_X         450
#define MAZES_Y         50
#define SETUPS_X        500
#define SETUPS_Y        50

/* Height of top teir of menus */
#define TIER1_HEIGHT    275

/* Number of fields in the grid */
#define MAX_GRIDS       4

/* Number of character width of each field of the combatants grid */
#define MAX_COMB_WID    12

/* Initial value for all the grid_val members */
#define UNDEFINED 255

/* Asking questions coordinates in rows and columns */
#define ASK_X		39
#define ASK_Y		10

/* The standard font for the interface */
#define INT_FONT       M_FONT

#define DISP_X 3
#define DISP_Y 3
#define DISP_WIDTH  80
#define DISP_HEIGHT 46

#endif
#ifdef S640x400
#define INT_FONT       S_FONT

#define DISP_X 2
#define DISP_Y 2
#define DISP_WIDTH  45
#define DISP_HEIGHT 45
#endif

#define iprint(str,x,y) \
  (display_mesg2(ANIM_WIN,str,x,y,INT_FONT))

#define clear_ask() \
  (clear_text_rc(ANIM_WIN,ASK_X,ASK_Y,50,4,INT_FONT))

#define MAIN_MENU        0

#define PLAY_MENU        (MAIN_MENU+1)
#define SETTINGS_MENU    (PLAY_MENU+1)
#define COMBATANTS_MENU  (SETTINGS_MENU+1)
#define VIEW_MENU        (COMBATANTS_MENU+1)
#define LOAD_MENU        (VIEW_MENU+1)
#define DESIGN_MENU      (LOAD_MENU+1)
#define HELP_MENU        (DESIGN_MENU+1)

#define MAZES_MENU       (HELP_MENU+1)
#define SETUPS_MENU      (MAZES_MENU+1)
#define GAMES_MENU       (SETUPS_MENU+1)
#define PLAYERS_MENU     (GAMES_MENU+1)
#define PROGRAMS_MENU    (PLAYERS_MENU+1)
#define VEHICLES_MENU    (PROGRAMS_MENU+1)
#define TEAMS_MENU       (VEHICLES_MENU+1)
#define NUM_MENU         (TEAMS_MENU+1)
#define FLAGS_MENU       (NUM_MENU+1)

#define MAX_MENUS        (FLAGS_MENU+1)
#define MAX_GAMES        5

#define SET_DENSITY    0
#define SET_DIFFICULTY 1
#define SET_OUTPOST    2
#define SET_SCROLL     3
#define SET_BOX_SLOW   4
#define SET_DISC_FRIC  5
#define SET_DISC_SLOW  6
