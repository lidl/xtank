/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** hit.c
*/

#include "xtank.h"
#include "loc.h"
#include "disc.h"
#include "vstructs.h"

extern Bumper_stat bumper_stat[];
extern Body_stat body_stat[];
extern Weapon_stat weapon_stat[];

#ifdef AMIGA
double temp_hitc;
#define drem(a,b) ( (temp_hitc = fmod(a,b)) > b/2 ? temp_hitc-b : temp_hitc )
#endif AMIGA

/*
** Bounces the vehicles off each other and damages them both.
** Elasticity of bounce determined from bumper elasticities of vehicles.
** Damage based on kinetic energy and elasticity of collision.
** Better bumpers result in less damage in general and less to that vehicle.
*/
vehicle_hit_vehicle(v1,v2)
     Vehicle *v1,*v2;
{
  float ang,bump1,bump2,elast;
  int dx,dy,damage;

  /* Compute delta position */
  dx = v2->loc->x - v1->loc->x;
  dy = v2->loc->y - v1->loc->y;

  /* Compute the elasticity of the collision, based on bumpers */
  bump1 = bumper_stat[v1->vdesc->bumpers].elasticity;
  bump2 = bumper_stat[v2->vdesc->bumpers].elasticity;
  elast = .5 + bump1 + bump2;

  /* Bounce the vehicles off each other */
  bounce_vehicles(v1,v2,dx,dy,elast);

  /* Damage vehicle on the side (height = 0) */
  damage = bounce_damage(v1->vector.xspeed - v2->vector.xspeed,
			 v1->vector.yspeed - v2->vector.yspeed,elast);
  ang = atan2((double) dy, (double) dx);
  damage_vehicle(v1,v2,(int) (damage * (1 - bump1)),ang,0);
  damage_vehicle(v2,v1,(int) (damage * (1 - bump2)),PI - ang,0);

  if(settings.commentator)
    comment(COS_BIG_SMASH,damage*3,v1,v2);
}

/*
** Bounces vehicle off of wall and damages them both.
** Damage based on kinetic energy and elasticity of collision.
** Better bumpers result in less damage in general and less to the vehicle.
** Wall in box (grid_x,grid_y) at direction dir.
*/
vehicle_hit_wall(v,grid_x,grid_y,dir)
     Vehicle *v;
     int grid_x,grid_y,dir;
{
  float ang,bump,elast;
  int dx,dy,damage;

  /* Compute elasticity of collision based on bumpers */
  bump = bumper_stat[v->vdesc->bumpers].elasticity;
  elast = .5 + bump;

  /* Compute relative angle and location of the wall */
  switch(dir) {
    case NORTH: ang = -PI/2; dx = 0;  dy = -1; break;
    case SOUTH: ang = PI/2;  dx = 0;  dy = 1;  break;
    case WEST:  ang = PI;    dx = -1; dy = 0;  break;
    case EAST:  ang = 0;     dx = 1;  dy = 0;  break;
    }

  /* Bounce vehicle off wall */
  bounce_vehicle_wall(v,dx,dy,elast);

  /* Damage vehicle and wall based on proper component of velocity */
  damage = bounce_damage(dx * v->vector.xspeed,dy * v->vector.yspeed,elast);
  damage_vehicle(v,(Vehicle *) NULL,(int) (damage * (1 - bump)),ang,0);
  damage_wall(grid_x,grid_y,dir,damage);
}

/*
** Returns damage prop to kinetic energy, inversely prop to elasticity.
*/
bounce_damage(xspeed,yspeed,elast)
     float xspeed,yspeed,elast;
{
  return (int) ((xspeed*xspeed + yspeed*yspeed) / (elast * 40.0));
}

