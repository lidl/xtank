/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** actions.c
*/

/*
$Author: lidl $
$Id: actions.c,v 1.1.1.1 1995/02/01 00:25:33 lidl Exp $
*/

#include "malloc.h"
#include "xtank.h"
#include "disc.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "vstructs.h"
#include "bullet.h"
#include "terminal.h"
#include "cosell.h"
#include "globals.h"
#include "assert.h"
#include "proto.h"


extern Terminal *term;
extern Weapon_stat weapon_stat[];
extern Settings settings;
extern Boolean game_paused;

extern int frame;	/* HAK 2/93 */
extern Map real_map;


/* Adjust velocity, keeping direction the same */

void adjust_speed(FLOAT *speedx, FLOAT *speedy, double adjust)
{
	double angle;

	angle = ATAN2(*speedy, *speedx);
	*speedx += (FLOAT)(adjust * COS(angle));
	*speedy += (FLOAT)(adjust * SIN(angle));
}

/*
** Adjusts the specified location by dx in the x direction, and
** dy in the y direction.
*/
adjust_loc(loc, dx, dy)
Loc *loc;
int dx, dy;
{
	/* adjust x coordinate */
	loc->x += dx;
	if ((loc->box_x += dx) < 0)
		do {
			loc->grid_x--;
		} while ((loc->box_x += BOX_WIDTH) < 0);
	else if (loc->box_x >= BOX_WIDTH)
		do {
			loc->grid_x++;
		} while ((loc->box_x -= BOX_WIDTH) >= BOX_WIDTH);

	/* adjust y coordinate */
	loc->y += dy;
	if ((loc->box_y += dy) < 0)
		do {
			loc->grid_y--;
		} while ((loc->box_y += BOX_HEIGHT) < 0);
	else if (loc->box_y >= BOX_HEIGHT)
		do {
			loc->grid_y++;
		} while ((loc->box_y -= BOX_HEIGHT) >= BOX_HEIGHT);
}

/* get_tele(): Although I wrore it in a convoluted way, All this does is look
** through the bullet list for the youngest teleguided missile fired by v. The
** pointer to this bullet is passed back in *b, and TRUE is returned if the 
** teleguided missile does indeed exist.
*/ 

Boolean get_tele(v,b)
Vehicle *v;
Bullet **b;
{
   extern Bset *bset;
   Bullet *temp;
   int maxlife=-1;
   Boolean GotIt;
   int i;

   *b=NULL;
   GotIt = FALSE;

   for (i=0; i<bset->number; i++) {
      temp = bset->list[i];

      if ((temp->disp_flgs & F_TELE) && temp->owner->number == v->owner->number) {

	 if (temp->life > maxlife) {
		maxlife = temp->life;
		*b = temp;
		GotIt = TRUE;
         }
      }
   }
   return(GotIt);
}
	 

/*
** turn_tow: This function looks through the BUllet list for the youngest TOW
** fired by the vehicle pointed to by v. It then changes the bullet's direction
** by 20*direction degrees. TRUE is returned if a TOW is found, FALSE otherwise.
*/

turn_tow(v,direction)
Vehicle *v;
float direction;
{
   Boolean GotOne;
   int i;
   extern Bset *bset;
   Bullet *b;
   float speed;
   Angle angle;

  /* 
   * The following for loop is a little messy, but all it does is look 
   * through the list of bullets until it finds the youngest live 
   * TOW owned by v. As it goes, it sets the status of the (non-RED) missiles.
   */

   b = NULL;
   GotOne = FALSE;

   for ( i = 0 ; i < bset->number ; i++ ) {
      if ((bset->list[i]->move_flgs & F_KEYB) &&
				  (bset->list[i]->owner->number == v->number)) {

         if (bset->list[i]->state != RED) bset->list[i]->state=YELLOW;

	 if ( (!GotOne) || ( b->life < bset->list[i]->life) ) {
	     GotOne = TRUE;
	     b = bset->list[i];
         }
      }
   }

   if (GotOne)
     {
	 if (b->state != RED) b->state = GREEN;

	 speed = sqrt(b->xspeed * b->xspeed + b->yspeed * b->yspeed);
         angle = ((direction * PI / 8.0) + ATAN2(b->yspeed,b->xspeed));
 
	 /* Make the bullet "snap" to one of the 16 directions (so the little
	  * ltorp car (and other directional bitmaps) look right) (HAK 3/93)
	  */
	 if((int)direction) {
		int num;
		if (angle < 0) angle += 2*PI;
		num = (int)((angle + PI / 16.0) * (8.0 / PI));
		angle = (Angle)num * PI / 8.0;
	 }

 	 b->xspeed = speed * cos(angle);
 	 b->yspeed = speed * sin(angle);
      }
   return(GotOne);
}   

