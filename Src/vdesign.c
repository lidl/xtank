/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** vdesign.c
*/

#include "xtank.h"
#include "gr.h"
#include "vehicle.h"
#include "vstructs.h"
#include "vdesign.h"
#include "menu.h"

#define DISPLAY_ROW   0
#define MENU_ROW      20
#define INPUT_ROW     34
#define LEV0_X   10
#define LEV0_Y   370
#define LEV1_X   150

#ifdef S1024x864
#define VEHICLE_X     382
#define VEHICLE_Y     175
#define VEHICLE_SIZE  70

#define VDESIGN_FONT  L_FONT
#endif
#ifdef S640x400
#define VEHICLE_X     150
#define VEHICLE_Y     75
#define VEHICLE_SIZE  40
#define VDESIGN_FONT  S_FONT
#endif

#define vprint(str,x,y) \
  (display_mesg2(ANIM_WIN,str,x,y,VDESIGN_FONT))

#define dprint(str,condition,y,x) \
  if(status == ON) \
    vprint(str,x,DISPLAY_ROW+y); \
  else if(condition) { \
    clear_text_rc(ANIM_WIN,x,y,30 - x%30,1,VDESIGN_FONT); \
    vprint(str,x,DISPLAY_ROW+y); \
  }

#define clear_vdesign_input() \
  clear_text_rc(ANIM_WIN,0,INPUT_ROW,80,8,VDESIGN_FONT)
#define clear_vdesign_display() \
  clear_text_rc(ANIM_WIN,0,DISPLAY_ROW,80,MENU_ROW-DISPLAY_ROW,VDESIGN_FONT)

/* Values for all the menus used */
#define MAIN_MENU         0
#define ENGINES_MENU      1
#define WEAPONS_MENU      2
#define ARMORS_MENU       3
#define BODIES_MENU       4
#define MOUNTS_MENU       5
#define SPECIALS_MENU     6
#define SUSPENSIONS_MENU  7
#define TREADS_MENU       8
#define BUMPERS_MENU      9
#define SIDES_MENU        10
#define WEAPNUMS_MENU     11
#define MAX_MENUS         12

static char *main_entries[] = {
  "Body", "Engine", "Weapons", "Armor type", "Armor values", "Specials",
  "Heat sinks", "Suspension", "Treads", "Bumpers", "Load vehicle",
  "Save vehicle", "Reset vehicle", "Quit" };

static char *mount_entries[] = {
  "Turret 1", "Turret 2", "Turret 3", "Front", "Back", "Left", "Right" };

static char *side_entries[] = {
  "Front", "Back", "Left", "Right", "Top", "Bottom", "All" };

static char *weapnum_entries[] = { "1", "2", "3", "4", "5", "6" };
  
static char
  *engine_entries      [VMAX_ENGINES],
  *weapon_entries      [VMAX_WEAPONS+1],
  *armor_entries       [VMAX_ARMORS],
  *body_entries        [VMAX_BODIES],
  *special_entries     [VMAX_SPECIALS],
  *suspension_entries  [VMAX_SUSPENSIONS],
  *tread_entries       [VMAX_TREADS],
  *bumper_entries      [VMAX_BUMPERS];

static Menu_int menu_sys;
static whichlevel[MAX_MENUS]={0,1,1,1,1,1,1,1,1,1,1,1};

static Vdesc design_vdesc;
static unsigned int problems;

/*
** Allows user to design a vehicle.
*/
design_vehicle()
{
  Vdesc *d;

  /* Initialize everything */
  d = &design_vdesc;
  init_vdesc(d);
  clear_window(ANIM_WIN);
  init_vdesign_interface(d);
  vdesign_specials_hil(d);
  vdesign_interface(d);
}

