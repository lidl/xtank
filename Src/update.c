/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** update.c
*/

#include "xtank.h"
#include "disc.h"
#include "vehicle.h"
#include "loc.h"
#include "vstructs.h"

extern int num_vehicles;
extern Vehicle *vehicle[];
extern Eset *eset;
extern Bset *bset;
extern Weapon_stat weapon_stat[];

#ifdef AMIGA
/*
** Makes loc = old_loc + (dx,dy)
*/
update_loc(old_loc,loc,dx,dy)
   Loc *old_loc,*loc;
   int dx,dy;  
{
  do {  
    (loc)->x = (old_loc)->x + dx;  
    (loc)->y = (old_loc)->y + dy;  
 
    (loc)->box_x = (old_loc)->box_x + dx;  
    (loc)->box_y = (old_loc)->box_y + dy;  
 
    if((loc)->box_x >= BOX_WIDTH) {  
      (loc)->box_x -= BOX_WIDTH;  
      (loc)->grid_x = (old_loc)->grid_x + 1;  
    }  
    else if((loc)->box_x < 0) {  
      (loc)->box_x += BOX_WIDTH;  
      (loc)->grid_x = (old_loc)->grid_x - 1;  
    }  
    else (loc)->grid_x = (old_loc)->grid_x;  
 
    if((loc)->box_y >= BOX_HEIGHT) {  
      (loc)->box_y -= BOX_HEIGHT;  
      (loc)->grid_y = (old_loc)->grid_y + 1;  
    }  
    else if((loc)->box_y < 0) {  
      (loc)->box_y += BOX_HEIGHT;  
      (loc)->grid_y = (old_loc)->grid_y - 1;  
    }  
    else (loc)->grid_y = (old_loc)->grid_y;  
  } while(0);
}
#endif AMIGA

/*
** Computes the state of the specified vehicle for one frame.
*/
update_vehicle(v)
     Vehicle *v;
{
  Loc *loc,*old_loc;
  Vector *vector;
  Box *b;
  float xadj,yadj;
  int i;

  vector = &v->vector;

  /* Decrement all weapon reload counters */
  if(v->num_weapons > 0)
    for(i = 0 ; i < v->num_weapons ; i++)
      if(v->weapon[i].reload_counter > 0)
	v->weapon[i].reload_counter--;

  /* Decrement fuel if the settings allow it*/
  if(v->fuel > 0) {
    if(!settings.no_wear)
      v->fuel -= (vector->drive * vector->drive) * FUEL_CONSUME;
  }
  else
    v->fuel = 0;

  /* Decrement heat by heat_sinks every five frames */
  if((frame % 5) == 0) {
    v->heat -= v->vdesc->heat_sinks + 1;
    if(v->heat < 0) v->heat = 0;
  }

  /* Stop vehicle from sliding every 16 frames */
  if((v->status & VS_sliding) && (frame & 0xf) == 0)
    v->status &= ~VS_sliding;

  /* Update vector */
  update_vector(v);

  /* Get pointer to box vehicle is in */
  b = &box[v->loc->grid_x][v->loc->grid_y];

  /* Handle interesting box types after xspeed and yspeed calculation */
  box_type_check(v,b,&xadj,&yadj);

  /* Update location */
  loc = v->old_loc;
  v->old_loc = old_loc = v->loc;
  v->loc = loc;

  update_loc(old_loc,loc,vector->xspeed+xadj,vector->yspeed+yadj);

  /* Update turrets */
  if(v->num_turrets > 0)
    for(i = 0 ; i < v->num_turrets ; i++)
      update_turret(&v->turret[i]);
}

