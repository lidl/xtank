/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** display.c
*/

#include "xtank.h"
#include "gr.h"
#include "vstructs.h"

#define VEHICLE_NAME_Y 25

extern Weapon_stat weapon_stat[];

/*
** Displays everything on the current terminal.
*/
display_terminal(status)
     unsigned int status;
{
  /* Display all of the windows */
  display_anim(status);
  display_cons(status);
  display_map(status);
  display_game(status);
  display_help(status);
  display_msg(status);
  display_status(status);
}

/*
** Displays the vehicles, walls, landmarks, bullets, and explosions in the
** animation window.
*/
display_anim(status)
     unsigned int status;
{
  extern int num_vehicles;
  extern Vehicle *vehicle[];
  Vehicle *v;
  int i;

  /* Check for being exposed */
  check_expose(ANIM_WIN,status);

  /* If we are turning this window on, clear it first */
  if(status == ON) {
    clear_window(ANIM_WIN);

    /* If paused, display the pause message */
    if(settings.game_speed == 0) display_pause_message();
  }

  /* If 3d mode is on, display in 3d */
  if(term->status & TS_3d) {
    display_anim_3d(status);
    return;
  }

  /* Display the walls and landmarks in the maze */
  display_maze(status);

  /* Display the vehicles in a manner dependent on their status */
  for(i = 0 ; i < num_vehicles ; i++) {
    v = vehicle[i];
    switch(status) {
      case REDISPLAY:
        if((v->status & VS_is_alive) && (v->status & VS_was_alive))
	  display_vehicle(v,REDISPLAY);
	else if(v->status & VS_is_alive)
	  display_vehicle(v,ON);
	else if(v->status & VS_was_alive)
	  display_vehicle(v,OFF);
	break;
      case ON:
	if(v->status & VS_is_alive)
	  display_vehicle(v,ON);
	break;
      case OFF:
	if(v->status & VS_was_alive)
	  display_vehicle(v,OFF);
      }
  }

  display_bullets(status);
  display_explosions(status);
}

/*
** Displays the specified vehicle and its turrets in the animation window.
*/
display_vehicle(v,status)
     Vehicle *v;
     unsigned int status;
{
  Loc *loc,*old_loc;
  Picture *pic;

  /* Erase the old vehicle picture */
  if(status != ON) {
    old_loc = v->old_loc;
    pic = &v->obj->pic[v->vector.old_rot];
    draw_picture(ANIM_WIN,old_loc->screen_x[term->num],
		 old_loc->screen_y[term->num],pic,DRAW_XOR,v->color);

    /* Erase the string showing name and team */
    if(settings.mode != SINGLE_MODE)
      draw_text(ANIM_WIN,
		old_loc->screen_x[term->num],
		old_loc->screen_y[term->num] + VEHICLE_NAME_Y,
		v->disp,S_FONT,DRAW_XOR,v->color);
  }

  /* Draw the new vehicle picture */
  if(status != OFF) {
    loc = v->loc;
    pic = &v->obj->pic[v->vector.rot];
    draw_picture(ANIM_WIN,loc->screen_x[term->num],
		 loc->screen_y[term->num],pic,DRAW_XOR,v->color);

    /* Display a string showing name and team */
    if(settings.mode != SINGLE_MODE)
      draw_text(ANIM_WIN,
		loc->screen_x[term->num],
		loc->screen_y[term->num] + VEHICLE_NAME_Y,
		v->disp,S_FONT,DRAW_XOR,v->color);
  }

  /* Display the vehicle's turrets */
  display_turrets(v,status);
}