/*
** Sometimes you may want to detonate the last tow fired. This sets the state
** to RED, which translates to "it's gonna BLOW!" in my code. Life is also cut
** short for the poor missile...
*/
det_tow(v)
Vehicle *v;
{
   Boolean GotOne;
   int i;
   extern Bset *bset;
   Bullet *b;

   b = NULL;
   GotOne = FALSE;

   for ( i = 0 ; i < bset->number ; i++ ) {
      if ((bset->list[i]->move_flgs & F_KEYB)&& 
			  (bset->list[i]->owner->number == v->number)) {
	 if ((!GotOne) || ( b->life < bset->list[i]->life)) {
	    GotOne = TRUE;
	    b = bset->list[i];
         }
      }
   }

   if (GotOne) 
	 b-> state = RED;

   return(GotOne);
}

/*
** Creates a bullet from the specified weapon type at the given location,
** owned by the vehicle v, moving at the specified angle.
*/
/*
 * New function for generating smart (has an internal coordinate)
 * bullets.
 * renamed from make_smart_bullet to make_bullet (HAK)
 * It now returns the bullet, so that the caller may
 * modify it... (HAK 4/93)
 */
Bullet *make_bullet(v, loc, type, angle, target)
Vehicle *v;
Loc *loc;
WeaponType type;
Angle angle;
lCoord *target;
{
	extern set_disc_team();
	Weapon_stat *ws;
	extern Bset *bset;
	Bullet *b;
	static int discteam = 0;

	if (bset->number >= MAX_BULLETS)
		return 0;				/* too many bullets */

	/* Initialize the bullet's owner and location */
	b = bset->list[bset->number++];
	b->owner = v;
	b->thrower = -1;
	b->loc1 = b->loc2 = b->old_old_loc = *loc;
	b->loc = &b->loc1;
	b->old_loc = &b->loc2;

	/* Find the weapon_stat structure */
	ws = &weapon_stat[(int) type];

	/* Set initial height */
	b->old_old_loc.z = b->old_loc->z = b->loc->z = ws->height;

	/* Compute the x and y components of the bullet's speed */
	b->xspeed = (FLOAT) ws->ammo_speed * cos(angle);
	b->yspeed = (FLOAT) ws->ammo_speed * sin(angle);

	/* Add the motion of the owner (if necessary) */
	if (settings.si.rel_shoot == TRUE && v != NULL
	    && !(b->creat_flgs & F_NREL) ) {
		b->xspeed += v->vector.xspeed;
		b->yspeed += v->vector.yspeed;
	}
	/* fill in stuff from the weapon-stat */
	b->safety = ws->safety;
	b->num_views = ws->num_views;
	b->other_flgs = ws->other_flgs;
	b->creat_flgs = ws->creat_flgs;
	b->disp_flgs = ws->disp_flgs;
	b->move_flgs = ws->move_flgs;
	b->hit_flgs = ws->hit_flgs;
	b->creat_func = ws->creat_func;
	b->disp_func = ws->disp_func;
	b->upd_func = ws->upd_func;
	b->hit_func = ws->hit_func;
	b->type = type;
	b->life = ws->frames;
	b->hurt_owner = FALSE;
	b->state = GREEN;

	if(b->disp_flgs & F_ROT)
		b->rot = b->old_rot = 0;
	else
		b->rot = b->old_rot = find_rot(angle, b->num_views);

	if(type==DISC)
		set_disc_team(b, discteam++);
	else {
		if(target) {
			b->target.y = target->y + rnd(11) - 5;
			b->target.x = target->x + rnd(11) - 5;
		}
		discteam = 0;
	}
	return(b);
}