/*
** Initializes the vdesign interface menus.
*/
init_vdesign_interface()
{
  menu_sys_window(&menu_sys, ANIM_WIN);

  menu_norm_make(&menu_sys, MAIN_MENU, "Vdesign", 14, 0,
		 LEV0_X, LEV0_Y, main_entries, M_FONT);
  menu_norm_make(&menu_sys, ENGINES_MENU, "Engines", VMAX_ENGINES, 0,
		 LEV1_X, LEV0_Y, engine_entries, M_FONT);
  menu_norm_make(&menu_sys, WEAPONS_MENU, "Weapons", VMAX_WEAPONS, 0,
		 LEV1_X, LEV0_Y, weapon_entries, M_FONT);
  menu_norm_make(&menu_sys, ARMORS_MENU, "Armors", VMAX_ARMORS, 0,
		 LEV1_X, LEV0_Y, armor_entries, L_FONT);
  menu_norm_make(&menu_sys, BODIES_MENU, "Bodies", VMAX_BODIES, 0,
		 LEV1_X, LEV0_Y, body_entries, M_FONT);
  menu_norm_make(&menu_sys, MOUNTS_MENU, "Mounts", 7, 0,
		 LEV1_X, LEV0_Y, mount_entries, L_FONT);
  menu_flag_make(&menu_sys, SPECIALS_MENU, "Specials", VMAX_SPECIALS, 0,
		 LEV1_X, LEV0_Y, special_entries, L_FONT);
  menu_norm_make(&menu_sys, SUSPENSIONS_MENU,"Suspensions",VMAX_SUSPENSIONS,0,
		 LEV1_X, LEV0_Y, suspension_entries, L_FONT);
  menu_norm_make(&menu_sys, TREADS_MENU, "Treads", VMAX_TREADS, 0,
		 LEV1_X, LEV0_Y, tread_entries, L_FONT);
  menu_norm_make(&menu_sys, BUMPERS_MENU, "Bumpers", VMAX_BUMPERS, 0,
		 LEV1_X, LEV0_Y, bumper_entries, L_FONT);
  menu_norm_make(&menu_sys, SIDES_MENU, "Sides", 7, 0,
		 LEV1_X, LEV0_Y, side_entries, L_FONT);
  menu_norm_make(&menu_sys, WEAPNUMS_MENU, "Number", MAX_WEAPONS, 0,
		 LEV1_X, LEV0_Y, weapnum_entries, L_FONT);
}

/*
** Sets up the correct highlighting on the specials menu
*/
vdesign_specials_hil(d)
     Vdesc *d;
{
  int i;

  menu_unhighlight(&menu_sys, SPECIALS_MENU);
  for(i = 0 ; i < VMAX_SPECIALS ; i++)
    if(d->specials & (1<<i)) menu_set_hil(&menu_sys,SPECIALS_MENU,i);
}