/*
** Displays all turrets for the specified vehicle in the animation window.
*/
display_turrets(v,status)
     Vehicle *v;
     unsigned int status;
{
  Picture *pic;
  Loc *loc,*old_loc;
  Object *obj;
  Coord *tcoord,*old_tcoord;
  Turret *t;
  int i;

  loc = v->loc;
  old_loc = v->old_loc;
  for(i = 0 ; i < v->num_turrets ; i++) {
    t = &v->turret[i];
    obj = t->obj;
    
    /* erase the old turret */
    if(status != ON) {
      old_tcoord = &v->obj->picinfo[v->vector.old_rot].turret_coord[i];
      pic = &obj->pic[t->old_rot];
      draw_picture(ANIM_WIN,old_loc->screen_x[term->num] + old_tcoord->x,
		   old_loc->screen_y[term->num] + old_tcoord->y,
		   pic,DRAW_XOR,v->color);
    }

    /* draw the new turret */
    if(status != OFF) {
      tcoord = &v->obj->picinfo[v->vector.rot].turret_coord[i];
      pic = &obj->pic[t->rot];
      draw_picture(ANIM_WIN,loc->screen_x[term->num] + tcoord->x,
		   loc->screen_y[term->num] + tcoord->y,
		   pic,DRAW_XOR,v->color);
    }
  }
}

/*
** If point_bullets is on, all non-disc bullets are drawn as points.
** Otherwise, the bullet bitmaps are used.
*/
display_bullets(status)
     unsigned int status;
{
  extern Object *bullet_obj;
  extern Bset *bset;
  Bullet *b;
  Picture *pic;
  int i;

  for(i = 0 ; i < bset->number ; i++) {
    b = bset->list[i];
    pic = &bullet_obj->pic[b->type];

    /* Erase the old picture of the bullet */
    if(status != ON) {
      if(b->life < weapon_stat[b->type].frames - 1 && b->life != -2)
	if(settings.point_bullets == TRUE && b->type != DISC)
	  draw_point(ANIM_WIN,b->old_loc->screen_x[term->num],
		     b->old_loc->screen_y[term->num],DRAW_XOR,
		     (b->owner != NULL)?b->owner->color:WHITE);
        else
	  draw_picture(ANIM_WIN,b->old_loc->screen_x[term->num],
		       b->old_loc->screen_y[term->num],
		       pic,DRAW_XOR,
		       (b->owner != NULL)?b->owner->color:WHITE);
    }

    /* Draw the new picture of the bullet */
    if(status != OFF) {
      if(b->life > 0 && b->life < weapon_stat[b->type].frames)
	if(settings.point_bullets == TRUE && b->type != DISC)
	  draw_point(ANIM_WIN,b->loc->screen_x[term->num],
		     b->loc->screen_y[term->num],DRAW_XOR,
		     (b->owner != NULL)?b->owner->color:WHITE);
        else
	  draw_picture(ANIM_WIN,b->loc->screen_x[term->num],
		       b->loc->screen_y[term->num],
		       pic,DRAW_XOR,
		       (b->owner != NULL)?b->owner->color:WHITE);
    }
  }
}

/*
** Displays all the explosions visible in the animation window.
*/
display_explosions(status)
     unsigned int status;
{
  extern Eset *eset;
  Exp *e;
  Picture *pic;
  int i;

  for(i = 0 ; i < eset->number ; i++) {
    e = eset->list[i];

    /* Compute the address of the old picture */
    pic = &e->obj->pic[e->obj->num_pics - e->life - 1];

    /* Erase the old picture of the explosion */
    if(status != ON)
      if(e->life < e->obj->num_pics)
	draw_picture(ANIM_WIN,e->old_screen_x[term->num],
		     e->old_screen_y[term->num],pic,DRAW_XOR,e->color);

    /* Draw the new picture of the explosion */
    if(status != OFF)
      if(e->life > 0) {
	pic++;
	draw_picture(ANIM_WIN,e->screen_x[term->num],
		     e->screen_y[term->num],pic,DRAW_XOR,e->color);
      }
  }
}

/* Draws a north wall */
#define draw_north_wall(b,x,y) \
  if(b->flags & NORTH_WALL) \
    draw_hor(ANIM_WIN,x,y,BOX_WIDTH,DRAW_XOR,WHITE)

/* Draws a west wall */
#define draw_west_wall(b,x,y) \
  if(b->flags & WEST_WALL) \
    draw_vert(ANIM_WIN,x,y,BOX_HEIGHT,DRAW_XOR,WHITE)