/*
** Updates the vector for the vehicle.
*/
update_vector(v)
     Vehicle *v;
{
  float xspeed, yspeed, xdrive, ydrive, xaccel, yaccel;
  float delta_heading, friction, max_diff, accel, ratio;
  Box *b;
  Vector *vector;

  vector = &v->vector;

  vector->old_angle = vector->angle;

  /* Update heading */
  if(v->fuel > 0) {
    if(v->safety == TRUE)
      delta_heading = v->turn_rate[abs((int) vector->speed)];
    else
      delta_heading = v->turn_rate[0];
  }
  else
    delta_heading = 0;

  switch(vector->heading_flag) {
    case CLOCKWISE:
      if((vector->heading += delta_heading) >= vector->desired_heading) {
	vector->heading = vector->desired_heading;
	vector->heading_flag = NO_ROTATION;
      }
      break;
    case COUNTERCLOCKWISE:
      if((vector->heading -= delta_heading) <= vector->desired_heading) {
	vector->heading = vector->desired_heading;
	vector->heading_flag = NO_ROTATION;
      }
      break;
    }

  /* Set drive to desired drive */
  if(v->fuel > 0)
    vector->drive = vector->desired_drive;
  else
    vector->drive = 0.0;

  /* If vehicle is not sliding, allow it to accelerate */
  if(!(v->status & VS_sliding)) {
    /* Break (heading,drive) & (angle, speed) into x & y components */
    xspeed = cos(vector->angle) * vector->speed;
    yspeed = sin(vector->angle) * vector->speed;
    xdrive = cos(vector->heading) * vector->drive;
    ydrive = sin(vector->heading) * vector->drive;

    /* If the vehicle owns discs, decrease the drive appropriately */
    if(v->num_discs > 0) {
      xdrive *= settings.disc_slow;
      ydrive *= settings.disc_slow;
    }

    /* Compute acceleration needed to make speed match drive */
    xaccel = xdrive - xspeed;
    yaccel = ydrive - yspeed;

    /* Get pointer to box vehicle is in */
    b = &box[v->loc->grid_x][v->loc->grid_y];

    /* Amount of friction depends on box type */
    friction = (b->type==SLIP)?settings.slip_friction:settings.normal_friction;

    /* If acc isn't in same direction as motion, then you're braking */
    max_diff = ((xspeed*xdrive+yspeed*ydrive<=0) ? BRAKING_ACC : v->vdesc->acc)
               * friction;

    /* Make certain you're not accelerating by more than allowed amount */
    accel = sqrt(xaccel*xaccel + yaccel*yaccel);
    if (accel > max_diff) {
      ratio = max_diff/accel;
      xaccel *= ratio;
      yaccel *= ratio;
    }
  
    /* Compute new angle and speed */
    yspeed += yaccel;
    xspeed += xaccel;
    assign_speed(vector,xspeed,yspeed);
  }
}

/*
** Computes the rotation (0 to 16) from the heading (-PI to PI).
*/
update_rotation(v)
     Vehicle *v;
{
  int views;

  views = v->obj->num_pics;
  v->vector.old_rot = v->vector.rot;
  v->vector.rot = ((int) ((v->vector.heading)/(2*PI)*views+views+.5))%views;
}

update_turret(t)
     Turret *t;
{
  float delta_angle;
  int views;
  Boolean angle_changed = TRUE;

  t->old_rot = t->rot;
  delta_angle = t->turn_rate;
  switch(t->angle_flag) {
    case NO_ROTATION:
      angle_changed = FALSE;
      break;
    case CLOCKWISE:
      if((t->angle += delta_angle) >= t->desired_angle) {
	t->angle = t->desired_angle;
	t->angle_flag = NO_ROTATION;
      }
      break;
    case COUNTERCLOCKWISE:
      if((t->angle -= delta_angle) <= t->desired_angle) {
	t->angle = t->desired_angle;
	t->angle_flag = NO_ROTATION;
      }
      break;
    }
  if(angle_changed == TRUE) {
    views = t->obj->num_pics;
    t->rot = ((int) ((t->angle)/(2*PI)*views + views + .5))%views;
  }
}

