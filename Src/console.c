/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** console.c
*/

#include "xtank.h"
#include "gr.h"
#include "vstructs.h"

extern Weapon_stat weapon_stat[];

/* Bar graph widths, heights and max values */
#define ARMOR_W  18
#define ARMOR_H  35
#define ARMOR_M  99
#define ARMOR_H2 16
#define ARMOR_M2 42

#define FUEL_W   102
#define FUEL_H   16
#define FUEL_M   1000
#define HEAT_M   100

#define disp_bar(w,h,max,vert,init) \
  display_bar(CONS_WIN,word->x,word->y,w,h,c->_entry[num].value,max,vert,init)

#define make_int_entry(val) \
  do { \
    (void) sprintf(e[num].string,"%-*d", \
	  	   console_word[num].len,e[num].value=val); \
    num++; \
  } while(0)

#define make_bar_entry(val) \
  do { \
    e[num].value = val; \
    num++; \
  } while(0)

#define make_str_entry(val) \
  do { \
    (void) sprintf(e[num].string,"%-*s",console_word[num].len,val); \
    num++; \
  } while(0)

#define update_int_entry(num,val) \
  do { \
    if(val != e[num].value) { \
      (void) sprintf(e[num].string,"%-*d",console_word[num].len, \
		     e[num].value = val); \
      c->change[c->num_changes++] = num; \
    } \
  } while(0)

#define update_bar_entry(num,val) \
  do { \
    if(val != e[num].value) { \
      e[num].value = val; \
      c->change[c->num_changes++] = num; \
    } \
  } while(0)

#define update_status_entry(num,val,array) \
  do { \
    if(val != e[num].value) { \
      (void) sprintf(e[num].string,"%-*s",console_word[num].len, \
		     array[e[num].value = val]); \
      c->change[c->num_changes++] = num; \
    } \
  } while(0)

static Word console_init[] = {
  { 0,  0, 40, "    Armor      Player:                  " },
  { 0,  1, 40, "                Score:         Kills:   " },
  { 0,  2, 40, "                Money:                  " },
  { 0,  3, 40, "                                        " },
  { 0,  4, 40, "              Vehicle:                  " },
  { 0,  5, 40, "                Speed:     x:    y:     " },
  { 0,  6, 40, "                 Fuel:                  " },
  { 0,  7, 40, "                                        " },
  { 0,  8, 40, "                 Heat:                  " },
  { 0,  9, 40, "                                        " },
  { 0, 10, 40, "              Console:       Radar:     " },
  { 0, 11, 40, "               Mapper:      Safety:     " },
  { 0, 12, 40, "                                        " },
  { 0, 13, 40, "# Weapon              Mt  Ammo  Status  " },
  { 0, 14, 40, "1                                       " },
  { 0, 15, 40, "2                                       " },
  { 0, 16, 40, "3                                       " },
  { 0, 17, 40, "4                                       " },
  { 0, 18, 40, "5                                       " },
  { 0, 19, 40, "6                                       " }
};

/* Names for console entries */
#define CONSOLE_player		0
#define CONSOLE_score		1
#define CONSOLE_kills    	2
#define CONSOLE_money		3

#define CONSOLE_vehicle		4
#define CONSOLE_speed		5
#define CONSOLE_coord_x		6
#define CONSOLE_coord_y		7
#define CONSOLE_fuel		8
#define CONSOLE_heat		9

#define CONSOLE_front_armor	10
#define CONSOLE_back_armor	11
#define CONSOLE_left_armor	12
#define CONSOLE_right_armor	13
#define CONSOLE_top_armor	14
#define CONSOLE_bottom_armor	15

#define CONSOLE_console		16
#define CONSOLE_mapper		17
#define CONSOLE_radar		18
#define CONSOLE_safety          19

#define CONSOLE_weapon		20
#define CONSOLE_mount		21
#define CONSOLE_ammo		22
#define CONSOLE_weapon_status	23

/* Number of entries for each weapon */
#define NUM_WEAPON_ENTRIES	4