/*
** Displays menus and prompts for all the parts of a vehicle.
*/
vdesign_interface(d)
     Vdesc *d;
{
  Event ev;
  char temp[256];
  int numev,menu,choice,row,i;
  int weapnum,weaptype;
  Boolean quit = FALSE;
  Boolean changed = FALSE;

  weapnum = 0;
  clear_window(ANIM_WIN);
  compute_vdesc(d);
  display_vdesc(d,ON);
  menu_display(&menu_sys, MAIN_MENU);

  do {
    /* Redisplay the current vehicle parameters if they've changed */
    if(changed) {
      compute_vdesc(d);
      display_vdesc(d,REDISPLAY);
      changed = FALSE;
    }
    if(win_exposed(ANIM_WIN)) {
      clear_window(ANIM_WIN);
      display_vdesc(d,ON);
      menu_system_expose(&menu_sys);
      expose_win(ANIM_WIN,FALSE);
    }

    numev=1;
    get_events(&numev, &ev);
    if (numev < 1) continue;
    switch(ev.type) {
      case EVENT_LBUTTON:
      case EVENT_MBUTTON:
      case EVENT_RBUTTON:
        menu = menu_hit(&menu_sys, ev.x, ev.y);
	if (menu==MENU_NULL) break;

	/* Erase all equal or higher level menus */
	erase_vdesign_menus(menu);

	/* Find out which choice on the menu was selected */
	menu_hit_p(&menu_sys, ev.x, ev.y, &menu, &choice);

	switch(menu) {
	  case MAIN_MENU:
	    switch (choice) {
	      case 0: menu_disphil(&menu_sys, BODIES_MENU, d->body); break;
	      case 1: menu_disphil(&menu_sys, ENGINES_MENU, d->engine); break;
	      case 2:
		menu_resize(&menu_sys,WEAPNUMS_MENU,
			    min(d->num_weapons + 1,MAX_WEAPONS));
		menu_display(&menu_sys, WEAPNUMS_MENU);
		break;
	      case 3: menu_disphil(&menu_sys,ARMORS_MENU,d->armor.type); break;
	      case 4: menu_display(&menu_sys, SIDES_MENU); break;
	      case 5: menu_display(&menu_sys, SPECIALS_MENU); break;
	      case 6:
		d->heat_sinks = input_int(ANIM_WIN,"Number of heat sinks",
					  0,INPUT_ROW,d->heat_sinks,0,99,
					  VDESIGN_FONT);
		changed = TRUE;
		menu_unhighlight(&menu_sys, MAIN_MENU);
		clear_vdesign_input();
		break;
	      case 7: menu_disphil(&menu_sys, SUSPENSIONS_MENU,d->suspension);
		      break;
	      case 8: menu_disphil(&menu_sys, TREADS_MENU, d->treads); break;
	      case 9: menu_disphil(&menu_sys, BUMPERS_MENU, d->bumpers); break;
	      case 10:
		vdesign_load(d);
		changed = TRUE;
		menu_unhighlight(&menu_sys, MAIN_MENU);
		break;
	      case 11:
		vdesign_save(d);
		changed = TRUE;
		menu_unhighlight(&menu_sys, MAIN_MENU);
		break;
	      case 12:
		if(confirm(ANIM_WIN,"reset vehicle",0,
			   INPUT_ROW,VDESIGN_FONT)) {
		  init_vdesc(d);
		  changed = TRUE;
		}
		menu_unhighlight(&menu_sys, MAIN_MENU);
		clear_vdesign_input();
		break;
	      case 13:
		if(confirm(ANIM_WIN,"quit",0,INPUT_ROW,VDESIGN_FONT))
		  quit = TRUE;
		menu_unhighlight(&menu_sys, MAIN_MENU);
		clear_vdesign_input();
		break;
	    }
	    break;
	  case ENGINES_MENU: d->engine = choice; changed = TRUE; break;
	  case WEAPONS_MENU: 
	    menu_erase(&menu_sys, WEAPONS_MENU);
	    if (choice < VMAX_WEAPONS - 1) {
	      /* Get mount for added weapon */
	      weaptype = choice;
	      menu_display(&menu_sys, MOUNTS_MENU);
	    }
	    else {
	      /* Delete this weapon */
	      if(weapnum != d->num_weapons) {
		d->weapon[weapnum] = d->weapon[--d->num_weapons];
		changed = TRUE;
	      }
	      menu_unhighlight(&menu_sys, MAIN_MENU);
	    }
	    break;
	  case ARMORS_MENU: d->armor.type = choice; changed = TRUE; break;
	  case BODIES_MENU: d->body = choice; changed = TRUE; break;
	  case MOUNTS_MENU:
	    menu_erase(&menu_sys, MOUNTS_MENU);
	    if (weapnum == d->num_weapons)
	      ++d->num_weapons;
	    d->weapon[weapnum] = weaptype;
	    d->mount[weapnum] = choice;
	    changed = TRUE;
	    menu_unhighlight(&menu_sys, MAIN_MENU);
	    break;
	  case SPECIALS_MENU: d->specials ^= 1<<choice; changed = TRUE; break;
	  case SUSPENSIONS_MENU: d->suspension = choice; changed = TRUE; break;
	  case TREADS_MENU: d->treads = choice; changed = TRUE; break;
	  case BUMPERS_MENU: d->bumpers = choice; changed = TRUE; break;
	  case SIDES_MENU:
	    row = INPUT_ROW;
	    for (i = 0 ; i < MAX_SIDES ; i++)
	      if(choice == MAX_SIDES || choice == i) {
		(void) sprintf(temp,"%s armor value",side_entries[i]);
		d->armor.side[i] = input_int(ANIM_WIN,temp,0,row++,
					     d->armor.side[i], MIN_ARMOR,
					     MAX_ARMOR, VDESIGN_FONT);
	      }
	    changed = TRUE;
	    menu_erase(&menu_sys, SIDES_MENU);
	    menu_unhighlight(&menu_sys, MAIN_MENU);
	    clear_vdesign_input();
	    break;
	  case WEAPNUMS_MENU:
	    weapnum = choice;
	    menu_erase(&menu_sys,WEAPNUMS_MENU);
	    if(weapnum < d->num_weapons)
	      menu_set_hil(&menu_sys, WEAPONS_MENU, d->weapon[weapnum]);
	    menu_display(&menu_sys, WEAPONS_MENU);
	    break;
	}
      }
  } while (quit == FALSE);
}

