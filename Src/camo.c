/*
** Copyright (c) 1992, 1993, 1994.
** Aaron Nabil-Eastlund
**
** $Id$
*/

#include "malloc.h"
#include "xtank.h"
#include "vehicle.h"
#include "graphics.h"
#include "globals.h"
#include "gr.h"
#include "proto.h"

#ifndef NO_CAMO

/* This is how long, after all stealth conditions are met,
 * it takes for a vehicles to regain it's "stealty"
 * configuration.
 */


extern int frame;

SpecialStatus special_stealth(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	Stealth *s;

	s = (Stealth *) record;

	switch (action) {

	  case SP_update:{

			  int i;
			  Boolean damaged = FALSE;
			  Boolean rad = FALSE;
			  Boolean weap = FALSE;

			  if (v->special[(SpecialType) RADAR].status == SP_on
				  || v->special[(SpecialType) NEW_RADAR].status == SP_on) {
				  rad = TRUE;
			  } else {

				  /*
		 * radar off, so check to see if he has any weapons on.
		 */

				  for (i = 0; i < v->num_weapons && !(v->weapon[i].status & WS_on); i++) ;

				  if (i != v->num_weapons) {
					  weap = TRUE;
				  } else {

					  /*
		     * radar & weapons off, check the vehicle's skin
		     */

					  for (i = FRONT; i < MAX_SIDES; i++)
			/* A little more lenient:  need 9/10 armor, instead
			 * of all.  (MEL and HAK 2/93) */
		  			  if (v->armor.side[i] < (v->vdesc->armor.side[i]*9)/10)
							  damaged = TRUE;
				  }
			  }

			  if (weap || rad || damaged) {
				  v->rcs = v->normal_rcs;
				  s->activate_frame = frame + STEALTH_DELAY;
			  } else if (frame > s->activate_frame)
				  v->rcs = v->stealthy_rcs;

		  }
		  break;

	  case SP_activate:
		  s->activate_frame = frame + STEALTH_DELAY;
		  return SP_on;
		  break;

	  case SP_deactivate:
		  v->rcs = v->normal_rcs;
		  break;

	  default:
		  break;

	}
	return;
}

SpecialStatus special_camo(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	Camo *c;

	c = (Camo *) record;

	switch (action) {

	  case SP_update:

		  if ((v->loc->x != v->old_loc->x) ||
			  (v->vector.old_rot != v->vector.rot) ||
			  (v->frame_weapon_fired == frame) ||
		  /* add in other conditions of camo here */
			  (v->loc->y != v->old_loc->y)) {
			  c->camo_countdown = v->time_to_camo;
			  v->camod = FALSE;

		  } else {
			  if (c->camo_countdown)
				  --c->camo_countdown;
			  else
				  v->camod = TRUE;
		  }
		  break;

	  case SP_activate:
		  c->camo_countdown = v->time_to_camo;
		  v->camod = FALSE;

		  return SP_on;
		  break;

	  case SP_deactivate:
		  v->camod = FALSE;
		  break;

	  default:
		  break;
	}

	return;
}


/*
 * PERSIST is how long a RDF trace is on the display max.
 *
 */


