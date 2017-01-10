/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "tanklimits.h"
#include "xtank.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "vstructs.h"
#include "menu.h"
#include "interface.h"
#include "setup.h"
#include "bullet.h"
#include "terminal.h"
#include "vehicle.h"
#include "proto.h"

#ifdef UNIX
#include <sys/param.h>
#ifdef NEED_DIRENT_H
#include <dirent.h>
#endif
#ifdef NEED_SYS_DIR_H
#include <sys/dir.h>
#endif
#endif /* UNIX */

#include <stdlib.h>		/* for malloc(), free() */
#include <string.h>		/* for strdup() */
#include <unistd.h>		/* for unlink() */
#include "clfkr.h"

extern int num_veh;

extern int num_terminals;
extern Terminal *terminal[];
extern Terminal *term;

extern int num_mdescs;
extern Mdesc *mdesc;

extern int num_vdescs;
extern Vdesc *vdesc;

extern int num_sdescs;
extern Sdesc *sdesc;

extern int num_prog_descs;
extern Prog_desc *prog_desc[];
extern Settings settings;

extern char *version1;
extern char *version2;
extern char *version3;

extern CLFkr command_options;

/* Added in a '1' for the new combatants menu (HAK 2/93) */
static int whichlevel[MAX_MENUS] =
{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3};

/* For convienence */
static char *done_entries[] =
{"Done"};

/* a menu in the combatants screen (HAK 2/93) */
static char *comb_entries[] =
{"Load", "Reload All"};

char **vehicles_entries = NULL, **mazes_entries = NULL, **setups_entries = NULL;

int number_of_machines;

char
**machine_names, **machine_entries;

/* Added "Remove" to main_entries[], and removed "Player" from
** view_entries[] (HAK 2/93)
*/
static char
 *main_entries[] =
{"Play", "Settings", "Combatants", "View", "Load",
 "Design", "Add players", "Text entry", "Remove", "Help", "Quit"},
 *play_entries[] =
{"Standard", "Players", "Robots", "Customized"},