/*
** Erases all menus on equal or higher levels than the specified menu.
*/
erase_vdesign_menus(mu)
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
** Sets the values in the vehicle description to a standard set.
*/
init_vdesc(d)
     Vdesc *d;
{
  int i;

  *d->name = '\0';
  strncpy(d->designer,term->player_name,MAX_STRING);

  /* Make sure there is something in the designer slot */
  if(d->designer[0] == '\0')
    (void) strcpy(d->designer,"Unknown");
    
  d->num_weapons = 0;
  d->engine = 0;
  d->body = 0;
  d->heat_sinks = 0;
  d->suspension = 1;
  d->treads = 1;
  d->bumpers = 1;

  d->armor.type = 0;
  for (i = 0 ; i < MAX_SIDES ; i++)
    d->armor.side[i] = 0;

  for (i = 0 ; i < VMAX_SPECIALS ; i++)
    d->specials |= (1 << i);
}

/*
** Computes the vehicle's parameters from the vehicle description.
** Returns problems flag.
*/
compute_vdesc(d)
     Vdesc *d;
{
  float max_acc,friction;
  int total_armor,size;
  int i;

  problems = 0;
  total_armor = 0;

  for(i = 0 ; i < MAX_SIDES ; i++) 
    total_armor += d->armor.side[i];

  size = body_stat[d->body].size;

  /* Compute weight, space and cost */
  d->weight = body_stat[d->body].weight +
              engine_stat[d->engine].weight + 
	      total_armor * armor_stat[d->armor.type].weight * size +
	      d->heat_sinks * heat_sink_stat.weight;

  d->space = engine_stat[d->engine].space +
             total_armor * armor_stat[d->armor.type].space * size +
	     d->heat_sinks * heat_sink_stat.space;

  d->cost = body_stat[d->body].cost +
            engine_stat[d->engine].cost +
	    total_armor * armor_stat[d->armor.type].cost * size +
	    d->heat_sinks * heat_sink_stat.cost +
	    suspension_stat[d->suspension].cost * size +
	    tread_stat[d->treads].cost * size +
	    bumper_stat[d->bumpers].cost * size;

  for(i = 0 ; i < d->num_weapons ; i++) {
    d->weight += weapon_stat[d->weapon[i]].weight;
    d->space += weapon_stat[d->weapon[i]].space;
    d->cost += weapon_stat[d->weapon[i]].cost;
    if (d->mount[i] < 3 && d->mount[i] + 1 > body_stat[d->body].turrets)
      problems |= BAD_MOUNT;
  }

  for(i = 0 ; i < VMAX_SPECIALS ; i++)
    if (d->specials & 1<<i)
      d->cost += special_stat[i].cost;

  if (d->weight > body_stat[d->body].weight_limit)
    problems |= OVER_WEIGHT;

  if (d->space > body_stat[d->body].space)
    problems |= OVER_SPACE;

  d->handling = body_stat[d->body].handling_base + 
              suspension_stat[d->suspension].handling_adj;

  /* Friction decreases max_speed and increases max_acc */
  friction = tread_stat[d->treads].friction;

  d->max_speed = pow((double) engine_stat[d->engine].power /
		     (double) body_stat[d->body].drag, .3333) / friction;

  /* Limit acceleration based on tread friction */
  d->acc = 16.0 * (float) engine_stat[d->engine].power / (float) d->weight;
  max_acc = BRAKING_ACC * friction;
  if (d->acc > max_acc) d->acc = max_acc;

  return (int) problems;
}