/* Draws fuel, ammo, armor, and goal in center, outpost wherever it is */
#define draw_type(b,line_x,line_y,fr) \
  switch(b->type) { \
    case FUEL: case AMMO: case ARMOR: case GOAL: \
      pic = &landmark_obj[0]->pic[b->type - 1]; \
      draw_picture(ANIM_WIN,line_x+BOX_WIDTH/2,line_y+BOX_HEIGHT/2, \
		   pic,DRAW_XOR,WHITE); \
      break; \
    case OUTPOST: \
      pic = &landmark_obj[0]->pic[b->type - 1]; \
      oc = outpost_coordinate(b,fr); \
      draw_picture(ANIM_WIN,line_x+oc->x,line_y+oc->y,pic,DRAW_XOR,WHITE); \
  }

/* Draws team character in the center of the box */
#define draw_team(b,x,y) \
  if(b->team != 0) { \
    buf[0] = team_char[b->team]; \
    draw_text(ANIM_WIN,x+BOX_WIDTH/2,y+BOX_HEIGHT/2-5, \
	      buf,S_FONT,DRAW_XOR,WHITE); \
  }

/*
** Displays all the walls and landmarks in the animation window.
*/
display_maze(status)
     unsigned int status;
{
  extern char team_char[];
  extern Object *landmark_obj[];
  extern Coord *outpost_coordinate();
  Coord *oc;
  Picture *pic;
  Intloc *sloc,*old_sloc;
  int right,bottom,i,j;
  int left_x,old_left_x,top_y,old_top_y;
  int line_x,old_line_x,line_y,old_line_y;
  int ox,oy;
  Box *b,*ob,temp;
  char buf[2];

  buf[1] = '\0';

  sloc = &term->loc;
  old_sloc = &term->old_loc;

  /*
  ** Draw just the north walls for the leftmost column of boxes
  ** Draw just the west walls for the topmost row of boxes
  ** except for upperleftmost one, which isn't done at all.
  ** For all the rest, do both the north and west walls.
  */
  switch(status) {
    case REDISPLAY:
      left_x = sloc->grid_x*BOX_WIDTH - sloc->x;
      old_left_x = old_sloc->grid_x*BOX_WIDTH - old_sloc->x;

      top_y = sloc->grid_y*BOX_HEIGHT - sloc->y;
      old_top_y = old_sloc->grid_y*BOX_HEIGHT - old_sloc->y;

      line_x = left_x;
      old_line_x = old_left_x;
      for(i = 0 ; i <= NUM_BOXES ; i++) {
	line_y = top_y;
	old_line_y = old_top_y;
	for(j = 0 ; j <= NUM_BOXES ; j++) {
	  b = &box[sloc->grid_x+i][sloc->grid_y+j];
	  ox = old_sloc->grid_x + i;
	  oy = old_sloc->grid_y + j;
	  ob = &box[ox][oy];

	  /* If the old box has been changed, get the old value */
	  if(ob->flags & BOX_CHANGED)
	    if(old_box(&temp,ox,oy)) ob = &temp;

	  /* Redisplay walls */
	  if(j) {
	    draw_north_wall(ob,old_line_x,old_line_y);
	    draw_north_wall(b,line_x,line_y);
	  }
	  if(i) {
	    draw_west_wall(ob,old_line_x,old_line_y);
	    draw_west_wall(b,line_x,line_y);
	  }

	  /* Redisplay type */
	  draw_type(ob,old_line_x,old_line_y,frame-1);
	  draw_type(b,line_x,line_y,frame)

	  /* Redisplay team */
	  draw_team(ob,old_line_x,old_line_y);
	  draw_team(b,line_x,line_y);

	  line_y += BOX_HEIGHT;
	  old_line_y += BOX_HEIGHT;
	}
	line_x += BOX_WIDTH;
	old_line_x += BOX_WIDTH;
      }
      break;
    case ON:
      right = sloc->grid_x + NUM_BOXES+1;
      bottom = sloc->grid_y + NUM_BOXES+1;
      
      left_x = sloc->grid_x*BOX_WIDTH - sloc->x;
      top_y = sloc->grid_y*BOX_HEIGHT - sloc->y;

      line_x = left_x;
      for(i = sloc->grid_x ; i < right ; i++) {
	line_y = top_y;
	for(j = sloc->grid_y ; j < bottom ; j++) {
	  b = &box[i][j];

	  /* Draw walls */
	  if(j != sloc->grid_y) draw_north_wall(b,line_x,line_y);
	  if(i != sloc->grid_x) draw_west_wall(b,line_x,line_y);

	  /* Draw type */
	  draw_type(b,line_x,line_y,frame)

	  /* Draw team */
	  draw_team(b,line_x,line_y);

	  line_y += BOX_HEIGHT;
	}
	line_x += BOX_WIDTH;
      }
      break;
    }
}