/*
** Destroys bullet b, and creates an explosion in its place.
** The type of the explosion is dependent on the damage.
*/
explode(b, damage)
Bullet *b;
int damage;
{
	unsigned int type;

	if (b->hit_flgs & AREA)
		expl_area(b);
	else {
		/* Determine type of explosion from the amount of damage the bullet did */
		type = (damage <= 4) ? (EXP_DAM0 + damage) : EXP_DAM4;
		make_explosion(b->loc, type, 0);
	}

	/* I think this makes bullets that explode before they are displayed
       not get erased, but I'm not sure... (sigh) */
	if (b->type != DISC) {
		if (b->life == weapon_stat[(int) b->type].frames - 1)
			b->life = -2;		/* undisplayed bullet */
		else
			b->life = -1;
	}
}


/* Lookup table for inverse-square-ish damage (actually: 1 / 2^x) (HAK 3/93) */
#define AREA_TBL_LEN 25

static FLOAT area_tbl[AREA_TBL_LEN] = {
1.000000, 0.793701, 0.629961, 0.500000, 0.396850, 0.314980, 0.250000, 0.198425,
0.157490, 0.125000, 0.099213, 0.078745, 0.062500, 0.049606, 0.039373, 0.031250,
0.024803, 0.019686, 0.015625, 0.012402, 0.009843, 0.007813, 0.006201, 0.004922,
0.003906
};

/* renamed expl_nuke to expl_area (HAK 3/93) */