/*
** Computes new locations for all the bullets, and removes dead ones.
*/
update_bullets()
{
  Loc *loc,*old_loc;
  Bullet *b;
  int i;

  for(i = 0 ; i < bset->number ; i++) {
    b = bset->list[i];

    /* decrement life and see if it's dead */
    while(--b->life < 0) {
      /* if not last on list fill up hole, otherwise, do next bullet */
      if(i != --bset->number) {
	bset->list[i] = bset->list[bset->number];
	bset->list[bset->number] = b;
	b = bset->list[i];
      }
      else
	break;
    }

    /* If it is a mine, seeker, slick, or disc run the special code for it */
    switch(b->type) {
      case MINE:
      case SLICK:  update_mine(b); break;
      case SEEKER: update_seeker(b); break;
      case DISC:   update_disc(b); break;
    }

    /* Update the bullet location */
    loc = b->old_loc;
    old_loc = b->old_loc = b->loc;
    b->loc = loc;
    
    update_loc(old_loc,loc,b->xspeed,b->yspeed);
  }
}

/*
** Stops the bullet after 5 frames of movement and lets it hurt its owner.
*/
update_mine(b)
     Bullet *b;
{
  if(b->life == weapon_stat[b->type].frames - 5) {
    b->xspeed = b->yspeed = 0.0;
    b->hurt_owner = TRUE;
  }
}

#define SEEKER_ACC 2

/*
** Looks for heat sources and moves towards them, leaving a trail of exhaust.
*/
update_seeker(b)
     Bullet *b;
{
  Loc *loc;
  float acc,sp,sp2,axs,ays,xdir,ydir;
  int dx,dy,seek,best_dx,best_dy,best_seek,best_heat,i;

  /* Make a trail of exhaust */
  make_explosion(b->loc,EXP_EXHAUST);

  /* Find all vehicles that would affect heat seeking */
  best_seek = 0;
  for(i = 0 ; i < num_vehicles ; i++) {
    /*
    ** Is vehicle within 3 boxes, in line of sight, in front of the seeker and
    ** hotter/closer than the previous targets?
    */
    loc = vehicle[i]->loc;
    dx = (int) (loc->x - b->loc->x);
    dy = (int) (loc->y - b->loc->y);
    
    seek = (50 + vehicle[i]->heat) * (BOX_WIDTH*BOX_WIDTH*9 - (dx*dx + dy*dy));
    if(seek > best_seek &&
       (dx * b->xspeed + dy * b->yspeed > 0) &&
       !intersect_wall(b->loc,loc)) {
      best_seek = seek;
      best_heat = vehicle[i]->heat;
      best_dx = dx;
      best_dy = dy;
    }
  }

  /* If we found something to seek, adjust speed components to follow it */
  if(best_seek > 0 && (rnd(30) < best_heat)) {
    sp = weapon_stat[b->type].ammo_speed;
    sp2 = sp * sp;
    xdir = ((b->xspeed > 0) ? 1 : -1);
    ydir = ((b->yspeed > 0) ? 1 : -1);
    axs = abs(b->xspeed);
    ays = abs(b->yspeed);
    if(b->xspeed * best_dy < b->yspeed * best_dx)
      acc = SEEKER_ACC;
    else
      acc = -SEEKER_ACC;

    if(axs > ays) {
      b->yspeed -= xdir * acc;
      if(abs(b->yspeed) >= sp) {
	b->xspeed = 0;
	b->yspeed = ydir * sp;
      }
      else
	b->xspeed = xdir * sqrt(sp2 - b->yspeed * b->yspeed);
    }
    else {
      b->xspeed += ydir * acc;
      if(abs(b->xspeed) >= sp) {
	b->yspeed = 0;
	b->xspeed = xdir * sp;
      }
      else
	b->yspeed = ydir * sqrt(sp2 - b->xspeed * b->xspeed);
    }
  }
}

/*
** Applies friction to disc in free flight, and computes its orbit when owned.
*/
update_disc(b)
     Bullet *b;
{
  float dx,dy;
  float dist;
  float angle,delta;

  /* If the disc is owned by someone, change its velocity to orbit him */
  if(b->owner != (Vehicle *) NULL) {
    /* compute the angle to the vehicle */
    dx = b->owner->loc->x - b->loc->x;
    dy = b->owner->loc->y - b->loc->y;
    angle = atan2(dy,dx);
    dist = dx*dx + dy*dy;
    
    /* Compute a delta which will bring us into orbit around our owner */
    if(dist <= DISC_ORBIT_SQ)
      delta = PI/2 * (2 - (dist / DISC_ORBIT_SQ));
    else
      delta = PI/2 * (DISC_ORBIT_SQ / dist);

    /* If disc_spin is set, orbit counterclockwise, otherwise, clockwise */
    if(b->owner->status & VS_disc_spin) angle += delta;
    else angle -= delta;

    /* Compute new xspeed and yspeed */
    b->xspeed = DISC_MED_SPEED * cos(angle);
    b->yspeed = DISC_MED_SPEED * sin(angle);
  }
  /* otherwise slow the disc down a bit */
  else {
    b->xspeed *= settings.disc_friction;
    b->yspeed *= settings.disc_friction;
  }
}