/*
** Displays mapper and radar in the map window.
*/
display_map(status)
     unsigned int status;
{
  unsigned int action;
  Vehicle *v;

  /* Check for being exposed */
  check_expose(MAP_WIN,status);

  if(status == ON)
    clear_window(MAP_WIN);

  if(settings.mode == BATTLE_MODE || settings.mode == DEMO_MODE) {
    /* Do the full mapper and radar */
    full_mapper(status);
    full_radar(status);
  }
  else {
    switch(status) {
      case REDISPLAY:
        action = SP_redisplay;
	break;
      case ON:
	action = SP_draw;
	break;
      case OFF:
	action = SP_erase;
	break;
      }

    v = term->vehicle;
    do_special(v,MAPPER,action);
    do_special(v,RADAR,action);
  }
}

/*
** Displays the console information in the console window.
*/
display_cons(status)
     unsigned int status;
{
  unsigned int action;

  /* Check for being exposed */
  check_expose(CONS_WIN,status);

  switch(status) {
    case REDISPLAY:
      action = SP_redisplay;
      break;
    case ON:
      clear_window(CONS_WIN);
      action = SP_draw;
      break;
    case OFF:
      action = SP_erase;
      break;
  }

  if(term->vehicle != (Vehicle *) NULL)
    do_special(term->vehicle,CONSOLE,action);
}

char *help_normal[8] = {
  "BASIC FUNCTIONS                                       DISC CONTROL         \
  GAME FUNCTIONS         3D FUNCTIONS            SYNC CONTROL",
  "space  fire weapons (left)     C   toggle console     s   spin <=          \
  Q   quit game          T   toggle 3D view      i   every frame",
  "t      turn turret (middle)    M   toggle mapper      d   spin toggle      \
  P   pause game         W   toggle wide         o   every 2 frames",
  "g      turn tank   (right)     R   toggle radar       f   spin =>          \
  <   slow down game     D   toggle distance     p   every 4 frames",
  "0-9    set forward drive       z   toggle safety      w   throw slow       \
  >   speed up game      E   toggle extend       [   every 8 frames",
  "-      reverse drive 9         c   stop               e   throw medium     \
                         L   toggle clipping     ]   every 16 frames",
  "!@#$%^ toggle weapons 1-6      v   speed up           r   throw fast",
  "return send message            x   slow down"
};

char *help_battle[8] = {
  "0-9     track vehicle",
  "button  move view",
  "space   pause game",
  "w       map battle windows",
  "W       unmap battle windows",
  "Q       quit",
  "",
  ""
};

/*
** Displays helpful information in the help window.
** If in battle or demo mode, the battle help is displayed.
*/
display_help(status)
     unsigned int status;
{
  int i;
  char **text;

  /* Check for being exposed */
  check_expose(HELP_WIN,status);

  if(status == ON) {
    clear_window(HELP_WIN);

    /* Determine which text to show in the help window */
    if(settings.mode == BATTLE_MODE || settings.mode == DEMO_MODE)
      text = help_battle;
    else
      text = help_normal;

    for(i = 0 ; i < 8 ; i++)
	display_mesg(HELP_WIN,text[i],i,S_FONT);
  }
}