/*
** Determines the result of bullet b hitting vehicle v at relative
** location (dx,dy).
**
** If the bullet is a disk, then the vehicle is made owner of the disc.
** Otherwise, the bullet explodes, and the vehicle is damaged.
*/
bul_hit_vehicle(v,b,dx,dy)
     Vehicle *v;
     Bullet *b;
     int dx,dy;
{
  float angle;
  int damage,height;

  switch(b->type) {
    case DISC:
      set_disc_owner(b,v);
      if(settings.commentator)
	comment(COS_OWNER_CHANGE,COS_IGNORE,v,(Vehicle *) NULL);
      break;
    case SLICK:
      v->status |= VS_sliding;
      if(settings.commentator)
	comment(COS_BEEN_SLICKED,0,v,(Vehicle *) NULL);
      break;
    default:
      /* Compute angle from center of vehicle that the bullet hits */
      angle = atan2((double) dy,(double) dx);
    
      /* Determine height of damage */
      if(b->type == MINE) height = -1;
      else if(b->type == SEEKER) height = 1;
      else height = 0;

      /* Damage the vehicle, finding out how much damage was done */
      damage = damage_vehicle(v,b->owner,weapon_stat[b->type].damage,
			    angle,height);
      
      /* Make an explosion of the appropriate type for the damage done */
      explode(b,damage);
    }
}

/*
** Explodes the bullet and damages the outpost if the bullet isn't a disc.
** Gives some points and money to the owner of the bullet, if any.
*/
bul_hit_outpost(b,bbox,grid_x,grid_y)
     Bullet *b;
     Box *bbox;
     int grid_x,grid_y;
{
  int damage;

  if(b->type != DISC) {
    /* Compute damage of the bullet, award points, and damage the outpost */
    damage = weapon_stat[b->type].damage;

    /* Give points if shooter is neutral or on different team than outpost */
    if(b->owner != (Vehicle *) NULL &&
       (b->owner->team == 0 || b->owner->team != bbox->team)) {
      b->owner->owner->score += damage << 6;
      b->owner->owner->money += damage << 8;
    }

    if(damage > 0 && change_box(bbox,grid_x,grid_y)) {
      /* Decrease the outpost's strength;  If it runs out, blow it up */
      if(bbox->strength > damage)
	bbox->strength -= damage;
      else {
	bbox->type = NORMAL;
	explode_location(b->loc,1,EXP_TANK);
      }
    }
    explode(b,damage);
  }
}

#define WALL_THICKNESS 0.5

/*
** Computes the location of the collision point, and determines
** the fate of bullet and wall depending on game settings.
*/
bul_hit_wall(b,grid_x,grid_y,dir)
     Bullet *b;
     int grid_x,grid_y;		/* coordinates of box containing wall flag */
     int dir;			/* direction bullet hit wall */
{
  float dx,dy;
  int dam;

  /* Compute x and y distances from current location to point of contact */
  switch(dir) {
    case NORTH:
      dy = (BOX_HEIGHT - b->loc->box_y) + WALL_THICKNESS;
      dx = dy * b->xspeed / b->yspeed;
      break;
    case SOUTH:
      dy = - (b->loc->box_y + WALL_THICKNESS);
      dx = dy * b->xspeed / b->yspeed;
      break;
    case WEST:
      dx = (BOX_WIDTH - b->loc->box_x) + WALL_THICKNESS;
      dy = dx * b->yspeed / b->xspeed;
      break;
    case EAST:
      dx = - (b->loc->box_x + WALL_THICKNESS);
      dy = dx * b->yspeed / b->xspeed;
    }

  /* Change the bullet's location to the point of contact with the wall */
  update_loc(b->loc,b->loc,dx,dy);

  /* When a disc hits a wall, it stops orbiting its owner */
  if(b->type == DISC) {
    /* Notify the commentator if a vehicle lost the disc on a wall */
    if(settings.commentator && b->owner != (Vehicle *) NULL)
      comment(COS_OWNER_CHANGE,COS_WALL_HIT,(Vehicle *) NULL,(Vehicle *) NULL);
    set_disc_owner(b,(Vehicle *) NULL);
  }
  /*
  ** If bullet is a disc, bounce the bullet.
  ** If wall isn't damaged by bullet, and ricochet is on, bounce the bullet,
  ** otherwise explode the bullet.
  */
  if(b->type == DISC)
    bounce_bullet(b,dir);
  else {
    dam = damage_wall(grid_x,grid_y,dir,weapon_stat[b->type].damage);
    if(dam == 0 && settings.ricochet == TRUE)
      bounce_bullet(b,dir);
    else
      explode(b,dam);
  }
}