static Word console_word[] = {
{ 23,  0, 19 },			/* Player */
{ 23,  1,  7 },			/* Score */
{ 38,  1,  2 },			/* Kills */
{ 23,  2, 10 },			/* Money */

{ 23,  4, 19 },			/* Vehicle */
{ 23,  5,  2 },			/* Speed */
{ 30,  5,  2 },			/* Coord_x */
{ 36,  5,  2 },			/* Coord_y */
{143, 68,  0 },			/* Fuel */
{143, 88,  0 },			/* Heat */

{ 35, 18,  0 },			/* Front Armor */
{ 35, 92,  0 },			/* Back Armor */
{ 11, 55,  0 },			/* Left Armor */
{ 59, 55,  0 },			/* Right Armor */
{ 35, 55,  0 },			/* Top Armor */
{ 35, 74,  0 },			/* Bottom Armor */

{ 23, 10,  3 },			/* Console */
{ 23, 11,  3 },			/* Mapper */
{ 36, 10,  3 },			/* Radar */
{ 36, 11,  3 },			/* Safety */

{  2, 14, 18 },			/* Weapon 1 */
{ 22, 14,  2 },			/* Mount */
{ 26, 14,  3 },			/* Ammo */
{ 32, 14,  8 },			/* Status */

{  2, 15, 18 },			/* Weapon 2 */
{ 22, 15,  2 },			/* Mount */
{ 26, 15,  3 },			/* Ammo */
{ 32, 15,  8 },			/* Status */

{  2, 16, 18 },			/* Weapon 3 */
{ 22, 16,  2 },			/* Mount */
{ 26, 16,  3 },			/* Ammo */
{ 32, 16,  8 },			/* Status */

{  2, 17, 18 },			/* Weapon 4 */
{ 22, 17,  2 },			/* Mount */
{ 26, 17,  3 },			/* Ammo */
{ 32, 17,  8 },			/* Status */

{  2, 18, 18 },			/* Weapon 5 */
{ 22, 18,  2 },			/* Mount */
{ 26, 18,  3 },			/* Ammo */
{ 32, 18,  8 },			/* Status */

{  2, 19, 18 },			/* Weapon 6 */
{ 22, 19,  2 },			/* Mount */
{ 26, 19,  3 },			/* Ammo */
{ 32, 19,  8 }			/* Status */
};

static char *console_sp_status[] = { "---", "off", "on", "brk" };
static char *console_sf_status[] = { "off", "on" };

static char *console_w_status[] = { "broken", "broken", "off", "on",
  "no ammo", "no ammo", "no ammo", "no ammo" };

static char *console_mount[] = { "T1", "T2", "T3", "F", "B", "L", "R" };

special_console(v,record,action)
     Vehicle *v;
     char *record;
     unsigned int action;
{
  Console *c;
  Entry *e;
  Weapon *w;
  Word *word;
  int num;
  int i;

  c = (Console *) record;

  switch(action) {
    case SP_update:
      /* Initialize the number of changes to zero */
      c->num_changes = 0;
      e = c->_entry;
      num = 0;

      /* Update player values */
      update_int_entry(CONSOLE_score,v->owner->score);
      update_int_entry(CONSOLE_kills,v->owner->kills);
      update_int_entry(CONSOLE_money,v->owner->money);

      /* Update vehicle values */
      update_int_entry(CONSOLE_speed,(int) v->vector.speed);
      update_int_entry(CONSOLE_coord_x,v->loc->grid_x);
      update_int_entry(CONSOLE_coord_y,v->loc->grid_y);
      update_bar_entry(CONSOLE_fuel,(int) v->fuel);
      update_bar_entry(CONSOLE_heat,v->heat);
      
      /* Update armor values */
      for(i = 0 ; i < MAX_SIDES ; i++)
	update_bar_entry(CONSOLE_front_armor + i,v->armor.side[i]);

      /* Update special values */
      for(i = 0 ; i < MAX_SPECIALS ; i++)
	update_status_entry(CONSOLE_console + i,
			    v->special[CONSOLE + i].status,
			    console_sp_status);

      /* Update safety value */
      update_status_entry(CONSOLE_safety,v->safety,console_sf_status);

      /* Update weapon values */
      num = CONSOLE_ammo;
      for(i = 0 ; i < v->num_weapons ; i++) {
	w = &v->weapon[i];
	update_int_entry(num,w->ammo);
	update_status_entry(num+1,w->status,console_w_status);
	num += NUM_WEAPON_ENTRIES;
      }
      break;
    case SP_redisplay:
      if(c->num_changes == 0) break;
      for(i = 0 ; i < c->num_changes ; i++) {
	num = c->change[i];
	word = &console_word[num];
	switch(num) {
	  case CONSOLE_front_armor:
	  case CONSOLE_back_armor:
	  case CONSOLE_left_armor:
	  case CONSOLE_right_armor:
	    disp_bar(ARMOR_W,ARMOR_H,ARMOR_M,TRUE,FALSE);
	    break;
	  case CONSOLE_top_armor:
	  case CONSOLE_bottom_armor:
	    disp_bar(ARMOR_W,ARMOR_H2,ARMOR_M2,TRUE,FALSE);
	    break;
	  case CONSOLE_fuel: disp_bar(FUEL_W,FUEL_H,FUEL_M,FALSE,FALSE); break;
	  case CONSOLE_heat: disp_bar(FUEL_W,FUEL_H,HEAT_M,FALSE,FALSE); break;
	  default:
	    word->str = c->_entry[num].string;
	    draw_text_rc(CONS_WIN,word->x,word->y,word->str,S_FONT,WHITE);
	    break;
	}
      }
      break;
    case SP_draw:
      /* Draw the text template */
      for(i = 0 ; i < 20 ; i++) {
	word = &console_init[i];
	draw_text_rc(CONS_WIN,word->x,word->y,word->str,S_FONT,WHITE);
      }

      /* Draw all the entries */
      for(num = 0 ; num < MAX_ENTRIES ; num++) {
	word = &console_word[num];
	switch(num) {
	  case CONSOLE_front_armor:
	  case CONSOLE_back_armor:
	  case CONSOLE_left_armor:
	  case CONSOLE_right_armor:
	    disp_bar(ARMOR_W,ARMOR_H,ARMOR_M,TRUE,TRUE);
	    break;
	  case CONSOLE_top_armor:
	  case CONSOLE_bottom_armor:
	    disp_bar(ARMOR_W,ARMOR_H2,ARMOR_M2,TRUE,TRUE);
	    break;
	  case CONSOLE_fuel: disp_bar(FUEL_W,FUEL_H,FUEL_M,FALSE,TRUE); break;
	  case CONSOLE_heat: disp_bar(FUEL_W,FUEL_H,HEAT_M,FALSE,TRUE); break;
	  default:
	    word->str = c->_entry[num].string;
	    draw_text_rc(CONS_WIN,word->x,word->y,word->str,S_FONT,WHITE);
	    break;
	}
      }
      break;
    case SP_erase:
      clear_window(CONS_WIN);
      break;
    case SP_activate:
      /*
      ** Initialize e and num, which are used by the macros
      ** make_str_entry, make_int_entry, and make_bar_entry.
      */
      e = c->_entry;
      num = 0;

      /* Make the combatant entries */
      make_str_entry(v->owner->name);
      make_int_entry(v->owner->score);
      make_int_entry(v->owner->kills);
      make_int_entry(v->owner->money);

      /* Make the vehicle entries */
      make_str_entry(v->name);
      make_int_entry((int) v->vector.speed);
      make_int_entry(v->loc->grid_x);
      make_int_entry(v->loc->grid_y);
      make_bar_entry((int) v->fuel);
      make_bar_entry(v->heat);

      /* Make the armor entries */
      for(i = 0 ; i < MAX_SIDES ; i++)
	make_bar_entry(v->armor.side[i]);

      /* Make the specials entries */
      for(i = 0 ; i < MAX_SPECIALS ; i++)
	make_str_entry(console_sp_status[v->special[CONSOLE+i].status]);

      /* Make the safety entry */
      make_str_entry(console_sf_status[v->safety]);

      /* Make the weapon entries */
      for(i = 0 ; i < v->num_weapons ; i++) {
	w = &v->weapon[i];
	make_str_entry(weapon_stat[w->type].type); /* name of weapon */
	make_str_entry(console_mount[w->mount]);
	make_int_entry(w->ammo);
	make_str_entry(console_w_status[w->status]);
      }
      break;
    case SP_deactivate:
      break;
  }
}

