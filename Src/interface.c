/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** interface.c
*/

#include "xtank.h"
#include "gr.h"
#include "menu.h"
#include "interface.h"
#include "program.h"

extern int num_terminals;
extern Terminal *terminal[];
extern int num_mdescs;
extern Mdesc mdesc[];
extern int num_vdescs;
extern Vdesc vdesc[];
extern int num_sdescs;
extern Sdesc sdesc[];
extern int num_prog_descs;
extern Prog_desc *prog_desc[];

static whichlevel[MAX_MENUS]={0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3};

/* For convienence */
static char *done_entries[] = { "Done" };

static char
  *games_entries[] =
                 {"Combat", "War", "Ultimate", "Capture", "Race"},
  *main_entries[] = 
                 {"Play", "Settings", "Combatants", "View", "Load", "Design",
		  "Add players", "Help", "Quit"},
  *play_entries[] = 
                 {"Standard","Players","Robots","Customized"},
  *settings_entries[] =
                 {"Vehicle","Maze","Game","Flags","Winning score","Difficulty",
		  "Outpost strength","Scroll speed","Box slowdown",
		  "Disc friction","Owner slowdown" },
  *view_entries[] =
                 {"Player", "Program", "Vehicle", "Maze", "Setup"},
  *load_entries[] =
                 {"Vehicle", "Maze", "Program", "Setup" },
  *design_entries[] =
                 {"Maze", "Vehicle"},
  *help_entries[] =
                 {"General", "Pictures", "Multi-player", "Games", "Vehicles",
		  "Mazes", "Setups", "Credits", "Motd"},
  *grid_entries[] =
                 {"Player", "Program", "Vehicle", "Team"},
  *num_entries[] =
                 {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"},
  *flags_entries[] =
                 {"Point bullets", "Ricochet", "Rel. shooting", "No wear",
		  "Restart", "Commentator", "Full map"},
  *programs_entries[MAX_PDESCS],
  *vehicles_entries[MAX_VDESCS],
  *mazes_entries[MAX_MDESCS],
  *setups_entries[MAX_SDESCS],
  *players_entries[MAX_TERMINALS];

/* Grid text entries, numeric values, and menus for the combatants interface */
static Byte grid_val[MAX_GRIDS][MAX_VEHICLES];
static char *grid_ent[MAX_GRIDS][MAX_VEHICLES];
static int grid_id[MAX_GRIDS];

/* Global so message system can use it */
char *teams_entries[] =
                 {"Neutral", "Red", "Orange", "Yellow", "Green", "Blue",
		  "Violet"};

/* Descriptions of program abilities */
static char *ability_desc[] = {
  "Plays Combat","Plays War","Plays Ultimate","Plays Capture","Plays Race",
  "Shoots","Explores","Dodges","Replenishes",
  "Uses teams","Uses mines","Uses slicks","Uses side mounts","Uses messages"
};

static Menu_int menu_sys;

/*
** Initializes the main interface menus.
*/
init_interface()
{
  menu_sys_window(&menu_sys, ANIM_WIN);

  menu_norm_make(&menu_sys, MAIN_MENU, "XTANK", 9, 0,
		 LEV0_X, LEV0_Y, main_entries, L_FONT);
  menu_norm_make(&menu_sys, PLAY_MENU, "Play", 4, 0,
		 LEV1_X, LEV0_Y, play_entries, M_FONT);
  menu_norm_make(&menu_sys, SETTINGS_MENU, "Settings", 11, 0,
		 LEV1_X, LEV0_Y, settings_entries, M_FONT);
  menu_norm_make(&menu_sys, VIEW_MENU, "View", 5, 0,
		 LEV1_X, LEV0_Y, view_entries, M_FONT);
  menu_norm_make(&menu_sys, LOAD_MENU, "Load", 4, 0,
		 LEV1_X, LEV0_Y, load_entries, M_FONT);
  menu_norm_make(&menu_sys, DESIGN_MENU, "Design", 2, 0,
		 LEV1_X, LEV0_Y, design_entries, M_FONT);
  menu_norm_make(&menu_sys, HELP_MENU, "Help", 9, 0,
		 LEV1_X, LEV0_Y, help_entries, M_FONT);
  menu_nohil_make(&menu_sys, GAMES_MENU, "Games", MAX_GAMES, 0,
		 LEV2_X, LEV0_Y, games_entries, M_FONT);
  menu_noti_make(&menu_sys, NUM_MENU, "", 11, 0,
		 LEV2_X, LEV0_Y, num_entries, M_FONT);
  menu_flag_make(&menu_sys, FLAGS_MENU, "Flags", 7, 0,
		 LEV3_X, LEV0_Y, flags_entries, M_FONT);

  /* Set up the correct highlighting for the flags menu */
  if(settings.point_bullets) menu_set_hil(&menu_sys,FLAGS_MENU,0);
  if(settings.ricochet) menu_set_hil(&menu_sys,FLAGS_MENU,1);
  if(settings.rel_shoot) menu_set_hil(&menu_sys,FLAGS_MENU,2);
  if(settings.no_wear) menu_set_hil(&menu_sys,FLAGS_MENU,3);
  if(settings.restart) menu_set_hil(&menu_sys,FLAGS_MENU,4);
  if(settings.commentator) menu_set_hil(&menu_sys,FLAGS_MENU,5);
  if(settings.full_map) menu_set_hil(&menu_sys,FLAGS_MENU,6);

  init_comb_menus();
}

/*
** Initializes the combatants interface menus.
*/
init_comb_menus()
{
  int grid_wid, height, i, j;
  
  grid_id[0]=MAX_MENUS;
  for (i=1; i<MAX_GRIDS; i++) {
    grid_id[i]=grid_id[i-1]+1;
  }

  for (i=0; i<num_prog_descs; ++i)
    programs_entries[i] = prog_desc[i]->name;

  for (i=0; i<num_vdescs; ++i)
    vehicles_entries[i] = vdesc[i].name;

  for (i=0; i<num_mdescs; ++i)
    mazes_entries[i] = mdesc[i].name;
  mazes_entries[i] = "Random";

  for (i=0; i<num_sdescs; ++i)
    setups_entries[i] = sdesc[i].name;

  for (i=0; i<num_terminals; ++i)
    players_entries[i] = terminal[i]->player_name;

  grid_wid = MAX_COMB_WID * font_string_width("M", M_FONT);

  for (i=0; i<MAX_GRIDS; i++) {
     for (j=0; j<MAX_VEHICLES; j++) {
       grid_ent[i][j] = "";
       grid_val[i][j] = UNDEFINED;
     }
     menu_recv_make(&menu_sys, grid_id[i], grid_entries[i], MAX_VEHICLES, 
		    grid_wid, GRID_X+(grid_wid+1)*i,GRID_Y,
		    grid_ent[i], M_FONT);
  }

  height = TIER1_HEIGHT;
  menu_nohil_make(&menu_sys, PROGRAMS_MENU, "Programs",num_prog_descs,0,
		  PROGRAMS_X, PROGRAMS_Y+height, programs_entries, M_FONT);
  menu_nohil_make(&menu_sys, VEHICLES_MENU, "Vehicles", num_vdescs, 0,
		  VEHICLES_X, VEHICLES_Y+height, vehicles_entries, M_FONT);
  menu_nohil_make(&menu_sys, PLAYERS_MENU, "Players", num_terminals, 0,
		  PLAYERS_X, PLAYERS_Y+height, players_entries, M_FONT);
  menu_nohil_make(&menu_sys, TEAMS_MENU, "Teams", 7, 0,
		  TEAMS_X, TEAMS_Y+height, teams_entries, M_FONT);
  menu_nohil_make(&menu_sys, MAZES_MENU, "Mazes", num_mdescs+1, 0,
		  MAZES_X, MAZES_Y+height, mazes_entries, M_FONT);
  menu_norm_make(&menu_sys, SETUPS_MENU, "Setups", num_sdescs, 0,
		 SETUPS_X, SETUPS_Y+height, setups_entries, M_FONT);
  menu_simp_make(&menu_sys, COMBATANTS_MENU, "", 1, 0, 
		 COMBATANTS_X, COMBATANTS_Y, done_entries, L_FONT);
}
  
/*
** The top level interface to xtank.
*/
main_interface()
{
   Event ev;
   int numev,menu,choice;

   set_terminal(0);
   init_interface();
#ifdef X11
   button_up(ANIM_WIN,TRUE);
   follow_mouse(ANIM_WIN,TRUE);
#endif   
   /* Display the motd then the title */
   display_file(ANIM_WIN,"Help/motd");
   display_title(TRUE);

   /* Display the settings and the main menu */
   display_settings();
   menu_display(&menu_sys,MAIN_MENU);

   do {
     if(win_exposed(ANIM_WIN)) {
       clear_window(ANIM_WIN);
       display_title(FALSE);
       menu_system_expose(&menu_sys);
       expose_win(ANIM_WIN,FALSE);
     }   

     if(win_exposed(GAME_WIN)) {
       display_settings();
       expose_win(GAME_WIN,FALSE);
     }

     numev=1;
     get_events(&numev, &ev);
     if (numev==1)
       switch(ev.type) {
         case EVENT_LBUTTON:
         case EVENT_MBUTTON:
	 case EVENT_RBUTTON:
	   menu = menu_hit(&menu_sys, ev.x, ev.y);
	   if (menu!=MENU_NULL) {
	      /* Erase all equal or higher level menus */
	      erase_other_menus(menu);

	      /* Find out which choice on the menu was selected */
	      menu_hit_p(&menu_sys, ev.x, ev.y, &menu, &choice);

	      switch(menu) {
	      case MAIN_MENU:
		 switch(choice) {
   		    case 0: menu_display(&menu_sys, PLAY_MENU); break;
		    case 1: menu_display(&menu_sys, SETTINGS_MENU); break;
    		    case 2: do_comb(); break;
  		    case 3: menu_display(&menu_sys, VIEW_MENU); break;
		    case 4: menu_display(&menu_sys, LOAD_MENU); break;
		    case 5: menu_display(&menu_sys, DESIGN_MENU); break;
		    case 6:
		      add_players();
		      menu_unhighlight(&menu_sys, MAIN_MENU);
		      break;
		    case 7: menu_display(&menu_sys, HELP_MENU); break;
		    case 8: return;
		    }
		 break;
	      case VIEW_MENU:
		 switch(choice) {
		    case 0: menu_display(&menu_sys, PLAYERS_MENU); break;
		    case 1: menu_display(&menu_sys, PROGRAMS_MENU); break;
		    case 2: menu_display(&menu_sys, VEHICLES_MENU); break;
		    case 3: menu_display(&menu_sys, MAZES_MENU); break;
		    case 4: menu_display(&menu_sys, SETUPS_MENU); break;
		 }
		 break;
	      case LOAD_MENU:
		 switch(choice) {
		    case 0: interface_load(VDESC); break;
		    case 1: interface_load(MDESC); break;
		    case 2: make_prog_desc(); expose_win(ANIM_WIN,TRUE); break;
		    case 3: interface_load(SDESC); break;
		 }
		 menu_unhighlight(&menu_sys, LOAD_MENU);
		 break;
	      case DESIGN_MENU:
		 switch(choice) {
		    case 0: design_maze(); expose_win(ANIM_WIN,TRUE); break;
		    case 1: design_vehicle(); expose_win(ANIM_WIN,TRUE); break;
		 }
		 menu_unhighlight(&menu_sys, DESIGN_MENU);
		 break;
	      case HELP_MENU:
		 switch(choice) {
		    case 0: display_file(ANIM_WIN,"Help/general"); break;
		    case 1: display_pics(); expose_win(ANIM_WIN,TRUE); break;
		    case 2: display_file(ANIM_WIN,"Help/multi-player"); break;
		    case 3: display_file(ANIM_WIN,"Help/games"); break;
		    case 4: display_file(ANIM_WIN,"Help/vehicles"); break;
		    case 5: display_file(ANIM_WIN,"Help/mazes"); break;
		    case 6: display_file(ANIM_WIN,"Help/setups"); break;
		    case 7: display_file(ANIM_WIN,"Help/credits"); break;
		    case 8: display_file(ANIM_WIN,"Help/motd"); break;
		 }
		 menu_unhighlight(&menu_sys, HELP_MENU);
		 expose_win(ANIM_WIN,TRUE);
		 break;
	      case GAMES_MENU:
		 settings.game = choice;
		 display_settings();
		 break;
	      case PLAYERS_MENU: do_view(PLAYERS_MENU,choice); break;
	      case PROGRAMS_MENU: do_view(PROGRAMS_MENU,choice); break;
	      case VEHICLES_MENU:
		 if(menu_is_up(&menu_sys,SETTINGS_MENU))
		   term->vdesc = choice;
		 else
		   do_view(VEHICLES_MENU,choice);
		 break;
	      case MAZES_MENU:
		 /*
		 ** If the settings menu is up, set the settings,
		 ** otherwise, display information about the maze.
		 */
		 if(menu_is_up(&menu_sys,SETTINGS_MENU)) {
		   if (choice<num_mdescs) {
		     settings.mdesc = &mdesc[choice];
		     settings.game = settings.mdesc->type;
		   }
		   else {
		     /* Random maze selected */
		     settings.mdesc = (Mdesc *) NULL;
		     menu_erase(&menu_sys, MAZES_MENU);
		     menu_unhighlight(&menu_sys, SETTINGS_MENU);
		     ask_maze_density();
		   }
		   display_settings();
		 }
		 else
		   if(choice < num_mdescs)
		     do_view(MAZES_MENU,choice);
		 break;
	      case SETUPS_MENU: do_view(SETUPS_MENU,choice); break;
	      case PLAY_MENU:
		 switch(choice) {
		   case 0: standard_combatants(); break;
		   case 1: player_combatants(); break;
		   case 2: robot_combatants(); break;
		   case 3: customized_combatants(); break;
		   }
		 interface_play();
		 menu_unhighlight(&menu_sys, PLAY_MENU);
		 break;
	      case SETTINGS_MENU:
		 switch (choice) {
		    case 0:  menu_display(&menu_sys, VEHICLES_MENU); break;
		    case 1:  menu_display(&menu_sys, MAZES_MENU); break;
		    case 2:  menu_display(&menu_sys, GAMES_MENU); break;
		    case 3:  menu_display(&menu_sys, FLAGS_MENU); break;
		    case 4:  ask_winning_score(); break;
		    case 5:  do_num(SET_DIFFICULTY,TRUE); break;
		    case 6:  do_num(SET_OUTPOST,TRUE); break;
		    case 7:  do_num(SET_SCROLL,TRUE); break;
		    case 8:  do_num(SET_BOX_SLOW,TRUE); break;
		    case 9:  do_num(SET_DISC_FRIC,TRUE); break;
		    case 10: do_num(SET_DISC_SLOW,TRUE); break;
		 }

		 /*
		 ** Unhighlight selection and redisplay settings if a
		 ** value in the settings was changed.
		 */
		 if(choice == 4) {
		   display_settings();
		   menu_unhighlight(&menu_sys, SETTINGS_MENU);
 		 }
		 break;
	       case NUM_MENU:
		 do_num(choice,FALSE);
		 break;
	       case FLAGS_MENU:
		 switch(choice) {
	           case 0:  settings.point_bullets ^= TRUE; break;
		   case 1:  settings.ricochet ^= TRUE; break;
		   case 2:  settings.rel_shoot ^= TRUE; break;
		   case 3:  settings.no_wear ^= TRUE; break;
		   case 4:  settings.restart ^= TRUE; break;
		   case 5:  settings.commentator ^= TRUE; break;
		   case 6:  settings.full_map ^= TRUE; break;
		   }
		 break;
	      }
	   }
	   break;
	}
  } while(TRUE);
}

/*
** Erases all menus on equal or higher levels than the specified menu.
*/
erase_other_menus(mu)
     int mu;
{
   int level,i;

   level = whichlevel[mu];

   /* Erase any other displayed menu that is of equal or higher level */
   for(i = 0 ; i < MAX_MENUS ; i++)
     if(menu_is_up(&menu_sys,i) && whichlevel[i] >= level && i != mu)
       menu_erase(&menu_sys,i);
}

/*
** Sets up menus to form a grid of combatants.  User picks spot on
** grid and then picks a menu choice to fill that spot.  This progresses
** until the user clicks on done.
*/
do_comb()
{
   int i, numev, menu, choice, gridsel, gridwhich, old_gridwhich, mv;
   Event ev;

   gridwhich = 0;
   gridsel = 0;
   menu_set_hil(&menu_sys, grid_id[gridwhich], gridsel);

   /* Expose the window to display the combatant menus */
   menu_sys_erase(&menu_sys);
   expose_win(ANIM_WIN,TRUE);

   do {
     if(win_exposed(ANIM_WIN)) {
       clear_window(ANIM_WIN);
       for (i=0; i<MAX_GRIDS; i++)
	 menu_display(&menu_sys, grid_id[i]);
       menu_display(&menu_sys, PROGRAMS_MENU);
       menu_display(&menu_sys, VEHICLES_MENU);
       menu_display(&menu_sys, PLAYERS_MENU);
       menu_display(&menu_sys, COMBATANTS_MENU);
       menu_display(&menu_sys, TEAMS_MENU);
       expose_win(ANIM_WIN,FALSE);
     }   
     numev=1;
     mv = 0;
     get_events(&numev, &ev);
     if (numev!=0) 
       switch(ev.type) {
       case EVENT_LBUTTON: ++mv;
       case EVENT_MBUTTON: ++mv;
       case EVENT_RBUTTON:
	  menu_hit_p(&menu_sys, ev.x, ev.y, &menu, &choice);
	  switch (menu) {
	  case COMBATANTS_MENU: numev = -1; break;
	  case PLAYERS_MENU:
	  case PROGRAMS_MENU:
	  case VEHICLES_MENU:
	  case TEAMS_MENU:
	     if (gridwhich == menu-PLAYERS_MENU) {
                grid_ent[gridwhich][gridsel] =
                  (menu==PLAYERS_MENU)?(players_entries[choice]):
                  (menu==PROGRAMS_MENU)?(programs_entries[choice]):
                  (menu==VEHICLES_MENU)?(vehicles_entries[choice]):
                    (teams_entries[choice]);
		grid_val[gridwhich][gridsel] = choice;
          case MENU_NULL:
		/* Left button moves across, middle button moves down */
		old_gridwhich = gridwhich;
		if(mv == 2) {
		  if(++gridwhich >= MAX_GRIDS) {
		    gridwhich = 0;
		    gridsel = (gridsel + 1) % MAX_VEHICLES;
		  }
		}
		else if(mv == 1) {
		  if(++gridsel >= MAX_VEHICLES) {
		    gridsel = 0;
		    gridwhich = (gridwhich + 1) % MAX_GRIDS;
		  }
		}

		/* Erase old highlight and set the new one */
		menu_erase(&menu_sys, grid_id[old_gridwhich]);
		if(old_gridwhich != gridwhich)
		  menu_display(&menu_sys, grid_id[old_gridwhich]);
		menu_disphil(&menu_sys, grid_id[gridwhich], gridsel);
	      }
	    break;
	  default:
	    /* Deals with clicks on the grid itself */
	    old_gridwhich = gridwhich;
	    gridwhich = menu-MAX_MENUS;
	    gridsel = choice;

	    /* Unhighlight the old menu if we moved to a new one */
	    if(old_gridwhich != gridwhich)
	      menu_unhighlight(&menu_sys, grid_id[old_gridwhich]);

	    /* Right button erases the item */
	    if(mv == 0) {
	      grid_ent[gridwhich][gridsel] = "";
	      grid_val[gridwhich][gridsel] = UNDEFINED;
	      menu_erase(&menu_sys, grid_id[gridwhich]);
	      menu_disphil(&menu_sys, grid_id[gridwhich], gridsel);
	    }

	    break;
	  }
       }
   } while (numev!=-1);
   
   /* Erase this set of menus, and put up the main interface */
   menu_sys_erase(&menu_sys);
   display_title(FALSE);
   menu_display(&menu_sys, MAIN_MENU);
}

/*
** Puts the information from the specified combatants grid row into the
** given combatant structure.  Returns -1 if there's no combatant on the row
*/
make_grid_combatant(c,row)
     Combatant *c;
     int row;
{
  c->vdesc = grid_val[2][row];
  if(c->vdesc == UNDEFINED) return -1;
  c->player[0] = grid_val[0][row];
  c->num_players = (c->player[0] == UNDEFINED) ? 0 : 1;
  c->program[0] = grid_val[1][row];
  c->num_programs = (c->program[0] == UNDEFINED) ? 0 : 1;
  c->team = grid_val[3][row];
  if(c->team == UNDEFINED) c->team = 0;
  return 0;
}

/*
** When called with init true, internally remembers the choice, and
** displays a menu with selections from 0 to 10.
** When called with init false, sets the initialized choice to the number.
*/
do_num(num,init)
     int num;
     Boolean init;
{
  static int choice;

  if(init) {
    choice = num;
    menu_disphil(&menu_sys, NUM_MENU, setting_num(choice));
  }
  else {
    set_setting(choice,num);
    display_settings();
  }
}

/*
** Concatenates two strings and prints result on specified line of game window.
*/
#define display_game_str(s1,s2,line) \
  do {(void) strcpy(temp,s1); \
  (void) strcat(temp,s2); \
  display_mesg(GAME_WIN,temp,line,M_FONT);} while(0)

/*
** Converts a format and number to a string, and prints result on a given line.
*/
#define display_game_num(format,num,line) \
  do {(void) sprintf(temp,format,num); \
  display_mesg(GAME_WIN,temp,line,M_FONT);} while(0)

/*
** Displays information about the settings in the game window.
*/
display_settings()
{
  extern char *mode_str[],*game_str[],*bool_str[];
  char temp[41];
  int line;

  clear_window(GAME_WIN);

  line = 0;
  display_game_str("Mode:  ",mode_str[settings.mode],line++);
  display_game_str("Game:  ",game_str[settings.game],line++);
  display_game_num("Speed: %d", settings.game_speed,line++);
  
  /* If maze is random, display density, else display name */
  if(settings.mdesc != (Mdesc *) NULL)
    display_game_str("Maze:  ",settings.mdesc->name,line++);
  else
    display_game_num("Maze:  Density %d", setting_num(SET_DENSITY),line++);
  display_game_num("Difficulty:       %d",settings.difficulty,line++);
  display_game_num("Winning score:    %d",settings.winning_score,line++);
  display_game_num("Outpost strength: %d",settings.outpost_strength,line++);
  display_game_num("Scroll speed:     %.0f",settings.scroll_speed,line++);
  display_game_num("Box slowdown:     %d",setting_num(SET_BOX_SLOW),line++);
  
  if(settings.game == ULTIMATE_GAME || settings.game == CAPTURE_GAME) {
    display_game_num("Disc friction:    %d",setting_num(SET_DISC_FRIC),line++);
    display_game_num("Owner slowdown:   %d",setting_num(SET_DISC_SLOW),line++);
  }
}

/*
** Sets the specified setting to the given number scaled from the range 0 - 10.
*/
set_setting(setting,num)
     int setting,num;
{
  switch(setting) {
    case SET_DENSITY:   settings.maze_density     = num * 10;            break;
    case SET_DIFFICULTY:settings.difficulty       = num;                 break;
    case SET_OUTPOST:   settings.outpost_strength = num;                 break;
    case SET_SCROLL:    settings.scroll_speed     = (float) num;         break;
    case SET_BOX_SLOW:  settings.box_slow      = 1.0 - (float) num / 20; break;
    case SET_DISC_FRIC: settings.disc_friction = 1.0 - (float) num /200; break;
    case SET_DISC_SLOW: settings.disc_slow     = 1.0 - (float) num / 10; break;
  }
}

/*
** Returns the value of the specified setting scaled to the range 0 - 10.
*/
setting_num(setting)
     int setting;
{
  switch(setting) {
    case SET_DENSITY:   return (int) (settings.maze_density / 10 + .5);
    case SET_DIFFICULTY:return settings.difficulty;
    case SET_OUTPOST:   return settings.outpost_strength;
    case SET_SCROLL:    return (int) (settings.scroll_speed + .5);
    case SET_BOX_SLOW:  return (int) ((1.0 - settings.box_slow) * 20 + .5);
    case SET_DISC_FRIC: return (int) ((1.0 - settings.disc_friction) * 200+.5);
    case SET_DISC_SLOW: return (int) ((1.0 - settings.disc_slow) * 10 + .5);
  }
  return 0;
}

/*
** Plays a game and explains failure to the user.
*/
interface_play()
{
  int line,i;

  clear_window(ANIM_WIN);
  if(play_game() == GAME_FAILED) {
    /* Report the result of the game to everyone */
    for(i = 0 ; i < num_terminals ; i++) {
      set_terminal(i);
      line = 3;
      iprint("This game failed to work either because there were no",0,line++);
      iprint("combatants or there was no room in the maze for them.",0,line++);
      iprint("Vehicles won't be placed on landmarks in the maze.",0,line++);
    }
    set_terminal(0);
    iprint("Any key or button to continue",0,line+1);
    wait_input();
  }
  expose_win(ANIM_WIN,TRUE);
  expose_win(GAME_WIN,TRUE);
}

/*
** Displays information about a vehicle, maze, setup, player, or program.
*/
do_view(menu,choice)
     int menu,choice;
{
  clear_window(ANIM_WIN);
  switch(menu) {
    case VEHICLES_MENU: display_vdesc(&vdesc[choice],ON); break;
    case MAZES_MENU: display_mdesc(&mdesc[choice]); break;
    case PROGRAMS_MENU: display_program(prog_desc[choice]); break;
    case SETUPS_MENU: break;
    case PLAYERS_MENU: break;
    }

  iprint("Hit any key or button to continue",0,50);
  wait_input();
  expose_win(ANIM_WIN,TRUE);
}

/*
** Displays information about the specified maze.
*/
display_mdesc(d)
     Mdesc *d;
{
  make_maze(d);
  display_mdesc_maze();
  display_mdesc_info(d);
}

/*
** Displays information about the specified program.
*/
display_program(p)
     Prog_desc *p;
{
  char temp[256],*ptr,*end_ptr;
  int row,width,len,i;

#ifdef S1024x864
  width = 75;
#endif
#ifdef S640x400
  width = 50;
#endif

  row = 0;
  sprintf(temp,"Name:    %s",p->name);      iprint(temp,0,row);  row++;
  sprintf(temp,"Author:  %s",p->author);    iprint(temp,0,row);  row++;
  sprintf(temp,"Skill:   %d",p->skill);     iprint(temp,0,row);  row++;
  sprintf(temp,"Vehicle: %s",p->vehicle);   iprint(temp,0,row);  row++;

  /* Print out abilities with word wrap */
  row++;
  iprint("Abilities:",0,row);  row++;
  temp[0] = '\0';
  i = 0;
  do {
    if(p->abilities & 1<<i) {
      ptr = ability_desc[i];
      if(strlen(temp) + strlen(ptr) > width - 7) {
	/* Print a row of text and start on a new one */
	iprint(temp,5,row);
	row++;
	strcpy(temp,ptr); strcat(temp,"  ");
      }
      else
	strcat(temp,ptr); strcat(temp,"  ");
    }
  } while(++i < MAX_PROG_ABILITIES);

  if(temp[0] != '\0') {
    iprint(temp,5,row);  row++;
  }

  /* Print out strategy with word wrap */
  row++;
  iprint("Strategy:",0,row);  row++;
  for(ptr = p->strategy, end_ptr = ptr + strlen(ptr);
      ptr < end_ptr ;
      ptr += len, row++) {
    len = end_ptr - ptr;

    if(len > width) {
      /* Do word wrap by looking for the nearest space to the left */
      len = width;
      while(len > 0 && ptr[len-1] != ' ') len--;

      /* If we have a word that is width long, display the width */
      if(len == 0) len = width;
    }
    strncpy(temp,ptr,len);
    temp[len] = '\0';
    iprint(temp,5,row);
  }
}

/*
** Prompts user for the number of players to add, and the display names
** for the terminals.  Checks the internet address, makes, and initializes
** each terminal.  Gets player information for each terminal.
*/
add_players()
{
#ifdef UNIX
  extern char *network_error_str[];
#endif
  extern char video_error_str[];
  extern int num_terminals;
  char name[80],result[256],*tmp;
  int num,ret,i;

  if(num_terminals == MAX_TERMINALS) {
    iprint("There is no room for more players.",ASK_X,ASK_Y);
    iprint("Hit any key or button to continue",ASK_X,ASK_Y+1);
    wait_input();
    clear_ask();
  }
  else {
    num = input_int(ANIM_WIN,"Number of players to add",ASK_X,ASK_Y,
		    1,0,MAX_TERMINALS - num_terminals,INT_FONT);
    clear_ask();
    for(i = 0 ; i < num ; i++) {
      input_string(ANIM_WIN,"Display name:",name,
		   ASK_X,ASK_Y,INT_FONT);
      if(name[0] == '\0') {
	clear_ask();
	break;
      }

#ifdef UNIX
      tmp = name;
      if(ret = check_internet(1,&tmp))
	strcpy(result,network_error_str[ret]);
      else {
#endif
	(void) strcpy(result,"Initializing ");
	(void) strcat(result,name);
	iprint(result,ASK_X,ASK_Y+1);
	flush_output();

	if(make_terminal(name))
	  strcpy(result,video_error_str);
	else {
	  set_terminal(num_terminals-1);
	  get_player_info();
	  players_entries[num_terminals-1] = term->player_name;
	  set_terminal(0);
	  strcpy(result,"Terminal initialized");
	}
#ifdef UNIX
      }
#endif
      iprint(result,ASK_X,ASK_Y+2);
      iprint("Hit any key or button to continue",ASK_X,ASK_Y+3);
      wait_input();
      clear_ask();
    }
    menu_resize(&menu_sys,PLAYERS_MENU,num_terminals);
  }
}

/*
** Removes the specified player's terminal from the terminal list, freeing
** all allocated storage and closing its display.  Will not remove player 0.
*/
remove_player(num)
     int num;
{
  extern int num_terminals;
  extern Terminal *terminal[];

  /* Check for bogus player number */
  if(num < 1 || num >= num_terminals) return;

  /* Close the terminal and fix up the terminal list and menu */
  close_terminal(terminal[num]);
  if(num != num_terminals - 1)
    terminal[num] = terminal[num_terminals - 1];
  num_terminals--;
  menu_resize(&menu_sys,PLAYERS_MENU,num_terminals);
}

/*
** Asks the current terminal for a player name and a vehicle name.
*/
get_player_info()
{
  extern char username[];
  int line,vdesc;

  clear_window(ANIM_WIN);
  sync_output(TRUE);

  line = 5;
  input_string(ANIM_WIN,"Enter player name:",term->player_name,0,
	       line,INT_FONT);

  /* If no name is given, use the username */
  if(term->player_name[0] == '\0')
    (void) strcpy(term->player_name,username);

  vdesc = ask_desc(VDESC,0,line+1);
  if(vdesc == -1) vdesc = 0;
  term->vdesc = vdesc;
}

/*
** Prompts the user for a program name.
** Compiles and loads the program, adding it to the program list.
** Reports any errors to the user, displaying the errors produced
** by the compiler or linker.
*/
make_prog_desc()
{
  extern char pathname[], programsdir[];
  static char *report[] = {
    "Program loaded","Improper filename","Compiler errors","Linker errors",
    "Can't read output","Can't parse symbol table","Missing description"};
  static char prev_filename[256];
  char *rindex();
  char *output_name,*error_name,filename[256],temp[256];
  char *code;
  Prog_desc *pdesc;
  int ret,line,i;
 
  clear_window(ANIM_WIN);
  line = 3;

  /* Check that there is room for another program */
  if(num_prog_descs >= MAX_PDESCS) {
    iprint("No room for more programs.  Key or button to continue",0,line);
    wait_input();
    return;
  }

  /* Prompt the user for the program name */
  iprint("Give full program filename for a .c or .o file",0,line++);
  sprintf(temp,"Enter filename [%s]:",prev_filename);
  input_string(ANIM_WIN,temp,filename,0,line++,INT_FONT);
  
  /* If nothing entered use default (if any), otherwise set default */
  if(filename[0] == '\0') {
    if(prev_filename[0] == '\0') return;
    else (void) strcpy(filename,prev_filename);
  }
  else {
    (void) strcpy(prev_filename,filename);
  }
  
  /* Prepend the path to the programs directory if necessary */
  if(!rindex(filename,'/')) {
    (void) strcpy(temp,pathname);
    (void) strcat(temp,programsdir);
    (void) strcat(temp,filename);
    (void) strcpy(filename,temp);
  }

  /* State the load request and flush */
  sprintf(temp,"Loading %s",filename);
  iprint(temp,0,line++);
  flush_output();

  /* Compile and load the program */
  error_name = "/tmp/xtank.error";
  output_name = "/tmp/xtank.output";
  pdesc = prog_desc[num_prog_descs];
  ret = compile_module(filename,(char **) &pdesc,&code,error_name,output_name);

  /* Report the result */
  (void) strcpy(temp,report[ret]);
  (void) strcat(temp,".  Key or button to continue.");
  iprint(temp,0,line);
  wait_input();

  /* If there are any errors, show the error file */
  if(ret == 2 || ret == 3) display_file(ANIM_WIN,error_name);
  else if(ret == 0) {
    pdesc->code = code;

    /* If program has been loaded before, free the previous one and replace */
    for(i = 0 ; i < num_prog_descs ; i++) {
      /* Look for a loaded program (code != NULL) with matching name */
      if(prog_desc[i]->code != (char *) NULL &&
	!strcmp(prog_desc[i]->name,pdesc->name)) {
	free(prog_desc[i]->code);
	break;
      }
    }

    /* Copy the pointers into the description and menu entries arrays */
    prog_desc[i] = pdesc;
    programs_entries[i] = pdesc->name;

    /* If new slot used, increment the count and resize the menu */
    if(i == num_prog_descs) {
      num_prog_descs++;
      menu_resize(&menu_sys,PROGRAMS_MENU,num_prog_descs);
    }
  }
}

/*
** Ask the user for a desc name, try to load it and then fix the menus.
*/
interface_load(type)
     int type;
{
  int max_descs;

  switch(type) {
    case MDESC: max_descs = num_mdescs; break;
    case VDESC: max_descs = num_vdescs; break;
    case SDESC: max_descs = num_sdescs; break;
  }

  /* If we just added to the end, fix the menu */
  if(ask_desc(type,ASK_X,ASK_Y) == max_descs) fix_desc_menu(type);
}    

/*
** Make the named desc, fix the desc menu, set the desc in the settings.
*/
interface_set_desc(type,name)
     int type;
     char *name;
{
  int num,max_descs,result;

  switch(type) {
    case MDESC: max_descs = num_mdescs; result = make_mdesc(name,&num); break;
    case VDESC: max_descs = num_vdescs; result = make_vdesc(name,&num); break;
    case SDESC: max_descs = num_sdescs; result = DESC_NOT_FOUND; break;
  }

  if(result == DESC_LOADED) {
    switch(type) {
      case MDESC:
        settings.mdesc = &mdesc[num];
	settings.game = settings.mdesc->type;
	break;
      case VDESC:
        term->vdesc = num;
	break;
      case SDESC:
	break;
      }
    if(num == max_descs) fix_desc_menu(type);
  }
}

/*
** A description has been loaded, so add an item to the right menu.
*/
fix_desc_menu(type)
     int type;
{
  switch(type) {
    case VDESC:
      vehicles_entries[num_vdescs-1] = vdesc[num_vdescs-1].name;
      menu_resize(&menu_sys,VEHICLES_MENU,num_vdescs);
      break;
    case MDESC:
      /* Leave last entry as the random maze */
      mazes_entries[num_mdescs] = mazes_entries[num_mdescs-1];
      mazes_entries[num_mdescs-1] = mdesc[num_mdescs-1].name;
      menu_resize(&menu_sys,MAZES_MENU,num_mdescs+1);
      break;
    case SDESC:
      setups_entries[num_sdescs-1] = sdesc[num_sdescs-1].name;
      menu_resize(&menu_sys,SETUPS_MENU,num_vdescs);
      break;
    }
}

/*
** Prompts the user for a vehicle, maze, or setup name and loads it.
** Returns number of last description loaded or -1 if none loaded.
*/
int ask_desc(type,row,col)
     int type;
{
  char prompt[80],resp[80],*format,*tname;
  int ret,num;

  switch(type) {
    case VDESC: tname = "vehicle"; break;
    case MDESC: tname = "maze"; break;
    case SDESC: tname = "setup"; break;
    }

  do {
    clear_ask();

    /* Prompt the user for the name */
    (void) sprintf(prompt,"Enter %s name:",tname);
    input_string(ANIM_WIN,prompt,resp,row,col,INT_FONT);

    /* If user enters nothing, don't bother trying to load */
    if(*resp == '\0') {
      ret = DESC_NOT_FOUND;
      break;
    }

    /* Load the proper description type */
    switch(type) {
      case VDESC: ret = make_vdesc(resp,&num); break;
      case MDESC: ret = make_mdesc(resp,&num); break;
      case SDESC: ret = DESC_NOT_FOUND; break;
      }

    /* Respond based on result of load */
    switch(ret) {
      case DESC_LOADED:     format = "%s loaded.  %s";  break;
      case DESC_NOT_FOUND:  format = "%s not found.  %s"; break;
      case DESC_NO_ROOM:    format = "no room for %s.  %s"; break;
      case DESC_BAD_FORMAT: format = "bad %s format.  %s"; break;
      }

    if (ret==DESC_LOADED) break;

    /* Explain result and ask if they want to try again */
    (void) sprintf(prompt,format,tname,"Try again? (y/n) [n]:");
    input_string(ANIM_WIN,prompt,resp,row,col+1,INT_FONT);
  } while(resp[0] == 'y');

  clear_ask();

  return ((ret == DESC_LOADED) ? num : -1);
}

ask_winning_score()
{
  if(settings.game == WAR_GAME)
    settings.winning_score = input_int(ANIM_WIN,"Winning score",ASK_X,ASK_Y,
				       75,0,100,INT_FONT);
  else
    settings.winning_score = input_int(ANIM_WIN,"Winning score",ASK_X,ASK_Y,
				       10000,0,100000,INT_FONT);
  clear_ask();
}

ask_maze_density()
{
  int t = input_int(ANIM_WIN,"Maze density",ASK_X,ASK_Y,3,0,10,INT_FONT);
  set_setting(SET_DENSITY,t);
  clear_ask();
}

/*
** Reads in the specified file and displays it in the specified window.
*/
display_file(w,filename)
     int w;
     char *filename;
{
  extern char *read_file();
  char *str;

  str = read_file(filename);
  display_long_str(w,str,INT_FONT);
  free(str);
}

/*
** Displays a long string in the specified window.  Properly formats
** tabs and newlines.  Prompts user to hit a key after each page,
** and at the end of the string.
*/
display_long_str(w,str,font)
     int w;
     char *str;
     int font;
{
  char c,temp[DISP_WIDTH+1];
  Boolean write = FALSE;
  int row,col;

  clear_window(w);
  col = 0;
  row = 0;

  do {
    switch(c = *(str++)) {
      case '\t':
        if(col < DISP_WIDTH-8)
	  do {
	    temp[col++] = ' ';
	  } while(col%8 != 0);
	else
	  write = TRUE;
	break;
      case '\n':
      case '\0':
	write = TRUE;
	break;
      default:
	temp[col++] = c;
	if(col > DISP_WIDTH) write = TRUE;
      }

    if(write == TRUE) {
      temp[col] = '\0';
      display_mesg2(w,temp,DISP_X,DISP_Y+row,font);
      col = 0;
      row++;
      write = FALSE;
    }

    if(row == DISP_HEIGHT-2 || c == '\0') {
      display_mesg2(w,"Hit any key or button to continue",
		    DISP_X + (DISP_WIDTH - 35)/2,DISP_Y+2+row,font);
      wait_input();
      clear_window(w);
      row = 0;
    }
  } while(c != '\0');
}

#define XTANK_X     ANIM_WIN_WIDTH/2
#define XTANK_Y     ANIM_WIN_HEIGHT/16
#define NUM_GLEAMS  30 
#define GLEAM_SPEED 10

/*
** Displays the title.  Sweeps gleams across it if gleams is true.
*/
display_title(gleams)
     Boolean gleams;
{
  extern Object *random_obj[],*exp_obj[];
  Object *gleam_obj,*title_obj;
  Picture *old_pic,*pic;
  char version[40];
  int x[NUM_GLEAMS],y[NUM_GLEAMS],num[NUM_GLEAMS];
  int width,height,offset_x,offset_y;
  int num_pics;
  int sweep;
  int i;

  /* Initialize a few variables */
  gleam_obj = exp_obj[1];
  title_obj = random_obj[XTANK_OBJ];
  (void) strcpy(version,"Version ");
  (void) strcat(version,VERSION);
  pic = &title_obj->pic[0];
  width = pic->width;
  height = pic->height;
  offset_x = pic->offset_x;
  offset_y = pic->offset_y;

  /* Show the title bitmap, copyright notice, author, and version number */
  draw_picture(ANIM_WIN,XTANK_X,XTANK_Y,pic,DRAW_COPY,WHITE);
  draw_text(ANIM_WIN,XTANK_X,XTANK_Y + offset_y + 8,
	    "Copyright 1988 by Terry Donahue",L_FONT,DRAW_COPY,WHITE);
  draw_text(ANIM_WIN,XTANK_X,XTANK_Y + offset_y + 25,
	    version,L_FONT,DRAW_COPY,WHITE);

  if(gleams == FALSE) return;

  /* Initialize x, y, and picture number for every gleam */
  num_pics = gleam_obj->num_pics;
  for(i = 0 ; i < NUM_GLEAMS ; i++) {
    x[i] = XTANK_X + rnd(width) - offset_x;
    y[i] = XTANK_Y + rnd(height) - offset_y;
    num[i] = -1;
  }

  /* Make the gleams sweep across the title from left to right */
  for(sweep = XTANK_X - offset_x ; sweep < XTANK_X + width*1.2;
      sweep += GLEAM_SPEED) {
    for(i = 0 ; i < NUM_GLEAMS ; i++) {
      if(num[i] == num_pics) continue;
      if(num[i] == num_pics - 1) {
	/* just erase the gleam if it at the last picture */
	pic = &gleam_obj->pic[num[i]];
	draw_picture(ANIM_WIN,x[i],y[i],pic,DRAW_XOR,WHITE);
	num[i]++;
      }
      else if(num[i] >= 0) {
	/* redisplay the gleam if it has a picture */
	old_pic = &gleam_obj->pic[num[i]];
	draw_picture(ANIM_WIN,x[i],y[i],old_pic,DRAW_XOR,WHITE);
	pic = &gleam_obj->pic[num[i]+1];
	draw_picture(ANIM_WIN,x[i],y[i],pic,DRAW_XOR,WHITE);
	num[i]++;
      }
      else if(x[i] < sweep) {
	/* start a gleam, since the sweep has passed */
	num[i]++;
	pic = &gleam_obj->pic[num[i]];
	draw_picture(ANIM_WIN,x[i],y[i],pic,DRAW_XOR,WHITE);
      }
    }
  }
}