/*
** Bounces bullet b against an obstacle at direction dir.  Causes bullet
** to hurt its owner.
*/
bounce_bullet(b,dir)
     Bullet *b;
     int dir;
{
  b->hurt_owner = TRUE;
  switch(dir) {
    case NORTH: case SOUTH: b->yspeed = - b->yspeed; break;
    case WEST:  case EAST:  b->xspeed = - b->xspeed;
    }
}

/*
** Damages a wall in box at (x,y) depending on direction of damage
** Returns amount of damage done to wall.
*/
damage_wall(x,y,dir,damage)
     int x,y;
     int dir;
     int damage;
{
  Box *b;
  int dest,wall;

  b = &box[x][y];

  /* See if it is a destructible wall */
  switch(dir) {
    case NORTH: case SOUTH:
      dest = b->flags & NORTH_DEST;
      if(dest == 0) return 0;
      wall = NORTH_WALL | NORTH_DEST;
      break;
    case WEST: case EAST:
      dest = b->flags & WEST_DEST;
      if(dest == 0) return 0;
      wall = WEST_WALL | WEST_DEST;
    }

  /* Percent chance of destruction equals twice the damage */
  if(rnd(100) < damage<<1)
    if(change_box(b,x,y)) b->flags &= ~wall;

  return damage;
} 

/*
** Damages the vehicle v in the location determined by the following:
**
** Height  Result
**   -1    damage bottom of vehicle
**   0     damage side of vehicle (depending on angle of damage to vehicle)
**   1     damage top of vehicle
**
** The vehicle is killed (by the damager) if the affected armor drops
** below 0.
*/
damage_vehicle(v,damager,damage,angle,height)
     Vehicle *v,*damager;
     int damage;
     float angle;
     int height;
{
  static int hits_off[7] = { 0, 0, 1, 1, 2, 2, 3 };
  int *side;
  float rel_angle;
  int s;

  /* Vehicles don't take damage if no_wear is set */
  if(settings.no_wear) return 0;

  side = v->armor.side;
  /* Find out which side the damage affects */
  switch(height) {
    case 0:
      /* Damage hit either FRONT, BACK, LEFT, or RIGHT side of vehicle */
      rel_angle = drem(v->vector.heading + PI/4 - angle,2*PI);

      if(rel_angle < -PI/2)     s = BACK;
      else if(rel_angle < 0)    s = RIGHT;
      else if(rel_angle < PI/2) s = FRONT;
      else                      s = LEFT;
      break;
    case -1:
      /* Damage hit BOTTOM side of vehicle */
      s = BOTTOM;
      break;
    case 1:
      /* Damage hit TOP side of vehicle */
      s = TOP;
      break;
    }

  /* Subtract the hits off of the armor, and apply the damage */
  damage = max(0,damage - hits_off[v->armor.type]);
  if(damage > 0) {
    side[s] -= damage;

    /* See if the armor is pierced */
    if(side[s] < 0) {
      side[s] = 0;
      kill_vehicle(v,damager);
    }
  }
  
  /* Return the number of points of damage done to the vehicle */
  return damage;
}

/*
** Destroys the victim, giving points to the owner of the killer vehicle.
** Creates explosions and shrapnel.
*/
kill_vehicle(victim,killer)
     Vehicle *victim,*killer;
{
  extern float rnd_interval();
  Weapon *w;
  float points;
  int num,i,j;

  /* If vehicle isn't alive, don't kill him again! */
  if(!(victim->status & VS_is_alive)) return;

  /*
  ** Give a kill, some points and some money to the killer
  ** if one exists, he is not the victim, and he is either neutral,
  ** or on a different team from the victim.
  */
  if(killer != (Vehicle *) NULL && killer != victim &&
     (killer->team == 0 || killer->team != victim->team)) {
    killer->owner->kills++;
    points = 1000 * (float) victim->vdesc->cost / (float) killer->vdesc->cost;
    killer->owner->score += (int) points;
    killer->owner->money += (int) points << 3;
  }

  /* Kill victim */
  victim->status &= ~VS_is_alive;

  /* Make a tank explosion around the victim */
  explode_location(victim->loc,1,EXP_TANK);

  /* Make random shrapnel (amount based on size of body) */
  num = body_stat[victim->vdesc->body].size << 1;
  for(i = 0 ; i < num ; i++)
    make_bullet((Vehicle *) NULL,victim->loc,rnd(14),rnd_interval(-PI,PI));

  /* Make ammo shrapnel (amount based on amount of ammo left in weapons) */
  num = victim->num_weapons;
  for(i = 0 ; i < num ; i++) {
    w = &victim->weapon[i];
    for(j = 0 ; j < (w->ammo >> 5) ; j++)
      make_bullet((Vehicle *) NULL,victim->loc,w->type,rnd_interval(-PI,PI));
  }

  /* Release at fast speed any discs the victim owned */
  release_discs(victim,DISC_FAST_SPEED,TRUE);

  /* Remove victim's flag from the maze */
  box[victim->old_loc->grid_x][victim->old_loc->grid_y].flags &= ~victim->flag;

  /* Send out a message about the victim's death */
  send_death_message(victim,killer);
}