/*
** Displays all information about vehicle.
*/
display_vdesc(d,status)
     Vdesc *d;
     unsigned int status;
{
  extern Object *vehicle_obj[];
  static Vdesc od;
  static unsigned int oproblems;
  Picture *pic;
  char temp[80];
  int i;

  (void) sprintf(temp, "Name:       %s", d->name);
  dprint(temp,(strcmp(od.name,d->name)),0,0);
  (void) sprintf(temp, "Designer:   %s", d->designer);
  dprint(temp,(strcmp(od.designer,d->designer)),1,0);
  (void) sprintf(temp, "Body:       %s", body_stat[d->body].type);
  dprint(temp,(od.body != d->body),2,0);
  (void) sprintf(temp, "Engine:     %s", engine_stat[d->engine].type);
  dprint(temp,(od.engine != d->engine),3,0);
  (void) sprintf(temp, "Heat Sinks: %d", d->heat_sinks);
  dprint(temp,(od.heat_sinks != d->heat_sinks),4,0);

  dprint("Specials:",(FALSE),5,0);
  for (i=0;i<VMAX_SPECIALS;++i) {
    dprint(((d->specials & 1<<i) ? special_stat[i].type : ""),
	   ((od.specials & 1<<i) != (d->specials & 1<<i)),i+5,12);
  }
  (void) sprintf(temp, "Suspension: %s", suspension_stat[d->suspension].type);
  dprint(temp,(od.suspension != d->suspension),8,0);
  (void) sprintf(temp, "Treads:     %s", tread_stat[d->treads].type);
  dprint(temp,(od.treads != d->treads),9,0);
  (void) sprintf(temp, "Bumpers:    %s", bumper_stat[d->bumpers].type);
  dprint(temp,(od.bumpers != d->bumpers),10,0);

  (void) sprintf(temp, "Cost:     $%d", d->cost);
  dprint(temp,(od.cost != d->cost),0,31);
  (void) sprintf(temp, "Weight:   %d/%d",d->weight,
		 body_stat[d->body].weight_limit);
  dprint(temp,(od.weight != d->weight || od.body != d->body),1,31);
  (void) sprintf(temp, "Space:    %d/%d",d->space, body_stat[d->body].space);
  dprint(temp,(od.space != d->space || od.body != d->body),2,31);
  (void) sprintf(temp, "Max Spd:  %.2f", d->max_speed);
  dprint(temp,(od.max_speed != d->max_speed),3,31);
  (void) sprintf(temp, "Accel:    %.2f", d->acc);
  dprint(temp,(od.acc != d->acc),4,31);
  (void) sprintf(temp, "Handling: %d", d->handling);
  dprint(temp,(od.handling != d->handling),5,31);

  dprint("Problems:",(FALSE),6,31);
  dprint(((problems & OVER_WEIGHT) ? "OVER WEIGHT LIMIT" : ""),
	 ((problems & OVER_WEIGHT) != (oproblems & OVER_WEIGHT)),6,41);
  dprint(((problems & OVER_SPACE) ? "OVER SPACE LIMIT" : ""),
	 ((problems & OVER_SPACE) != (oproblems & OVER_SPACE)),7,41);
  dprint(((problems & BAD_MOUNT) ? "BAD WEAPON MOUNT" : ""),
	 ((problems & BAD_MOUNT) != (oproblems & BAD_MOUNT)),8,41);

  dprint("# Weapon             Mount",(FALSE),12,0);
  for (i=0;i<d->num_weapons;++i) {
    (void) sprintf(temp, "%1d %-18s %s", i+1, weapon_stat[d->weapon[i]].type,
		    mount_entries[d->mount[i]]);
    dprint(temp,
	   ((od.weapon[i] != d->weapon[i]) ||
	    (od.mount[i] != d->mount[i]) ||
	    (i >= od.num_weapons)),
	   i+13,0);
  }
  
  /* Erase any extra lines if redisplaying */
  if(status == REDISPLAY)
    for(i = d->num_weapons ; i < od.num_weapons ; i++)
      dprint("",(TRUE),i+13,0);

  (void) sprintf(temp, "Armor: %s", armor_stat[d->armor.type].type);
  dprint(temp,(od.armor.type != d->armor.type),12,31);
  for (i=0;i<MAX_SIDES;++i) {
    (void) sprintf(temp, "%-6s  %d", side_entries[i], d->armor.side[i]);
    dprint(temp,(od.armor.side[i] != d->armor.side[i]),i+13,33);
  }

  /* Draw a picture of the body inside a box */
  if(status == REDISPLAY && od.body != d->body)
    draw_filled_rect(ANIM_WIN,VEHICLE_X-VEHICLE_SIZE/2,
		     VEHICLE_Y-VEHICLE_SIZE/2,
		     VEHICLE_SIZE,VEHICLE_SIZE,DRAW_COPY,BLACK);
  if(status == ON || od.body != d->body) {
    draw_rect(ANIM_WIN,VEHICLE_X-VEHICLE_SIZE/2,VEHICLE_Y-VEHICLE_SIZE/2,
	      VEHICLE_SIZE,VEHICLE_SIZE,DRAW_COPY,WHITE);
    pic = &vehicle_obj[d->body]->pic[12];
    draw_picture(ANIM_WIN,VEHICLE_X,VEHICLE_Y,pic,DRAW_COPY,WHITE);
  }

  /* Remember the vehicle description for next time */
  od = *d;
  oproblems = problems;
}