#ifdef S1024x864
#define VEHICLE_X 0
#define VEHICLE_Y 0
#define VEHICLE_H 53
#define VEHICLE_W1 47
#define VEHICLE_W2 61

#define BULLET_X  180
#define BULLET_Y  0
#define BULLET_H  28

#define LAND_X    440
#define LAND_Y    0
#define LAND_H    30
#define LAND_W    85

#define EXP_X     0
#define EXP_Y     555
#define EXP_H     45
#define EXP_W     230

#define MSG_X     50
#define MSG_Y     46
#define MSG_FONT  M_FONT

#define PIC_X   30
#define PIC_Y   50
#define TEXT_OFFSET  50
#endif

/*
** Displays pictures of all bodies, bullets, explosions, and landmarks.
*/
display_pics()
{
  extern Object *vehicle_obj[], *bullet_obj, *exp_obj[], *landmark_obj[];
  extern int num_vehicle_objs,num_exp_objs;
  int view,orbit,max_pics,split,vheight;

  clear_window(ANIM_WIN);
  view = 0;
  orbit = 0;
  
  /* Draw the vehicles in one column */
  draw_text(ANIM_WIN,BULLET_X/2,VEHICLE_Y+5,"Vehicles",L_FONT,DRAW_COPY,WHITE);
  draw_objs(vehicle_obj,TRUE,0,num_vehicle_objs,view,VEHICLE_X,VEHICLE_Y,
	    VEHICLE_H);

  /* Draw the explosions in two columns */
  draw_text(ANIM_WIN,LAND_X/2,EXP_Y+5,
	    "Explosions",L_FONT,DRAW_COPY,WHITE);
  split = (num_exp_objs+1) >> 1;
  draw_objs(exp_obj,TRUE,0,split,view,EXP_X,EXP_Y,EXP_H);
  draw_objs(exp_obj,TRUE,split,num_exp_objs,view,EXP_X+EXP_W,EXP_Y,EXP_H);

  /* Draw the bullets in a column */
  draw_text(ANIM_WIN,(BULLET_X + LAND_X)/2,BULLET_Y+5,
	    "Bullets",L_FONT,DRAW_COPY,WHITE);
  draw_obj(bullet_obj,0,BULLET_X,BULLET_Y,BULLET_H);

  /* Draw the landmarks in three columns */
  draw_text(ANIM_WIN,(LAND_X + ANIM_WIN_WIDTH)/2,LAND_Y+5,
	    "Landmarks",L_FONT,DRAW_COPY,WHITE);
  draw_text(ANIM_WIN,LAND_X + PIC_X + LAND_W,LAND_Y+38,
	    "Game      Map   Design",M_FONT,DRAW_COPY,WHITE);
  draw_obj(landmark_obj[0],1,LAND_X,LAND_Y+LAND_H,LAND_H);
  draw_obj(landmark_obj[1],2,LAND_X+LAND_W,LAND_Y+LAND_H,LAND_H);
  draw_obj(landmark_obj[2],3,LAND_X+2*LAND_W,LAND_Y + 3,LAND_H);

  /* Put in separator rectangles between the pictures */
  draw_filled_rect(ANIM_WIN,BULLET_X-1,0,3,EXP_Y,DRAW_COPY,WHITE);
  draw_filled_rect(ANIM_WIN,LAND_X-1,0,3,ANIM_WIN_HEIGHT,DRAW_COPY,WHITE);
  draw_filled_rect(ANIM_WIN,EXP_X,EXP_Y-1,ANIM_WIN_WIDTH,3,DRAW_COPY,WHITE);

  display_mesg2(ANIM_WIN,"Hit any key or button to continue",
		MSG_X,MSG_Y,MSG_FONT);

  /* Animate the explosions and vehicles until a key or button is pressed */
  max_pics = vehicle_obj[0]->num_pics;
  vheight = num_vehicle_objs/2*VEHICLE_H;
  while(!scan_input()) {
    if(++view >= max_pics) {
      view = 0;
      orbit++;
    }

    /* Rotate the vehicles around once every 16 orbits */
    if((orbit & 0xf) == 0) {
      /* Draw vehicles in 2 sets to reduce flicker */
      draw_filled_rect(ANIM_WIN,VEHICLE_X+PIC_X-VEHICLE_W1/2,VEHICLE_Y+PIC_Y
		       -VEHICLE_H/2,VEHICLE_W1,vheight,DRAW_COPY,BLACK);
      draw_objs(vehicle_obj,FALSE,0,num_vehicle_objs/2,view,VEHICLE_X,
		VEHICLE_Y,VEHICLE_H);
      draw_filled_rect(ANIM_WIN,VEHICLE_X+PIC_X-VEHICLE_W2/2,
		       VEHICLE_Y+PIC_Y-VEHICLE_H/2+vheight,
		       VEHICLE_W2,vheight,DRAW_COPY,BLACK);
      draw_objs(vehicle_obj,FALSE,num_vehicle_objs/2,num_vehicle_objs,view,
		VEHICLE_X,VEHICLE_Y+vheight,VEHICLE_H);
    }

    draw_filled_rect(ANIM_WIN,EXP_X+PIC_X-EXP_H/2,EXP_Y+PIC_Y-EXP_H/2,
		     EXP_H,split*EXP_H,DRAW_COPY,BLACK);
    draw_objs(exp_obj,FALSE,0,split,view,EXP_X,EXP_Y,EXP_H);
    draw_filled_rect(ANIM_WIN,EXP_X+PIC_X-EXP_H/2+EXP_W,EXP_Y+PIC_Y-EXP_H/2,
		     EXP_H,(num_exp_objs-split)*EXP_H,DRAW_COPY,BLACK);
    draw_objs(exp_obj,FALSE,split,num_exp_objs,view,EXP_X+EXP_W,EXP_Y,EXP_H);
    sync_output(FALSE);
  }
}