SpecialStatus special_rdf(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	Rdf *r;
	int i;
	long l;
	Trace *t;

	r = (Rdf *) record;

	switch (action) {

	  case SP_draw:
	  case SP_erase:
		  for (i = 0; i < MAX_VEHICLES; i++) {
			  t = &r->trace[v->number][i];
			  if (t->is_drawn) {
				  draw_line(MAP_WIN, grid2map(t->drawn.start_x) + MAP_OFF,
							grid2map(t->drawn.start_y) + MAP_OFF,
							grid2map(t->drawn.end_x) + MAP_OFF,
							grid2map(t->drawn.end_y) + MAP_OFF,
							DRAW_XOR, t->drawn.color);

				  if (action == SP_erase)
					  t->is_drawn = FALSE;
			  }
		  }
		  break;

	  case SP_update:
		  if (v->loc->x != v->old_loc->x
#ifndef RDF_DEBUG
			  || v->special[(SpecialType) NEW_RADAR].status == SP_on
			  || v->special[(SpecialType) RADAR].status == SP_on
#endif /* RDF_DEBUG */
			  || v->loc->y != v->old_loc->y) {
			  if (!r->zapped) {
				  for (i = 0; i < MAX_VEHICLES; i++) {
					  t = &r->trace[v->number][i];
					  t->to_draw = FALSE;
				  }
				  r->zapped = TRUE;
			  }
			  return;
		  }
		  if (r->zapped) {
			  for (i = 0; i < MAX_VEHICLES; i++)
				  v->illum[i].color = -1;
			  r->zapped = FALSE;
		  }
		  for (i = 0; i < MAX_VEHICLES; i++) {

			  if (!tstflag(actual_vehicles[i].status, VS_is_alive)
			  || ((v->team == actual_vehicles[i].team) && v->team != NEUTRAL)
				  || actual_vehicles[i].have_IFF_key[v->number])
				  continue;

			  t = &r->trace[v->number][i];

			  if (v->illum[i].color != -1) {
				  long dx, dy;

				  t->draw.start_x = v->loc->grid_x;
				  t->draw.start_y = v->loc->grid_y;


#ifdef RDF_DEBUG
				  t->draw.end_x = v->illum[i].gx;
				  t->draw.end_y = v->illum[i].gy;
#else /* RDF_DEBUG */

				  /*
		     * could add a little randomness, too
		     * if the robots get too sneaky
		     * + rnd_interval(-.25, .25)
		     */

				  dx = v->illum[i].gx - v->loc->grid_x;
				  dy = v->illum[i].gy - v->loc->grid_y;

				  /* idist doesn't work here, don't even think it */
				  l = (long) SQRT((double) ((dx * dx) + (dy * dy)));

				  t->draw.end_x = v->loc->grid_x + (dx * (32 / l));
				  t->draw.end_y = v->loc->grid_y + (dy * (32 / l));
#endif /* RDF_DEBUG */
				  t->draw.color = (v->illum[i].color == GREEN) ? RDF_SAFE : v->illum[i].color;
				  t->to_draw = PERSIST;
			  } else if (t->to_draw)
				  --t->to_draw;
			  v->illum[i].color = -1;
		  }

		  break;

	  case SP_deactivate:
	  case SP_activate:
		  for (i = 0; i < MAX_VEHICLES; i++) {
			  v->illum[i].color = -1;
			  t = &r->trace[v->number][i];
			  t->to_draw = t->is_drawn = FALSE;
			  r->zapped = TRUE;
		  }
		  if (action == SP_activate)
			  return SP_on;
		  break;


	  case SP_redisplay:

		  for (i = 0; i < MAX_VEHICLES; i++) {

			  t = &r->trace[v->number][i];

			  /*
		 * Erase this trace if it is drawn and there is a new one
		 * waiting to take it's place or it's time has expired
		 */

			  if (t->is_drawn && (t->to_draw == PERSIST || !t->to_draw)) {
				  draw_line(MAP_WIN, grid2map(t->drawn.start_x) + MAP_OFF,
							grid2map(t->drawn.start_y) + MAP_OFF,
							grid2map(t->drawn.end_x) + MAP_OFF,
							grid2map(t->drawn.end_y) + MAP_OFF,
							DRAW_XOR, t->drawn.color);

				  t->is_drawn = FALSE;
			  }
			  /*
		 * Draw this trace if it has just been added to the
		 * trace list
		 */

			  if (t->to_draw == PERSIST) {
				  draw_line(MAP_WIN, grid2map(t->draw.start_x) + MAP_OFF,
							grid2map(t->draw.start_y) + MAP_OFF,
							grid2map(t->draw.end_x) + MAP_OFF,
							grid2map(t->draw.end_y) + MAP_OFF,
							DRAW_XOR, t->draw.color);

				  t->is_drawn = TRUE;
				  t->drawn = t->draw;
			  }
		  }


		  break;

	  default:
		  break;
	}
}

#endif