expl_area(b)	/* Lots-o-changes 2/93 (HAK and MEL) */
Bullet *b;
{
	double dist, angle, hitsoff, nuke_sqr;
	int x, y, dx, dy, i, j, damage, face, opp, adj1, adj2, index, weap_dam;
	Box *bx;
	Vehicle *v;
	Loc oloc;

	weap_dam = weapon_stat[b->type].damage;

	/* range is dependant on damage */
	nuke_sqr = (double)(weap_dam * weap_dam);

	index = 8 - (int)(rint(weap_dam * (8.0 / 200.0)));
	index &= (~((unsigned int)1));
	make_explosion(b->loc, EXP_NUKE, index);

/* Deal with vehicles... */
	for (i = 0; i < num_veh_alive; i++) {
		v = live_vehicles[i];
		dx = v->loc->x - b->loc->x;
		dy = v->loc->y - b->loc->y;
		dist = (double) (dx * dx + dy * dy);

		if ((int)dist <= nuke_sqr) {

			/* ignore him if he's in a peace square */
			bx = &real_map[b->loc->grid_x][b->loc->grid_y];
			if (bx->type == PEACE) {
				/* if the vehicle is close enough */
				if (!(b->loc->box_x < BOX_WIDTH / 2 - LANDMARK_WIDTH / 2 ||
				      b->loc->box_x > BOX_WIDTH / 2 + LANDMARK_WIDTH / 2 ||
				      b->loc->box_y < BOX_HEIGHT / 2 - LANDMARK_HEIGHT / 2 ||
				      b->loc->box_y > BOX_HEIGHT / 2 + LANDMARK_HEIGHT / 2)) {
					/* if the peace square is for YOUR team */
					if (bx->team == v->team || bx->team == NEUTRAL) {
					   continue;
					}
				}
			}

			/* get an index to the table */
			index = dist * (AREA_TBL_LEN / nuke_sqr);
			if (index >= AREA_TBL_LEN)
				index = AREA_TBL_LEN - 1;

/* Damage varies on the different sides based on which side is facing the
   center of the explosion (HAK) */

                        angle = ATAN2((double) b->loc->y - v->loc->y,
                                      (double) b->loc->x - v->loc->x);
                        face = (int)find_affected_side(v, angle);
                        switch (face) {
                        case FRONT:
                                adj1 = RIGHT;
                                adj2 = LEFT;
                                opp = BACK;
                                break;
                        case BACK:
                                adj1 = RIGHT;
                                adj2 = LEFT;
                                opp = FRONT;
                                break;
                        case RIGHT:
                                adj1 = FRONT;
                                adj2 = BACK;
                                opp = LEFT;
                                break;
                        case LEFT:
                                adj1 = FRONT;
                                adj2 = BACK;
                                opp = RIGHT;
                                break;
                        }
                        hitsoff = (double)armor_stat[v->vdesc->armor.type].defense;

                        /* Make damage dependant on armor type as well... (HAK and MEL 2/93) */
			hitsoff /= 2;
			if(++hitsoff <= 0) hitsoff=1;

			damage = (int)(weap_dam * area_tbl[index] / hitsoff);

                        v->armor.side[face] -= damage;
                        v->armor.side[opp] -= damage/8;
                        v->armor.side[adj1] -= damage/4;
                        v->armor.side[adj2] -= damage/4;
                        v->armor.side[TOP] -= damage/4;
                        v->armor.side[BOTTOM] -= damage/4;
                        for ( j = FRONT; j <= BOTTOM; j++) {
                                if (v->armor.side[j] < 0) {
                                        v->armor.side[j] = 0;
                                        kill_vehicle(v, b->owner);
                                }
                        }
		}
	}
/* Deal with outposts... */ /* Check a 3x3 box around the det for outposts */
        x=b->loc->grid_x;
        y=b->loc->grid_y;
        if(x<=0)
           x=1;
        if(y<=0)
           y=1;
        if(x>=GRID_WIDTH-1)
           x=GRID_WIDTH-2;
        if(y>=GRID_HEIGHT-1)
           y=GRID_HEIGHT-2;
        for(i=x-1;i<=x+1;i++)
           for(j=y-1;j<=y+1;j++) {
                bx = &real_map[i][j];
                if (bx->type == OUTPOST) {
                        outpost_loc(bx, &oloc, i, j);
                        dx = oloc.x - b->loc->x;
                        dy = oloc.y - b->loc->y;
                        dist = (double)(dx*dx + dy*dy);
			if ((int)dist <= nuke_sqr) {
				index = dist*(AREA_TBL_LEN / nuke_sqr);
				if (index >= AREA_TBL_LEN)
					index = AREA_TBL_LEN - 1;
				damage = (int)(weap_dam * area_tbl[index]);
			} else
				damage=0;

                        b->owner->owner->score += MIN(damage, (int)bx->strength) << 6;
                        b->owner->owner->money += MIN(damage, (int)bx->strength) << 8;
                        change_box(bx, i, j);
                        if ((int)bx->strength > damage)
                                bx->strength -= damage;
                        else {
                                bx->type = NORMAL;
                                explode_location(&oloc, 1, EXP_TANK);
                        }
                }
            }
/* Deal with walls.... */

	{
		int walld = (int)(weap_dam/4);
		damage_wall(x, y, NORTH, 50); /* That should almost always kill the wall */
		damage_wall(x, y, WEST, 50);
		if (!(x+2>GRID_WIDTH))
		   damage_wall(x+1, y, WEST, 50);
		if (!(y+2>GRID_HEIGHT))
		   damage_wall(x, y+1, NORTH, 50);
	}

        invalidate_maps();
}