/*
** Puts a bar on the screen in the specified location.
*/
display_bar(w,x,y,width,height,val,mx,vert,init)
     int w,x,y,width,height,val,mx;
     Boolean vert,init;
{
  int filled;

  if(val > mx) val = mx;

  if(mx > 0) {
    if(vert) filled = (height-2) * val / mx;
    else     filled = (width-2) * val / mx;
  }
  else
    filled = 0;

  /* Draw the outline rectangle on initialization */
  if(init)
    draw_rect(w,x,y,width-1,height-1,DRAW_COPY,WHITE);

  /* Only erase the rest of the bar area during non-initialization */
  if(vert) {
    draw_filled_rect(w,x+1,y+height-1-filled,width-2,filled,DRAW_COPY,WHITE);
    if(!init)
      draw_filled_rect(w,x+1,y+1,width-2,height-2-filled,DRAW_COPY,BLACK);
  }
  else {
    draw_filled_rect(w,x+1,y+1,filled,height-2,DRAW_COPY,WHITE);
    if(!init)
      draw_filled_rect(w,x+1+filled,y+1,width-2-filled,height-2,
		       DRAW_COPY,BLACK);
  }
}

#ifdef notdef
/*
** Redisplays the bar with the new value.
*/
redisplay_bar(w,x,y,width,height,val,new,mx,vert)
     int w,x,y,width,height,val,new,mx;
     Boolean vert;
{
  int delta,filled;

  if(mx > 0) {
    delta = abs(new-val);

    if(vert) filled = (height-2) * delta / mx;
    else     filled = (width-2) * delta / mx;

    if(filled) {
      if(vert) draw_filled_rect(w,x+1,y+height-1-filled,width-2,filled,
				DRAW_XOR,WHITE);
      else     draw_filled_rect(w,x+1,y+1,filled,height-2,
				DRAW_XOR,WHITE);
    }
  }
}    
#endif notdef