/*
** Prompts user for vehicle name, loads that vehicle into the specified vdesc.
*/
vdesign_load(d)
     Vdesc *d;
{
  char name[80];

  input_string(ANIM_WIN,"Enter vehicle name:",name,0,INPUT_ROW,VDESIGN_FONT);
  if(name[0] != '\0') {
    if(load_vdesc(d,name) == DESC_LOADED) {
      vprint("Vehicle loaded.",0,INPUT_ROW+1);
      vdesign_specials_hil(d);
    }
    else
      vprint("Error.  Vehicle not loaded.",0,INPUT_ROW+1);
  }
  vprint("Hit any key or button to continue.",0,INPUT_ROW+3);
  wait_input();
  clear_vdesign_input();
}

/*
** Checks if vehicle can be saved, if so, asks for a name, and saves
** the vehicle description.
*/
vdesign_save(d)
     Vdesc *d;
{
  char name[80];

  if(problems)
    vprint("Vehicle cannot be saved since it has problems.",0,INPUT_ROW);
  else {
    input_string(ANIM_WIN,"Enter vehicle name:",name,0,INPUT_ROW,VDESIGN_FONT);
    if(name[0] != '\0') {
      (void) strncpy(d->name,name,MAX_STRING);
      d->name[MAX_STRING-1] = '\0';
      if(save_vdesc(d) == DESC_SAVED) {
	vprint("Vehicle saved.",0,INPUT_ROW+1);
 	interface_set_desc(VDESC,name);
      }
      else
	vprint("Vehicle name already used.  Try another.",0,INPUT_ROW+1);
    }
  }

  vprint("Hit any key or button to continue.",0,INPUT_ROW+3);
  wait_input();
  clear_vdesign_input();
}

/*
** Puts the proper strings into the menus.
*/
init_vdesign()
{
  int i;

  for(i = 0 ; i < VMAX_WEAPONS ; i++)
    weapon_entries[i] = weapon_stat[i].type;
  weapon_entries[VMAX_WEAPONS-1] = "Remove weapon";

  for(i = 0 ; i < VMAX_ARMORS ; i++)
    armor_entries[i] = armor_stat[i].type;

  for(i = 0 ; i < VMAX_ENGINES ; i++)
    engine_entries[i] = engine_stat[i].type;

  for(i = 0 ; i < VMAX_SUSPENSIONS ; i++)
    suspension_entries[i] = suspension_stat[i].type;

  for(i = 0 ; i < VMAX_TREADS ; i++)
    tread_entries[i] = tread_stat[i].type;

  for(i = 0 ; i < VMAX_BUMPERS ; i++)
    bumper_entries[i] = bumper_stat[i].type;

  for(i = 0 ; i < VMAX_BODIES ; i++)
    body_entries[i] = body_stat[i].type;

  for(i = 0 ; i < VMAX_SPECIALS ; i++)
    special_entries[i] = special_stat[i].type;
}