/*
** Draws all the objects in the array in a vertical column, starting
** at the specified location and working downwards in jumps of height.
** The specified view is used for each object, provided it exists.
*/
draw_objs(obj,text,first,last,view,x,y,height)
     Object *obj[];
     Boolean text;
     int first,last,view,x,y,height;
{
  int i;

  for(i = first ; i < last ; i++)
    if(view < obj[i]->num_pics)
      draw_picture_string(obj[i],view,(text ? obj[i]->type : ""),x+PIC_X,
			  y+PIC_Y+height*(i-first),0);
}

/*
** Draws all the views of a given object in a vertical column, starting
** at the specified location and working downwards in jumps of height.
*/
draw_obj(obj,type,x,y,height)
     Object *obj;
     int type,x,y,height;
{
  extern Weapon_stat weapon_stat[];
  extern Object *bullet_obj;
  static char *box_type_name[] = {
    "","fuel","ammo","armor","goal","outpost","scroll",
    "","","","","","","","slip","slow","start" };
  char *str;
  int adj,i;

  for(i = 0 ; i < obj->num_pics ; i++) {
    adj = 0;
    str = "";
    if(type == 0) str = weapon_stat[i].type;
    else if(type == 2) adj = -4;
    else if(type == 3) {
      /* Skip drawing the empty pixmap for normal landmark */
      if(i == 0) continue;
      str = box_type_name[i];
      adj = -13;
    }
    draw_picture_string(obj,i,str,x+PIC_X,y+PIC_Y+height*i,adj);
  }
}

/*
** Draws the specified view of the object with the string written beneath
** at the specified location in the animation window.  The adj parameter
** is added to the picture coordinates but not the text coordinates.
*/
draw_picture_string(obj,view,str,x,y,adj)
     Object *obj;
     char *str;
     int view,x,y,adj;
{
  Picture *pic;

  pic = &obj->pic[view];

  draw_picture(ANIM_WIN,x+adj,y+adj,pic,DRAW_COPY,WHITE);
  if(str[0] != '\0')
    draw_text(ANIM_WIN,x + TEXT_OFFSET + font_string_width(str,M_FONT)/2,
	      y - font_height(M_FONT)/2,str,M_FONT,DRAW_COPY,WHITE);
}