#define EXP_SPREAD 15

/*
** Makes the given number of explosions of the given type around the location.
*/
explode_location(loc,num,type)
     Loc *loc;
     int num,type;
{
  Loc exp_loc;
  int exp_dx,exp_dy;
  int i;

  for(i = 0 ; i < num ; i++) {
    exp_loc = *loc;
    exp_dx = rnd(EXP_SPREAD << 1) - EXP_SPREAD;
    exp_dy = rnd(EXP_SPREAD << 1) - EXP_SPREAD;
    update_loc(&exp_loc,&exp_loc,exp_dx,exp_dy);
    make_explosion(&exp_loc,(unsigned int) type);
  }
}

/*
** Computes new velocities for the vehicles after a collision with
** v2 having a relative position of (dx,dy) with respect to v1.
*/
bounce_vehicles(v1,v2,dx,dy,elast)
     Vehicle *v1,*v2;
     int dx,dy;
     float elast;
{
  Vehicle *temp;
  float v1x,v1y,v2x,v2y;
  float slope,mratio,c;

  /* Make v1 be on the left */
  if(dx < 0) {
    dx = -dx;
    dy = -dy;
    temp = v1;
    v1 = v2;
    v2 = temp;
  }

  v1x = v1->vector.xspeed;
  v1y = v1->vector.yspeed;
  v2x = v2->vector.xspeed;
  v2y = v2->vector.yspeed;

  if(dx == 0)
    slope = (dy > 0) ? 999.0 : -999.0;
  else
    slope = (float) dy / (float) dx;

  if(v2->vdesc->weight == 0)
    mratio = 999.0;
  else
    mratio = v1->vdesc->weight / v2->vdesc->weight;

  c = ((v1x - v2x) + slope * (v1y - v2y)) /
      ((1.0 + mratio) * (slope*slope + 1.0));
  c *= 1.0 + elast;

  assign_speed(&v1->vector, v1x - c, v1y - c * slope);
  assign_speed(&v2->vector, v2x + c * mratio, v2y + c * mratio * slope);
}

/*
** Computes new velocity for the vehicle after a collision with a wall
** at a relative position of (dx,dy).
*/
bounce_vehicle_wall(v,dx,dy,elast)
     Vehicle *v;
     int dx,dy;
     float elast;
{
  float vx,vy;
  float slope,c;

  vx = v->vector.xspeed;
  vy = v->vector.yspeed;

  if(dx < 0) {
    dx = -dx;
    dy = -dy;
  }

  if(dx == 0)
    slope = (dy > 0) ? 999.0 : -999.0;
  else
    slope = (float) dy / (float) dx;

  c = (vx + slope * vy) / (slope*slope + 1.0);
  c *= 1.0 + elast;

  assign_speed(&v->vector, vx - c, vy - c * slope);
}

/*
** Computes the speed and angle from the xspeed and yspeed and sets the
** the xspeed, yspeed, speed, and angle of the vector.
*/
assign_speed(vec,xsp,ysp)
     Vector *vec;
     float xsp,ysp;
{
  vec->xspeed = xsp;
  vec->yspeed = ysp;
  vec->speed = sqrt(xsp*xsp + ysp*ysp);
  vec->angle = atan2(ysp,xsp);
}