/*
** Does the specified action for the specified special in the
** specified vehicle.
*/
do_special(v, special_num, action)
Vehicle *v;
SpecialType special_num;
unsigned int action;
{
	Special *s;
        int i;

	/* deleted all of the REPAIR crap, let the special handle it -ane */

	/* First make sure the special is really there */
	s = &v->special[(int) special_num];
	if (s->status == SP_nonexistent)
		return;

	switch (action) {

 /*
  * In order for a special to refuse
  * to activate, the specials can now return a value
  * in the case of an SP_activate & return either a
  * SP_on or a SP_off.             --ane
  */

/*
 * Needs a new paradigm.  SP_break is getting called asynchronus to the 
 * main animate loop and generally fucking things up.
 *
 * OK, first lets classify when/where each type of SP_x can be called.  This
 * is roughly in order that they can occur in the main animation loop.
 *
 * SP_toggle/on/off.  Early in the animate loop, when getting user/program 
 * input.  Toggle is used by humans, on/off by robots, although there is no
 * reason the converse is excluded.
 * Set_terminal must have been called to set vid. (it is)
 *
 * SP_update.  Almost the last thing (just before the update_locs/display
 * and SP_redisplay below) in the main animation loop. Also check for
 * flag to do a break/fix on the special. 
 * Set_terminal *not* called, must loop thru vid's manually.
 *
 * SP_redisplay/draw/erase. The last thing (mostly) in the main loop;
 * display_terminal calls display_anim which updates the special's displays.
 * Set_terminal must have been called to set vid. (it is)
 *
 * SP_activate, SP_deactivate.  SP_activate is called in init_specials to
 * start the special when the tank started.  SP_deactivate is called by 
 * zap_specials (the opposite of init_specials). Toggle/on/off also call 
 * activate/deactivate.
 *
 */

 
	  case SP_update:
                  if (s->damage_flag == SPDF_break) {
		      if (s->status == SP_on) {
			  s->status = SP_off;
			  (*s->proc) (v, s->record, SP_deactivate);
                          for (i=0; i<num_terminals; i++)
                              if (terminal[i]->vehicle == v) {
                                  vid = (Video *) terminal[i]->video;
			          (*s->proc) (v, s->record, SP_erase);
                              }
		      }
		      s->status = SP_broken;
                  } else if (s->damage_flag == SPDF_fix) {
		      s->status = (*s->proc) (v, s->record, SP_activate);
	    	      if (s->status == SP_on)
                              for (i=0; i<num_terminals; i++)
                                  if (terminal[i]->vehicle == v) {
                                      vid = (Video *) terminal[i]->video;
			              (*s->proc) (v, s->record, SP_draw);
                                  }
		  }
                  s->damage_flag = SPDF_clear;
                  /* no break */
	  case SP_redisplay:
	  case SP_draw:
	  case SP_erase:
		  if (s->status == SP_on)
			  (*s->proc) (v, s->record, action);
		  break;
	  case SP_activate:
		  if (s->status == SP_off) {
			  s->status = (*s->proc) (v, s->record, SP_activate);
		  }
		  break;
	  case SP_deactivate:
		  if (s->status == SP_on) {
			  s->status = SP_off;
			  (*s->proc) (v, s->record, SP_deactivate);
		  }
		  break;
	  case SP_toggle:
		  if (s->status == SP_off) {
			  s->status = (*s->proc) (v, s->record, SP_activate);
			  if (s->status == SP_on)
				  (*s->proc) (v, s->record, SP_draw);
		  } else if (s->status == SP_on) {
			  s->status = SP_off;
			  (*s->proc) (v, s->record, SP_deactivate);
			  (*s->proc) (v, s->record, SP_erase);
		  }
		  break;
	  case SP_on:
		  if (s->status == SP_off) {
		      s->status = (*s->proc) (v, s->record, SP_activate);
	    	      if (s->status == SP_on)
			  (*s->proc) (v, s->record, SP_draw);
		  }
		  break;
	  case SP_off:
		  if (s->status == SP_on) {
			  s->status = SP_off;
			  (*s->proc) (v, s->record, SP_deactivate);
			  (*s->proc) (v, s->record, SP_erase);
		  }
		  break;
	  default:
		  printf("do_special:  Unimplemented function.\n");
		  break;

	}
}

/*
** Moves the view on the current terminal by dx vertically and dy horizontally.
** The terminal stops tracking.
*/
move_view(dx, dy)
int dx, dy;
{
	Intloc new;

	/* switch view to a non-existent vehicle */
	switch_view(-1);

	/* compute the new viewing location */
	new.x = term->loc.x + dx;
	new.y = term->loc.y + dy;

	new.grid_x = new.x / BOX_WIDTH;
	new.grid_y = new.y / BOX_HEIGHT;

	/* If the new location is in the grid, move the view there */
	if (new.grid_x >= 0 && new.grid_x < GRID_WIDTH - 4 &&
		new.grid_y >= 0 && new.grid_y < GRID_HEIGHT - 4)
		term->loc = new;
}

