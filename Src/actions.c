/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** actions.c
*/

#include "xtank.h"
#include "disc.h"
#include "gr.h"
#include "vstructs.h"

Weapon_stat weapon_stat[];

/*
** Adjusts the specified location by dx in the x direction, and
** dy in the y direction.
*/
adjust_loc(loc,dx,dy)
     Loc *loc;
     int dx,dy;
{
  /* adjust x coordinate */
  loc->x += dx;
  if((loc->box_x += dx) < 0)
    do {
      loc->grid_x--;
    } while((loc->box_x += BOX_WIDTH) < 0);
  else if(loc->box_x >= BOX_WIDTH)
    do {
      loc->grid_x++;
    } while((loc->box_x -= BOX_WIDTH) >= BOX_WIDTH);

  /* adjust y coordinate */
  loc->y += dy;
  if((loc->box_y += dy) < 0)
    do {
      loc->grid_y--;
    } while((loc->box_y += BOX_HEIGHT) < 0);
  else if(loc->box_y >= BOX_HEIGHT)
    do {
      loc->grid_y++;
    } while((loc->box_y -= BOX_HEIGHT) >= BOX_HEIGHT);
}

/*
** Creates a bullet from the specified weapon type at the given location,
** owned by the vehicle v, moving at the specified angle.
*/
make_bullet(v,loc,type,angle)
     Vehicle *v;
     Loc *loc;
     int type;
     float angle;
{
  Weapon_stat *ws;
  extern Bset *bset;
  Bullet *b;

  if(bset->number >= MAX_BULLETS) return; /* too many bullets */

  /* Initialize the bullet's owner and location */
  b = bset->list[bset->number++];
  b->owner = v;
  b->loc1 = b->loc2 = *loc;
  b->loc = &b->loc1;
  b->old_loc = &b->loc2;

  /* Find the weapon_stat structure */
  ws = &weapon_stat[type];

  /* Compute the x and y components of the bullet's speed */
  b->xspeed = (float) ws->ammo_speed * cos(angle);
  b->yspeed = (float) ws->ammo_speed * sin(angle);

  /* Add the motion of the owner (if necessary) */
  if(settings.rel_shoot == TRUE && v != NULL) {
    b->xspeed += v->vector.xspeed;
    b->yspeed += v->vector.yspeed;
  }

  b->type = type;
  b->life = ws->frames;
  b->hurt_owner = FALSE;
}

/*
** Destroys bullet b, and creates an explosion in its place.
** The type of the explosion is dependent on the damage.
*/
explode(b,damage)
     Bullet *b;
     int damage;
{
  unsigned int type;

  /* Determine type of explosion from the amount of damage the bullet did */
  type = (damage <= 4) ? (EXP_DAM0 + damage) : EXP_DAM4;
  make_explosion(b->loc,type);

  /*
  ** I think this makes bullets that explode before they are displayed not
  ** not get erased, but I'm not sure... (sigh)
  */

  if(b->life == weapon_stat[b->type].frames - 1)
    b->life = -2; /* undisplayed bullet */
  else
    b->life = -1;
}

/*
** Creates an explosion of the specified type at the specified location.
*/
make_explosion(loc,type)
     Loc *loc;
     unsigned int type;
{
  extern int num_terminals;
  extern Object *exp_obj[];
  extern Eset *eset;
  Exp *e;
  int tnum;
  
  if(eset->number >= MAX_EXPS) return;
  e = eset->list[eset->number++];
  e->x = (int) loc->x;
  e->y = (int) loc->y;
  e->z = (int) loc->z;
  for(tnum = 0 ; tnum < num_terminals ; tnum++) {
    e->old_screen_x[tnum] = e->screen_x[tnum] = loc->screen_x[tnum];
    e->old_screen_y[tnum] = e->screen_y[tnum] = loc->screen_y[tnum];
  }
  e->obj = exp_obj[type];
  e->life = e->obj->num_pics + 1;
  e->color = WHITE;
}

/*
** Does the specified action for the specified special in the
** specified vehicle.
*/
do_special(v,special_num,action)
     Vehicle *v;
     int special_num;
     unsigned int action;
{
  Special *s;

  /* First make sure the special is really there */
  s = &v->special[special_num];
  if(s->status == SP_nonexistent) return;

  switch(action) {
    case SP_update:
    case SP_redisplay:
    case SP_draw:
    case SP_erase:
      if(s->status == SP_on) (*s->proc)(v,s->record,action);
      break;
    case SP_activate:
      if(s->status == SP_off) {
	s->status = SP_on;
	(*s->proc)(v,s->record,SP_activate);
      }
      break;
    case SP_deactivate:
      if(s->status == SP_on) {
	s->status = SP_off;
	(*s->proc)(v,s->record,SP_deactivate);
      }
      break;
    case SP_toggle:
      if(s->status == SP_off) {
	s->status = SP_on;
	(*s->proc)(v,s->record,SP_activate);
	(*s->proc)(v,s->record,SP_draw);
      }
      else if(s->status == SP_on) {
	s->status = SP_off;
	(*s->proc)(v,s->record,SP_deactivate);
	(*s->proc)(v,s->record,SP_erase);
      }
      break;
    case SP_break:
      if(s->status == SP_on) {
	s->status = SP_off;
	(*s->proc)(v,s->record,SP_deactivate);
	(*s->proc)(v,s->record,SP_erase);
      }
      s->status = SP_broken;
      break;
    case SP_repair:
      s->status = SP_on;
      (*s->proc)(v,s->record,SP_activate);
      (*s->proc)(v,s->record,SP_draw);
      break;
    }
}