/*
** Decrements explosion lives and removes dead explosions from the set.
*/
update_explosions()
{
  Exp *e;
  int i;

  for(i = 0 ; i < eset->number ; i++) {
    e = eset->list[i];
    /* Decrement life to see if it's dead */
    while(!(e->life--)) {
      /* If not last one, move it */
      if(i != --eset->number) {
	eset->list[i] = eset->list[eset->number];
	eset->list[eset->number] = e;
	e = eset->list[i];
      }
    }
  }
}

/*
** Moves vehicle maze flags around when vehicles move from box to box.
*/
update_maze_flags()
{
  Vehicle *v;
  Loc *loc,*old_loc;
  int i;

  for(i = 0 ; i < num_vehicles ; i++) {
    v = vehicle[i];

    /* don't do anything if the vehicle isn't alive */
    if(!(v->status & VS_is_alive)) continue;

    loc = v->loc;
    old_loc = v->old_loc;
    if((loc->grid_x != old_loc->grid_x) || (loc->grid_y != old_loc->grid_y)) {
      box[old_loc->grid_x][old_loc->grid_y].flags &= ~v->flag;
      box[loc->grid_x][loc->grid_y].flags |= v->flag;
    }
  }
}

/*
** Updates all specials for all vehicles.
*/
update_specials()
{
  Vehicle *v;
  int i,num;

  for(i = 0 ; i < num_vehicles ; i++) {
    v = vehicle[i];
    for(num = 0 ; num < MAX_SPECIALS ; num++)
      do_special(v,num,SP_update);
  }
}

/*
** Updates the screen locations of the terminal, vehicles, bullets,
** and explosions for the current terminal.
*/
update_screen_locs()
{
  extern Terminal *term;
  Vehicle *v;
  Exp *e;
  Loc *bloc;
  int sx,sy;
  int i;

  /* If terminal is tracking a vehicle, compute screen loc from vehicle loc */
  v = term->vehicle;
  if(v != (Vehicle *) NULL) {
    /* Compute the loc of the upper left corner of the animation window */
    term->loc.grid_x = v->loc->grid_x - NUM_BOXES/2;
    term->loc.grid_y = v->loc->grid_y - NUM_BOXES/2;
    term->loc.x = v->loc->x - NUM_BOXES*BOX_WIDTH/2;
    term->loc.y = v->loc->y - NUM_BOXES*BOX_HEIGHT/2;
  }

  /* Terminal screen offset */
  sx = term->loc.x;
  sy = term->loc.y;

  /* Compute screen coordinates for the vehicles */
  for(i = 0 ; i < num_vehicles ; i++) {
    v = vehicle[i];
    v->loc->screen_x[term->num] = v->loc->x - sx;
    v->loc->screen_y[term->num] = v->loc->y - sy;
  }

  /* Compute the screen coordinates for the bullets */
  for(i = 0 ; i < bset->number ; i++) {
    bloc = bset->list[i]->loc;
    bloc->screen_x[term->num] = bloc->x - sx;
    bloc->screen_y[term->num] = bloc->y - sy;
  }

  /* Compute the screen coordinates for the explosions */
  for(i = 0 ; i < eset->number ; i++) {
    e = eset->list[i];
    e->old_screen_x[term->num] = e->screen_x[term->num];
    e->old_screen_y[term->num] = e->screen_y[term->num];
    e->screen_x[term->num] = e->x - sx;
    e->screen_y[term->num] = e->y - sy;
  }
}