#define INCR_WRAP(a,b) {if ((a) == (b)) (a) = 0; else (a)++;}
#define DECR_WRAP(a,b) {if ((a) == 0) (a) = (b); else (a)--;}
int next_live_tank()
{
	int iCurrentTank = -1;

	if (term->vehicle) {
		iCurrentTank = term->vehicle->number;

		INCR_WRAP(iCurrentTank, num_veh - 1);

		while (!IsVehicleAlive(iCurrentTank)) {
			INCR_WRAP(iCurrentTank, num_veh - 1);
		}
	}
	return (iCurrentTank);
}

int previous_live_tank()
{
	int iCurrentTank = -1;

	if (term->vehicle) {
		iCurrentTank = term->vehicle->number;

		DECR_WRAP(iCurrentTank, num_veh - 1);

		while (!IsVehicleAlive(iCurrentTank)) {
			DECR_WRAP(iCurrentTank, num_veh - 1);
		}
	}
	return (iCurrentTank);
}

int IsVehicleAlive(num)
int num;
{
	int i;

	for (i = 0; i < num_veh_alive; i++) {
		if (live_vehicles[i]->number == num) {
			break;
		}
	}

	return ((i != num_veh_alive));
}


/*
** Switches the view on the current terminal to the perspective of
** the vehicle numbered num.  If num is not a legal vehicle number,
** the terminal stops tracking.
*/

switch_view(num)
int num;
{
	extern int num_terminals;
	extern Terminal *terminal[];
	Vehicle *old_vehicle;
	int i;

	if (num >= 0) {
		/* remember who we were tracking */
		old_vehicle = term->vehicle;

		/* find the owner of the vehicle with that number */
		for (i = 0; i < num_veh_alive; i++) {
			if (live_vehicles[i]->number == num) {
				term->vehicle = live_vehicles[i];
				break;
			}
		}

		/* If we didn't find the vehicle number, or found the one that we
	   used to be tracking, don't expose the windows */
		if (old_vehicle == term->vehicle)
			return;

		if (old_vehicle)
			old_vehicle->owner->num_players--;
		for (i = 0; i < num_terminals && terminal[i] != term; i++) ;
		assert(i != num_terminals);
		term->vehicle->owner->player[term->vehicle->owner->num_players++] = i;
	} else {
		if (term->vehicle != NULL)
			term->vehicle->owner->num_players--;
		term->vehicle = NULL;	/* stop tracking */
	}

	/* Expose all the windows which need to be redisplayed after the switch */
	expose_win(ANIM_WIN, TRUE);
	expose_win(CONS_WIN, TRUE);
	expose_win(MAP_WIN, TRUE);
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
	draw_text(ANIM_WIN, PAUSE_X, PAUSE_Y,
			  "Game paused -- RETURN to unpause  SPACE to single step",
			  M_FONT, DRAW_XOR, WHITE);
}

/*
** If state is TRUE, sets game_speed to 0 to pause.
** If state is FALSE, restores game_speed to old value.
*/
pause_game(state)
Boolean state;
{
	if (state == TRUE) {
		/* If not already paused, set game_speed to 0 */
		if (!game_paused) {
			display_pause_message();
			game_paused = TRUE;
		}
	} else {
		/* If paused, restore old game_speed */
		if (game_paused) {
			display_pause_message();
			game_paused = FALSE;
		}
	}
}

/*
** Sets game speed to specified value if it is reasonable.
*/
set_game_speed(spd)
int spd;
{
	if (spd > 0 && spd <= MAX_GAME_SPEED)
		settings.game_speed = spd;
	start_real_counter(1 + 999999 / settings.game_speed);
}

/*
** Deals with game pause.  Waits if game is running faster than it should.
*/
check_game_speed()
{
	char reply;

	if (settings.game_speed == MAX_GAME_SPEED && !game_paused)
		return;
	if (game_paused) {
		/* Game is paused, so wait for a space bar or return */
		do {
			reply = get_reply();
		} while (reply != ' ' && reply != '\r' && reply != -1);

		/* Unpause game on a return, single step on a space bar */
		if (reply == '\r')
			pause_game(FALSE);
	}
	wait_for_real_counter();
}