/* Added "Madman discs" to settings_entries (HAK 3/93) */

 *settings_entries[] =
{
	"Flags", "Maze", "Winning score", "Force Specials",
	"Game", "Shocker Walls", "Outpost strength", "Difficulty",
	"Scroll speed", "Box slowdown", "Disc friction", "Throwing speed",
	"Disc damage", "Disc heat", "Owner slowdown","Madman # discs", "Vehicle",
	"Save settings", "Load Settings"}, *view_entries[] =
{"Maze", "Vehicle", "Program", "Setup"}, *load_entries[] =
{"Maze", "Vehicle", "Program", "Setup"}, *design_entries[] =
{"Maze", "Vehicle"}, *help_entries[] =
{"General", "Pictures", "Multi-player", "Games",
 "Vehicles", "Mazes", "Setups", "Credits", "Motd",
 "Newsgroups", "Release Notes"}, *grid_entries[] =
{"Player", "Program", "Vehicle", "Team"}, *num_entries[] =
{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"}, *flags_entries[] =
{"Point bullets", "Ricochet", "Rel. shooting",
 "No wear", "Restart", "Commentator", "Full map",
 "Pay to Play", "Relative Disc", "War:Goals Only",
 "Ultimate:Own Goal", "Robots don't Win",
 "Scale Armor to Max", "No name tags",
 "Team Scoring", 
 "Players can Teleport", "Discs can Teleport",
 "'port from team", "'port from neutral",
 "'port to team", "'port to neutral",
 "'port from any to any"
}, *programs_entries[MAX_PDESCS], *force_entries[MAX_SPECIALS], *players_entries[MAX_TERMINALS];

char force_states[MAX_SPECIALS];

/* Grid text entries, numeric values, and menus for the combatants interface */
static Byte grid_val[MAX_GRIDS][MAX_VEHICLES];
static char *grid_ent[MAX_GRIDS][MAX_VEHICLES];
static int grid_id[MAX_GRIDS];

/* Global so message system can use it */
char *games_entries[] =
{"Combat", "War", "Ultimate", "Capture",
 "Race", "Madman"};
char *teams_entries[] =
{"Neutral", "Red", "Orange", "Yellow", "Green", "Blue",
 "Violet"};

/* Descriptions of program abilities */
static char *ability_desc[] =
{"Plays Combat", "Plays War", "Plays Ultimate",
 "Plays Capture", "Plays Race", "Shoots",
 "Explores", "Dodges", "Replenishes",
 "Uses teams", "Uses mines", "Uses slicks",
 "Uses side mounts", "Uses messages"};

static Menu_int menu_sys;

void
reset_dynamic_entries(void)
{
	int i;
	int val;

	/*
         * Since grid_ent[2][] points into vdesc, which was realloc()'d
         * in make_vdesc(), we need to reset it:  -KNG 930915
         */

        for (i = 0; i < MAX_VEHICLES; i++) {
          val = grid_val[2][i];
          grid_ent[2][i] = (val == UNDEFINED ? "" : vdesc[val].name);
        }
 
	menu_set_choices(&menu_sys, VEHICLES_MENU, vehicles_entries);
	menu_set_choices(&menu_sys, MAZES_MENU, mazes_entries);
	menu_set_choices(&menu_sys, SETUPS_MENU, setups_entries);

	fix_desc_menu(VDESC);
	fix_desc_menu(MDESC);
	fix_desc_menu(SDESC);

	for (i = 0; i < num_vdescs; ++i) {
		vehicles_entries[i] = vdesc[i].name;
	}

	if (num_mdescs) {
		for (i = 0; i < num_mdescs; ++i) {
			mazes_entries[i] = mdesc[i].name;
		}
		mazes_entries[i] = "Random";
	}
	for (i = 0; i < num_sdescs; ++i) {
		setups_entries[i] = sdesc[i].name;
	}
}

/*
** Initializes the main interface menus.
*/
void
init_flags_hil(void)
{
	menu_unhighlight(&menu_sys, FLAGS_MENU);

	/* Set up the correct highlighting for the flags menu */
	if (settings.point_bullets)
		menu_set_hil(&menu_sys, FLAGS_MENU, 0);
	if (settings.si.ricochet)
		menu_set_hil(&menu_sys, FLAGS_MENU, 1);
	if (settings.si.rel_shoot)
		menu_set_hil(&menu_sys, FLAGS_MENU, 2);
	if (settings.si.no_wear)
		menu_set_hil(&menu_sys, FLAGS_MENU, 3);
	if (settings.si.restart)
		menu_set_hil(&menu_sys, FLAGS_MENU, 4);
	if (settings.commentator)
		menu_set_hil(&menu_sys, FLAGS_MENU, 5);
	if (settings.si.full_map)
		menu_set_hil(&menu_sys, FLAGS_MENU, 6);
	if (settings.si.pay_to_play)
		menu_set_hil(&menu_sys, FLAGS_MENU, 7);
	if (settings.si.relative_disc)
		menu_set_hil(&menu_sys, FLAGS_MENU, 8);
	if (settings.si.war_goals_only)
		menu_set_hil(&menu_sys, FLAGS_MENU, 9);
	if (settings.si.ultimate_own_goal)
		menu_set_hil(&menu_sys, FLAGS_MENU, 10);
	if (settings.robots_dont_win)
		menu_set_hil(&menu_sys, FLAGS_MENU, 11);
	if (settings.max_armor_scale)
		menu_set_hil(&menu_sys, FLAGS_MENU, 12);
	if (settings.si.no_nametags)
		menu_set_hil(&menu_sys, FLAGS_MENU, 13);
	if (settings.si.team_score)
		menu_set_hil(&menu_sys, FLAGS_MENU, 14);
	if (settings.si.player_teleport)
		menu_set_hil(&menu_sys, FLAGS_MENU, 15);
	if (settings.si.disc_teleport)
		menu_set_hil(&menu_sys, FLAGS_MENU, 16);
	if (settings.si.teleport_from_team)
		menu_set_hil(&menu_sys, FLAGS_MENU, 17);
	if (settings.si.teleport_from_neutral)
		menu_set_hil(&menu_sys, FLAGS_MENU, 18);
	if (settings.si.teleport_to_team)
		menu_set_hil(&menu_sys, FLAGS_MENU, 19);
	if (settings.si.teleport_to_neutral)
		menu_set_hil(&menu_sys, FLAGS_MENU, 20);
	if (settings.si.teleport_any_to_any)
		menu_set_hil(&menu_sys, FLAGS_MENU, 21);
}

void
MakeForceString(char *pcTemp, int iNum)
{
	int iVal = force_states[iNum];

	(void) sprintf(pcTemp, "%-12s    ", special_stat[iNum].type);
	if (iVal == INT_FORCE_ON) {
		strcat(pcTemp, "ON");
	} else if (iVal == INT_FORCE_OFF) {
		strcat(pcTemp, "OFF");
	} else {
		strcat(pcTemp, "DONT");
	}

	if (!force_entries[iNum]) {
		force_entries[iNum] = (char *) malloc(40);	/* use define */
	}
	strcpy(force_entries[iNum], pcTemp);
}

void
init_interface(void)
{
	int iCtr;
	char acTemp[40];
	static int iFirstTime = 1;

	if (iFirstTime) {
		iFirstTime = 0;

		for (iCtr = 0; iCtr < MAX_SPECIALS; iCtr++) {
			force_entries[iCtr] = (char *) 0;
			force_states[iCtr] = INT_FORCE_DONT;
		}
	} else {
		for (iCtr = 0; iCtr < MAX_SPECIALS; iCtr++) {
			force_states[iCtr] = INT_FORCE_DONT;
			if (force_entries[iCtr]) {
				free(force_entries[iCtr]);
				force_entries[iCtr] = (char *) 0;
			}
		}
	}

	for (iCtr = 0; iCtr < MAX_SPECIALS; iCtr++) {
		MakeForceString(acTemp, iCtr);
	}

	init_players();
	menu_sys_window(&menu_sys, ANIM_WIN);

/* Changed 10 to 11:  (HAK 2/93) */
	menu_norm_make(&menu_sys, MAIN_MENU, "XTANK", 11, 0,
				   LEV0_X, LEV0_Y, main_entries, L_FONT);
	menu_flag_make(&menu_sys, MACHINE_MENU, "Machines",
				   number_of_machines, 0,
				   LEV1_X, LEV0_Y, machine_entries, M_FONT);
	menu_norm_make(&menu_sys, PLAY_MENU, "Play", 4, 0,
				   LEV1_X, LEV0_Y, play_entries, M_FONT);
	menu_norm_make(&menu_sys, SETTINGS_MENU, "Settings",
				   (sizeof(settings_entries) / sizeof(char *)), 0,	/* GHS */
				   LEV1_X, LEV0_Y, settings_entries, M_FONT);
/* Changed 5 to 4:  (HAK 2/93) */
	menu_norm_make(&menu_sys, VIEW_MENU, "View", 4, 0,
				   LEV1_X, LEV0_Y, view_entries, M_FONT);
	menu_norm_make(&menu_sys, LOAD_MENU, "Load", 4, 0,
				   LEV1_X, LEV0_Y, load_entries, M_FONT);
	menu_norm_make(&menu_sys, DESIGN_MENU, "Design", 2, 0,
				   LEV1_X, LEV0_Y, design_entries, M_FONT);
	menu_norm_make(&menu_sys, HELP_MENU, "Help", 11, 0,
				   LEV1_X, LEV0_Y, help_entries, M_FONT);
	menu_nohil_make(&menu_sys, GAMES_MENU, "Games",
					(sizeof(games_entries) / sizeof(char *)), 0,
					LEV2_X, LEV0_Y, games_entries, M_FONT);

	menu_noti_make(&menu_sys, NUM_MENU, "", 11, 0,
				   LEV2_X, LEV0_Y, num_entries, M_FONT);
	menu_flag_make(&menu_sys, FLAGS_MENU, "Flags",
				   (sizeof(flags_entries) / sizeof(char *)),	/* GHS */
				   0, LEV3_X, LEV0_Y, flags_entries, M_FONT);
	menu_left_make(&menu_sys, FORCE_MENU, "Force Specials   Opt",
				   (sizeof(force_entries) / sizeof(char *)),
				   0, LEV2_X, LEV0_Y, force_entries, M_FONT);

	init_flags_hil();

	init_comb_menus();
}

/*
** Initializes the combatants interface menus.
*/
void
init_comb_menus(void)
{
	int grid_wid, i, j;

	grid_id[0] = MAX_MENUS;
	for (i = 1; i < MAX_GRIDS; i++) {
		grid_id[i] = grid_id[i - 1] + 1;
	}

	for (i = 0; i < num_prog_descs; ++i)
		programs_entries[i] = prog_desc[i]->name;

	for (i = 0; i < num_vdescs; ++i) {
		vehicles_entries[i] = vdesc[i].name;
	}

	for (i = 0; i < num_mdescs; ++i) {
		mazes_entries[i] = mdesc[i].name;
	}
	mazes_entries[i] = "Random";

	for (i = 0; i < num_sdescs; ++i) {
		setups_entries[i] = sdesc[i].name;
	}

	for (i = 0; i < num_terminals; ++i) {
		players_entries[i] = terminal[i]->player_name;
	}

	grid_wid = 1 + MAX_COMB_WID * font_string_width("M", M_FONT);

	for (i = 0; i < MAX_GRIDS; i++) {
		for (j = 0; j < MAX_VEHICLES; j++) {
			grid_ent[i][j] = "";
			grid_val[i][j] = UNDEFINED;
		}
		menu_recv_make(&menu_sys, grid_id[i], grid_entries[i], MAX_VEHICLES,
					   grid_wid, GRID_X + (grid_wid + 1) * i, GRID_Y,
					   grid_ent[i], M_FONT);
		menu_set_frame(&menu_sys, grid_id[i], 1);
	}

	/* changed this: (HAK 2/93) */
	menu_simp_make(&menu_sys, DONE_MENU, "", 1, 0,
				   DONE_X, DONE_Y,
				   done_entries, L_FONT);
	/* added this: (HAK 2/93) */
	menu_noti_make(&menu_sys, COMBATANTS_MENU, "", 2, 0,
				COMBATANTS_X, COMBATANTS_Y,
				comb_entries, L_FONT);
	menu_nohil_make(&menu_sys, PLAYERS_MENU, "Players", num_terminals, 0,
					PLAYERS_X, PLAYERS_Y + TIER1_HEIGHT,
					players_entries, M_FONT);

/* Changed GRID_X to PLAYERS_X in the next three, so we can move 'em relative
 * to the grid.  We want to slide it all over so the players menu doesn't
 * collide with the main menu (HAK 2/93)
 */
	menu_nohil_make(&menu_sys, PROGRAMS_MENU, "Programs", num_prog_descs, 0,
					PLAYERS_X + (grid_wid), PROGRAMS_Y + TIER1_HEIGHT,
					programs_entries, M_FONT);
	menu_scroll_make(&menu_sys, VEHICLES_MENU, "Vehicles", num_vdescs, 0,
					 PLAYERS_X + (grid_wid * 2), VEHICLES_Y + TIER1_HEIGHT,
					 vehicles_entries, M_FONT);
	menu_nohil_make(&menu_sys, TEAMS_MENU, "Teams", 7, 0,
					PLAYERS_X + (grid_wid * 3), TEAMS_Y + TIER1_HEIGHT,
					teams_entries, M_FONT);
	menu_scroll_make(&menu_sys, MAZES_MENU, "Mazes", num_mdescs + 1, 0,
					 GRID_X + (grid_wid * 2), MAZES_Y + TIER1_HEIGHT,
					 mazes_entries, M_FONT);
	menu_norm_make(&menu_sys, SETUPS_MENU, "Setups", num_sdescs, 0,
				   SETUPS_X, SETUPS_Y + TIER1_HEIGHT,
				   setups_entries, M_FONT);
}

int
sub_interface_main(int choice)
{
	int retval = 0;

	switch (choice) {
	  case 0:
		  menu_display(&menu_sys, PLAY_MENU);
		  break;
	  case 1:
		  menu_display(&menu_sys, SETTINGS_MENU);
		  break;
	  case 2:
		  do_comb();
		  break;
	  case 3:
		  menu_display(&menu_sys, VIEW_MENU);
		  break;
	  case 4:
		  menu_display(&menu_sys, LOAD_MENU);
		  break;
	  case 5:
		  menu_display(&menu_sys, DESIGN_MENU);
		  break;
	  case 6:
		  menu_display(&menu_sys, MACHINE_MENU);
		  break;
	  case 7:
		  add_players();
		  menu_unhighlight(&menu_sys, MAIN_MENU);
		  break;
	  case 8:	/* Inserted this case (HAK 2/93) */
		  menu_display(&menu_sys, PLAYERS_MENU);
		  break;
	  case 9:
		  menu_display(&menu_sys, HELP_MENU);
		  break;
	  case 10:
		  retval = 1;
		  break;
	}
	return (retval);
}

int
sub_interface_force(int choice)
{
	int iVal;
	char acTemp[40];

	iVal = force_states[choice];

	if (iVal == INT_FORCE_DONT) {
		iVal = INT_FORCE_ON;
	} else if (iVal == INT_FORCE_ON) {
		iVal = INT_FORCE_OFF;
	} else if (iVal == INT_FORCE_OFF) {
		iVal = INT_FORCE_DONT;
	}
	force_states[choice] = iVal;

	MakeForceString(acTemp, choice);

	menu_redraw(&menu_sys, FORCE_MENU);

	return (0);
}

void
sub_interface_view(int choice)
{
	switch (choice) {
	  case 0:
		  menu_display(&menu_sys, MAZES_MENU);
		  break;
	  case 1:
		  menu_display(&menu_sys, VEHICLES_MENU);
		  break;
	  case 2:
		  menu_display(&menu_sys, PROGRAMS_MENU);
		  break;
	  case 3:
		  menu_display(&menu_sys, SETUPS_MENU);
		  break;
/* (HAK 2/93) Need the players menu for the "Remove Player" item in the
    Main Menu. */
/*
	  case 4:
		  menu_display(&menu_sys, PLAYERS_MENU);
		  break;
*/
	}
}

void
sub_interface_load(int choice)
{
	switch (choice) {
	  case 0:
		  interface_load(MDESC);
		  break;
	  case 1:
		  interface_load(VDESC);
		  break;
	  case 2:
		  make_prog_desc();
		  expose_win(ANIM_WIN, TRUE);
		  break;
	  case 3:
		  interface_load(SDESC);
		  break;
	}
	menu_unhighlight(&menu_sys, LOAD_MENU);
}

void
sub_interface_design(int choice)
{
	switch (choice) {
	  case 0:
		  design_maze();
		  expose_win(ANIM_WIN, TRUE);
		  break;
	  case 1:
		  design_vehicle();
		  expose_win(ANIM_WIN, TRUE);
		  break;
	}
	menu_unhighlight(&menu_sys, DESIGN_MENU);
}

void
sub_interface_help(int choice)
{
	switch (choice) {
	  case 0:
		  display_file(ANIM_WIN, "Help/general");
		  break;
	  case 1:
		  display_pics();
		  expose_win(ANIM_WIN, TRUE);
		  break;
	  case 2:
		  display_file(ANIM_WIN, "Help/multi-player");
		  break;
	  case 3:
		  display_file(ANIM_WIN, "Help/games");
		  break;
	  case 4:
		  display_file(ANIM_WIN, "Help/vehicles");
		  break;
	  case 5:
		  display_file(ANIM_WIN, "Help/mazes");
		  break;
	  case 6:
		  display_file(ANIM_WIN, "Help/setups");
		  break;
	  case 7:
		  display_file(ANIM_WIN, "Help/credits");
		  break;
	  case 8:
		  display_file(ANIM_WIN, "Help/motd");
		  break;
	  case 9:
		  display_file(ANIM_WIN, "Help/xtank.FAQ");
		  break;
	  case 10:
		  display_file(ANIM_WIN, "Help/release-notes");
		  break;
	}
	menu_unhighlight(&menu_sys, HELP_MENU);
	expose_win(ANIM_WIN, TRUE);
}

void
sub_interface_maze(int choice, EventType button)
{
	/* If the settings menu is up, set the settings, otherwise,  */
	/* display information about the maze.                       */

	if (menu_is_up(&menu_sys, SETTINGS_MENU) && button == EVENT_LBUTTON) {
		if (choice < num_mdescs) {
			settings.mdesc = &mdesc[choice];
			settings.si.game = settings.mdesc->type;
		} else {
			/* Random maze selected */
			settings.mdesc = (Mdesc *) NULL;
			menu_erase(&menu_sys, MAZES_MENU);
			menu_unhighlight(&menu_sys, SETTINGS_MENU);
			ask_maze_density();
		}
		display_settings();
	} else if (choice < num_mdescs) {
		do_view(MAZES_MENU, choice);
	}
}

void
sub_interface_play(int choice)
{
	switch (choice) {
	  case 0:
		  standard_combatants();
		  break;
	  case 1:
		  player_combatants();
		  break;
	  case 2:
		  robot_combatants();
		  break;
	  case 3:
		  customized_combatants();
		  break;
	}
	interface_play();
	menu_unhighlight(&menu_sys, PLAY_MENU);
}

void
sub_interface_machine(int choice)
{
	add_given_player(choice);
}

void
sub_interface_settings(int choice)
{
	char acFileName[MAXPATHLEN];
	static int inited = 0;
	static char acPrevFileName[MAXPATHLEN];

	if (!inited) {
		acPrevFileName[0] = '\0';
		acFileName[0] = '\0';

		inited = 1;
	}
	switch (choice) {
	  case 0:
		  menu_display(&menu_sys, FLAGS_MENU);
		  break;
	  case 1:
		  menu_display(&menu_sys, MAZES_MENU);
		  break;
	  case 2:
		  ask_winning_score();
		  /* Unhighlight selection and redisplay settings */
		  /* if a value in the settings was changed.      */
		  display_settings();
		  menu_unhighlight(&menu_sys, SETTINGS_MENU);
		  break;
	  case 3:
		  /* force specials */
		  menu_display(&menu_sys, FORCE_MENU);
		  break;
	  case 4:
		  menu_display(&menu_sys, GAMES_MENU);
		  break;
	  case 5:
		  do_num(SET_SHOCKERWALL, TRUE);
		  break;
	  case 6:
		  do_num(SET_OUTPOST, TRUE);
		  break;
	  case 7:
		  do_num(SET_DIFFICULTY, TRUE);
		  break;
	  case 8:
		  do_num(SET_SCROLL, TRUE);
		  break;
	  case 9:
		  do_num(SET_BOX_SLOW, TRUE);
		  break;
	  case 10:
		  do_num(SET_DISC_FRIC, TRUE);
		  break;
	  case 11:
		  do_num(SET_DISC_SPEED, TRUE);
		  break;
	  case 12:
		  do_num(SET_DISC_DAMAGE, TRUE);
		  break;
	  case 13:
		  do_num(SET_DISC_HEAT, TRUE);
		  break;
	  case 14:
		  do_num(SET_DISC_SLOW, TRUE);
		  break;
	  case 15: /* Added this case (HAK 3/93) */
		  do_num(SET_MAD_DISKS, TRUE);
		  break;
	  case 16:
		  menu_display(&menu_sys, VEHICLES_MENU);
		  break;
	  case 17:
		  clear_window(ANIM_WIN);
		  iprint("Enter the settings filename.", 0, 8);
		  input_filename(ANIM_WIN, acPrevFileName, acFileName,
						 10, INT_FONT, 256);
		  save_settings(acFileName);
		  set_terminal(0);
		  expose_win(ANIM_WIN, TRUE);
		  break;
	  case 18:
		  clear_window(ANIM_WIN);
		  iprint("Enter the settings filename.", 0, 8);
		  input_filename(ANIM_WIN, acPrevFileName, acFileName,
						 10, INT_FONT, 256);
		  load_settings(acFileName);
		  display_settings();
		  init_flags_hil();
		  expose_win(ANIM_WIN, TRUE);
		  break;
	}
}

void
sub_interface_flags(int choice)
{
	switch (choice) {
	  case 0:
		  settings.point_bullets ^= TRUE;
		  break;
	  case 1:
		  settings.si.ricochet ^= TRUE;
		  break;
	  case 2:
		  settings.si.rel_shoot ^= TRUE;
		  break;
	  case 3:
		  settings.si.no_wear ^= TRUE;
		  break;
	  case 4:
		  settings.si.restart ^= TRUE;
		  break;
	  case 5:
		  settings.commentator ^= TRUE;
		  break;
	  case 6:
		  settings.si.full_map ^= TRUE;
		  break;
	  case 7:
		  settings.si.pay_to_play ^= TRUE;
		  if (settings.si.pay_to_play) {
			  settings.si.restart = TRUE;
			  menu_set_hil(&menu_sys, FLAGS_MENU, 4);
		  }
		  break;
	  case 8:
		  settings.si.relative_disc ^= TRUE;
		  break;
	  case 9:
		  settings.si.war_goals_only ^= TRUE;
		  break;
	  case 10:
		  settings.si.ultimate_own_goal ^= TRUE;
		  break;
	  case 11:
		  settings.robots_dont_win ^= TRUE;
		  break;
	  case 12:
		  settings.max_armor_scale ^= TRUE;
		  break;
	  case 13:
		  settings.si.no_nametags ^= TRUE;
		  break;
	  case 14:
		  settings.si.team_score ^= TRUE;
		  break;
	  case 15:
		  settings.si.player_teleport ^= TRUE;
		  break;
	  case 16:
		  settings.si.disc_teleport ^= TRUE;
		  break;
	  case 17:
		  settings.si.teleport_from_team ^= TRUE;
		  break;
	  case 18:
		  settings.si.teleport_from_neutral ^= TRUE;
		  break;
	  case 19:
		  settings.si.teleport_to_team ^= TRUE;
		  break;
	  case 20:
		  settings.si.teleport_to_neutral ^= TRUE;
		  break;
	  case 21:
		  settings.si.teleport_any_to_any ^= TRUE;
		  break;
	}
}

/*
** The top level interface to xtank.
*/

int
main_interface(void)
{
	Event ev;
	int numev, menu, choice;
	int itemp;

	set_terminal(0);
	init_interface();

	/* Jimmy - set default maze to first in load list */
	settings.mdesc = &mdesc[0];

	/* Jimmy - set corresponding game type */
	settings.si.game = settings.mdesc->type;

	/* Jimmy - enter all players in combatants grid */
	for (itemp = 0; itemp < num_terminals; itemp++)
		fix_combantants(itemp);

#ifdef X11
	button_up(ANIM_WIN, TRUE);
	follow_mouse(ANIM_WIN, TRUE);
#endif

	if (command_options.UseSetting) {
		if (!load_settings(command_options.Settings)) {
			puts("Failed to load settings file.\n");
			return (0);
		}
	}
	if (command_options.AutoStart) {
		if (command_options.UseSetting) {
			sub_interface_play(3);
		} else {
			sub_interface_play(0);
		}

		command_options.AutoStart = FALSE;

		if (command_options.AutoExit) {
			return (0);
		}
	}
	/* Display the 10 then the title */
	display_file(ANIM_WIN, "Help/motd");
	display_title(TRUE);

	/* Display the settings and the main menu */
	display_settings();
	menu_display(&menu_sys, MAIN_MENU);

	while (1) {
		if (win_exposed(ANIM_WIN)) {
			clear_window(ANIM_WIN);
			display_title(FALSE);
			menu_system_expose(&menu_sys);
			expose_win(ANIM_WIN, FALSE);
		}
		if (win_exposed(GAME_WIN)) {
			display_settings();
			expose_win(GAME_WIN, FALSE);
		}
		numev = 1;
		get_events(&numev, &ev);
		if (numev == 0)
			continue;
		switch (ev.type) {
		  case EVENT_LBUTTON:
		  case EVENT_MBUTTON:
		  case EVENT_RBUTTON:
			  menu = menu_hit(&menu_sys, ev.x, ev.y);
			  if (menu != MENU_NULL) {
				  /* Erase all equal or higher level menus */
				  erase_other_menus(menu);

				  /* Find out which choice on the menu was selected */
				  menu_hit_p(&menu_sys, &ev, &menu, &choice, &itemp);

				  switch (menu) {
					case MAIN_MENU:
						if (sub_interface_main(choice)) {
							return (0);
						}
						break;

					case VIEW_MENU:
						sub_interface_view(choice);
						break;

					case LOAD_MENU:
						sub_interface_load(choice);
						break;

					case DESIGN_MENU:
						sub_interface_design(choice);
						break;

					case HELP_MENU:
						sub_interface_help(choice);
						break;

					case GAMES_MENU:
						settings.si.game = (Game) choice;
						display_settings();
						break;

					case PLAYERS_MENU:
						/* HAK 2/93
						do_view(PLAYERS_MENU, choice);
						*/
						menu_unhighlight(&menu_sys, PLAYERS_MENU);
						if (choice)
							remove_player(choice);

						expose_win(ANIM_WIN, TRUE);
						break;

					case PROGRAMS_MENU:
						do_view(PROGRAMS_MENU, choice);
						break;

					case VEHICLES_MENU:
						if (menu_is_up(&menu_sys, SETTINGS_MENU))
							term->vdesc = choice;
						else
							do_view(VEHICLES_MENU, choice);
						break;

					case MAZES_MENU:
						sub_interface_maze(choice, ev.type);
						break;

					case SETUPS_MENU:
						do_view(SETUPS_MENU, choice);
						break;

					case MACHINE_MENU:
						sub_interface_machine(choice);
						break;

					case PLAY_MENU:
						sub_interface_play(choice);
						if (command_options.AutoExit) {
							return (0);
						}
						break;

					case SETTINGS_MENU:
						sub_interface_settings(choice);
						break;

					case NUM_MENU:
						do_num(choice, FALSE);
						break;

					case FLAGS_MENU:
						sub_interface_flags(choice);
						break;

					case FORCE_MENU:
						sub_interface_force(choice);
						break;
				  }
			  }
			  break;
		}
	}
}

/*
** Erases all menus on equal or higher levels than the specified menu.
*/
void
erase_other_menus(int mu)
{
	int level, i;

	level = whichlevel[mu];

	/* Erase any other displayed menu that is of equal or higher level */
	for (i = 0; i < MAX_MENUS; i++)
		if (menu_is_up(&menu_sys, i) && whichlevel[i] >= level && i != mu)
			menu_erase(&menu_sys, i);
}

static int gridsel = 0;			/* the current row in the combatants menu */

/* return 1 if we should quit */

static int
handle_comb_button(Event *evp, int mv)
{
	/* mv - the mouse button number, from left */
	int gi;
	int choice;
	int menu;
	int old_gridsel = gridsel;
	int itemp;
	int just_scrolled = 0;

	menu_hit_p(&menu_sys, evp, &menu, &choice, &just_scrolled);
	switch (menu) {
	  case DONE_MENU:	/* quit button */ /*HAK 2/93*/
		  return 1;
	  case COMBATANTS_MENU: /* Added to handle the new menu (HAK 2/93) */
		  switch(choice) {
		    case 0:
			comb_load_v(); /*prompt for vehicle name and load*/
			break;
		    case 1:
			comb_load_all(); /*reload all prev. loaded vehicles*/
			break;
		  }
		  mv = -1;
		  break;
	  case PROGRAMS_MENU:
		  grid_ent[menu - PLAYERS_MENU][gridsel] = programs_entries[choice];
		  grid_val[menu - PLAYERS_MENU][gridsel] = choice;
		  break;
	  case PLAYERS_MENU:
		  /* search list to see if he has already been chosen */
		  for (itemp = 0; itemp < MAX_VEHICLES; itemp++) {
			  if (!strcmp(players_entries[choice],
						  grid_ent[menu - PLAYERS_MENU][itemp]))
				  break;
		  }
		  if (itemp != MAX_VEHICLES)
			  break;
		  grid_ent[menu - PLAYERS_MENU][gridsel] = players_entries[choice];
		  grid_val[menu - PLAYERS_MENU][gridsel] = choice;
		  break;
	  case VEHICLES_MENU:
		  grid_ent[menu - PLAYERS_MENU][gridsel] = vehicles_entries[choice];
		  grid_val[menu - PLAYERS_MENU][gridsel] = choice;
		  break;
	  case TEAMS_MENU:
		  grid_ent[menu - PLAYERS_MENU][gridsel] = teams_entries[choice];
		  grid_val[menu - PLAYERS_MENU][gridsel] = choice;
		  break;
	  case MENU_NULL:			/* background */
		  break;
	  default:					/* clicks on the grid itself */
		  if (mv == 2) {		/* right button erases the item */
			  grid_ent[menu - MAX_MENUS][choice] = "";
			  grid_val[menu - MAX_MENUS][choice] = UNDEFINED;
		  }
		  gridsel = choice;		/* select in any case */

		  mv = -1;				/* so we don't change gridsel again */
		  break;
	}

	if (!just_scrolled) {
		switch (mv) {
		  case 0:				/* left button moves up a row */
			  if (--gridsel < 0)
				  gridsel = MAX_VEHICLES - 1;
			  break;
		  case 2:				/* right button moves down a row */
			  if (++gridsel >= MAX_VEHICLES)
				  gridsel = 0;
			  break;
		}

		for (gi = 0; gi < MAX_GRIDS; ++gi) {
			menu_erase(&menu_sys, grid_id[gi]);
			if (old_gridsel != gridsel)
				menu_display(&menu_sys, grid_id[gi]);
			menu_disphil(&menu_sys, grid_id[gi], gridsel);
		}
	}
	return 0;
}

/*
** Sets up menus to form a grid of combatants.  User picks spot on
** grid and then picks a menu choice to fill that spot.  This progresses
** until the user clicks on done.
*/
void
do_comb(void)
{
	int should_quit = 0;
	int i, numev, mv;
	Event ev;

	/* initial hightlight */
	for (i = 0; i < MAX_GRIDS; ++i) {
		menu_set_hil(&menu_sys, grid_id[i], gridsel);
	}

	/* Expose the window to display the combatant menus */
	menu_sys_erase(&menu_sys);
	expose_win(ANIM_WIN, TRUE);

	do {
		if (win_exposed(ANIM_WIN)) {
			clear_window(ANIM_WIN);
			for (i = 0; i < MAX_GRIDS; i++) {
				menu_display(&menu_sys, grid_id[i]);
			}
			menu_display(&menu_sys, DONE_MENU);  /* HAK */
			menu_display(&menu_sys, PROGRAMS_MENU);
			menu_display(&menu_sys, VEHICLES_MENU);
			menu_display(&menu_sys, PLAYERS_MENU);
			menu_display(&menu_sys, COMBATANTS_MENU);
			menu_display(&menu_sys, TEAMS_MENU);
			expose_win(ANIM_WIN, FALSE);
		}
		numev = 1;
		mv = 0;
		get_events(&numev, &ev);
		if (numev != 0) {
			switch (ev.type) {
			  case EVENT_RBUTTON:
				  ++mv;
			  case EVENT_MBUTTON:
				  ++mv;
			  case EVENT_LBUTTON:
				  should_quit = handle_comb_button(&ev, mv);
				  break;
			}
		}
	} while (!should_quit);

	/* Erase this set of menus, and put up the main interface */
	menu_sys_erase(&menu_sys);
	display_title(FALSE);
	menu_display(&menu_sys, MAIN_MENU);
}

/*
** Puts the information from the specified combatants grid row into the
** given combatant structure.  Returns -1 if there's no combatant on the row
*/
int
make_grid_combatant(Combatant *c, int row)
{
	c->vdesc = grid_val[2][row];
	if (c->vdesc == UNDEFINED)
		return -1;
	c->player[0] = grid_val[0][row];
	c->num_players = (c->player[0] == UNDEFINED) ? 0 : 1;
	c->program[0] = grid_val[1][row];
	c->num_programs = (c->program[0] == UNDEFINED) ? 0 : 1;
	c->team = grid_val[3][row];
	if (c->team == UNDEFINED)
		c->team = NEUTRAL;
	if (c->player[0] == UNDEFINED && c->program[0] == UNDEFINED)
		return -1;
	return 0;
}

/*
** Doesn't belong here...
*/
int
combatant_to_grid(Combatant *c, int row)
{
	int val;
	char *ptr;

	if (c->vdesc == UNDEFINED) {
		return -1;
	}
	grid_val[2][row] = c->vdesc;
	grid_ent[2][row] = vehicles_entries[c->vdesc];

	/* PLAYER */
	val = c->player[0];
	if (val == UNDEFINED) {
		ptr = "";
	} else {
		ptr = players_entries[val];
	}
	grid_val[0][row] = val;
	grid_ent[0][row] = ptr;

	/* PROGRAM */
	val = c->program[0];
	if (val == UNDEFINED) {
		ptr = "";
	} else {
		ptr = programs_entries[c->program[0]];
	}
	grid_val[1][row] = val;
	grid_ent[1][row] = ptr;

	/* TEAM */
	if (c->team == UNDEFINED) {
		c->team = NEUTRAL;
	}
	grid_val[3][row] = c->team;
	grid_ent[3][row] = teams_entries[c->team];
	if (c->player[0] == UNDEFINED && c->program[0] == UNDEFINED) {
		return -1;
	}
	return 0;
}

/*
** When called with init true, internally remembers the choice, and
** displays a menu with selections from 0 to 10.
** When called with init false, sets the initialized choice to the number.
*/
void
do_num(int num, Boolean init)
{
	static int choice;

	if (init) {
		choice = num;
		menu_disphil(&menu_sys, NUM_MENU, setting_num(choice));
	} else {
		set_setting(choice, num);
		display_settings();
	}
}

/*
** Concatenates two strings and prints result on specified line of game window.
*/
#define display_game_str(s1,s2,line) \
    ((void) strcpy(temp,s1), \
     (void) strcat(temp,s2), \
     display_mesg(GAME_WIN,temp,line,M_FONT))

/*
** Converts a format and number to a string, and prints result on a given line.
*/
#define display_game_num(format,num,line) \
    ((void) sprintf(temp,format,num), \
     display_mesg(GAME_WIN,temp,line,M_FONT))

/*
** Displays information about the settings in the game window.
*/
void
display_settings(void)
{
	char temp[41];
	int line;

	clear_window(GAME_WIN);

	line = 0;
	display_game_str("Game:  ", games_entries[(int) settings.si.game], line++);
	display_game_num("Speed: %d", settings.game_speed, line++);

	/* If maze is random, display density, else display name */
	if (settings.mdesc != (Mdesc *) NULL)
		display_game_str("Maze:  ", settings.mdesc->name, line++);
	else
		display_game_num("Maze:  Density %d", setting_num(SET_DENSITY),
						 line++);
	display_game_num("Difficulty:       %d", settings.difficulty, line++);
	if (settings.si.game != ULTIMATE_GAME && settings.si.game != CAPTURE_GAME) {
		display_game_num("Winning score:    %d", settings.si.winning_score,
						 line++);
	}
	display_game_num("Outpost strength: %d", settings.si.outpost_strength,
					 line++);
	display_game_num("Scroll speed:     %.0f", settings.si.scroll_speed,
					 line++);
	display_game_num("Box slowdown:     %d", setting_num(SET_BOX_SLOW),
					 line++);
	display_game_num("Shocker Wall Str: %d", settings.si.shocker_walls,
					 line++);

	if (settings.si.game == ULTIMATE_GAME || settings.si.game == CAPTURE_GAME) {
		display_game_num("Disc friction:    %d", setting_num(SET_DISC_FRIC),
						 line++);
		display_game_num("Throwing speed:   %d", setting_num(SET_DISC_SPEED),
						 line++);
		display_game_num("Disc damage:      %d", setting_num(SET_DISC_DAMAGE),
						 line++);
		display_game_num("Disc heat:        %d", setting_num(SET_DISC_HEAT),
						 line++);
		display_game_num("Owner slowdown:   %d", setting_num(SET_DISC_SLOW),
						 line++);
	}
	if (settings.si.game == MADMAN_GAME)
		display_game_num("# discs:          %d", setting_num(SET_MAD_DISKS), line++);
}

/*
** Sets the specified setting to the given number scaled from the range 0 - 10.
*/
void
set_setting(int setting, int num)
{
	switch (setting) {
	  case SET_DENSITY:
		  settings.maze_density = num * 10;
		  break;
	  case SET_DIFFICULTY:
		  settings.difficulty = num;
		  break;
	  case SET_OUTPOST:
		  settings.si.outpost_strength = num;
		  break;
	  case SET_SHOCKERWALL:
		  settings.si.shocker_walls = num;
		  break;
	  case SET_SCROLL:
		  settings.si.scroll_speed = (float) num;
		  break;
	  case SET_BOX_SLOW:
		  settings.si.box_slowdown = 1.0 - (float) num / 20;
		  break;
	  case SET_DISC_FRIC:
		  settings.si.disc_friction = 1.0 - (float) num / 200;
		  break;
	  case SET_DISC_SLOW:
		  settings.si.owner_slowdown = 1.0 - (float) num / 10;
		  break;
	  case SET_DISC_DAMAGE:
		  settings.si.disc_damage = (float) num / 10;
		  break;
	  case SET_DISC_HEAT:
		  settings.si.disc_heat = 1.0 - (float) num / 10;
		  break;
	  case SET_DISC_SPEED:
		  settings.si.disc_speed = (float) num / 10;
		  break;
	  case SET_MAD_DISKS:
		  settings.si.num_discs = (num ? num : 1);
		  break;
	}
}

/*
** Returns the value of the specified setting scaled to the range 0 - 10.
*/
int
setting_num(int setting)
{
	switch (setting) {
	  case SET_DENSITY:
		  return (int) (settings.maze_density / 10 + .5);
	  case SET_DIFFICULTY:
		  return settings.difficulty;
	  case SET_OUTPOST:
		  return settings.si.outpost_strength;
	  case SET_SCROLL:
		  return (int) (settings.si.scroll_speed + .5);
	  case SET_BOX_SLOW:
		  return (int) ((1.0 - settings.si.box_slowdown) * 20 + .5);
	  case SET_DISC_FRIC:
		  return (int) ((1.0 - settings.si.disc_friction) * 200 + .5);
	  case SET_DISC_SLOW:
		  return (int) ((1.0 - settings.si.owner_slowdown) * 10 + .5);
	  case SET_DISC_DAMAGE:
		  return (int) (settings.si.disc_damage * 10 + .5);
	  case SET_DISC_HEAT:
		  return (int) ((1.0 - settings.si.disc_heat) * 10 + .5);
	  case SET_SHOCKERWALL:
		  return settings.si.shocker_walls;
	  case SET_DISC_SPEED:
		  return (int) (settings.si.disc_speed * 10 + .5);
	  case SET_MAD_DISKS:
		  return settings.si.num_discs;
	}
	return 0;
}

/*
** Plays a game or explains failure to the user.
*/
void
interface_play(void)
{
	int line, i;

	for (i = 0; i < num_terminals; i++) {
		set_terminal(i);
		reset_video();			/* %%% this is dumb */
	}

	clear_window(ANIM_WIN);
	if (play_game() == GAME_FAILED) {
		/* Report the result of the game to everyone */
		for (i = 0; i < num_terminals; i++) {
			set_terminal(i);
			line = 3;
			if (!command_options.AutoStart) {
				iprint("This game failed to work either because there were no",
					   0, line++);
				iprint("combatants or there was no room in the maze for them.",
					   0, line++);
				iprint("Vehicles won't be placed on landmarks in the maze.",
					   0, line++);
			} else {
				fprintf(stderr,
				   "This game failed to work either because there were no");
				fprintf(stderr,
				   "combatants or there was no room in the maze for them.");
				fprintf(stderr,
					  "Vehicles won't be placed on landmarks in the maze.");
				break;
			}
		}
		set_terminal(0);
		if (!command_options.AutoStart) {
			iprint("Any key or button to continue", 0, line + 1);
			wait_input();
		}
	}
	expose_win(ANIM_WIN, TRUE);
	expose_win(GAME_WIN, TRUE);
}

/*
** Displays information about a vehicle, maze, setup, player, or program.
*/
void
do_view(int menu, int choice)
{
	clear_window(ANIM_WIN);
	switch (menu) {
	  case VEHICLES_MENU:
		  display_vdesc(&vdesc[choice], ON);
		  break;
	  case MAZES_MENU:
		  display_mdesc(&mdesc[choice]);
		  break;
	  case PROGRAMS_MENU:
		  display_program(prog_desc[choice]);
		  break;
	  case SETUPS_MENU:
		  /* display_sdesc(&sdesc[choice]); */
		  break;
/*  We're using players menu for something else now (HAK 2/93)
	  case PLAYERS_MENU:
		  break;
*/
	}

	iprint("Hit any key or button to continue", 0, 50);
	wait_input();
	expose_win(ANIM_WIN, TRUE);
}

/*
** Displays information about the specified maze.
*/
void
display_mdesc(Mdesc *d)
{
	make_maze(d);
	display_mdesc_maze();
	display_mdesc_info(d);
}

/*
** Displays information about the specified program.
*/
void
display_program(Prog_desc *p)
{
	char temp[256], *ptr, *end_ptr;
	int row, width, len, i;

#ifdef S1024x864
	width = 75;
#endif

#ifdef S640x400
	width = 50;
#endif

	row = 0;
	sprintf(temp, "Name:    %s", p->name);
	iprint(temp, 0, row);
	row++;
	sprintf(temp, "Author:  %s", p->author);
	iprint(temp, 0, row);
	row++;
	sprintf(temp, "Skill:   %d", p->skill);
	iprint(temp, 0, row);
	row++;
	sprintf(temp, "Vehicle: %s", p->vehicle);
	iprint(temp, 0, row);
	row++;

	/* Print out abilities with word wrap */
	row++;
	iprint("Abilities:", 0, row);
	row++;
	temp[0] = '\0';
	i = 0;
	do {
		if (p->abilities & 1 << i) {
			ptr = ability_desc[i];
			if ((int) (strlen(temp) + strlen(ptr)) > (int) (width - 7)) {
				/* Print a row of text and start on a new one */
				iprint(temp, 5, row);
				row++;
				strcpy(temp, ptr);
				strcat(temp, "  ");
			} else
				strcat(temp, ptr);
			strcat(temp, "  ");
		}
	} while (++i < MAX_PROG_ABILITIES);

	if (temp[0] != '\0') {
		iprint(temp, 5, row);
		row++;
	}
	/* Print out strategy with word wrap */
	row++;
	iprint("Strategy:", 0, row);
	row++;
	for (ptr = p->strategy, end_ptr = ptr + strlen(ptr);
		 ptr < end_ptr;
		 ptr += len, row++) {
		len = end_ptr - ptr;

		if (len > width) {
			/* Do word wrap by looking for the nearest space to the left */
			len = width;
			while (len > 0 && ptr[len - 1] != ' ')
				len--;

			/* If we have a word that is width long, display the width */
			if (len == 0)
				len = width;
		}
		strncpy(temp, ptr, len);
		temp[len] = '\0';
		iprint(temp, 5, row);
	}
}

/*
** Prompts user for the number of players to add, and the display names
** for the terminals.  Checks the internet address, makes, and initializes
** each terminal.  Gets player information for each terminal.
*/
void
add_players(void)
{

#ifdef UNIX
	extern char *network_error_str[];
#endif

	extern char video_error_str[];
	extern int num_terminals;
	char name[80], result[256], *tmp;
	int num, ret, i;

	if (num_terminals == MAX_TERMINALS) {
		iprint("There is no room for more players.", ASK_X, ASK_Y);
		iprint("Hit any key or button to continue", ASK_X, ASK_Y + 1);
		wait_input();
		clear_ask(ASK_X, ASK_Y);  /* HAK 2/93 */
	} else {
		num = input_int(ANIM_WIN, "Number of players to add", ASK_X, ASK_Y,
						1, 0, MAX_TERMINALS - num_terminals, INT_FONT);
		clear_ask(ASK_X, ASK_Y);  /* HAK 2/93 */
		for (i = 0; i < num; i++) {
			input_string(ANIM_WIN, "Display name:", name,
						 ASK_X, ASK_Y, INT_FONT, 256);	/* FIXIT */
			if (name[0] == '\0') {
				clear_ask(ASK_X, ASK_Y);  /* HAK 2/93 */
				break;
			}
#ifdef UNIX
			tmp = name;
			if (ret = check_internet(1, &tmp))
				strcpy(result, network_error_str[ret]);
			else {
#endif

				(void) strcpy(result, "Initializing ");
				(void) strcat(result, name);
				iprint(result, ASK_X, ASK_Y + 1);
				flush_output();

				if (make_terminal(name))
					strcpy(result, video_error_str);
				else {
					set_terminal(num_terminals - 1);
					get_player_info();
					players_entries[num_terminals - 1] = term->player_name;
					set_terminal(0);
					strcpy(result, "Terminal initialized");
					/* Jimmy - add new player to combatants grid */
					fix_combantants(num_terminals - 1);
				}

#ifdef UNIX
			}
#endif

			iprint(result, ASK_X, ASK_Y + 2);
			iprint("Hit any key or button to continue", ASK_X, ASK_Y + 3);
			wait_input();
			clear_ask(ASK_X, ASK_Y);  /* HAK 2/93 */
		}
		menu_resize(&menu_sys, PLAYERS_MENU, num_terminals);
	}
}

void
add_given_player(int choice)
{
	char *name = machine_names[choice];

#ifdef UNIX
	extern char *network_error_str[];

#endif
	extern char video_error_str[];
	extern int num_terminals;
	char result[256], *tmp;
	int num, ret, i;

	if (num_terminals == MAX_TERMINALS) {
		iprint("There is no room for more players.", ASK_X, ASK_Y + 15);
		iprint("Hit any key or button to continue", ASK_X, ASK_Y + 1 + 15);
		wait_input();
		clear_ask(ASK_X, ASK_Y);  /* HAK 2/93 */
	} else {
#ifdef UNIX
		tmp = name;
		if (ret = check_internet(1, &tmp))
			strcpy(result, network_error_str[ret]);
		else {
#endif

			(void) strcpy(result, "Initializing ");
			(void) strcat(result, name);
			iprint(result, ASK_X, ASK_Y + 1 + 15);
			flush_output();

			if (make_terminal(name))
				strcpy(result, video_error_str);
			else {
				set_terminal(num_terminals - 1);
				get_player_info();
				players_entries[num_terminals - 1] = term->player_name;
				set_terminal(0);
				menu_set_hil(&menu_sys, MACHINE_MENU, choice);
				strcpy(result, "Terminal initialized");
				/* Jimmy - add new player to combatants grid */
				fix_combantants(num_terminals - 1);
			}

#ifdef UNIX
		}
#endif

		iprint(result, ASK_X, ASK_Y + 2 + 15);
		iprint("Hit any key or button to continue", ASK_X, ASK_Y + 3 + 15);
		wait_input();
		clear_text_rc(ANIM_WIN, ASK_X, ASK_Y + 15, 50, 4, INT_FONT);
	}
	menu_resize(&menu_sys, PLAYERS_MENU, num_terminals);
}

/*
** Removes the specified player's terminal from the terminal list, freeing
** all allocated storage and closing its display.  Will not remove player 0.
**
** Several fixes in here by HAK 2/93
*/
void
remove_player(int num)
{
	extern int num_terminals;
	extern Terminal *terminal[];
	extern Video *oldvid;   /* Used by make_picture */

	/* Check for bogus player number */
	if (num < 1 || num >= num_terminals)
		return;

	/* Clear oldvid if it points to the term about to be closed, */
	/* so make_picture won't die next time it's called. */
	if (oldvid == (Video *)terminal[num]->video)
		oldvid = NULL;

	/* Close the terminal and fix up the terminal list and menu */
	close_terminal(terminal[num]);
	if (num != num_terminals - 1) {
		terminal[num] = terminal[num_terminals - 1];
		players_entries[num] = players_entries[num_terminals - 1];
	}
	players_entries[num_terminals - 1] = 0;
	terminal[num_terminals - 1] = (Terminal *) 0;
	num_terminals--;
	menu_resize(&menu_sys, PLAYERS_MENU, num_terminals);
}

/*
** Asks the current terminal for a player name and a vehicle name.
*/
void
get_player_info(void)
{
	extern char *get_default();
	extern char username[];
	char buf[80];
	int line, vd;
	int itemp, duplicate;

	clear_window(ANIM_WIN);
	beep_window();
	sync_output(TRUE);

	line = 5;

	do {
		duplicate = FALSE;
		display_mesg2(ANIM_WIN, "                               ",
					  0, line, INT_FONT);

		if (vid->kludge.player_name[0] == '\0') {
			if (!command_options.AutoStart) {
				input_string(ANIM_WIN, "Enter player name:", term->player_name, 0,
							 line, INT_FONT, MAXPNAME /*MAX_STRING - 1*/ );
			}
		} else {
			strncpy(term->player_name, vid->kludge.player_name, MAXPNAME);
			term->player_name[MAXPNAME] = '\0';

			sprintf(buf, "Player Name: %s", term->player_name);
			display_mesg2(ANIM_WIN, buf, 0, line, INT_FONT);
		}

		/* If no name is given, use the username */
		if (term->player_name[0] == '\0') {
			(void) strcpy(term->player_name, username);
		}
		for (itemp = 0; itemp < num_terminals; itemp++) {
			if (terminal[itemp] == term) {
				continue;		/* don't check against myself */
			}
			if (!strcmp(terminal[itemp]->player_name, term->player_name)) {
				/* display "wrong butthead try again" */
				display_mesg2(ANIM_WIN, "Name used.  Please try another.",
							  0, line + 1, INT_FONT);
				duplicate = TRUE;
				vid->kludge.player_name[0] = '\0';
				break;
			}
		}
		if (command_options.AutoStart && duplicate) {
			puts("Duplicate names specified but not allowed!");
			exit(1);
		}
	}
	while (duplicate);

	line++;

	display_mesg2(ANIM_WIN, "                               ",
				  0, line, INT_FONT);
	if (!command_options.AutoStart) {
		vd = ask_desc(VDESC, 0, line);
	} else {
		vd = 0;
	}
	flush_output();
	if (vd == -1)
		vd = 0;
	term->vdesc = vd;
	if (vdesc && term->vdesc) {	/* GHS 9/12/90 - kludge *//* GHS 9/12/90 - kludge */
		menu_resize(&menu_sys, VEHICLES_MENU, num_vdescs);	/* GHS 9/12/90 - kludge */
	}							/* GHS 9/12/90 - kludge */
	term->mouse_speed = vid->kludge.mouse_speed;
	term->mouse_drive = vid->kludge.mouse_drive;
	term->mouse_heat = vid->kludge.mouse_heat;
	term->keymap = vid->kludge.keymap;
}

void
input_filename(int iWindow, char *pcPrevFileName, char *pcFileName,
	int iLineNum, int iFont, int iMaxLen)
{
	char temp[MAXPATHLEN];

	sprintf(temp, "Enter filename [%s]:", pcPrevFileName);

	input_string(iWindow, temp, pcFileName, 0, iLineNum, iFont, iMaxLen);

	/* If nothing entered use default (if any), otherwise set default */
	if (pcFileName[0] == '\0') {
		if (pcPrevFileName[0]) {
			(void) strcpy(pcFileName, pcPrevFileName);
		}
	} else {
		(void) strcpy(pcPrevFileName, pcFileName);
	}
}

/*
** Prompts the user for a program name.
** Compiles and loads the program, adding it to the program list.
** Reports any errors to the user, displaying the errors produced
** by the compiler or linker.
*/
void
make_prog_desc(void)
{
	char filename[MAXPATHLEN];
	static char prev_filename[MAXPATHLEN];
	int ret, line, i;

	clear_window(ANIM_WIN);
	line = 3;

	/* Prompt the user for the program name */
	iprint("Give full program filename for a .c or .o file", 0, line++);

	input_filename(ANIM_WIN, prev_filename, filename, line++, INT_FONT, 256);

	load_prog_desc(filename, FALSE);
}

void
load_prog_desc(char *filename, int batch)
{
	extern char pathname[], programsdir[];
	static char *report[] =
	{
		"Program loaded", "Improper filename", "Compiler errors",
		"Linker errors", "Can't read output", "Can't parse symbol table",
		"Missing description"};
#if 0
	char *strdup();
#endif
	char *ptr;

#if !defined(hpux) && !defined(i860) && !defined(NeXT) && !defined(__alpha) \
    && !defined(sun)
	char *rindex();
#endif
	char output_name[MAXPATHLEN], temp[MAXPATHLEN];
	char *error_name, *code;
	Prog_desc *pdesc;
	int ret, line, i;

	line = 6;

	/* Check that there is room for another program */
	if (num_prog_descs >= MAX_PDESCS) {
		if (batch) {
			iprint("No room for more programs.  Key or button to continue", 0,
				   line);
			wait_input();
			return;
		} else {
			fprintf(stderr, "No room for more programs.\n");
			exit(3);
		}
	}
	ptr = strdup(filename);
	assert(ptr);

	/* Prepend the path to the programs directory if necessary */
	if (rindex((char *) filename, '/') == NULL) {
		(void) strcpy(temp, pathname);
		(void) strcat(temp, "/");
		(void) strcat(temp, programsdir);
		(void) strcat(temp, "/");
		(void) strcat(temp, (char *) filename);
		(void) strcpy((char *) filename, temp);
	}
	if (!batch) {
		/* State the load request and flush */
		sprintf(temp, "Loading %s", filename);
		iprint(temp, 0, line++);
		flush_output();
	}
	/* Compile and load the program */
	error_name = "/tmp/xtank.error";

	/* Vary the output filename so that certain compilers on */
	/* certain architectures don't get confused. */
	/* This is portable across all systems, however. */

	sprintf(temp,"/tmp/xtank.output.%d",num_prog_descs);
	strcpy(output_name,temp);

	pdesc = prog_desc[num_prog_descs];

	ret = compile_module(filename, (char **) &pdesc, &code, error_name,
						 output_name);

	if (!batch) {
		/* Report the result */
		(void) strcpy(temp, report[ret]);
		(void) strcat(temp, ".  Key or button to continue.");
		iprint(temp, 0, line);
		wait_input();
	}
	/* If there are any errors, show the error file */
	if (ret == 2 || ret == 3) {
		if (batch) {
			fprintf(stderr, "Linker/compiler errors\n");
		} else {
			display_file(ANIM_WIN, error_name);
		}
	} else {
		if (ret == 0) {
			pdesc->code = code;

			/* If program has been loaded before, free the previous one and
			   replace */
			for (i = 0; i < num_prog_descs; i++) {
				/* Look for a loaded program (code != NULL) with matching name */
				if (prog_desc[i]->code != (char *) NULL &&
					!strcmp(prog_desc[i]->name, pdesc->name)) {
					free(prog_desc[i]->code);
					break;
				}
			}

			/* Copy the pointers into the description and menu entries arrays */
			prog_desc[i] = pdesc;
			programs_entries[i] = pdesc->name;
			pdesc->filename = ptr;

			/* If new slot used, increment the count and resize the menu */
			if (i == num_prog_descs) {
				num_prog_descs++;
				menu_resize(&menu_sys, PROGRAMS_MENU, num_prog_descs);
			}
		}
	}

	unlink(error_name);
	unlink(output_name);
}

/*
** Ask the user for a desc name, try to load it and then fix the menus.
*/
void
interface_load(int type)
{
#ifdef OLD
	int max_descs;

	switch (type) {
	  case MDESC:
		  max_descs = num_mdescs;
		  break;
	  case VDESC:
		  max_descs = num_vdescs;
		  break;
	  case SDESC:
		  max_descs = num_sdescs;
		  break;
	}

	/* If we just added to the end, fix the menu */
	if (ask_desc(type, ASK_X, ASK_Y) == max_descs)
		fix_desc_menu(type);
#else
	ask_desc(type, ASK_X, ASK_Y);
#endif
}

/*
** Make the named desc, fix the desc menu, set the desc in the settings.
*/
void
interface_set_desc(int type, char *name)
{
	int num, max_descs, result;

	switch (type) {
	  case MDESC:
		  max_descs = num_mdescs;
		  result = make_mdesc(name, &num);
		  break;
	  case VDESC:
		  max_descs = num_vdescs;
		  result = make_vdesc(name, &num);
		  break;
	  case SDESC:
		  max_descs = num_sdescs;
		  result = make_sdesc(name, &num);
		  break;
	}

	if (result == DESC_LOADED) {
		switch (type) {
		  case MDESC:
			  settings.mdesc = &mdesc[num];
			  settings.si.game = settings.mdesc->type;
			  break;
		  case VDESC:
			  term->vdesc = num;
			  break;
		  case SDESC:
			  break;
		}
#ifdef OLD
		if (num == max_descs)
			fix_desc_menu(type);
#endif
	}
}

/*
** A description has been loaded, so add an item to the right menu.
*/
void
fix_desc_menu(int type)
{
	switch (type) {
	  case VDESC:
		  /* vehicles_entries[num_vdescs - 1] = vdesc[num_vdescs - 1].name; */
		  menu_resize(&menu_sys, VEHICLES_MENU, num_vdescs);
		  break;
	  case MDESC:
		  /* Leave last entry as the random maze */
		  /* mazes_entries[num_mdescs] = mazes_entries[num_mdescs - 1]; */
		  /* mazes_entries[num_mdescs - 1] = mdesc[num_mdescs - 1].name; */
		  menu_resize(&menu_sys, MAZES_MENU, num_mdescs + 1);
		  break;
	  case SDESC:
		  /* setups_entries[num_sdescs - 1] = sdesc[num_sdescs - 1].name; */
		  menu_resize(&menu_sys, SETUPS_MENU, num_vdescs);
		  break;
	}
}

/*
** Prompts the user for a vehicle, maze, or setup name and loads it.
** Returns number of last description loaded or -1 if none loaded.
*/
int
ask_desc(int type, int row, int col)
{
	char prompt[80], resp[80], *format, *tname;
	int ret, num;

	switch (type) {
	  case VDESC:
		  tname = "vehicle";
		  break;
	  case MDESC:
		  tname = "maze";
		  break;
	  case SDESC:
		  tname = "setup";
		  break;
	}

	do {
		if (!*vid->kludge.tank_name) {
/* Changed the clear_ask macro to use a position argument HAK 2/93 */
			clear_ask(row, col);

			/* Prompt the user for the name */
			(void) sprintf(prompt, "Enter %s name:", tname);
			input_string(ANIM_WIN, prompt, resp, row, col, INT_FONT, 256);

			/* If user enters nothing, don't bother trying to load */
			if (*resp == '\0') {
				ret = DESC_NOT_FOUND;
				break;
			}
		} else {
			strcpy(resp, vid->kludge.tank_name);
			*vid->kludge.tank_name = '\0';

			sprintf(prompt, "Vehicle name: %s", resp);
			display_mesg2(ANIM_WIN, prompt, row, col, INT_FONT);
		}
		/* Load the proper description type */
		switch (type) {
		  case VDESC:
			  ret = make_vdesc(resp, &num);
			  break;
		  case MDESC:
			  ret = make_mdesc(resp, &num);
			  if (ret == DESC_LOADED) {
				  settings.mdesc = &mdesc[num];
				  display_settings();
			  }
			  break;
		  case SDESC:
			  ret = DESC_NOT_FOUND;
			  break;
		}

		/* Respond based on result of load */
		switch (ret) {
		  case DESC_LOADED:
			  format = "%s loaded.  %s";
			  break;
		  case DESC_NOT_FOUND:
			  format = "%s not found.  %s";
			  break;
		  case DESC_NO_ROOM:
			  format = "no room for %s.  %s";
			  break;
		  case DESC_BAD_FORMAT:
			  format = "bad %s format.  %s";
			  break;
		}

		if (ret == DESC_LOADED)
			break;

		/* Explain result and ask if they want to try again */
		(void) sprintf(prompt, format, tname, "Try again? (y/n) [n]:");
		input_string(ANIM_WIN, prompt, resp, row, col + 1, INT_FONT, 256);
	} while (resp[0] == 'y');

	clear_ask(ASK_X, ASK_Y);  /* HAK 2/93 */

	return ((ret == DESC_LOADED) ? num : -1);
}

void
ask_winning_score(void)
{
	if (settings.si.game == WAR_GAME)
		settings.si.winning_score = input_int(ANIM_WIN, "Winning score",
											  ASK_X, ASK_Y,
											  75, 0, 100, INT_FONT);
	else
		settings.si.winning_score = input_int(ANIM_WIN, "Winning score",
											  ASK_X, ASK_Y,
										   20000, 0, 1024 * 1024, INT_FONT);
	clear_ask(ASK_X, ASK_Y);  /* HAK 2/93 */
}

void
ask_maze_density(void)
{
	int t = input_int(ANIM_WIN, "Maze density", ASK_X, ASK_Y, 3, 0, 10,
					  INT_FONT);

	set_setting(SET_DENSITY, t);
	clear_ask(ASK_X, ASK_Y);  /* HAK 2/93 */
}

/*
** Reads in the specified file and displays it in the specified window.
*/
void
display_file(int w, char *filename)
{
	extern char *read_file();
	char *str;

	if (str = read_file(filename)) {
		display_long_str(w, str, INT_FONT);
		free(str);
	}
}

/*
** Displays a long string in the specified window.  Properly formats
** tabs and newlines.  Prompts user to hit a key after each page,
** and at the end of the string.
*/
void
display_long_str(int w, char *str, int font)
{
	char c, temp[DISP_WIDTH + 1];
	Boolean write = FALSE;
	int row, col;

	clear_window(w);
	col = 0;
	row = 0;

	do {
		switch (c = *(str++)) {
		  case '\t':
			  if (col < DISP_WIDTH - 8)
				  do {
					  temp[col++] = ' ';
				  } while (col % 8 != 0);
			  else
				  write = TRUE;
			  break;
		  case '\n':
		  case '\0':
			  write = TRUE;
			  break;
		  default:
			  temp[col++] = c;
			  if (col > DISP_WIDTH)
				  write = TRUE;
		}

		if (write == TRUE) {
			temp[col] = '\0';
			display_mesg2(w, temp, DISP_X, DISP_Y + row, font);
			col = 0;
			row++;
			write = FALSE;
		}
		if (row == DISP_HEIGHT - 2 || c == '\0') {
			display_mesg2(w, "Hit any key or button to continue",
					DISP_X + (DISP_WIDTH - 35) / 2, DISP_Y + 2 + row, font);
			wait_input();
			clear_window(w);
			row = 0;
		}
	} while (c != '\0');
}

#define XTANK_X     (ANIM_WIN_WIDTH/2)
#define XTANK_Y     (ANIM_WIN_HEIGHT/16)
#define NUM_GLEAMS  30
#define GLEAM_SPEED 10

/*
** Displays the title.  Sweeps gleams across it if gleams is true.
*/
void
display_title(Boolean gleams)
{
	Object *gleam_obj, *title_obj, *terp_obj, *rhino_obj, *trike_obj;
	Object *cycle_obj;
	Picture *old_pic, *pic;
	Picture *terp;
	char uofm[80];
	char fake[2];
	int x[NUM_GLEAMS], y[NUM_GLEAMS], num[NUM_GLEAMS];
	int width, height, offset_x, offset_y;
	int num_pics;
	int sweep, count;
	int i, rx, ry, tx, ty, rot, cx, cy;



	/* Initialize a few variables */
	gleam_obj = exp_obj[1];
	title_obj = random_obj[XTANK_OBJ];
	terp_obj = random_obj[TERP_OBJ];
	rhino_obj = vehicle_obj[7];
	trike_obj = vehicle_obj[10];
	cycle_obj = vehicle_obj[0];
	(void) strcpy(uofm, "A University of Maryland Engineering Dept. hack");
	pic = &title_obj->pic[0];
	terp = &terp_obj->pic[0];
	width = pic->width;
	height = pic->height;
	offset_x = pic->offset_x;
	offset_y = pic->offset_y;

	/* Show the UM Terp */
	draw_picture(ANIM_WIN, ANIM_WIN_WIDTH, XTANK_Y * 12, terp, DRAW_COPY, RED);

	/* Show the title bitmap, copyright notice, author, and version number */
	draw_picture(ANIM_WIN, XTANK_X, XTANK_Y, pic, DRAW_COPY, WHITE);


	draw_text(ANIM_WIN, XTANK_X, XTANK_Y + offset_y + 8,
			  "Copyright 1988 by Terry Donahue", L_FONT, DRAW_COPY, WHITE);

	draw_text(ANIM_WIN, XTANK_X, XTANK_Y + offset_y + 25,
			  version1, L_FONT, DRAW_COPY, WHITE);
	draw_text(ANIM_WIN, XTANK_X, XTANK_Y * 15 + offset_y - 17,
			  version2, L_FONT, DRAW_COPY, WHITE);
	draw_text(ANIM_WIN, XTANK_X, XTANK_Y * 15 + offset_y,
			  version3, L_FONT, DRAW_COPY, WHITE);

	if (gleams == FALSE)
		return;

	if (get_num_default("titlehack", "Wizbang", 0)) {
		ry = 700;
		count = 0;
		i = 0;
		rot = 8;
		tx = 150;
		cx = 690;
		cy = 700;
		fake[1] = '\0';
		for (rx = -50; rx < 690; rx += 10) {
			pic = &rhino_obj->pic[0];
			draw_picture(ANIM_WIN, rx, ry, pic, DRAW_XOR, BLUE);
			sync_output(TRUE);
			draw_picture(ANIM_WIN, rx, ry, pic, DRAW_XOR, BLUE);
			pic = &cycle_obj->pic[rot];
			if (cx < 400 && count < 6) {
				if (count == 0) {
					rot--;
					cy = 720;
				}
				if (count == 1) {
					rot--;
					cy = 750;
				}
				if (count == 2)
					rot++;
				if (count == 3)
					rot++;
				if (count == 4)
					rot++;
				if (count == 5)
					rot--;
				count++;
			}
			draw_picture(ANIM_WIN, cx, cy, pic, DRAW_XOR, GREEN);
			sync_output(TRUE);
			draw_picture(ANIM_WIN, cx, cy, pic, DRAW_XOR, GREEN);
			if (rx > 150 && i < 47) {
				fake[0] = uofm[i];
				draw_text(ANIM_WIN, tx, ry, fake, L_FONT, DRAW_COPY, WHITE);
				i++;
				tx += 10;
			}
			cx -= 10;
		}
		sync_output(TRUE);

		/* trike attack - bigmac */

		pic = &trike_obj->pic[4];
		rx = 600;
		tx = -50;
		ty = 601;
		for (ry = -50, tx = -50; ry < 601; ry += 2, tx += 2) {
			pic = &trike_obj->pic[4];
			draw_picture(ANIM_WIN, rx, ry, pic, DRAW_XOR, BLUE);
			draw_picture(ANIM_WIN, rx, ry, pic, DRAW_XOR, BLUE);
			pic = &trike_obj->pic[0];
			if (tx < 600) {
				draw_picture(ANIM_WIN, tx, ty, pic, DRAW_XOR, BLUE);
				draw_picture(ANIM_WIN, tx, ty, pic, DRAW_XOR, BLUE);
			}
		}

		/* make one trike explode */


		/* trike has hit terp - now spin off into logo */
		rot = 4;
		ry = 601;

		for (i = 0; i < 250; i++) {
			draw_picture(ANIM_WIN, rx, ry, pic, DRAW_XOR, BLUE);
			draw_picture(ANIM_WIN, rx, ry, pic, DRAW_XOR, BLUE);

			rx--;
			ry -= 2;
			pic = &trike_obj->pic[rot];
			rot++;
			if (rot == 15)
				rot = 0;
		}
	}
	/* Initialize x, y, and picture number for every gleam */
	num_pics = gleam_obj->num_pics;
	for (i = 0; i < NUM_GLEAMS; i++) {
		x[i] = XTANK_X + rnd(width) - offset_x;
		y[i] = XTANK_Y + rnd(height) - offset_y;
		num[i] = -1;
	}


	/* Make the gleams sweep across the title from left to right */
	for (sweep = XTANK_X - offset_x; sweep < XTANK_X + width * 1.2;
		 sweep += GLEAM_SPEED) {
		for (i = 0; i < NUM_GLEAMS; i++) {
			if (num[i] == num_pics)
				continue;
			if (num[i] == num_pics - 1) {
				/* just erase the gleam if it at the last picture */
				pic = &gleam_obj->pic[num[i]];
				draw_picture(ANIM_WIN, x[i], y[i], pic, DRAW_XOR, WHITE);
				num[i]++;
			} else if (num[i] >= 0) {
				/* redisplay the gleam if it has a picture */
				old_pic = &gleam_obj->pic[num[i]];
				draw_picture(ANIM_WIN, x[i], y[i], old_pic, DRAW_XOR, WHITE);
				pic = &gleam_obj->pic[num[i] + 1];
				draw_picture(ANIM_WIN, x[i], y[i], pic, DRAW_XOR, WHITE);
				num[i]++;
			} else if (x[i] < sweep) {
				/* start a gleam, since the sweep has passed */
				num[i]++;
				pic = &gleam_obj->pic[num[i]];
				draw_picture(ANIM_WIN, x[i], y[i], pic, DRAW_XOR, WHITE);
			}
		}
	}
}

/* Jimmy */
void
fix_combantants(int nt)
{
	/* put player name in grid */
	grid_ent[PLAYERS_MENU - PLAYERS_MENU][nt] =
	  players_entries[nt];

	grid_val[PLAYERS_MENU - PLAYERS_MENU][nt] =
	  nt;

	if (terminal[nt]->vdesc > 1000) {
		fprintf(stderr, "%s's terminal seems to have an invalid vdesc %d\n",
				terminal[nt]->player_name, terminal[nt]->vdesc);
		terminal[nt]->vdesc = 0;
	}
	/* put player's choice of vehicle in grid */
	grid_ent[VEHICLES_MENU - PLAYERS_MENU][nt] =
	  vehicles_entries[terminal[nt]->vdesc];

	grid_val[VEHICLES_MENU - PLAYERS_MENU][nt] =
	  terminal[nt]->vdesc;

	/* put player's team in grid - not very sophisticated, but it's better */
	/* than nothing... */
	grid_ent[TEAMS_MENU - PLAYERS_MENU][nt] =
	  teams_entries[(nt) % 2 + 1];

	grid_val[TEAMS_MENU - PLAYERS_MENU][nt] =
	  (nt) % 2 + 1;
}

void
comb_load_v(void)  /* added 2/93 by HAK */
{
	ask_desc(VDESC, 5, 48);
	menu_unhighlight(&menu_sys, COMBATANTS_MENU);

	expose_win(ANIM_WIN, TRUE);
}

void
comb_load_all(void)  /* added 2/93 by HAK */
{
        int i, num;
        char temp[MAXNAMLEN];

        for (i = 0; i < num_vdescs; i++)
        {
                strcpy(temp, vdesc[i].name);
                make_vdesc(temp, &num);
/* Apparently, make_vdesc will clear the string pointed to by temp.  Thus,
   we use the temp variable, rather than passing the pointer to the vdesc
   string.
*/
        }
        menu_unhighlight(&menu_sys, COMBATANTS_MENU);
}