/*
** Moves the view on the current terminal by dx vertically and dy horizontally.
** The terminal stops tracking.
*/
move_view(dx,dy)
     int dx,dy;
{
  Intloc new;

  /* switch view to a non-existent person */
  switch_view(-1);

  /* compute the new viewing location */
  new.x = term->loc.x + dx;
  new.y = term->loc.y + dy;

  new.grid_x = new.x / BOX_WIDTH;
  new.grid_y = new.y / BOX_HEIGHT;

  /* If the new location is in the grid, move the view there */
  if(new.grid_x >= 0 && new.grid_x < GRID_WIDTH - 4 &&
     new.grid_y >= 0 && new.grid_y < GRID_HEIGHT - 4)
    term->loc = new;
}

/*
** Switches the view on the current terminal to the perspective of
** the vehicle numbered num.  If num is not a legal vehicle number,
** the terminal stops tracking.
**
** Should check for access_on flag before allowing the switch.
*/
switch_view(num)
     int num;
{
  extern int num_vehicles;
  extern Vehicle *vehicle[];
  Vehicle *old_vehicle;
  int i;

  if(num >= 0 && num < num_vehicles) {
    /* remember who we were tracking */
    old_vehicle = term->vehicle;

    /* find the owner of the vehicle with that number */
    for(i = 0 ; i < num_vehicles ; i++)
      if(vehicle[i]->number == num) {
	term->vehicle = vehicle[i];
	break;
      }

    /*
    ** If we didn't find the vehicle number, or found the one that
    ** we used to be tracking, don't expose the windows
    */
    if(old_vehicle == term->vehicle) return;
  }
  else {
    /* switch view to a NULL vehicle to stop tracking */
    term->vehicle = (Vehicle *) NULL;
  }

  /* Expose all the windows which need to be redisplayed after the switch */
  expose_win(ANIM_WIN,TRUE);
  expose_win(CONS_WIN,TRUE);
  if(settings.mode != BATTLE_MODE && settings.mode != DEMO_MODE)
    expose_win(MAP_WIN,TRUE);
}

/*
** Disc functions
*/

/*
** Releases all discs owned by a vehicle.  Sets speed of discs to speed.
** Allows one update if delay is set.  This allows robots to throw to
** an accuracy of one frame.
*/
release_discs(v,speed,delay)
     Vehicle *v;
     float speed;
     Boolean delay;
{
  extern Bset *bset;
  Bullet *b;
  float ratio;
  int i;

  if(v->num_discs > 0) {
    /* Disc currently moving at DISC_MED_SPEED, so compute ratio */
    ratio = speed / DISC_MED_SPEED;

    for(i = 0 ; i < bset->number ; i++) {
      b = bset->list[i];

      if(b->type == DISC && b->owner == v) {
	if(delay) update_disc(b);
	b->xspeed *= ratio;
	b->yspeed *= ratio;
	b->owner = (Vehicle *) NULL;

	/* Inform the commentator about the throw */
	if(settings.commentator)
	  comment(COS_OWNER_CHANGE,COS_IGNORE,(Vehicle *) NULL,
		  (Vehicle *) NULL);
      }
    }
    v->num_discs = 0;
  }
}

/*
** Makes all discs owned by the vehicle spin in the specified direction.
*/
set_disc_orbit(v,dir)
     Vehicle *v;
     int dir;
{
  switch(dir) {
    case CLOCKWISE:        v->status &= ~VS_disc_spin; break;
    case COUNTERCLOCKWISE: v->status |= VS_disc_spin; break;
    case TOGGLE:           v->status ^= VS_disc_spin; break;
    }
}

/*
** Sets the owner of the specified disc to the specified vehicle.
*/
set_disc_owner(b,v)
     Bullet *b;
     Vehicle *v;
{
  /* Take it away from previous owner */
  if(b->owner != (Vehicle *) NULL) b->owner->num_discs--;

  /* Give it to new owner */
  b->owner = v;
  if(b->owner != (Vehicle *) NULL) b->owner->num_discs++;
}

/*
** Game speed control functions
*/

#define PAUSE_X ANIM_WIN_WIDTH/2
#define PAUSE_Y 10

/*
** Xors the pause message onto the animation window.
*/
display_pause_message()
{
  draw_text(ANIM_WIN,PAUSE_X,PAUSE_Y,
	    "Game paused -- RETURN to unpause  SPACE to single step",
	    M_FONT,DRAW_XOR,WHITE);
}
    
/*
** If state is TRUE, sets game_speed to 0 to pause.
** If state is FALSE, restores game_speed to old value.
*/
pause_game(state)
     Boolean state;
{
  static int old_speed;

  if(state == TRUE) {
    /* If not already paused, set game_speed to 0 */
    if(settings.game_speed != 0) {
      display_pause_message();
      old_speed = settings.game_speed;
      settings.game_speed = 0;
    }
  }    
  else {
    /* If paused, restore old game_speed */
    if(settings.game_speed == 0) {
      display_pause_message();
      settings.game_speed = old_speed;
    }
  }
}

/*
** Sets game speed to specified value if it is reasonable.
*/
set_game_speed(spd)
     int spd;
{
  if(spd > 0 && spd <= MAX_GAME_SPEED) settings.game_speed = spd;
}

/*
** Deals with game pause.  Waits if game is running faster than it should.
*/
check_game_speed()
{
  char reply;

  if(settings.game_speed == MAX_GAME_SPEED) return;
  if(settings.game_speed == 0) {
    /* Game is paused, so wait for a space bar or return */
    do {
      reply = get_reply();
    } while(reply != ' ' && reply != '\r');

    /* Unpause game on a return, single step on a space bar */
    if(reply == '\r') pause_game(FALSE);
  }
}
